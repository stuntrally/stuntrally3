#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "App.h"
#include "AppGui.h"
#include "CScene.h"
#include "settings.h"
#include "ShapeData.h"
#include "SceneXml.h"
#include "paths.h"
#include "CData.h"
#include "PresetsXml.h"
#include "CGui.h"

#include <OgrePrerequisites.h>
#include <OgreVector4.h>
#include <OgreCommon.h>
#include "GraphicsSystem.h"
#include "OgreHlmsTerra.h"
#include "OgreHlmsPbsTerraShadows.h"
#include "Terra.h"
#include <OgreHlmsTerraDatablock.h>
#include <OgreTextureGpuManager.h>
#include <OgreGpuResource.h>
#include <OgreHlmsTerraPrerequisites.h>
#include <OgreHlmsSamplerblock.h>
using namespace Ogre;


//  ⛰️ Terrain
//-----------------------------------------------------------------------------------------------
void CScene::CreateTerrain1(int n, bool upd)
{
	auto sn = toStr(n);
	// if (mTerra)  return;
	SceneManager *mgr = app->mGraphicsSystem->getSceneManager();
	
	HlmsManager *hlmsMgr = app->mRoot->getHlmsManager();
	HlmsDatablock *db = 0;

	LogO("C--T Terrain create mat " + sn);

	String mtrName = "SR3_TerraMtr" + sn;
	db = app->mGraphicsSystem->hlmsTerra->createDatablock(
		mtrName.c_str(), mtrName.c_str(),
		HlmsMacroblock(), HlmsBlendblock(), HlmsParamVec() );
	assert( dynamic_cast<HlmsTerraDatablock *>( db ) );
	HlmsTerraDatablock *tdb = static_cast<HlmsTerraDatablock *>( db );
	
	//  tex filtering
	HlmsSamplerblock sb;
	app->InitTexFilters(&sb);
	TextureGpuManager *texMgr = app->mRoot->getRenderSystem()->getTextureGpuManager();

	const auto& td = sc->tds[n];
	// bool tripl = false;  // test
	int ter_tripl = app->pSet->ter_tripl;
	bool bTripl = ter_tripl && n == 0 && td.triplCnt > 0;
	LogO("C--T Terrain tripl layers: " + toStr(td.triplCnt)+" on: "+toStr(bTripl)+" gfx: "+toStr(ter_tripl));

	if (bTripl)  //^
	{	tdb->setDetailTriplanarDiffuseEnabled(true);
		if (ter_tripl > 1)
			tdb->setDetailTriplanarNormalEnabled(true);  // fixme black spots..
		// tdb->setDetailTriplanarRoughnessEnabled(true);  // no tex, not used
		// tdb->setDetailTriplanarMetalnessEnabled(true);
	}

	// tdb->setBrdf(TerraBrdf::Default);  //
	// tdb->setBrdf(TerraBrdf::BlinnPhong);  // +💡
	tdb->setBrdf(TerraBrdf::BlinnPhongLegacyMath);  // +💡 rough+
	// tdb->setBrdf(TerraBrdf::BlinnPhongFullLegacy);  //- white
	// tdb->setBrdf(TerraBrdf::BlinnPhongSeparateDiffuseFresnel);  //** no fresnel-?
	// tdb->setBrdf(TerraBrdf::CookTorranceSeparateDiffuseFresnel);  //** no specular?
	// tdb->setBrdf(TerraBrdf::CookTorrance);  // dull tiny spec-+
	// tdb->setBrdf(TerraBrdf::DefaultUncorrelated);  // dark-
	// tdb->setBrdf(TerraBrdf::DefaultSeparateDiffuseFresnel);  //- mirror?-
	// tdb->setDiffuse(Vector3(1,1,1));


	///  🏔️ Layer Textures  ------------------------------------------------
	const Real fTer = td.fTerWorldSize;
	// const Real fTer = td.fTriangleSize * td.iVertsX;
	int ls = td.layers.size();
	for (int i=0; i < ls; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];
		// di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String path = PATHS::Data() + "/terrain/";
		String d_d, d_s, n_n, n_h;
		
		///  diffuse  ----
		d_d = l.texFile;  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");
		d_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");
		n_n = l.texNorm;  // ends with _n
		n_h = StringUtil::replaceAll(l.texNorm,"_n.","_h.");


		auto tex = texMgr->createOrRetrieveTexture(d_d,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tdb->setTexture( TERRA_DETAIL0 + i, tex );
			tdb->setSamplerblock( TERRA_DETAIL0 + i, sb );
		}
		tex = texMgr->createOrRetrieveTexture(n_n,
			GpuPageOutStrategy::Discard, CommonTextureTypes::NormalMap, "General" );
		if (tex)
		{	tdb->setTexture( TERRA_DETAIL0_NM + i, tex );
			tdb->setSamplerblock( TERRA_DETAIL0_NM + i, sb );
		}

		/*n_h = d_s = "white.png";  // todo: _r _m terrain textures..
		tex = texMgr->createOrRetrieveTexture(n_h,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tdb->setTexture( TERRA_DETAIL_ROUGHNESS0 + i, tex );
			tdb->setSamplerblock( TERRA_DETAIL_ROUGHNESS0 + i, sb );
		}
		tex = texMgr->createOrRetrieveTexture(d_s,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tdb->setTexture( TERRA_DETAIL_METALNESS0 + i, tex );
			tdb->setSamplerblock( TERRA_DETAIL_METALNESS0 + i, sb );
		}*/
		Real sc = bTripl ?
			// fTer / l.tiling * 0.0005 :  //?? big
			fTer / l.tiling * 0.002 :  //** fixme!  ?? small
			fTer / l.tiling;
		tdb->setDetailMapOffsetScale( i, Vector4(0,0, sc,sc) );
		
		//  find mat params from presets
		auto s = d_d;  s = s.substr(0, s.find_first_of('.'));
		auto pt = data->pre->GetTer(s);
		if (!pt)
			LogO("Ter mtr error! not in presets: "+d_d);
		else
		{	tdb->setMetalness(i, pt->metal);  // fresnel, specular ?..
			tdb->setRoughness(i, pt->rough);
			LogO("* Ter lay: "+d_d+" met:"+fToStr(pt->metal)+
				"  ro: "+fToStr(pt->rough)+(pt->reflect ? "  refl" : ""));
			if (pt->reflect && app->mCubeReflTex)
				tdb->setTexture( TERRA_REFLECTION, app->mCubeReflTex );  // par
		}
		// tdb->setEmissive(0.5);  // todo: lava..
	}


	//  🆕 Create  ------------------------------------------------
	auto id = Id::generateNewId<MovableObject>();
	LogO("---T Terrain create " + sn + "  id " + toStr(id) + "  n " + toStr(n));

#ifdef SR_EDITOR
	auto dyn = SCENE_DYNAMIC;  // ed can move ter- rem?
#else
	auto dyn = SCENE_STATIC;
#endif

	auto* mTerra = new Terra( //si,
		n, id,
		&mgr->_getEntityMemoryManager( dyn ),
		mgr, RQG_Terrain, app->mRoot->getCompositorManager2(),
		app->mCamera, false );
	
	mTerra->mtrName = mtrName;
	// mTerra->setCustomSkirtMinHeight(0.8f); //?-
	mTerra->setCastShadows( false );
	mTerra->sc = sc;

	if (upd)  //  ed update ter only
		ters[n] = mTerra;
	else
	{	//  create all
		ters.push_back(mTerra);  //+
		if (n == 0)  // 1st ter only
			ter = mTerra;
	}

	//  ⛰️ Heightmap  ------------------------------------------------
	LogO("---T Terrain Hmap load");
	int size = td.iVertsX;
	Real tri = td.fTriangleSize, sizeXZ = tri * size;  //td.fTerWorldSize;
	// LogO("Ter size: " + toStr(td.iVertsX));// +" "+ toStr((td.iVertsX)*sizeof(float))

	bool any = !mTerra->bNormalized;
	mTerra->load( size, size,
		td.hfHeight, td.iVertsXold,
		Vector3(
			td.posX,
			any ? 0.5f : 0.f,  //** why y?
			td.posZ + td.ofsZ ),
		Vector3(
			sizeXZ,
			any ? 1.f : mTerra->fHRange,  //** ter norm scale..
			sizeXZ ),
		// true, true);
		false, false);  // #ifdef SR_ED?

	// if (mTerra->m_blendMapTex)
	tdb->setTexture( TERRA_DETAIL_WEIGHT, mTerra->blendmap.texture );  //**


	SceneNode *rootNode = mgr->getRootSceneNode( dyn );
	mTerra->node = rootNode->createChildSceneNode( dyn );
	mTerra->node->attachObject( mTerra );

	db = hlmsMgr->getDatablock( mtrName );
	mTerra->setDatablock( db );


	//  terra shadows,  hlms listener
	if (!mHlmsPbsTerraShadows && n == 0)  // 1st ter only?-
	{
		LogO("---T Terrain shadows");
		mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
		mHlmsPbsTerraShadows->setTerra( mTerra );
		
		//  Set the PBS listener so regular objects also receive terrain shadows
		Hlms *hlmsPbs = app->mRoot->getHlmsManager()->getHlms( HLMS_PBS );
		hlmsPbs->setListener( mHlmsPbsTerraShadows );
	}
	LogO("C--T Terrain created " + sn);
}


//  💥 destroy  ------------------------------------------------
void CScene::DestroyTerrain1(int n)
{
	LogO("D--T destroy Terrain "+toStr(n+1));

	Root *root = app->mGraphicsSystem->getRoot();
	Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

	if (n == 0)  // 1st ter only-?
	if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
	{
		hlmsPbs->setListener( 0 );
		delete mHlmsPbsTerraShadows;  mHlmsPbsTerraShadows = 0;
	}
	auto mtr = ters[n]->mtrName;

	delete ters[n];  ters[n] = 0;

	if (!mtr.empty())
	{
		LogO("D--T Terrain dest mat " + toStr(n));
		try
		{	app->mGraphicsSystem->hlmsTerra->destroyDatablock(mtr);
		}catch(...)
		{	}
		// ter->mtrName = "";
	}
}

//  💥 Destroy all
void CScene::DestroyTerrains()
{
	for (int i=0; i < ters.size(); ++i)
		DestroyTerrain1(i);
	ters.clear();
}


//  ⛓️ util
void CScene::TerNext(int add)
{
	int all = ters.size();
	assert(all > 0);
	terCur = (terCur + add + all) % all;
	ter = ters[terCur];
	td = &sc->tds[terCur];

#ifdef SR_EDITOR
	app->gui->SldUpd_Ter();
#endif
	// String fname = getHmap(terCur, false);//-
	// td->getFileSize(fname);
}

//  ⛓️⛰️ util all Ter H
Ogre::Real CScene::getTerH(Ogre::Real x, Ogre::Real z)
{
	for (auto ter : ters)
	if (ter)
	{
		Vector3 p(x, 0.f, z);
		if (ter->getHeightAt(p))
			return p.y;
	}
	return 0.f;  //-
}

bool CScene::getTerH(Ogre::Vector3& pos)  // sets y
{
	for (auto ter : ters)
	{
		Vector3 p = pos;
		if (ter && ter->getHeightAt(p))
		{
			pos = p;
			return true;
		}
	}
	return false;
}
