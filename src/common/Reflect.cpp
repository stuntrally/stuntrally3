#include "pch.h"
#include "Reflect.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "App.h"
#include "Cam.h"
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
	mPlanarReflections->beginFrame();
	// LogO("ws pre: "+toStr(workspace->getId()));
}

//-----------------------------------------------------------------------------------
void ReflectListener::passEarlyPreExecute( CompositorPass *pass )
{
	//  Ignore non-scene passes (clear etc)
	if (pass->getType() != PASS_SCENE)
		return;

	assert( dynamic_cast<const CompositorPassSceneDef *>( pass->getDefinition() ) );
	const CompositorPassSceneDef *passDef =
		static_cast<const CompositorPassSceneDef *>( pass->getDefinition() );

	// LogO("ws pass: "+passDef->mProfilingId);  //toStr(pass->getParentNode()->getId() ));

	//  Ignore scene passes that belong to a shadow node.
	if (passDef->mShadowNodeRecalculation == SHADOW_NODE_CASTER_PASS)
		return;

	//  Ignore scene passes we haven't specifically tagged to receive reflections
	if (passDef->mIdentifier != 25001)
		return;
	CompositorPassScene *passScene = static_cast<CompositorPassScene *>( pass );
	Camera *camera = passScene->getCamera();
	// camera->setLodBias(0.1f);  // how for refl?

	//  The Aspect Ratio must match that of the camera we're reflecting.
	mPlanarReflections->update( camera, camera->getAutoAspectRatio()
		? pass->getViewportAspectRatio( 0u ) : camera->getAspectRatio() );
}


//-----------------------------------------------------------------------------------
void FluidReflect::DestroyReflect()
{
	if (nd)	{	app->mSceneMgr->destroySceneNode(nd);  nd = 0;  }
	if (item){	app->mSceneMgr->destroyItem(item);  item = 0;  }
	if (!sMesh.empty())
	{	MeshManager::getSingleton().remove(sMesh);
	v1::MeshManager::getSingleton().remove(sMesh);  sMesh = "";  }

	CompositorWorkspace *workspace = app->mGraphicsSystem->getCompositorWorkspace();
	if (mWorkspaceListener)
		workspace->removeListener( mWorkspaceListener );
	delete mWorkspaceListener;  mWorkspaceListener = 0;

	delete mPlanarRefl;  mPlanarRefl = 0;
}

//-----------------------------------------------------------------------------------
void FluidReflect::CreateReflect()
{
	Root *root = app->mGraphicsSystem->getRoot();

	bool useComputeMipmaps = false;
#if !OGRE_NO_JSON
	useComputeMipmaps =
		root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM );
#endif

	//  Setup PlanarReflections
	mPlanarRefl = new PlanarReflections( app->mSceneMgr, root->getCompositorManager2(), 1.0, 0 );
	mWorkspaceListener = new ReflectListener( mPlanarRefl );
	{
		// app->mWorkspaces
		LogO("---- WS add Listener");
		CompositorWorkspace *workspace = app->mGraphicsSystem->getCompositorWorkspace();
		workspace->addListener( mWorkspaceListener );
	}

	uint32 size = app->pSet->GetTexSize(app->pSet->water_reflect);
	// mPlanarRefl->setMaxActiveActors( 1u, "PlanarReflections",
	// 	true, si, si, false, PFG_RGBA8_UNORM_SRGB, useComputeMipmaps );  // no mipmaps
	
	// The rest of the reflections do
	mPlanarRefl->setMaxActiveActors( 2u, "PlanarReflections",
		true, size, size, true, PFG_RGBA8_UNORM_SRGB, useComputeMipmaps );
	
	const Vector2 mirrorSize( 4000.0f, 4000.0f );
	//  Create the plane mesh
	//  Note that we create the plane to look towards +Z; so that sceneNode->getOrientation
	//  matches the orientation for the PlanarReflectionActor
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



	//  Setup mirror for PBS
	//---------------------------------------------------------------------
	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	assert( dynamic_cast<HlmsPbs2 *>( hlms ) );
	HlmsPbs2 *pbs = static_cast<HlmsPbs2 *>( hlms );
	pbs->setPlanarReflections( mPlanarRefl );

	item = app->mSceneMgr->createItem( planeMesh, SCENE_DYNAMIC );
	item->setDatablock( "WaterReflect" );
	item->setCastShadows(false);
	app->SetTexWrap(item, true);

	nd = app->mSceneMgr->getRootSceneNode( SCENE_DYNAMIC )->createChildSceneNode( SCENE_DYNAMIC );
	nd->setPosition( -0, -17.f, 0 );  // -13.5f
	// nd->setPosition( -5, 2.5f, 0 );  // z|
	// nd->setOrientation( Quaternion( Radian( Math::HALF_PI ), Vector3::UNIT_Y ) );  // z|
	nd->setOrientation( Quaternion( Radian( -Math::HALF_PI ), Vector3::UNIT_X ) );  // ?_
	// nd->setScale( Vector3( 1.f, 1.f, 1.f ) );
	// nd->setScale( Vector3( 0.75f, 0.5f, 1.0f ) );  // z| ?
	nd->attachObject( item );


	actor = mPlanarRefl->addActor( PlanarReflectionActor(
		nd->getPosition(), mirrorSize, // * Vector2( 0.75f, 0.5f ), // z| ?
		nd->getOrientation() ) );

	PlanarReflections::TrackedRenderable tracked(
		item->getSubItem( 0 ), item,
		Vector3::UNIT_Z, Vector3( 0, 0, 0 ) );  // z|
		// Vector3::UNIT_Y, Vector3( 0, 0, 0 ) );  // ?-
	mPlanarRefl->addRenderable( tracked );
	
	// mPlanarRefl->removeRenderable( tracked );  // ?
	// mPlanarRefl->removeActor( actor );
}

