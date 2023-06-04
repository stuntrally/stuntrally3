#include "pch.h"
#include "Reflect.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "App.h"
#include "Cam.h"
#include "CScene.h"
#include "SceneXml.h"
#include "GraphicsSystem.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>
// #include <OgreTextureGpuManager.h>

#include "HlmsPbs2.h"
#include <OgreHlmsManager.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorNode.h>
#include <Compositor/OgreCompositorWorkspace.h>
// #include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>

#include <OgreItem.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include "OgrePlanarReflections.h"
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
using namespace Ogre;


void ReflectListener::workspacePreUpdate( CompositorWorkspace *workspace )
{
	mPlanarRefl->beginFrame();
}

//-----------------------------------------------------------------------------------
void ReflectListener::passEarlyPreExecute( CompositorPass *pass )
{
	//  Ignore clear etc
	if (pass->getType() != PASS_SCENE)
		return;

	assert( dynamic_cast<const CompositorPassSceneDef *>( pass->getDefinition() ) );
	const CompositorPassSceneDef *passDef =
		static_cast<const CompositorPassSceneDef *>( pass->getDefinition() );

	auto id = passDef->mIdentifier;
	// auto s = passDef->mProfilingId;
	// LogO("ws pass: "+s);  //toStr(pass->getParentNode()->getId() ));

#define OFF_FOG  //

#if defined(SR_EDITOR) && defined(OFF_FOG)
	bool rtt = id >= 11100 && id < 11103;  // road, ter
	if (rtt)
	{	app->scn->UpdFog(0, 1);  //- off fog in RTTs
		app->scn->UpdSun();
	}
#endif
	//  Ignore shadows
	if (passDef->mShadowNodeRecalculation == SHADOW_NODE_CASTER_PASS)
		return;

	//  Ignore passes not tagged to receive reflections
	bool prvCam = id == 11103;
	if (id != 22201 && !prvCam)  // main or prv cam
		return;
#if defined(SR_EDITOR) && defined(OFF_FOG)
	app->scn->UpdFog(prvCam, 0);  //- on fog back  main or prv cam
	app->scn->UpdSun();
#endif
	
	CompositorPassScene *passScene = static_cast<CompositorPassScene *>( pass );
	Camera *camera = passScene->getCamera();

	//  aspect ratio must match that of the camera we're reflecting
	mPlanarRefl->update( camera, camera->getAutoAspectRatio()
		? pass->getViewportAspectRatio( 0u ) : camera->getAspectRatio() );
}


//-----------------------------------------------------------------------------------
void FluidsReflect::DestroyRTT()
{
	CompositorWorkspace *ws = app->mGraphicsSystem->getCompositorWorkspace();
	if (mWsListener)
		ws->removeListener( mWsListener );
	delete mWsListener;  mWsListener = 0;

	delete mPlanarRefl;  mPlanarRefl = 0;
}

void FluidsReflect::CreateRTT()
{
	Root *root = app->mGraphicsSystem->getRoot();
	bool useCompute = false;
#if !OGRE_NO_JSON
	useCompute = root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM );
#endif

	mPlanarRefl = new PlanarReflections( app->mSceneMgr, root->getCompositorManager2(), 1.0, 0 );
	uint32 size = app->pSet->GetTexSize(app->pSet->water_reflect);
	
	mPlanarRefl->setMaxActiveActors( 1u, "PlanarReflections",
		true, size, size, 0/*?mipmaps*/, PFG_RGBA8_UNORM_SRGB, useCompute /*, mWsListener*/ );

	
	mWsListener = new ReflectListener( app, mPlanarRefl );

	// app->mWorkspaces
	LogO("++++ WS add Listener");
	CompositorWorkspace *workspace = app->mGraphicsSystem->getCompositorWorkspace();
	workspace->addListener( mWsListener );


	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	assert( dynamic_cast<HlmsPbs2 *>( hlms ) );
	HlmsPbs2 *pbs = static_cast<HlmsPbs2 *>( hlms );
	pbs->setPlanarReflections( mPlanarRefl );
}


//  Setup PlanarReflections
//-----------------------------------------------------------------------------------
void FluidsReflect::CreateFluids()
{

	//  plane
	const Vector2 mirrorSize( 4000.0f, 4000.0f );
	//  so that node->getOrientation matches the orientation for the PlanarReflectionActor
	v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
		"PlaneMirror", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Plane( Vector3::UNIT_Z, 0.0f ),  // z|
		mirrorSize.x, mirrorSize.y,
		30, 30, true, 1,  // par from size
		120.f, 120.f, Vector3::UNIT_Y,  // z|
		v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );
	sMesh = "PlaneMirror";
	MeshPtr planeMesh = MeshManager::getSingleton().createByImportingV1(
		sMesh, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		// planeMeshV1.get(), true, true, true );
		planeMeshV1.get(), false, false, true, false );

	//  item, node
	item = app->mSceneMgr->createItem( planeMesh, SCENE_DYNAMIC );
	item->setDatablock( "WaterReflect" );
	item->setCastShadows(false);
	app->SetTexWrap(item, true);

	nd = app->mSceneMgr->getRootSceneNode( SCENE_DYNAMIC )->createChildSceneNode( SCENE_DYNAMIC );
	float h = -17.f;
	if (!app->scn->sc->fluids.empty())
		h = app->scn->sc->fluids[0].pos.y + 0.2f;  // 1st fluid
	nd->setPosition( 0, h, 0 );

	nd->setOrientation( Quaternion( Radian( -Math::HALF_PI ), Vector3::UNIT_X ) );  // _
	nd->attachObject( item );


	//  actor, tracked
	actor = mPlanarRefl->addActor( PlanarReflectionActor(
		nd->getPosition(), mirrorSize, nd->getOrientation() ));

	PlanarReflections::TrackedRenderable tracked(
		item->getSubItem(0), item, Vector3::UNIT_Z, Vector3(0, 0, 0) );  // |
	mPlanarRefl->addRenderable( tracked );
	
	// mPlanarRefl->removeRenderable( tracked );  // ?
	// mPlanarRefl->removeActor( actor );
}


void FluidsReflect::DestroyFluids()
{
	if (nd)	{	app->mSceneMgr->destroySceneNode(nd);  nd = 0;  }
	if (item){	app->mSceneMgr->destroyItem(item);  item = 0;  }
	if (!sMesh.empty())
	{	MeshManager::getSingleton().remove(sMesh);
	v1::MeshManager::getSingleton().remove(sMesh);  sMesh = "";  }
}
