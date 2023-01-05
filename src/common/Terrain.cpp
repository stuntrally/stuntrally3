#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "ShapeData.h"
#include "GuiCom.h"
#include "CScene.h"
#include "SceneXml.h"
#include "pathmanager.h"
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


///  OLD  Setup Terrain
//---------------------------------------------------------------------------------------------------------------
/*void CScene::SetupTerrain()
{
	// UpdTerErr();

	mTerrainGlobals->setLayerBlendMapSize(4);  // we use our own rtt, so reduce this
	mTerrainGlobals->setLightMapDirection(sun->getDerivedDirection());
	// mTerrainGlobals->setSkirtSize(1);  // low because in water reflect

	// di.terrainSize = sc->td.iTerSize; // square []-
	// di.worldSize = sc->td.fTerWorldSize;  //di.inputScale = td.Hmax;

	///  layer textures
	int ls = sc->td.layers.size();
	di.layerList.resize(ls);
	for (int i=0; i < ls; ++i)
	{
		TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String p = PATHMANAGER::Data() + (app->pSet->tex_size > 0 ? "/terrain/" : "/terrain_s/");
		String d_d, d_s, n_n, n_h;
		
		///  diff
		d_d = l.texFile;  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");

		if (!PATHMANAGER::FileExists(p+ d_d))
			texLayD[i].LoadTer(p+ "grass_green_d.jpg", p+ "grass_green_n.jpg", 0.f);
		else
		if (PATHMANAGER::FileExists(p+ d_s))
			texLayD[i].LoadTer(p+ d_d, p+ d_s, 0.f);
		else  // use _s from norm tex name
		{	d_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");
			texLayD[i].LoadTer(p+ d_d, p+ d_s, 0.f);
		}
		///  norm
		n_n = l.texNorm;  // ends with _n
		n_h = StringUtil::replaceAll(l.texNorm,"_n.","_h.");

		if (PATHMANAGER::FileExists(p+ n_n))
			texLayN[i].LoadTer(p+ n_n, p+ n_h, 1.f);
		else
			texLayN[i].LoadTer(p+ "flat_n.png", p+ n_h, 1.f);
		
		di.layerList[i].textureNames.push_back("layD"+toStr(i));
		di.layerList[i].textureNames.push_back("layN"+toStr(i));
	}
}*/


///--------------------------------------------------------------------------------------------------------------
//  Create Terrain
///--------------------------------------------------------------------------------------------------------------
void CScene::CreateTerrain(bool bNewHmap, bool terLoad)
{
	Ogre::Timer tm;
	terrain = 0;

	
	// UpdShaderParams();
	// UpdLayerPars();
	

	///  Load Heightmap data --------
	Ogre::Timer ti;
	if (terLoad || bNewHmap)
	{
		String fname = app->gcom->TrkDir() + (bNewHmap ? "heightmap-new.f32" : "heightmap.f32");

		int fsize = sc->td.getFileSize(fname);
		int size = fsize / sizeof(float);

		sc->td.hfHeight.clear();
		sc->td.hfHeight.resize(size);

		//  load f32 HMap +
		{
			std::ifstream fi;
			fi.open(fname.c_str(), std::ios_base::binary);
			fi.read((char*)&sc->td.hfHeight[0], fsize);
			fi.close();
		}
		
	   	//**  new  .. GetTerMtrIds() from blendmap ..
	#ifndef SR_EDITOR
		app->blendMtr.resize(size);
		// memset(app->blendMtr,0,size2);  // zero

		app->blendMapSize = sqrt(size);
		// sc->td.layersAll[0].surfId = 0;  //par ter mtr..
	#endif
	}


	//; CreateBlendTex();  //+

	LogO(String(":::* Time Hmap: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();  // 4MB ~13ms

	//; UpdBlendmap();


	app->CreateTerrain();

	terrain = app->mTerra;  //set ptr
	grass->terrain = app->mTerra;
	grass->mSceneMgr = app->mSceneMgr;
	
	LogO(String(":::* Time Terrain: ") + fToStr(tm.getMilliseconds(),0,3) + " ms");
}


//  save ter hmap to mem (all editing would be lost)
void CScene::copyTerHmap()
{
	if (!terrain)  return;
	//; float *fHmap = terrain->getHeightData();
	// int size = sc->td.iVertsX * sc->td.iVertsX * sizeof(float);
	// memcpy(sc->td.hfHeight, fHmap, size);
}


//  Destroy
void CScene::DestroyTerrain()
{
	/*for (int i=0; i < 6; ++i)
	{
		texLayD[i].Destroy();
		texLayN[i].Destroy();
	}*/
	terrain = 0;
	app->DestroyTerrain();
}


void CScene::UpdBlendmap()
{
	if (terrain)
		terrain->blendmap.Update();
}


//  🎳 Bullet Terrain
//---------------------------------------------------------------------------------------------------------------
void CScene::CreateBltTerrain()
{
	btHeightfieldTerrainShape* hfShape = new btHeightfieldTerrainShape(
		sc->td.iVertsXold, sc->td.iVertsXold,
		&sc->td.hfHeight[0], sc->td.fTriangleSize,
		-1300.f,1300.f, 2, PHY_FLOAT,false);  //par- max height
	
	hfShape->setUseDiamondSubdivision(true);

	btVector3 scl(sc->td.fTriangleSize, sc->td.fTriangleSize, 1);
	hfShape->setLocalScaling(scl);
	hfShape->setUserPointer((void*)SU_Terrain);

	btCollisionObject* col = new btCollisionObject();
	col->setCollisionShape(hfShape);

	//  offset new hmaps, even 2^n
	bool n = sc->td.iVertsXold == sc->td.iVertsX;
	Real nofs = n ? 0.5f * sc->td.fTriangleSize : 0.f;

	btVector3 ofs(-nofs, -nofs, 0.0);
	btTransform tr;  tr.setIdentity();  tr.setOrigin(ofs);
	col->setWorldTransform(tr);

	col->setFriction(0.9);   //+
	col->setRestitution(0.0);
	//col->setHitFraction(0.1f);
	col->setCollisionFlags(col->getCollisionFlags() |
		btCollisionObject::CF_STATIC_OBJECT /*| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
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

	for (int i=0; i < 4; ++i)
	{
		btVector3 vpl(px[i], py[i], 0);
		btCollisionShape* shp = new btStaticPlaneShape(vpl,0);
		shp->setUserPointer((void*)SU_Border);
		
		btTransform tr;  tr.setIdentity();
		tr.setOrigin(vpl * -0.5 * sc->td.fTerWorldSize + ofs);

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
