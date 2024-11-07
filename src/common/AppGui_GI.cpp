#include "pch.h"
#include "AppGui.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
// #include "CData.h"
#include "GraphicsSystem.h"

#include <OgreRoot.h>
// #include <OgreItem.h>
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

	mDebugVisMode = VctVoxelizer::DebugVisualizationNone;
	mIfdDebugVisMode = IrradianceField::DebugVisualizationNone;

	mVoxelizer = new VctVoxelizer( Id::generateNewId<VctVoxelizer>(),
			mRoot->getRenderSystem(), mRoot->getHlmsManager(), true );

	mIrradianceField = new IrradianceField( mRoot, mSceneMgr );

	HlmsManager *hlms = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) );

	hlmsPbs->setIrradianceField( mIrradianceField );

	GIVoxelizeScene();  // !

	LogO(String("*()* GI Init end: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}

//  🌄💥 GI destroy
//-----------------------------------------------------------------------------------
void AppGui::DestroyGI()
{
	delete mIrradianceField;  mIrradianceField = 0;
	delete mVctLighting;  mVctLighting = 0;
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
	//if (mGraphicsSystem->getRenderWindow()->isVisible())
	//{
	//    static int frame = 0;
	//    if (frame > 1)
	//    {
	//        mVoxelizer->build( mSceneMgr );
	//        mVctLighting->update( mSceneMgr, mNumBounces );
	//    }
	//    ++frame;
	//}

	if (mIrradianceField)
	if (GIgetMode() == IfdVct || GIgetMode() == Ifd)
		mIrradianceField->update( mUseRasterIrradianceField ? 4u : 200u );
}


//  ⛓️ utils
//-----------------------------------------------------------------------------------
bool AppGui::GIneedsVoxels() const
{
	return !( GIgetMode() == Ifd && mUseRasterIrradianceField );
}

//-----------------------------------------------------------------------------------
void AppGui::GInextVisMode( int add )
{
	const auto none = VctVoxelizer::DebugVisualizationNone;
	const auto all = none + 2;
	mDebugVisMode = (mDebugVisMode + all + add) % all;

	if (mDebugVisMode <= none)
	{
		mVctLighting->setDebugVisualization( false, mSceneMgr );
		mVoxelizer->setDebugVisualization(
			static_cast<VctVoxelizer::DebugVisualizationMode>( mDebugVisMode ), mSceneMgr );
	}else
	{
		mVoxelizer->setDebugVisualization( none, mSceneMgr );
		mVctLighting->setDebugVisualization( true, mSceneMgr );
	}

	const bool showItems = mDebugVisMode == none ||
		mDebugVisMode == VctVoxelizer::DebugVisualizationEmissive;

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
void AppGui::GInextIfdProbeVisMode( int add )
{
	const auto all = IrradianceField::DebugVisualizationNone + 1;
	mIfdDebugVisMode = (mIfdDebugVisMode + all + add) % all;

	SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

	mIrradianceField->setDebugVisualization(
		static_cast<IrradianceField::DebugVisualizationMode>( mIfdDebugVisMode ),
		sceneManager, 5u );
}

//-----------------------------------------------------------------------------------
void AppGui::GInextIrradianceField( int add )
{
	HlmsManager *hlms = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) );

	GiMode gi = GIgetMode();
	gi = static_cast<GiMode>( (gi + NumGiModes + add) % NumGiModes );

	if (gi != Ifd && gi != IfdVct)
		  // Disable IFD vis while not in use
		mIrradianceField->setDebugVisualization( IrradianceField::DebugVisualizationNone, 0,
												mIrradianceField->getDebugTessellation() );
	else  // Restore IFD vis if it was in use
		mIrradianceField->setDebugVisualization(
			static_cast<IrradianceField::DebugVisualizationMode>( mIfdDebugVisMode ),
			mGraphicsSystem->getSceneManager(), 5u );

	switch (gi)
	{
	case NoGI:
		hlmsPbs->setIrradianceField( 0 );
		hlmsPbs->setVctLighting( 0 );  break;
	case Ifd:
		hlmsPbs->setIrradianceField( mIrradianceField );
		hlmsPbs->setVctLighting( 0 );  break;
	case Vct:
		hlmsPbs->setIrradianceField( 0 );
		hlmsPbs->setVctLighting( mVctLighting );  break;
	case IfdVct:
		hlmsPbs->setIrradianceField( mIrradianceField );
		hlmsPbs->setVctLighting( mVctLighting );  break;
	case NumGiModes:  break;
	}
}

//  Voxelize scene
//-----------------------------------------------------------------------------------
void AppGui::GIVoxelizeScene()
{
	Ogre::Timer ti;
	LogO("*[]* GI Voxelize begin");

	mItems.clear();

	//  add objects
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		Object& o = scn->sc->objects[i];
		mItems.push_back(o.it);
	}
	//--------

	SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
	sceneManager->updateSceneGraph();

	mVoxelizer->removeAllItems();

	IrradianceFieldSettings ifSettings;
	Aabb fieldAabb( Aabb::BOX_NULL );

	if (GIneedsVoxels())
	{
		auto itor = mItems.begin(), endt = mItems.end();
		while (itor != endt)
			mVoxelizer->addItem( *itor++, false );

		mVoxelizer->autoCalculateRegion();
		mVoxelizer->dividideOctants( 1u, 1u, 1u );

		mVoxelizer->build( sceneManager );

		if (!mVctLighting)
		{
			mVctLighting = new VctLighting( Id::generateNewId<VctLighting>(), mVoxelizer, true );
			mVctLighting->setAllowMultipleBounces( true );

			HlmsManager *hlms = mRoot->getHlmsManager();
			assert( dynamic_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) ) );
			HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlms->getHlms( HLMS_PBS ) );

			hlmsPbs->setVctLighting( mVctLighting );
		}

		mVctLighting->update( sceneManager, mNumBounces, mThinWallCounter );

		fieldAabb.setExtents( mVoxelizer->getVoxelOrigin(),
								mVoxelizer->getVoxelOrigin() + mVoxelizer->getVoxelSize() );
	}

	if ((GIgetMode() == Ifd || GIgetMode() == IfdVct) && mUseRasterIrradianceField)
	{
		auto itor = mItems.begin(), endt = mItems.end();
		while (itor != endt)
		{
			fieldAabb.merge( (*itor)->getWorldAabb() );
			++itor;
		}

		/*for( int i=0; i < 3 ; ++i)
			ifSettings.mNumProbes[i] = 1u;
		ifSettings.mIrradianceResolution = 16u;
		ifSettings.mDepthProbeResolution = 32u;*/

		//  Generate IFD via raster
		ifSettings.mRasterParams.mWorkspaceName = "IrradianceFieldRasterWorkspace";
		ifSettings.mRasterParams.mCameraNear = 0.01f;
		ifSettings.mRasterParams.mCameraFar = fieldAabb.getSize().length() * 2.0f;
	}

	mIrradianceField->initialize( ifSettings,
		fieldAabb.getMinimum(), fieldAabb.getSize(), mVctLighting );

	LogO(String("*[]* GI Voxelize end: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
