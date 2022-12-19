#include "pch.h"
#include "CGame.h"
#include <OgrePrerequisites.h>
#include <OgreVector4.h>
#include "GraphicsSystem.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"

#include <OgreHlmsTerraDatablock.h>
#include <OgreTextureGpuManager.h>
#include <OgreGpuResource.h>
#include <OgreHlmsTerraPrerequisites.h>
#include <OgreHlmsSamplerblock.h>

//  SR
#include "CScene.h"
#include "settings.h"
#include "ShapeData.h"
#include "SceneXml.h"
#include "pathmanager.h"

#include "Def_Str.h"
#include "RenderConst.h"
using namespace Ogre;


//  Terrain
//-----------------------------------------------------------------------------------------------------------------------------
void App::CreateTerrain()
{
	if (mTerra) return;
	Root *root = mGraphicsSystem->getRoot();
	SceneManager *mgr = mGraphicsSystem->getSceneManager();
	SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
	
	HlmsManager *hlmsManager = root->getHlmsManager();
	HlmsDatablock *datablock = 0;

	LogO("---- new Terra json mat");

#if 0  // default .json
	mtrName = "TerraExampleMaterial";  // from .json
	// mGraphicsSystem->hlmsTerra->loadMaterial( "FileName for logging", defaultResourceGroupForLoadingTextures, jsonString, "" );
#else
	mtrName = "TerraExampleMaterial2";
	datablock = mGraphicsSystem->hlmsTerra->createDatablock(
		mtrName.c_str(), mtrName.c_str(),
		HlmsMacroblock(), HlmsBlendblock(), HlmsParamVec() );
	assert( dynamic_cast<HlmsTerraDatablock *>( datablock ) );
	HlmsTerraDatablock *tblock = static_cast<HlmsTerraDatablock *>( datablock );
	
	//  tex filtering
	HlmsSamplerblock sampler;
	sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
	sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sampler.mMaxAnisotropy = pSet->anisotropy;
	sampler.mU = TAM_WRAP;  sampler.mV = TAM_WRAP;  sampler.mW = TAM_WRAP;
	TextureGpuManager *texMgr = root->getRenderSystem()->getTextureGpuManager();


	tblock->setBrdf(TerraBrdf::BlinnPhongLegacyMath);
	// tblock->setBrdf(TerraBrdf::BlinnPhongSeparateDiffuseFresnel);  //** no fresnel-?
	// tblock->setBrdf(TerraBrdf::CookTorranceSeparateDiffuseFresnel);  //** no fresnel-?
	// tblock->setBrdf(TerraBrdf::CookTorrance);
	// tblock->setBrdf(TerraBrdf::DefaultUncorrelated);
	// tblock->setBrdf(TerraBrdf::DefaultSeparateDiffuseFresnel);
	// tblock->setFresnel();
	// tblock->setDiffuse(Vector3(1,0,0));

	///  Layer Textures  ----
	const Real fTer = sc->td.fTerWorldSize;  //= fTriangleSize * iTerSize;
	int ls = sc->td.layers.size();
	for (int i=0; i < ls; ++i)
	{
		TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		// di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String path = PATHMANAGER::Data() + "/terrain/";
		String d_d, d_s, n_n, n_h;
		
		///  diff
		d_d = l.texFile;  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");
		d_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");
		n_n = l.texNorm;  // ends with _n
		n_h = StringUtil::replaceAll(l.texNorm,"_n.","_h.");


		auto tex = texMgr->createOrRetrieveTexture(d_d,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tblock->setTexture( TERRA_DETAIL0 + i, tex );
			tblock->setSamplerblock( TERRA_DETAIL0 + i, sampler );
		}
		tex = texMgr->createOrRetrieveTexture(n_n,
			GpuPageOutStrategy::Discard, CommonTextureTypes::NormalMap, "General" );
		if (tex)
		{	tblock->setTexture( TERRA_DETAIL0_NM + i, tex );
			tblock->setSamplerblock( TERRA_DETAIL0_NM + i, sampler );
		}
		/*n_h = d_s = "white.png";  // todo: _r _m terrain textures..
		tex = texMgr->createOrRetrieveTexture(n_h,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tblock->setTexture( TERRA_DETAIL_ROUGHNESS0 + i, tex );
			tblock->setSamplerblock( TERRA_DETAIL_ROUGHNESS0 + i, sampler );
		}
		tex = texMgr->createOrRetrieveTexture(d_s,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tblock->setTexture( TERRA_DETAIL_METALNESS0 + i, tex );
			tblock->setSamplerblock( TERRA_DETAIL_METALNESS0 + i, sampler );
		}*/
		Real sc = fTer / l.tiling;
		tblock->setDetailMapOffsetScale( i, Vector4(0,0, sc,sc) );
		tblock->setMetalness(i, 0.2);
		tblock->setRoughness(i, 0.5);
	}
	// const HlmsSamplerblock *sam = tblock->getSamplerblock( TERRA_DETAIL0 );
	// sam->setTexture()

	// tblock->setDetailTextureProperty();

#endif

	LogO("---- new Terra");

	mTerra = new Terra( Id::generateNewId<MovableObject>(),
						&mgr->_getEntityMemoryManager( SCENE_STATIC ),
						mgr, RQG_Terrain, root->getCompositorManager2(),
						mGraphicsSystem->getCamera(), false );
	// mTerra->setCustomSkirtMinHeight(0.8f); //?-
	mTerra->setCastShadows( false );
	mTerra->sc = scn->sc;
	scn->terrain = mTerra;

	LogO("---- Terra load");

	//  Heightmap  ------------------------------------------------
	Real sizeXZ = sc->td.fTriangleSize * (sc->td.iVertsX-1);  //sc->td.fTerWorldSize;
	float ofs = sc->td.fTriangleSize;  // ofs fix, 1025 to 1024 verts etc
	LogO("Ter size: " + toStr(sc->td.iVertsX));// +" "+ toStr((sc->td.iVertsX)*sizeof(float))

	bool any = !mTerra->bNormalized;
	mTerra->load(
		sc->td.iVertsX-1, sc->td.iVertsY-1, 
		sc->td.hfHeight, sc->td.iVertsX,
		Vector3( 0.f,    any ? 0.5f : 0.f, ofs ),  //** y why?
		Vector3( sizeXZ, any ? 1.f : mTerra->fHRange, sizeXZ ),  //** ter norm scale..
		// true, true);
		false, false);

	// if (mTerra->m_blendMapTex)
	tblock->setTexture( TERRA_DETAIL_WEIGHT, mTerra->m_blendMapTex );  //**

	SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
	node->attachObject( mTerra );

	LogO("---- Terra attach");

	datablock = hlmsManager->getDatablock( mtrName );
	mTerra->setDatablock( datablock );

	mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
	mHlmsPbsTerraShadows->setTerra( mTerra );
	//Set the PBS listener so regular objects also receive terrain shadows
	Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
	hlmsPbs->setListener( mHlmsPbsTerraShadows );
}


void App::DestroyTerrain()
{
	LogO("D--- destroy Terrain");

	Root *root = mGraphicsSystem->getRoot();
	Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

	if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
	{
		hlmsPbs->setListener( 0 );
		delete mHlmsPbsTerraShadows;  mHlmsPbsTerraShadows = 0;
	}
	delete mTerra;  mTerra = 0;

	if (!mtrName.empty())
	{	mGraphicsSystem->hlmsTerra->destroyDatablock(mtrName);
		mtrName = "";
	}
}
