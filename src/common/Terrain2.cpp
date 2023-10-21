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
#include <OgreVector3.h>
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
void CScene::CreateTerrain1(int n)
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
	auto* texMgr = app->mRoot->getRenderSystem()->getTextureGpuManager();

	//  triplanar  ----
	const auto& td = sc->tds[n];
	bool terTripl = td.triplCnt > 0 &&  n <= app->pSet->g.tripl_horiz;  // 0 ter, 1 horiz, 2
	int tex_tripl = app->pSet->g.ter_tripl;  // off, diff, norm
	bool bTripl = tex_tripl > 0 && terTripl;
	LogO("C--T Terrain tripl layers: " + toStr(td.triplCnt)+" ter: "+toStr(terTripl)+" tex: "+toStr(tex_tripl));

	if (bTripl)  //**  set tripl
	{	tdb->setDetailTriplanarDiffuseEnabled(true);
		if (tex_tripl >= 2 && terTripl)
			tdb->setDetailTriplanarMetalnessEnabled(true);
		//tdb->setDetailTriplanarRoughnessEnabled(true);  // no tex, not used yet
		if (tex_tripl >= 3 && terTripl)
			tdb->setDetailTriplanarNormalEnabled(true);  // fixme black spots..
	}
	tdb->bEmissive = td.emissive;

	// tdb->setBrdf(TerraBrdf::Default);  //
	// tdb->setBrdf(TerraBrdf::BlinnPhong);  // +💡
	tdb->setBrdf(TerraBrdf::BlinnPhongLegacyMath);  // +💡 rough+
	// tdb->setBrdf(TerraBrdf::BlinnPhongFullLegacy);  //- white bad
	// tdb->setBrdf(TerraBrdf::BlinnPhongSeparateDiffuseFresnel);  //** no fresnel-?
	// tdb->setBrdf(TerraBrdf::CookTorranceSeparateDiffuseFresnel);  //** no specular?
	// tdb->setBrdf(TerraBrdf::CookTorrance);  // dull tiny spec-+
	// tdb->setBrdf(TerraBrdf::DefaultUncorrelated);  // dark-
	// tdb->setBrdf(TerraBrdf::DefaultSeparateDiffuseFresnel);  //- mirror?-
	// tdb->setDiffuse(Vector3(1,1,1));


	///  🏔️ Layer Textures  ------------------------------------------------
	const Real fTer = td.fTerWorldSize;
	// const Real fTer = td.fTriangleSize * td.iVertsX;
	int ls = std::min(4, (int)td.layers.size());
	for (int i=0; i < ls; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];
		// di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String path = PATHS::Data() + "/terrain/";
		String d_d, d_s, d_r, n_n, n_s, fname;
		
		//  diffuse  ----
		d_d = l.texFile;  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");  // _s
		// d_r = StringUtil::replaceAll(l.texNorm,"_d.","_r.");  // _r
		//  specular  ----
		n_n = l.texNorm;  // _n
		n_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");  // _s


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

		/* // todo: _r rough terrain textures..
		tex = texMgr->createOrRetrieveTexture( "white.png", //d_r,
			GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
		if (tex)
		{	tdb->setTexture( TERRA_DETAIL_ROUGHNESS0 + i, tex );
			tdb->setSamplerblock( TERRA_DETAIL_ROUGHNESS0 + i, sb );
		}*/
		
		//  find _s  for specular, reflective (metal) or emissive_terrain
		fname = d_s;
		if (!PATHS::FileExists(path + fname))
		{	fname = n_s;
			if (!PATHS::FileExists(path + fname))
			{	fname = d_d;
				// LogO("Ter diff: "+d_d+"  _s from: "+fname);
		}	}
		//if (par..
		if (PATHS::FileExists(path + fname))
		{
			tex = texMgr->createOrRetrieveTexture(fname,
				GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
			if (tex)
			{	tdb->setTexture( TERRA_DETAIL_METALNESS0 + i, tex );
				tdb->setSamplerblock( TERRA_DETAIL_METALNESS0 + i, sb );
		}	}
		
		// Real ofy = bTripl ? 0.499 : 0.0, ofx = bTripl ? 0.501 : 0.0,  // meh
		Real of = bTripl ? 0.499 : 0.0,
			sc = bTripl ? 1.f / l.tiling : fTer / l.tiling;
		tdb->setDetailMapOffsetScale( i, Vector4(of,of, sc,sc) );
		
		//  find mat params from presets  ----
		auto s = d_d;  s = s.substr(0, s.find_first_of('.'));
		auto pt = data->pre->GetTer(s);
		if (!pt)
			LogO("Ter mtr error! not in presets: "+d_d);
		else
		{	tdb->setMetalness(i, pt->metal);
			tdb->setRoughness(i, pt->rough);
			tdb->setDiffuse(Vector3(td.clrDiff.x, td.clrDiff.y, td.clrDiff.z));
		
			// todo: diffuse, fresnel, specular, emissive color ?.. gui
			LogO("* Ter lay: "+d_d+"  _s from: "+fname+"  met:"+fToStr(pt->metal)+
				"  ro: "+fToStr(pt->rough)+(pt->reflect ? "  refl" : ""));
			
			if (app->mCubeReflTex && td.reflect != -1 &&  // disable
				(td.reflect == 1 ||  // force
				 (pt->reflect && td.reflect == 0)))  // whole auto, from layers
				tdb->setTexture( TERRA_REFLECTION, app->mCubeReflTex );
		}
	}


	//  🆕 Create  ------------------------------------------------
	LogO("---T Terrain create " + sn + "  n " + toStr(n));

#ifdef SR_EDITOR
	auto dyn = SCENE_DYNAMIC;  // ed can move ter- rem?
#else
	auto dyn = SCENE_STATIC;
#endif

	auto rqg = 
#if 0  // test 1
		RQG_Terrain
#else
		td.iHorizon>=2 ? RQG_Horizon2 :
		td.iHorizon==1 ? RQG_Horizon1 : RQG_Terrain;
#endif
	auto* mTerra = new Terra(
		app, sc, n,
		&mgr->_getEntityMemoryManager( dyn ),
		mgr, rqg,
		app->mRoot->getCompositorManager2(),
		app->mCamera, false );
	
	mTerra->mtrName = mtrName;
	// mTerra->setCustomSkirtMinHeight(0.8f); //?-
	mTerra->setCastShadows( false );
	mTerra->tDb = tdb;

	// if (upd)  //  ed update ter only
	// 	ters[n] = mTerra;
	// else
	{	//  create all
		ters.push_back(mTerra);  //+
		if (n == 0)  // 1st ter only
			ter = mTerra;
	}
	updTerLod();

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
	#ifdef SR_EDITOR
	tdb->setBlendmapTest( app->gui->bDebugBlend );
	#endif


	db = hlmsMgr->getDatablock( mtrName );
	mTerra->setDatablock( db );

	SceneNode *rootNode = mgr->getRootSceneNode( dyn );
	mTerra->node = rootNode->createChildSceneNode( dyn );
	mTerra->node->attachObject( mTerra );
	// mTerra->node->_getFullTransformUpdated();


	//  terra shadows,  hlms listener
	if (!mHlmsPbsTerraShadows && n == 0 /*&& !upd*/)  // 1st ter only?-
	{
		LogO("---T Terrain shadows  Pbs + set Listener");
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
		LogO("D--T Terrain shadows  Pbs - unset Listener");
		hlmsPbs->setListener( 0 );
		delete mHlmsPbsTerraShadows;  mHlmsPbsTerraShadows = 0;
	}
	auto mtr = ters[n]->mtrName;

	if (ters[n]->node)
		app->mSceneMgr->destroySceneNode(ters[n]->node);
	ters[n]->node = 0;

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
	app->gui->updTersTxt();
	app->gui->SldUpd_Ter();
#endif
}

//  ⛓️⛰️ util all Ter H
Ogre::Real CScene::getTerH(Ogre::Real x, Ogre::Real z)
{
	for (auto ter : ters)
	if (ter)
	{
		Vector3 p(x, 0.f, z);
		if (ter->getHeightAt(p))
			return p.y;  // todo: max of all ter?
	}
	return 0.f;  //-
}

//  all ters
bool CScene::getTerH(Ogre::Vector3& pos)  // sets y
{
	for (auto ter : ters)
	{
		Vector3 p = pos;
		if (ter && ter->getHeightAt(p))
		{
			pos.y = p.y;
			//pos = p;  //?
			return true;
		}
	}
	return false;
}

//  1 ter
bool CScene::getTerH(int id, Ogre::Vector3& pos)  // sets y
{
	assert(id < ters.size());
	Vector3 p = pos;
	if (ters[id] && ters[id]->getHeightAt(p))
	{
		pos.y = p.y;
		//pos = p;  //?
		return true;
	}
	return false;
}
