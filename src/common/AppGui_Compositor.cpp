#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "settings.h"
#include "settings_com.h"
#include "AppGui.h"
#include "Cam.h"
#include "RenderConst.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>
#include <OgreTextureGpuManager.h>

#include <OgreHlmsManager.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <OgreDepthBuffer.h>
#include <Compositor/Pass/OgreCompositorPassDef.h>
#include <OgrePixelFormatGpu.h>
#include <OgreRenderPassDescriptor.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
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
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
using namespace Ogre;


//  util
String AppGui::getWsInfo()
{
	auto* mgr = mRoot->getCompositorManager2();
	return " workspaces: "+ toStr(vWorkspaces.size())+" ("+toStr(mgr->getNumWorkspaces())+") nodes: "+ toStr(vNodes.size());
}
String AppGui::getWorkspace(bool worksp, int plr)
{
	bool refr = pSet->g.water_refract;
	String old = refr ? "" : "Old";
	String ssao = !refr && pSet->ssao ? "_SSAO" : "";  // todo: ssao and refract..
	
	// return "RefractionsWorkspaceMsaa";
	return (worksp ? "SR3_Workspace" : "SR3_Render")
	 	+ old + toStr(plr) + ssao;
}

const String sNewNode = "SR3_Render_New";
const String sNewWork = "SR3_Render_New_WS";


//  🪄 Create Compositor  main render setup
//-----------------------------------------------------------------------------------------
void AppGui::CreateCompositor()
{
	LogO("C+## Create Compositor "+getWsInfo());
	auto* mgr = mRoot->getCompositorManager2();
	CompositorNodeDef* nd =0;
	CompositorTargetDef* td =0;
	CompositorPassSceneDef *ps =0;

	//  node
	// if (mgr->hasNodeDefinition(sNewNode))
	// 	mgr->removeNodeDefinition(sNewNode);
	{
		nd = mgr->addNodeDefinition(sNewNode);
		vNodes.push_back(nd);
		
		nd->addTextureSourceName("rt_wnd", 0, TextureDefinitionBase::TEXTURE_INPUT);

		/*nd->setNumLocalTextureDefinitions(2);  //*
		{
			auto* td = nd->addTextureDefinition( "rtt1" );
			// td->widthFactor = 0.5f;  td->heightFactor = 0.5f;
			td->format = PFG_RGBA8_UNORM_SRGB;
			td->fsaa = "";  // auto

			td = nd->addTextureDefinition( "depth" );
			td->depthBufferFormat = PFG_D32_FLOAT;

			auto* rtv = nd->addRenderTextureView( "rtt1" );
			rtv->depthAttachment.textureName = "depth";
		}/**/

		// nd->addRenderTextureView()

		// Input channels
		/*nd->addTextureSourceName( "rt_input", 0, TextureDefinitionBase::TEXTURE_INPUT );
		nd->addTextureSourceName( "rt_output", 1, TextureDefinitionBase::TEXTURE_INPUT );
		*/
		nd->mCustomIdentifier = "new-1";
		//if( pass->getParentNode()->getDefinition()->mCustomIdentifier == "CustomString" )

		nd->setNumTargetPass(3);  //*  targets

		// td = nd->addTargetPass( "rtt1" );
		td = nd->addTargetPass( "rt_wnd" );

			ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
			ps->setAllLoadActions( LoadAction::Clear );
			ps->mProfilingId = "Main Render";
			// ps->mIdentifier = 20001;
			ps->mLastRQ = 199;  //RQG_Fluid-1
			
			switch (pSet->g.shadow_type)  // shadows
			{
			case Sh_None:  break;  // none
			case Sh_Depth: ps->mShadowNode = csShadow;  break;
			case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
			}

		//  ⏲️ Hud  --------
			ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
			// ps->setAllLoadActions( LoadAction::Clear );
			ps->setAllStoreActions( StoreAction::Store );
			ps->mProfilingId = "Main HUD";
			ps->mIdentifier = 10007;

			ps->mFirstRQ = 220;
			ps->mLastRQ = 230;
			ps->setVisibilityMask(0x00002080);  // RV_Hud, RV_Particles
			ps->mIncludeOverlays = false;

			// executed in all eyes, not views
			// execution_mask			0xff
			// executed on first view/eye
			// execution_mask			0x01
			// affected by modifier, apply to the whole screen
			// viewport_modifier_mask	0x00
	
		//  🎛️ Gui, add MyGUI pass  --------
			CompositorPassDef *pass;
			pass = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
			pass->mProfilingId = "Gui";  pass->mIdentifier = 99900;
			// if (!vr_mode)  // single gui, fullscreen
			{
				pass->mExecutionMask = 0x02;
				pass->mViewportModifierMask = 0x00;
			}
		/**/
		// CompositorPassQuadDef *passQuad;
		// passQuad = static_cast<CompositorPassQuadDef *>( td->addPass( PASS_QUAD ) );
		// passQuad->setAllLoadActions( LoadAction::DontCare );
		// passQuad->mMaterialName = "Postprocess/Glass";
		// passQuad->addQuadTextureSource( 0, "rt_input" );

		// Output channels
		// nd->setNumOutputChannels( 1 );
		// nd->mapOutputChannel( 0, "rt_output" );
		
		// nd->setNumOutputChannels( 2 );
		// nd->mapOutputChannel( 0, "rtt1" );
		// nd->mapOutputChannel( 1, "depth" );
	}

	//  workspace
	// if (mgr->hasWorkspaceDefinition(sNewWork))
	// 	mgr->removeWorkspaceDefinition(sNewWork);
	{
		CompositorWorkspaceDef *workDef = mgr->addWorkspaceDefinition( sNewWork );
		workDef->connectExternal( 0, nd->getName(), 0 );
		vWorkDefs.push_back(workDef);
	}
}


//  common  todo: drop ..
//-----------------------------------------------------------------------------------------
void AppGui::AddGuiShadows(bool vr_mode, int plr, bool gui)
{
	LogO("C*## AddGuiShadows "+getWsInfo());

	if (pSet->ssao)
		return;
	bool old = !pSet->g.water_refract;
	auto* mgr = mRoot->getCompositorManager2();
	

	CompositorNodeDef* node = mgr->getNodeDefinitionNonConst(getWorkspace(0, plr));
	// LogO("--++ WS Target Gui passes "+toStr(node->getNumTargetPasses()));

	CompositorTargetDef* target = node->getTargetPass(old ? 0 : 3);  // par!-
	auto passes = target->getCompositorPasses();
	// for (auto& p : passes)
		// LogO("--++ WS  pass:  type "+toStr(p->getType())+"  id "+toStr(p->mIdentifier)+"  "+p->mProfilingId);
	CompositorPassDef* pass = passes.back();

	//  🎛️ Gui, add MyGUI pass  ----
	if (gui)
	{
		if (pass->getType() != PASS_CUSTOM)
		{
			pass = target->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
			pass->mProfilingId = "Gui";  pass->mIdentifier = 99900;
		}
		if (!vr_mode)  // single gui, fullscreen
		{
			pass->mExecutionMask = 0x02;
			pass->mViewportModifierMask = 0x00;
	}	}

	//  🌒 shadows  ----
	const int np = old ? 1 : 0;
	target = node->getTargetPass(0);  // 0  // par!-
	passes = target->getCompositorPasses();
	// LogO("--++ WS Target Shdw passes "+toStr(node->getNumTargetPasses()));
	// for (auto& p : passes)
	// 	LogO("--++ WS  pass:  type "+toStr(p->getType())+"  id "+toStr(p->mIdentifier)+"  "+p->mProfilingId);

	assert( dynamic_cast<CompositorPassSceneDef *>(passes[np]) );
	CompositorPassSceneDef* ps = static_cast<CompositorPassSceneDef *>(passes[np]);
	if (!ps)
	{	LogO("--++ WS Error! Can't set shadow");  return;  }
	
	switch (pSet->g.shadow_type)
	{
	case Sh_None:  break;  // none
	case Sh_Depth: ps->mShadowNode = csShadow;  break;
	case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
	}
}


//  💥 destroy Compositor
//-----------------------------------------------------------------------------------------
void AppGui::DestroyCompositor()
{
	auto* mgr = mRoot->getCompositorManager2();
	LogO("D=## Destroy === "+getWsInfo());

	for (auto* ws : vWorkspaces)
	{
		mgr->removeWorkspace( ws );
	}
	vWorkspaces.clear();
			
	if (mgr->hasShadowNodeDefinition(csShadow))
		mgr->removeShadowNodeDefinition(csShadow);

	for (auto* wd : vWorkDefs)
	{
		mgr->removeWorkspaceDefinition( wd->getName() );
	}
	vWorkDefs.clear();

	for (auto* nd : vNodes)
	{
		mgr->removeNodeDefinition( nd->getName() );
	}
	vNodes.clear();
	// mgr->removeAllWorkspaces();  // not necessary..? drops all from .compositor
	// mgr->removeAllWorkspaceDefinitions();  // todo: cube, planar, all from code?
	// mgr->removeAllNodeDefinitions();
	// mgr->removeAllShadowNodeDefinitions();
	// mgr->removeAllCameras();  //-
}


//-----------------------------------------------------------------------------------------
//  🪄 Setup Compositor
//-----------------------------------------------------------------------------------------
void AppGui::SetupCompositor()
{
	LogO("C^## Setup Compositor "+getWsInfo());
	// LogO("C### setup Compositor");
	auto* rndSys = mRoot->getRenderSystem();
	auto* texMgr = rndSys->getTextureGpuManager();
	auto* mgr = mRoot->getCompositorManager2();
	
	DestroyCompositor();  //

	//  🌒 shadows
	bool esm = pSet->g.shadow_type == 2;
	static bool first =	true;
	if (first && pSet->g.shadow_type > 0)
	{
		if (esm)  setupESM();
		createPcfShadowNode();
		if (esm)  createEsmShadowNodes();  // fixme..
		// first = false;
	}


	//  🔮 create Reflections  ----
	CreateCubeReflect();  //


	//  🪄 Create Compositor  ----
	CreateCompositor();


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


	// mGraphicsSystem->restartCompositor();
	// createShadowMapDebugOverlays();

	// mEnableForwardPlus;
	// mLodCameraName ?

#ifndef SR_EDITOR
	//**  game,  Hud has own render
	// ps->mVisibilityMask = RV_MaskGameAll + RV_Hud3D[0]+ RV_Hud3D[1];
	// ps->mLastRQ = RQ_OVERLAY; //RQG_CarGhost;  // no, hides pacenotes-
#endif


	//  🖥️ Viewports
	//-----------------------------------------------------------------------------------------
	DestroyCameras();  // 💥🎥

	for (int i = 0; i < MAX_Players; ++i)
		mDims[i].Default();

	/*if (vr_mode)
	{
		//  👀 VR mode  ---- ----  wip meh todo use OpenVR
		//.................................................................................
		const Real eyeSide = 0.5f, eyeFocus = 0.45f, eyeZ = -10.f;  // dist

		AddGuiShadows(vr_mode, 0, true);

		auto camL = CreateCamera( "EyeL", 0,
			Vector3(-eyeSide, 0.f,0.f), Vector3(eyeFocus * -eyeSide, 0.f, eyeZ) );
		auto camR = CreateCamera( "EyeR", camL->nd,
			Vector3( eyeSide, 0.f,0.f), Vector3(eyeFocus *  eyeSide, 0.f, eyeZ) );
		mCamera = camL->cam;

		//  add Workspace
		CompositorWorkspace* ws1,*ws2;
		LogO("--++ WS add:  VR Eye L, all: "+toStr(mgr->getNumWorkspaces()));
		auto str = getWorkspace(1, 0);
		const IdString wsName( str );
		ws1 = mgr->addWorkspace( mSceneMgr, ext,
				camL->cam, wsName,
				true, -1, 0, 0,
				Vector4( 0.0f, 0.0f, 0.5f, 1.0f ),
				0x01, 0x01 );

		LogO("--++ WS add:  VR Eye R, all: "+toStr(mgr->getNumWorkspaces()));
		ws2 = mgr->addWorkspace( mSceneMgr, ext, 
				camR->cam, wsName,
				true, -1, 0, 0,
				Vector4( 0.5f, 0.0f, 0.5f, 1.0f ),
				0x02, 0x02 );

		// mWorkspaces.push_back(ws1);
		// mWorkspaces.push_back(ws2);

		// LogO("C### Created VR workspaces: "+toStr(mWorkspaces.size()));
		// return ws1;
	} else*/

#ifndef SR_EDITOR
	//  👥 Split Screen   ---- ---- ---- ----
	//.....................................................................................
	if (views > 1)
	{
		for (int i = 0; i < views; ++i)
		{
			bool f1 = i > 0;

			AddGuiShadows(vr_mode, i, i==views-1);  // last only gui

			auto c = CreateCamera( "Player" + toStr(i), 0, Vector3::ZERO, Vector3::NEGATIVE_UNIT_Z );
			if (i==0)
				mCamera = c->cam;
			
			auto& d = mDims[i];
			d.SetDim(views, !pSet->split_vertically, i);

			//  add Workspace
			LogO("--++ WS add:  Split Screen "+toStr(i)+", all: "+toStr(mgr->getNumWorkspaces()));
			auto str = getWorkspace(1, i);  // todo ..!
			const IdString wsName( str );
			CompositorWorkspace* w =
				mgr->addWorkspace( mSceneMgr, ext,
					c->cam, wsName,
					true, -1, 0, 0,
					Vector4(d.left0, d.top0, d.width0, d.height0),
					f1 ? 0x02 : 0x01, f1 ? 0x02 : 0x01 );

			vWorkspaces.push_back(w);
			// mWorkspaces.push_back(w);
		}

		// LogO("C### Created Split workspaces: "+toStr(mWorkspaces.size()));
		// return mWorkspaces[0];
	}
	else  // 🖥️ Single View
#endif
	//.....................................................................................
	{
		auto c = CreateCamera( "Player", 0, Vector3(0,150,0), Vector3(0,0,0) );
		mCamera = c->cam;

		AddGuiShadows(vr_mode, 0, true);  //

		//  add Workspace
		LogO("--++ WS add:  Main, all: "+toStr(mgr->getNumWorkspaces()));
		// auto str = getWorkspace(1, 0);  // st
		auto str = sNewWork;  // new
		const IdString wsName( str );
		auto ws = mgr->addWorkspace( mSceneMgr, ext, c->cam, wsName, true );  // in .compositor
		// ws->addListener(listener);
		vWorkspaces.push_back(ws);
		// LogO("C### Created Single workspaces: "+toStr(mWorkspaces.size()));

		// createShadowMapDebugOverlays();  //-
		// return ws;
	}
}


//  💥🎥 Destroy Camera
void AppGui::DestroyCameras()
{
	LogO("D### destroy Cameras-");
	/*for (auto cam : mCams)  // todo ?
	{
		if (cam.nd)  mSceneMgr->destroySceneNode(cam.nd);  cam.nd = 0;
		if (cam.cam)  mSceneMgr->destroyCamera(cam.cam);
	}*/
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

//  🆕🎥 Create Camera
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
//  #define USEnodes  //- crashes in ogre upd frustum later
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
	cam->setFarClipDistance( pSet->g.view_distance );
	cam->setAutoAspectRatio( true );
	cam->setLodBias( pSet->g.lod_bias );

#ifdef USEnodes
	cam->detachFromParent();
	nd->attachObject( cam );
#endif

	Cam c;
	c.cam = cam;
#ifdef USEnodes
	c.nd = nd;
#endif
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
