#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "App.h"
// #include "Cam.h"
#include "FluidsReflect.h"

#include "CScene.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "CData.h"
#include "ShapeData.h"
#include "GraphicsSystem.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>
// #include <OgreTextureGpuManager.h>

#include "HlmsPbs2.h"
// #include "Terra.h"
#include <OgreHlmsManager.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorNode.h>
#include <Compositor/OgreCompositorWorkspace.h>
// #include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>

#include <OgreItem.h>
#include <OgreVector2.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreRenderable.h>
#include "OgrePlanarReflections.h"
#include <OgreBitwise.h>
#include <OgreStagingTexture.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
using namespace Ogre;

#ifndef SR_EDITOR
	#include "game.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>


void ReflectListener::workspacePreUpdate( CompositorWorkspace *workspace )
{
	if (mPlanarRefl)
		mPlanarRefl->beginFrame();
}

//-----------------------------------------------------------------------------------
void ReflectListener::passEarlyPreExecute( CompositorPass *pass )
{
	// return;  //!-
	// assert( dynamic_cast<const CompositorPassSceneDef *>( pass->getDefinition() ) );  //!?-
	const CompositorPassSceneDef *passDef =
		static_cast<const CompositorPassSceneDef *>( pass->getDefinition() );

	auto id = passDef->mIdentifier;
	auto s = passDef->mProfilingId;
	//** LogO("ws pass: "+toStr(id)+" "+s);  //toStr(pass->getParentNode()->getId() ));

	//  Ignore clear etc  ---
	if (pass->getType() != PASS_SCENE)
		return;

	//  Ignore shadows ---
	if (passDef->mShadowNodeRecalculation == SHADOW_NODE_CASTER_PASS)
		return;

	CompositorPassScene *passScene = static_cast<CompositorPassScene *>( pass );
	Camera *camera = passScene->getCamera();

#if 0  // todo: upd ter here? for splitscreen
	///  ⛰️ Terrain  ----?
	if (app->mGraphicsSystem->getRenderWindow()->isVisible())
	{
		// Force update the shadow map every frame
		const float lightEpsilon = 0.0001f;  //** 0.0f slow
		for (auto* ter : app->scn->ters)
		if (ter)
			ter->update( !app->scn->sun ? -Vector3::UNIT_Y :
				app->scn->sun->getDerivedDirectionUpdated(), camera, lightEpsilon );
	}
#endif

#define OFF_FOG  //-

#if defined(SR_EDITOR) && defined(OFF_FOG)
	bool rtt = //id == 10101 11101 
		id == 22201;  //?..
		// id >= 22201 && id <= 22299;
		//id >= 11100 && id < 11110;  // road, ter
	if (rtt)
	{	app->scn->UpdFog(0, 1);  //- off fog in RTTs
		app->scn->UpdSun();
	}
#endif

#if defined(SR_EDITOR) && defined(OFF_FOG)
	rtt = id == 22299 || id == 10001 || id == 33301;  //?..
	if (rtt)
	{
		app->scn->UpdFog(
			id != 10001
			, 0);  //- on fog back  main or prv cam
		app->scn->UpdSun();
	}
#endif

	//  Ignore passes not tagged to receive reflections/refract
	bool reflect = id == 22202 || id == 33302   // rtt
				|| id == 10002 || id == 21002;  // main
	if (!reflect)
		return;

	//camera->setLodBias()  // not here, wont work

	//  aspect ratio must match that of the camera we're reflecting
	if (mPlanarRefl)
		mPlanarRefl->update( camera, camera->getAutoAspectRatio()
			? pass->getViewportAspectRatio( 0u ) : camera->getAspectRatio() );
}


//-----------------------------------------------------------------------------------
void FluidsReflect::DestroyRTT()
{
	LogO("D~~~ destroy Fluids RTT");

	Root *root = app->mGraphicsSystem->getRoot();
	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	assert( dynamic_cast<HlmsPbs2 *>( hlms ) );
	HlmsPbs2 *pbs = static_cast<HlmsPbs2 *>( hlms );
	pbs->setPlanarReflections( 0 );  // off

	if (mWsListener)
	for (auto* ws : app->mWorkspaces)
		ws->removeListener( mWsListener );
	delete mWsListener;  mWsListener = 0;

	delete mPlanarRefl;  mPlanarRefl = 0;
}

void FluidsReflect::CreateRTT()
{
	LogO("C~~~ create Fluids RTT");
	if (mPlanarRefl)
	{	LogO("C~~~ create Fluids RTT ALREADY done!");
		return;
	}
	Root *root = app->mGraphicsSystem->getRoot();
	bool useCompute = false;
#if !OGRE_NO_JSON
	useCompute = root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM );
#endif

	auto* sc = app->scn->sc;
	int all = sc->fluids.size();
	if (all > 0)
	{
		mPlanarRefl = new PlanarReflections(
			app->pSet, app->mSceneMgr, root->getCompositorManager2(),
			50.0, 0 );  //** par-? 500
		uint32 size = app->pSet->GetTexSize(app->pSet->g.water_reflect);

		mPlanarRefl->setMaxActiveActors( all,
			"PlanarReflections",
			// true,  // accurate-
			false, //par?
			size, size, 1, // mipmaps
			PFG_RGBA8_UNORM_SRGB, useCompute /*, mWsListener*/ );
	}
	
	if (!mWsListener)
	{
		LogO("--++ WS add Listener");
		mWsListener = new ReflectListener( app, mPlanarRefl );

		for (auto* ws : app->mWorkspaces)
		{	ws->removeListener( mWsListener );  // not necessary
			ws->addListener( mWsListener );
	}	}

	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	assert( dynamic_cast<HlmsPbs2 *>( hlms ) );
	HlmsPbs2 *pbs = static_cast<HlmsPbs2 *>( hlms );
	pbs->setPlanarReflections( mPlanarRefl );  // only one
}


//----------------------------------------------------------------------------------------------------------------------
///  🆕 create Fluid areas  . . . . . . . 
//----------------------------------------------------------------------------------------------------------------------
void FluidsReflect::CreateFluids()
{
	LogO("C~~~ create Fluids");
	auto dyn = SCENE_STATIC;
	auto* sc = app->scn->sc;
#ifdef SR_EDITOR
	dyn = SCENE_DYNAMIC;
	app->UpdFluidBox();
#endif

	bool reflect = app->pSet->g.water_reflect;
	
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		//  plane, mesh  ----
		Plane p(Vector3::UNIT_Z, 0.f);

		String sMesh = "WtrMesh"+toStr(i), sMesh2 = "WtrPlane"+toStr(i);
		const Vector2 v2size(fb.size.x, fb.size.z),
			uvTile(fb.tile.x*fb.size.x, fb.tile.y*fb.size.z);

		v1::MeshPtr meshV1 = v1::MeshManager::getSingleton().createPlane(
			sMesh, rgDef,
			p, v2size.x, v2size.y,
			// 1,1, true, 1,  // fix-
			32,32, true, 1,  //par- steps /fake  buggy-
			uvTile.x, uvTile.y,
			Vector3::UNIT_Y,  // up vec for uv
			v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

		meshV1->buildTangentVectors();
		MeshPtr mesh = MeshManager::getSingleton().createByImportingV1(
			sMesh2, rgDef,
			meshV1.get(), false, false, false, false );
			// meshV1.get(), false, false, true, false );
			// meshV1.get(), true, true, true );  //-	
		
		MeshManager::getSingleton().remove(sMesh);  // not needed
		// meshV1->unload();  //


		//  item, node  ----
		SceneManager *mgr = app->mSceneMgr;
		SceneNode *rootNode = mgr->getRootSceneNode( dyn );
		Item* item[2] ={0,0};
		SceneNode* node[2] ={0,0};
		
		String sMtr = fb.id == -1 ? "" :
			app->scn->data->fluids->fls[fb.id].material +
			(fb.hq == 0 || app->pSet->g.water_refract == 0 ? "_lq" : "");

		//  [0] above  [1] below
		for (int n=0; n < 1; ++n)  // 2  // todo: FIXME refract wirefr bad
		{
			item[n] = mgr->createItem( mesh, dyn );
			item[n]->setDatablock( sMtr );
			item[n]->setCastShadows( false );
			auto rqg = fb.hq == 2 ? RQG_Fluid+1 : RQG_Fluid;
			// LogO("fluid "+iToStr(i,2)+" n "+toStr(n)+"  "+sMtr+"  rqg "+toStr(rqg));  //-

			item[n]->setRenderQueueGroup( rqg );  item[n]->setVisibilityFlags( RV_Fluid );
			app->SetTexWrap(item[n]);
			
			node[n] = rootNode->createChildSceneNode( dyn );
			node[n]->setPosition( fb.pos + Vector3(0, n ? -0.05f : 0, 0));  // 2nd down
			node[n]->setOrientation( Quaternion(
				(n ? -1.f : 1.f) * Radian( -Math::HALF_PI ), Vector3::UNIT_X ) );  // _
			node[n]->attachObject( item[n] );
		}

		//  actor, tracked  ----
		PlanarReflectionActor* actor =0;
		PlanarReflections::TrackedRenderable* tracked =0;
		if (reflect && mPlanarRefl && fb.hq >= 2)  // quality
		{
			actor = mPlanarRefl->addActor( PlanarReflectionActor(
				node[0]->getPosition(), v2size, node[0]->getOrientation() ));
	        
			if (0)  // i==0)  // par main big wtr only?
			{
				mPlanarRefl->reserve( i, actor );  // always win
				actor->mActivationPriority = i;
			}

			tracked = new PlanarReflections::TrackedRenderable(
				item[0]->getSubItem(0), item[0],
				Vector3::UNIT_Z, Vector3(0, 0, 0) );
			mPlanarRefl->addRenderable( *tracked );
		}

		//  add  ----
		OgreFluid fl;
		fl.tracked = tracked;  fl.actor = actor;
		fl.sMesh = sMesh;  fl.sMesh2 = sMesh2;
		for (int n=0; n < 2; ++n)
		{	fl.item[n] = item[n];
			fl.node[n] = node[n];
		}
		fluids.push_back(fl);
	}

	#ifndef SR_EDITOR  // game
		CreateBltFluids();
	#endif
}

//  💥 destroy
//----------------------------------------------------------------------------------------------------------------------
void FluidsReflect::DestroyFluids()
{
	LogO("D~~~ destroy Fluids");
	
	for (auto& fl : fluids)
	{
		if (fl.tracked)
		{
			mPlanarRefl->removeRenderable( fl.tracked->renderable );
			delete fl.tracked;
		}
		if (fl.actor)
			mPlanarRefl->destroyActor(fl.actor);

		for (int n=0; n < 2; ++n)
		{	if (fl.node[n])  app->mSceneMgr->destroySceneNode(fl.node[n]);
			if (fl.item[n])  app->mSceneMgr->destroyItem(fl.item[n]);
		}
		v1::MeshManager::getSingleton().remove(fl.sMesh);
		MeshManager::getSingleton().remove(fl.sMesh2);
	}
	fluids.clear();
}


//  🎳 bullet
//----------------------------------------------------------------------------------------------------------------------
void FluidsReflect::CreateBltFluids()
{
	auto* sc = app->scn->sc;
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		const FluidParams& fp = sc->pFluidsXml->fls[fb.id];

		///  add bullet trigger box   . . . . . . . . .
		btVector3 pc(fb.pos.x, -fb.pos.z, fb.pos.y -fb.size.y/2);  // center
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);
		//tr.setRotation(btQuaternion(0, 0, fb.rot.x*PI_d/180.f));

		btCollisionObject* bco = 0;
		float t = sc->tds[0].fTerWorldSize*0.5f;  // not bigger than terrain  // 1st ter!-
		btScalar sx = std::min(t, fb.size.x*0.5f), sy = std::min(t, fb.size.z*0.5f), sz = fb.size.y*0.5f;

	//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
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
	//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _

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
