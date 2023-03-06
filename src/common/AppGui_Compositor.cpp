#include "pch.h"
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
#include <OgreTextureGpuManager.h>

#include <OgreHlmsPbs.h>
#include <OgreHlmsManager.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>

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


//  🪞 planar reflect  water
class PlanarReflectionsWorkspaceListener : public CompositorWorkspaceListener
{
	PlanarReflections *mPlanarReflections;

public:
	PlanarReflectionsWorkspaceListener( PlanarReflections *planarReflections ) :
		mPlanarReflections( planarReflections )
	{
	}
	virtual ~PlanarReflectionsWorkspaceListener() {}

	virtual void workspacePreUpdate( CompositorWorkspace *workspace )
	{
		mPlanarReflections->beginFrame();
	}

	virtual void passEarlyPreExecute( CompositorPass *pass )
	{
		// Ignore non-scene passes
		if( pass->getType() != PASS_SCENE )
			return;
		assert( dynamic_cast<const CompositorPassSceneDef *>( pass->getDefinition() ) );
		const CompositorPassSceneDef *passDef =
			static_cast<const CompositorPassSceneDef *>( pass->getDefinition() );

		// Ignore scene passes that belong to a shadow node.
		if( passDef->mShadowNodeRecalculation == SHADOW_NODE_CASTER_PASS )
			return;

		// Ignore scene passes we haven't specifically tagged to receive reflections
		if( passDef->mIdentifier != 25001 )
			return;

		CompositorPassScene *passScene = static_cast<CompositorPassScene *>( pass );
		Camera *camera = passScene->getCamera();

		// Note: The Aspect Ratio must match that of the camera we're reflecting.
		mPlanarReflections->update( camera, camera->getAspectRatio() );
	}
};


//-----------------------------------------------------------------------------------
void AppGui::createReflectiveSurfaces()
{
	Root *root = mGraphicsSystem->getRoot();

	bool useComputeMipmaps = false;
#if !OGRE_NO_JSON
	useComputeMipmaps =
		root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM );
#endif

	// Setup PlanarReflections
	mPlanarRefl =
		new PlanarReflections( mSceneMgr, root->getCompositorManager2(), 1.0, 0 );
	mWorkspaceListener = new PlanarReflectionsWorkspaceListener( mPlanarRefl );
	{
		CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
		workspace->addListener( mWorkspaceListener );
	}

	// The perfect mirror doesn't need mipmaps.
	mPlanarRefl->setMaxActiveActors( 1u, "PlanarReflectionsReflectiveWorkspace", true, 512,
											512, false, PFG_RGBA8_UNORM_SRGB,
											useComputeMipmaps );
	// The rest of the reflections do.
	mPlanarRefl->setMaxActiveActors( 2u, "PlanarReflectionsReflectiveWorkspace", true, 512,
											512, true, PFG_RGBA8_UNORM_SRGB,
											useComputeMipmaps );
	const Vector2 mirrorSize( 10.0f, 10.0f );

	// Create the plane mesh
	// Note that we create the plane to look towards +Z; so that sceneNode->getOrientation
	// matches the orientation for the PlanarReflectionActor
	v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
		"Plane Mirror Unlit", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Plane( Vector3::UNIT_Z, 0.0f ), mirrorSize.x, mirrorSize.y, 1, 1, true, 1, 1.0f,
		1.0f, Vector3::UNIT_Y, v1::HardwareBuffer::HBU_STATIC,
		v1::HardwareBuffer::HBU_STATIC );
	MeshPtr planeMesh = MeshManager::getSingleton().createByImportingV1(
		"Plane Mirror Unlit", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		planeMeshV1.get(), true, true, true );

	//---------------------------------------------------------------------
	// Setup mirror for Unlit.
	//---------------------------------------------------------------------
	Item *item = mSceneMgr->createItem( planeMesh, SCENE_DYNAMIC );
	SceneNode *nd = mSceneMgr->getRootSceneNode( SCENE_DYNAMIC )
										->createChildSceneNode( SCENE_DYNAMIC );
	nd->setPosition( 5, 5, 0 );
	nd->setOrientation(
		Quaternion( Radian( -Math::HALF_PI ), Vector3::UNIT_Y ) );
	nd->attachObject( item );
	// item->setCastShadows( false );
	item->setVisibilityFlags( 1u );  // Do not render this plane during the reflection phase.

	PlanarReflectionActor *actor = mPlanarRefl->addActor( PlanarReflectionActor(
		nd->getPosition(), mirrorSize, nd->getOrientation() ) );

	Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( HLMS_UNLIT );

	HlmsMacroblock macroblock;
	HlmsBlendblock blendblock;
	String datablockName( "Mirror_Unlit" );
	HlmsUnlitDatablock *mirror =
		static_cast<HlmsUnlitDatablock *>( hlmsUnlit->createDatablock(
			datablockName, datablockName, macroblock, blendblock, HlmsParamVec() ) );
	mPlanarRefl->reserve( 0, actor );
	// Make sure it's always activated (i.e. always win against other actors)
	// unless it's not visible by the camera.
	actor->mActivationPriority = 0;
	mirror->setTexture( 0, mPlanarRefl->getTexture( 0 ) );
	mirror->setEnablePlanarReflection( 0, true );
	item->setDatablock( mirror );

	//---------------------------------------------------------------------
	// Setup mirror for PBS.
	//---------------------------------------------------------------------
	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	assert( dynamic_cast<HlmsPbs *>( hlms ) );
	HlmsPbs *pbs = static_cast<HlmsPbs *>( hlms );
	//__
	pbs->setPlanarReflections( mPlanarRefl );

	item = mSceneMgr->createItem( planeMesh, SCENE_DYNAMIC );
	item->setDatablock( "GlassRoughness" );
	nd = mSceneMgr->getRootSceneNode( SCENE_DYNAMIC )
					->createChildSceneNode( SCENE_DYNAMIC );
	nd->setPosition( -5, 2.5f, 0 );
	nd->setOrientation(
		Quaternion( Radian( Math::HALF_PI ), Vector3::UNIT_Y ) );
	nd->setScale( Vector3( 0.75f, 0.5f, 1.0f ) );
	nd->attachObject( item );

	actor = mPlanarRefl->addActor( PlanarReflectionActor(
		nd->getPosition(), mirrorSize * Vector2( 0.75f, 0.5f ),
		nd->getOrientation() ) );

	PlanarReflections::TrackedRenderable trackedRenderable(
		item->getSubItem( 0 ), item, Vector3::UNIT_Z, Vector3( 0, 0, 0 ) );
	mPlanarRefl->addRenderable( trackedRenderable );
}

	
//-----------------------------------------------------------------------------------------
//  🪄 Setup Compositor
//-----------------------------------------------------------------------------------------
CompositorWorkspace* AppGui::SetupCompositor()
{
	// LogO("C### setup Compositor");
	auto* rndSys = mRoot->getRenderSystem();
	auto* texMgr = rndSys->getTextureGpuManager();
	auto* mgr = mRoot->getCompositorManager2();
	
	//  💥 destroy old
	LogO("D### destroy Compositor, workspaces: "+ toStr(mWorkspaces.size()));
	for (auto ws : mWorkspaces)
	{
		// LogO("D### setup Compositor rem workspace");
		mgr->removeWorkspace( ws );
	}
	mWorkspaces.clear();


	//  🌒 shadows
	bool esm = pSet->shadow_type == 2;
	static bool first =	true;
	if (first && pSet->shadow_type > 0)
	{
		if (esm)  setupESM();
		createPcfShadowNode();
		if (esm)  createEsmShadowNodes();  // fixme..
		first = false;
	}


	//  🔮 create Reflections  ----
	CreateCubeReflect();  //


	//  Render window external channels  ----
	CompositorChannelVec ext( mCubeReflTex ? 2 : 1 );
	ext[0] = mWindow->getTexture();
	if (mCubeReflTex)
		ext[1] = mCubeReflTex;  // 🔮

	
#ifndef SR_EDITOR  // game
	const int views = pSet->game.local_players;
	bool vr_mode = pSet->game.vr_mode;
#else
	const int views = 1;  // ed
	bool vr_mode = pSet->vr_mode;
#endif


	//  🎛️ Gui, add MyGUI pass  ----
	CompositorNodeDef* node = mgr->getNodeDefinitionNonConst("SR3_Render");
	CompositorTargetDef* target = node->getTargetPass(0);
	auto passes = target->getCompositorPasses();
	CompositorPassDef* pass = passes.back();

	if (pass->getType() != PASS_CUSTOM)
	{
		pass = target->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
	}
	if (!vr_mode)  // single gui, fullscreen
	{
		pass->mExecutionMask = 0x02;
		pass->mViewportModifierMask = 0x00;
	}


	//  🌒 shadows
	const int is = 1;  // 1
	assert( dynamic_cast<CompositorPassSceneDef *>(passes[is]) );
	CompositorPassSceneDef* ps = static_cast<CompositorPassSceneDef *>(passes[is]);
	
	switch (pSet->shadow_type)
	{
	case 0:  break;  // none
	case 1:  ps->mShadowNode = "ShadowMapFromCodeShadowNode";  break;
	case 2:  ps->mShadowNode = chooseEsmShadowNode();  break;
	}

	// mGraphicsSystem->restartCompositor();
	// createShadowMapDebugOverlays();

	// mEnableForwardPlus;
	// mLodCameraName ?

#ifndef SR_EDITOR
	//  game, Hud has own render
	ps->mVisibilityMask = 0x000FFFFF - RV_Hud;
	// ps->mLastRQ = RQ_OVERLAY; //RQG_CarGhost;  // no, hides pacenotes-
#endif


	//  🖥️ Viewports
	//-----------------------------------------------------------------------------------------
	DestroyCameras();  // 💥🎥

	for (int i = 0; i < 4; ++i)
		mDims[i].Default();

	const IdString wsName( "SR3_Workspace" );

	if (vr_mode)
	{
		//  👀 VR mode  ---- ----
		//.................................................................................
		const Real eyeSide = 0.5f, eyeFocus = 0.45f, eyeZ = -10.f;  // dist

		auto camL = CreateCamera( "EyeL", 0,
			Vector3(-eyeSide, 0.f,0.f), Vector3(eyeFocus * -eyeSide, 0.f, eyeZ) );
		auto camR = CreateCamera( "EyeR", camL->nd,
			Vector3( eyeSide, 0.f,0.f), Vector3(eyeFocus *  eyeSide, 0.f, eyeZ) );
		mCamera = camL->cam;

		CompositorWorkspace* ws1,*ws2;
		ws1 = mgr->addWorkspace( mSceneMgr, ext,
				camL->cam, wsName,
				true, -1, 0, 0,
				Vector4( 0.0f, 0.0f, 0.5f, 1.0f ),
				0x01, 0x01 );

		ws2 = mgr->addWorkspace( mSceneMgr, ext, 
				camR->cam, wsName,
				true, -1, 0, 0,
				Vector4( 0.5f, 0.0f, 0.5f, 1.0f ),
				0x02, 0x02 );

		mWorkspaces.push_back(ws1);
		mWorkspaces.push_back(ws2);

		LogO("C### Created VR workspaces: "+toStr(mWorkspaces.size()));
		return ws1;
	}

#ifndef SR_EDITOR
	//  👥 splitscreen   ---- ---- ---- ----
	//.....................................................................................
	else if (views > 1)
	{
		for (int i = 0; i < views; ++i)
		{
			bool f1 = i > 0;
			auto c = CreateCamera( "Player" + toStr(i), 0, Vector3::ZERO, Vector3::NEGATIVE_UNIT_Z );
			if (i==0)
				mCamera = c->cam;
			
			auto& d = mDims[i];
			d.SetDim(views, !pSet->split_vertically, i);

			CompositorWorkspace* w =
				mgr->addWorkspace( mSceneMgr, ext,
					c->cam, wsName,
					true, -1, 0, 0,
					Vector4(d.left0, d.top0, d.width0, d.height0),
					f1 ? 0x02 : 0x01, f1 ? 0x02 : 0x01 );
			// CompositorTargetDef* target = w->getTargetPass(0);
			// auto passes = target->getCompositorPasses();
			// passes[1]->

			mWorkspaces.push_back(w);
		}

		LogO("C### Created Split workspaces: "+toStr(mWorkspaces.size()));
		return mWorkspaces[0];
	}
#endif
	else  // 🖥️ single view
	//.....................................................................................
	{
		auto c = CreateCamera( "Player", 0, Vector3(0,150,0), Vector3(0,0,0) );
		mCamera = c->cam;

		auto ws = mgr->addWorkspace( mSceneMgr, ext, c->cam, wsName, true );  // in .compositor
		mWorkspaces.push_back(ws);
		mGraphicsSystem->mWorkspace = ws;
		LogO("C### Created Single workspaces: "+toStr(mWorkspaces.size()));

		// createShadowMapDebugOverlays();
		return ws;
	}
}


//  💥🎥 destroy camera
void AppGui::DestroyCameras()
{
	LogO("D### destroy Cameras");
	// for (auto cam : mCameras)
		// mSceneMgr->destroyCamera(cam);
	// mSceneMgr->destroyAllCameras();
	mCams.clear();  // for game, not all
}

Cam* AppGui::findCam(String name)
{
	for (Cam& c : mCamsAll)
		if (c.name == name)
			return &c;
	return 0;
}

//  🆕🎥 create camera
//--------------------------------------------------------------------------------
Cam* AppGui::CreateCamera(String name,
	SceneNode* nd, Vector3 pos, Vector3 lookAt)
{
	Cam* fc = findCam(name);
	if (fc)
	{
		LogO("---: got cam: " + name);
		mCams.push_back(*fc);
		return fc;
	}
// #define USEnodes  //- crashes in ogre upd frustum later
#ifdef USEnodes
	if (!nd)
	{	nd = mSceneMgr->getRootSceneNode(SCENE_DYNAMIC)->createChildSceneNode(SCENE_DYNAMIC);
		nd->setName( name + "CamNd" );
		LogO("c--: new cam node: " + name + "CamNd" );
		nd->setPosition( 0, 50, 100 );  // whatever, upd later
	}
#endif
	Camera* cam = mSceneMgr->createCamera( name );
	LogO("c--: new cam: " + name);

	cam->setPosition( pos );
	cam->lookAt( lookAt );
	cam->setNearClipDistance( 0.1f );
	cam->setFarClipDistance( pSet->view_distance );
	cam->setAutoAspectRatio( true );
	cam->setLodBias( pSet->lod_bias );

#ifdef USEnodes
	cam->detachFromParent();
	nd->attachObject( cam );
#endif

	Cam c;
	c.cam = cam;  c.nd = nd;
	c.name = name;
	
	mCamsAll.push_back(c);
	mCams.push_back(c);
	return &mCams.back();
}


//---------------------------------------------------------------------------------
#if 0
	// Use one node to control both cameras
	mCamerasNode = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )
						->createChildSceneNode( Ogre::SCENE_DYNAMIC );
	mCamerasNode->setName( "Cameras Node" );

	mCamerasNode->setPosition( 0, 5, 15 );

	mEyeCameras[0] = mSceneManager->createCamera( "Left Eye" );
	mEyeCameras[1] = mSceneManager->createCamera( "Right Eye" );

	const Ogre::Real eyeDistance = 0.5f;
	const Ogre::Real eyeFocusDistance = 0.45f;

	for( int i = 0; i < 2; ++i )
	{
		const Ogre::Vector3 camPos( eyeDistance * ( Ogre::Real( i ) * 2 - 1 ), 0, 0 );
		mEyeCameras[i]->setPosition( camPos );

		Ogre::Vector3 lookAt( eyeFocusDistance * ( Ogre::Real( i ) * 2 - 1 ), -5, -15 );
		// Ogre::Vector3 lookAt( 0, 0, 0 );

		// Look back along -Z
		mEyeCameras[i]->lookAt( lookAt );
		mEyeCameras[i]->setNearClipDistance( 0.2f );
		mEyeCameras[i]->setFarClipDistance( 1000.0f );
		mEyeCameras[i]->setAutoAspectRatio( true );

		// By default cameras are attached to the Root Scene Node.
		mEyeCameras[i]->detachFromParent();
		mCamerasNode->attachObject( mEyeCameras[i] );
	}

	mCamera = mEyeCameras[0];
#endif


#if 0
	if( arg.keysym.sym == SDLK_F5 )
	{
		Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

		assert( dynamic_cast<Ogre::HlmsPbs *>( hlms ) );
		Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs *>( hlms );

		Ogre::HlmsPbs::ShadowFilter nextFilter = static_cast<Ogre::HlmsPbs::ShadowFilter>(
			( pbs->getShadowFilter() + 1u ) % Ogre::HlmsPbs::NumShadowFilter );

		pbs->setShadowSettings( nextFilter );

		if( nextFilter == Ogre::HlmsPbs::ExponentialShadowMaps )
			setupShadowNode( true );
		else
			setupShadowNode( false );
	}
#endif
