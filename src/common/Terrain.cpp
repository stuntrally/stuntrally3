#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "ShapeData.h"
#include "GuiCom.h"
#include "CScene.h"
#include "SceneXml.h"
#include "paths.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
	#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#else
	#include "CGame.h"
	#include "game.h"
	#include "settings.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <OgreTimer.h>

#include "Grass.h"
#include "Terra.h"
using namespace Ogre;


///--------------------------------------------------------------------------------------------------------------
//  Create Terrain
///--------------------------------------------------------------------------------------------------------------
void CScene::CreateTerrains(bool terLoad)
{
	int all = sc->tds.size();
	for (int i = 0; i < all; ++i)
		CreateTerrain(i, terLoad);
}
void CScene::updTerLod()
{
	//assert( ters.size() == sc->tds.size() );
	if (ters.empty())  return;
	int s = std::min(ters.size(), sc->tds.size());
	
	for (int i=0; i < ters.size(); ++i)
	{
		int lod = sc->tds[i].iHorizon > 0 ?
			app->pSet->g.horiz_lod : app->pSet->g.ter_lod;
		ters[i]->iLodMax = 6 - lod;  //par`
	}
}

String CScene::getHmap(int n, bool bNew)
{
	String fname = app->gcom->TrkDir() + "heightmap" 
		+ (n > 0 ? toStr(n+1) : "")
		+ (bNew ? "-new" : "") + ".f32";
	return fname;
}

void CScene::CreateTerrain(int n, bool terLoad)
{
	assert(n < sc->tds.size());
	Ogre::Timer tm;
	auto si = toStr(n+1);
	auto& td = sc->tds[n];

#ifndef SR_EDITOR  // ed all
	if (td.iHorizon > app->pSet->g.horizons)
		return;
#endif

	///  Load Heightmap data --------
	bool bNewHmap = n == iNewHmap;
	Ogre::Timer ti;
	if (terLoad || bNewHmap)
	{
	#ifndef SR_EDITOR  // game
		String fname = getHmap(n, 0);
		bool exists = PATHS::FileExists(fname);
		if (!exists)
			LogO("Terrains error! No hmap file: "+fname);
	#else  // ed
		String fname = getHmap(n, bNewHmap);
		bool exists = PATHS::FileExists(fname);
		if (!exists)
		{	fname = getHmap(n, 0);
			exists = PATHS::FileExists(fname);
		}
	#endif
		int fsize = exists ? td.getFileSize(fname) : 512*512*4;  // anything const-
		int size = fsize / sizeof(float);

		td.hfHeight.clear();
		td.hfHeight.resize(size);  // flat 0

		//  load f32 HMap +
		if (exists)
		{
			std::ifstream fi;
			fi.open(fname.c_str(), std::ios_base::binary);
			fi.read((char*)&td.hfHeight[0], fsize);
			fi.close();
		}
		
	   	//**  new  .. // todo GetTerMtrIds() from blendmap ..
		#ifndef SR_EDITOR
		if (n==0)  // 1st ter only-
		{
			app->blendMtr.resize(size);
			// memset(app->blendMtr,0,size2);  // zero

			app->blendMapSize = sqrt(size);
			// sc->td.layersAll[0].surfId = 0;  //par ter mtr..
		}
		#endif
	}


	//; CreateBlendTex();  //+

	LogO(String(":::* Time ter-")+si+" Hmap: " + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();  // 4MB ~13ms

	//; UpdBlendmap();


	CreateTerrain1(n);
	updTerLod();

	// ter = mTerra;  //set ptr
	if (n == 0)  // 1st ter only-
		grass->terrain = ter;
	grass->mSceneMgr = app->mSceneMgr;
	
	LogO(String(":::* Time ter-")+si+" Ter: " + fToStr(tm.getMilliseconds(),0,3) + " ms");
}


//?  save ter hmap to mem (all editing would be lost)
void CScene::copyTerHmap()
{
	if (!ter)  return;
	//; float *fHmap = terrain->getHeightData();  // todo: ?
	// int size = sc->td.iVertsX * sc->td.iVertsX * sizeof(float);
	// memcpy(sc->td.hfHeight, fHmap, size);
}

void CScene::UpdBlendmap()
{
	// if (ter)
	// 	ter->blendmap.Update();
	if (ters[terCur])
		ters[terCur]->blendmap.Update();
}


//  🎳 Bullet Terrain
//---------------------------------------------------------------------------------------------------------------
void CScene::CreateBltTerrains()
{
	int iLastHoriz = 0;
	for (const auto& td : sc->tds)
		if (td.iHorizon > iLastHoriz)
			iLastHoriz = td.iHorizon;
		
	//  can drive outside regular terrains 0
	auto max_horiz = app->pSet->g.horizons;
	bool drive_horiz =
	#ifdef SR_EDITOR
		false;
	#else
		app->pSet->game.drive_horiz;
	#endif


	for (const auto& td : sc->tds)
	if (td.iHorizon <= max_horiz &&
		(td.collis || td.iHorizon > 0 && drive_horiz))
	{
		btVector3 pos(td.posX, -td.posZ +
			// 1st ter   has:  td.ofsZ = fTriangleSize on convert
			// next ters  (made new in sr3-ed)  have: 0  but need same as above
			(td.ofsZ == 0.f ? td.fTriangleSize : 0.f), 0.f);

		btHeightfieldTerrainShape* hfShape = new btHeightfieldTerrainShape(
			td.iVertsXold, td.iVertsXold,
			&td.hfHeight[0], td.fTriangleSize,
			-1300.f,1300.f, 2, PHY_FLOAT,false);  //par- max height
		
		hfShape->setUseDiamondSubdivision(true);

		btVector3 scl(td.fTriangleSize, td.fTriangleSize, 1);
		hfShape->setLocalScaling(scl);
		hfShape->setUserPointer((void*)SU_Terrain);

		btCollisionObject* col = new btCollisionObject();
		col->setCollisionShape(hfShape);

		//  offset new hmaps, even 2^n
		bool of = td.iVertsXold == td.iVertsX;
		Real xofs = of ? 0.5f * td.fTriangleSize : 0.f;

		btVector3 ofs(-xofs, -xofs, 0.0);
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pos + ofs);
		col->setWorldTransform(tr);

		col->setFriction(0.9);   //+
		col->setRestitution(0.0);
		//col->setHitFraction(0.1f);
		col->setCollisionFlags(col->getCollisionFlags() |
			btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
		#ifndef SR_EDITOR  // game
			app->pGame->collision.world->addCollisionObject(col);
			app->pGame->collision.shapes.push_back(hfShape);
		#else
			app->world->addCollisionObject(col);
		#endif
		
		
		#ifndef SR_EDITOR
		///  border planes []
		const float px[4] = {-1, 1, 0, 0};
		const float py[4] = { 0, 0,-1, 1};
		const bool b[4] = {td.bL, td.bR, td.bF, td.bB};  // disable chks

		//  last horiz all []
		bool last = drive_horiz && td.iHorizon == iLastHoriz;
		bool skip = drive_horiz && td.iHorizon == 0;  // regular ters

		if (!skip || last)
		for (int i=0; i < 4; ++i)
		if (b[i] || last)
		{
			btVector3 vpl(px[i], py[i], 0);
			btCollisionShape* shp = new btStaticPlaneShape(vpl,0);
			shp->setUserPointer((void*)SU_Border);
			
			btTransform tr;  tr.setIdentity();
			tr.setOrigin(pos + vpl * -0.49 * td.fTerWorldSize + ofs);  //par 0.5-

			btCollisionObject* col = new btCollisionObject();
			col->setCollisionShape(shp);
			col->setWorldTransform(tr);
			col->setFriction(0.3);   //+
			col->setRestitution(0.0);
			col->setCollisionFlags(col->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);

			app->pGame->collision.world->addCollisionObject(col);
			app->pGame->collision.shapes.push_back(shp);
		}
		#endif
	}
}
