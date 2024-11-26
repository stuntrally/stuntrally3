#include "pch.h"
#include "AppGui.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
// #include "CData.h"
#include "GuiCom.h"
#include "Terra.h"

#include <OgreRoot.h>
#include <OgreItem.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreHlmsPbs.h>

#include <Vct/OgreVctLighting.h>
#include <Vct/OgreVctVoxelizer.h>
#include <IrradianceField/OgreIrradianceField.h>
#include <Utils/MeshUtils.h>
#include <Utils/TestUtils.h>
using namespace Ogre;


//  🌄🆕 GI Init
//-----------------------------------------------------------------------------------
void AppGui::InitGI()
{
	Ogre::Timer ti;
	LogO("*()* GI init");

	iVctDebugVis = VctVoxelizer::DebugVisualizationNone;
	iIfdDebugVis = IrradianceField::DebugVisualizationNone;

	mVoxelizer = new VctVoxelizer( Id::generateNewId<VctVoxelizer>(),
		mRoot->getRenderSystem(), mRoot->getHlmsManager(), true );

	mIFD = new IrradianceField( mRoot, mSceneMgr );

	HlmsPbs* hlmsPbs = getHlmsPbs();

	hlmsPbs->setIrradianceField( mIFD );

	LogO("*()* GI init  mode: "+GIstrMode());

	GIVoxelizeScene();  // !

	GItoggletVctQuality();
	// GInextIrradianceField(1);
	// GInextVisMode( 1 );
	// GInextIfdProbeVisMode( 1 );
	GIupdText();

	LogO(String("*()* GI Init end: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}

//  🌄💥 GI destroy
//-----------------------------------------------------------------------------------
void AppGui::DestroyGI()
{
	delete mIFD;  mIFD = 0;
	delete mVCT;  mVCT = 0;
	// mVoxelizer->removeAllItems();

	delete mVoxelizer;  mVoxelizer = 0;
	{
		HlmsManager *hlms = mRoot->getHlmsManager();
		assert( dynamic_cast<HlmsPbs*>( hlms->getHlms( HLMS_PBS ) ) );
		HlmsPbs *hlmsPbs = static_cast<HlmsPbs*>( hlms->getHlms( HLMS_PBS ) );

		hlmsPbs->setVctLighting( 0 );
		hlmsPbs->setIrradianceField( 0 );
	}
	// delete mTestUtils;  mTestUtils = 0;
}

//  🌄💫 GI update
//-----------------------------------------------------------------------------------
void AppGui::UpdateGI()
{
	/*if (mGraphicsSystem->getRenderWindow()->isVisible())
	{
	    static int frame = 0;
	    if (frame > 1)
	    {
	        mVoxelizer->build( mSceneMgr );
	        mVctLighting->update( mSceneMgr, mNumBounces );
	    }
	    ++frame;
	}*/  // ? ^

	if (mIFD)
	if (GIgetMode() == IfdVct || GIgetMode() == Ifd)
		mIFD->update( bIfdRaster ? 4u : 200u );
}


//  🌄🔁 Voxelize scene
//-----------------------------------------------------------------------------------
void AppGui::GIVoxelizeScene()
{
	Ogre::Timer ti;
	LogO("*[]* GI Voxelize begin");

	mItems.clear();

	LogO("*[]* GI items");

	// mItems.push_back(scn->itSky[0]);  // no-
	
	//  add objects
	for (const Object& o : scn->sc->objects)
		if (!o.dyn && o.it)  // static
			mItems.push_back(o.it);
	
	//  add roads
	const int lod = 0;
	for (const auto& road : scn->roads)
		for (RoadSeg& rs : road->vSegs)
		{	Item* it;
			it = rs.road[lod].it;  if (it)  mItems.push_back(it);
			it = rs.wall[lod].it;  if (it)  mItems.push_back(it);
			// it = rs.blend[lod].it;  if (it)  mItems.push_back(it);
		}
	
	//  add fluids  // todo: not for size, too big..
	// for (const auto& fl : scn->refl.fluids)
	// 	mItems.push_back(fl.item[0]);

	//  add veget  all  // todo: big only
	// for (const auto& veg : scn->vegetItemsGI)
	// 	mItems.push_back(veg);
	//--------

	mSceneMgr->updateSceneGraph();

	mVoxelizer->removeAllItems();

	//  todo: ter aabb only ..
	// Aabb terAabb = scn->ter->getWorldAabb();
	Real tws = scn->sc->tds[0].fTerWorldSize;
	Aabb terAabb(Vector3(0,0,0), Vector3(tws,tws,tws ));  // h min max.. ?
	LogO("*[]* GI ter size "+fToStr(tws));

	IrradianceFieldSettings ifdSet;  // IFD cfg
	ifdSet.mNumRaysPerPixel = iIfdRaysPpx;
    ifdSet.mIrradianceResolution = iIfdResolution;
    ifdSet.mDepthProbeResolution = iIfdDepthProbeRes;

	Aabb aabb( Aabb::BOX_NULL );

	if (GIneedsVoxels())
	{
		auto itor = mItems.begin(), endt = mItems.end();
		while (itor != endt)
			mVoxelizer->addItem( *itor++, false );

		mVoxelizer->autoCalculateRegion();
		// mVoxelizer->setRegionToVoxelize(false, terAabb, terAabb );  // todo

		int div = 1; //2;  //1 //par?
		mVoxelizer->dividideOctants( div, div, div );

		mVoxelizer->build( mSceneMgr );

		if (!mVCT)
		{
			mVCT = new VctLighting( Id::generateNewId<VctLighting>(), mVoxelizer, true );
			mVCT->setAllowMultipleBounces( true );

			HlmsPbs* hlmsPbs = getHlmsPbs();
			hlmsPbs->setVctLighting( mVCT );
		}
		mVCT->update( mSceneMgr, iVctBounces, fVctThinWall );

		aabb.setExtents( mVoxelizer->getVoxelOrigin(),
			mVoxelizer->getVoxelOrigin() + mVoxelizer->getVoxelSize() );
	}

	if ((GIgetMode() == Ifd || GIgetMode() == IfdVct) && bIfdRaster)
	{
		auto itor = mItems.begin(), endt = mItems.end();
		while (itor != endt)
		{
			aabb.merge( (*itor)->getWorldAabb() );
			++itor;
		}

		/*for (int i=0; i < 3; ++i)
			ifSet.mNumProbes[i] = 1u;
		ifSet.mIrradianceResolution = 16u;
		ifSet.mDepthProbeResolution = 32u;*/

		//  Generate IFD via raster
		ifdSet.mRasterParams.mWorkspaceName = "IrradianceFieldRasterWorkspace";
		ifdSet.mRasterParams.mCameraNear = 0.01f;
		ifdSet.mRasterParams.mCameraFar = aabb.getSize().length() * 2.0f;
	}

	mIFD->initialize( ifdSet, aabb.getMinimum(), aabb.getSize(), mVCT );

	LogO(String("*[]* GI Voxelize end: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


//  🛠️ info text
//-----------------------------------------------------------------------------------
void AppGui::GIupdText()
{
	if (!gcom->txGIinfo)  return;
	HlmsManager *hlmsManager = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlmsManager->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlmsManager->getHlms( HLMS_PBS ) );

	static const String visModes[] =
	{	"Albedo", "Normal", "Emissive", "None", "Lighting"  };

	static const String ifdProbeVisModes[] =
	{	"Irradiance", "Depth", "None"  };

 	String s = "GI mode: " + GIstrMode() +
		"   VCT: " + (hlmsPbs->getVctFullConeCount() ? "HQ" : "LQ");
 	s += "\n";
	if (mVCT)
	{
		s += String("VCT: ") + (mVCT->isAnisotropic() ? "Anisotropic" : "Isotropic");
		s += "  Num bounces: "+toStr(iVctBounces) + "  thin wall: "+fToStr(fVctThinWall);
		auto* vox = mVCT->getVoxelizer();
		
		s += String("\nVoxelizer:  ");
		auto v = vox->getVoxelCellSize();
		s += "cell: "+fToStr(v.x)+", "+fToStr(v.y)+"  ";
		v = vox->getVoxelSize();  // region
		s += "world: "+fToStr(v.x)+", "+fToStr(v.y)+"  ";
		v = vox->getVoxelResolution();
		s += "res: "+fToStr(v.x)+", "+fToStr(v.y)+"  ";
	}
 	s += "\nVis mode: " + visModes[iVctDebugVis];

	auto gi = GIgetMode();
	if (gi == IfdVct || gi == Ifd)
	{	s += String("\nGenerate IFD via: ") + (bIfdRaster ? "Raster" : "Voxels");
		s += String("  IFD debug vis: ") + ifdProbeVisModes[iIfdDebugVis];
	}
	gcom->txGIinfo->setCaption(s);
}


//  Next GI method
//-----------------------------------------------------------------------------------
void AppGui::GInextMethod( int add )
{
	GiMode gi = GIgetMode();
	gi = static_cast<GiMode>( (gi + NumGiModes + add) % NumGiModes );

	if (mIFD)
		if (gi != Ifd && gi != IfdVct)
			//  disable IFD vis  while not in use
			mIFD->setDebugVisualization(
				IrradianceField::DebugVisualizationNone, 0, iIfdVisSphereVert);
		else  //  restore IFD vis  if it was in use
			GIIfdVisUpd();

	auto* hlmsPbs = getHlmsPbs();
	switch (gi)
	{
	case NoGI:
		hlmsPbs->setIrradianceField( 0 );
		hlmsPbs->setVctLighting( 0 );  break;
	case Ifd:
		hlmsPbs->setIrradianceField( mIFD );
		hlmsPbs->setVctLighting( 0 );  break;
	case Vct:
		hlmsPbs->setIrradianceField( 0 );
		hlmsPbs->setVctLighting( mVCT );  break;
	case IfdVct:
		hlmsPbs->setIrradianceField( mIFD );
		hlmsPbs->setVctLighting( mVCT );  break;
	case NumGiModes:  break;
	}
}


//  ⛓️ utils
//-----------------------------------------------------------------------------------
bool AppGui::GIneedsVoxels() const
{
	return !( GIgetMode() == Ifd && bIfdRaster );
}

//  next GI mode
//-----------------------------------------------------------------------------------
void AppGui::GInextVctVis( int add )
{
	const auto none = VctVoxelizer::DebugVisualizationNone;
	const auto all = none + 2;
	iVctDebugVis = (iVctDebugVis + all + add) % all;

	if (iVctDebugVis <= none)
	{
		mVCT->setDebugVisualization( false, mSceneMgr );
		mVoxelizer->setDebugVisualization(
			static_cast<VctVoxelizer::DebugVisualizationMode>( iVctDebugVis ), mSceneMgr );
	}else
	{
		mVoxelizer->setDebugVisualization( none, mSceneMgr );
		mVCT->setDebugVisualization( true, mSceneMgr );
	}

	const bool showItems = iVctDebugVis == none ||
		iVctDebugVis == VctVoxelizer::DebugVisualizationEmissive;

	auto itor = mItems.begin(), endt = mItems.end();
	while (itor != endt)
	{
		(*itor)->setVisible( showItems );
		++itor;
	}
}

//-----------------------------------------------------------------------------------
void AppGui::GItoggletVctQuality()
{
	HlmsManager *hlms = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) );
	hlmsPbs->setVctFullConeCount( !hlmsPbs->getVctFullConeCount() );
}

//-----------------------------------------------------------------------------------
GiMode AppGui::GIgetMode() const
{
	HlmsManager *hlms = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) );

	const bool hasVct = hlmsPbs->getVctLighting() != 0;
	const bool hasIfd = hlmsPbs->getIrradianceField() != 0;
	
	if (hasVct && hasIfd)	return IfdVct;
	else if (hasIfd)		return Ifd;
	else if (hasVct)		return Vct;
	else					return NoGI;
	return NoGI;
}

String AppGui::GIstrMode() const
{
	GiMode gi = GIgetMode();
	switch (gi)
	{
	case Ifd:     return "IFD";
	case Vct:     return "VCT";
	case IfdVct:  return "IFD+VCT";
	case NoGI:    return "None";
	case NumGiModes:  break;
	}
	return "None";
}


//-----------------------------------------------------------------------------------
void AppGui::GInextIfdProbeVis( int add )
{
	const auto all = IrradianceField::DebugVisualizationNone + 1;
	iIfdDebugVis = (iIfdDebugVis + all + add) % all;

	GIIfdVisUpd();
}

void AppGui::GIIfdVisUpd()
{
	if (!mIFD)  return;
	mIFD->setDebugVisualization(
		static_cast<IrradianceField::DebugVisualizationMode>( iIfdDebugVis ),
		mSceneMgr, iIfdVisSphereVert );
}

HlmsPbs* AppGui::getHlmsPbs()
{
	HlmsManager *hlms = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>(hlms->getHlms(HLMS_PBS)) );
	return static_cast<HlmsPbs *>(hlms->getHlms(HLMS_PBS));
}
