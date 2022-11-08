#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "CData.h"
#include "ShapeData.h"

// #include "WaterRTT.h"
#include "CScene.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
	#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#else
	#include "CGame.h"
	#include "game.h"
	//#include "settings.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include "OgreCommon.h"
#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreTimer.h>

#include <OgreItem.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreManualObject2.h>
#include <OgreHlmsPbsPrerequisites.h>
#include <OgreHlmsPbsDatablock.h>
using namespace Ogre;


///  create Fluid areas  . . . . . . . 
//----------------------------------------------------------------------------------------------------------------------
void CScene::CreateFluids()
{
#ifdef SR_EDITOR
	app->UpdFluidBox();
#endif
	if (!mNdFluidsRoot)
		mNdFluidsRoot = app->mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_STATIC/*"FluidsRootNode"*/);
			
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		//  plane
		Plane p;  p.normal = Vector3::UNIT_Y;  p.d = 0;
		String sMesh = "WaterMesh"+toStr(i), sMesh2 = "WtrPlane"+toStr(i);

		v1::MeshPtr meshV1 = v1::MeshManager::getSingleton().createPlane(
			sMesh, rgDef,
			p, fb.size.x, fb.size.z,
			6,6, true, 1,
			fb.tile.x*fb.size.x, fb.tile.y*fb.size.z, Vector3::UNIT_Z,
			v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

		MeshPtr mesh = MeshManager::getSingleton().createByImportingV1(
			sMesh2, rgDef,
			meshV1.get(), true, true, true );
		
		meshV1->unload();


		SceneManager *mgr = app->mSceneMgr;
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

		Item* item = mgr->createItem( mesh, SCENE_STATIC );
		String sMtr = fb.id == -1 ? "" : data->fluids->fls[fb.id].material;  //"Water"+toStr(1+fb.type)
		
		item->setDatablock( sMtr );  item->setCastShadows( false );
		item->setRenderQueueGroup( RQG_Fluid );  item->setVisibilityFlags( RV_Terrain );

		
		//  wrap tex
		HlmsSamplerblock sampler;
		sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
		sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
		sampler.mMaxAnisotropy = app->pSet->anisotropy;
		sampler.mU = TAM_WRAP;  sampler.mV = TAM_WRAP;  sampler.mW = TAM_WRAP;
		assert( dynamic_cast<Ogre::HlmsPbsDatablock *>( item->getSubItem( 0 )->getDatablock() ) );
		HlmsPbsDatablock *datablock =
			static_cast<Ogre::HlmsPbsDatablock *>( item->getSubItem( 0 )->getDatablock() );
		datablock->setSamplerblock( PBSM_DIFFUSE, sampler );
		datablock->setSamplerblock( PBSM_NORMAL, sampler );

		
		SceneNode* node = rootNode->createChildSceneNode( SCENE_STATIC );
		node->setPosition( fb.pos );  //, Quaternion(Degree(fb.rot.x),Vector3::UNIT_Y)
		node->attachObject( item );
		
		vFlSMesh.push_back(sMesh);  vFlSMesh2.push_back(sMesh2);
		vFlIt.push_back(item);  vFlNd.push_back(node);

	#ifndef SR_EDITOR  // game
		CreateBltFluids();
	#endif
	}		
}

void CScene::CreateBltFluids()
{
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		const FluidParams& fp = sc->pFluidsXml->fls[fb.id];

		///  add bullet trigger box   . . . . . . . . .
		btVector3 pc(fb.pos.x, -fb.pos.z, fb.pos.y -fb.size.y/2);  // center
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);
		//tr.setRotation(btQuaternion(0, 0, fb.rot.x*PI_d/180.f));

		btCollisionObject* bco = 0;
		float t = sc->td.fTerWorldSize*0.5f;  // not bigger than terrain
		btScalar sx = std::min(t, fb.size.x*0.5f), sy = std::min(t, fb.size.z*0.5f), sz = fb.size.y*0.5f;
		
	if (0 && fp.solid)  /// test random ray jumps meh-
	{
		const int size = 16;
		float* hfHeight = new float[size*size];
		int a = 0;
		for (int y=0; y<size; ++y)
		for (int x=0; x<size; ++x)
			hfHeight[a++] = 0.f;
		btHeightfieldTerrainShape* hfShape = new btHeightfieldTerrainShape(
			size, size, hfHeight, 1.f,
			-13.f,13.f, 2, PHY_FLOAT,false);  //par- max height
		
		hfShape->setUseDiamondSubdivision(true);

		btVector3 scl(sx, sy, sz);
		hfShape->setLocalScaling(scl);
		
		size_t id = SU_Fluid;  if (fp.solid)  id += fp.surf;
		hfShape->setUserPointer((void*)id);

		bco = new btCollisionObject();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(hfShape);  bco->setWorldTransform(tr);
		bco->setFriction(0.9);   //+
		bco->setRestitution(0.0);  //bco->setHitFraction(0.1f);
		bco->setCollisionFlags(bco->getCollisionFlags() |
			btCollisionObject::CF_STATIC_OBJECT /*| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
	
		bco->setUserPointer(new ShapeData(ST_Fluid, 0, &fb));  ///~~
	#ifndef SR_EDITOR
		app->pGame->collision.world->addCollisionObject(bco);
		app->pGame->collision.shapes.push_back(hfShape);
		fb.cobj = bco;
	#else
		app->world->addCollisionObject(bco);
	#endif

	}else{

		btCollisionShape* bshp = 0;
		bshp = new btBoxShape(btVector3(sx,sy,sz));

		//  solid surf
		size_t id = SU_Fluid;  if (fp.solid)  id += fp.surf;
		bshp->setUserPointer((void*)id);
		bshp->setMargin(0.1f); //

		bco = new btCollisionObject();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);

		if (!fp.solid)  // fluid
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE/**/);
		else  // solid
		{	bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT);
			bco->setFriction(0.6f);  bco->setRestitution(0.5f);  //par?..
		}

		bco->setUserPointer(new ShapeData(ST_Fluid, 0, &fb));  ///~~
	#ifndef SR_EDITOR
		app->pGame->collision.world->addCollisionObject(bco);
		app->pGame->collision.shapes.push_back(bshp);
		fb.cobj = bco;
	#else
		app->world->addCollisionObject(bco);
	#endif
	}
		
	}
#ifdef SR_EDITOR
	app->UpdObjPick();
#endif
}

void CScene::DestroyFluids()
{
	for (int i=0; i < vFlNd.size(); ++i)
	{
		vFlNd[i]->detachAllObjects();
		app->mSceneMgr->destroyItem(vFlIt[i]);
		app->mSceneMgr->destroySceneNode(vFlNd[i]);
		v1::MeshManager::getSingleton().remove(vFlSMesh[i]);
		MeshManager::getSingleton().remove(vFlSMesh2[i]);
	}
	vFlNd.clear();  vFlIt.clear();
	vFlSMesh.clear();  vFlSMesh2.clear();
}

#ifdef SR_EDITOR
void App::UpdFluidBox()
{
	int fls = scn->sc->fluids.size();
	bool bFluids = edMode == ED_Fluids && fls > 0 && !bMoveCam;
	if (fls > 0)
		iFlCur = std::max(0, std::min(iFlCur, fls-1));

	if (!ndFluidBox)  return;
	ndFluidBox->setVisible(bFluids);
	if (!bFluids)  return;
	
	FluidBox& fb = scn->sc->fluids[iFlCur];
	ndFluidBox->setPosition(fb.pos);
	ndFluidBox->setScale(fb.size);
}

void App::UpdMtrWaterDepth()
{
	float fl = edMode == ED_Fluids ? 0.f : 1.f;
	sh::Factory::getInstance().setSharedParameter("waterDepth", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(fl)));
}
#endif


//;  Water rtt, setup and recreate
/*void CScene::UpdateWaterRTT(Camera* cam)
{
	mWaterRTT->setRTTSize(ciShadowSizesA[app->pSet->water_rttsize]);
	mWaterRTT->setReflect(app->pSet->water_reflect);
	mWaterRTT->setRefract(app->pSet->water_refract);

	mWaterRTT->setViewerCamera(cam);
	mWaterRTT->mSceneMgr = app->mSceneMgr;

	if (!sc->fluids.empty())  // first fluid's plane
		mWaterRTT->setPlane(Plane(Vector3::UNIT_Y, sc->fluids.front().pos.y));
	mWaterRTT->recreate();
	mWaterRTT->setActive(!sc->fluids.empty());
}
*/
