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
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopy.h>
#include <Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>

#include <OgreItem.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
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

//  node  New Refract todo
if (pSet->g.water_refract)
{
/* 1 -------------------- */
	{	nd = mgr->addNodeDefinition("SR3_Refract_first");
		vNodes.push_back(nd);
		
		nd->setNumLocalTextureDefinitions(2);  //*  textures
		{
			auto* td = nd->addTextureDefinition( "depthBuffer" );
			td->depthBufferFormat = PFG_D32_FLOAT;
			td->fsaa = "";  // auto

			td = nd->addTextureDefinition( "rtt_first" );
			// td->widthFactor = 0.5f;  td->heightFactor = 0.5f;
			td->format = PFG_UNKNOWN;  //PFG_RGBA8_UNORM_SRGB;  // target_format
			td->fsaa = "";  // auto

			auto* rtv = nd->addRenderTextureView( "rtt_first" );  // rtv
			rtv->depthAttachment.textureName = "depthBuffer";
		}
		nd->mCustomIdentifier = "1-first";
		
		nd->setNumTargetPass(1);  //*  targets
		td = nd->addTargetPass( "rtt_first" );
		td->setNumPasses(1);
		{
			ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
			ps->setAllLoadActions( LoadAction::Clear );
            ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
			ps->mStoreActionDepth = StoreAction::Store;
            ps->mStoreActionStencil = StoreAction::DontCare;

			ps->mProfilingId = "Render First";  //  "Opaque + Regular Transparents"
			// ps->mIdentifier = 10001;
			ps->mLastRQ = 199;  //RQG_Fluid-1
			ps->mIncludeOverlays = false;
			
			switch (pSet->g.shadow_type)  // shadows
			{
			case Sh_None:  break;  // none
			case Sh_Depth: ps->mShadowNode = csShadow;  break;
			case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
			}
		}
		nd->setNumOutputChannels( 2 );  //  out
		nd->mapOutputChannel( 0, "rtt_first" );
		nd->mapOutputChannel( 1, "depthBuffer" );
	}
/* 2 ---------- */
	{	nd = mgr->addNodeDefinition("SR3_DepthResolve");
		vNodes.push_back(nd);
		
		nd->addTextureSourceName("gBufferDepthBuffer", 0, TextureDefinitionBase::TEXTURE_INPUT);

		nd->setNumLocalTextureDefinitions(1);  //*  textures
		{
			auto* td = nd->addTextureDefinition( "resolvedDepthBuffer" );
			td->depthBufferFormat = PFG_D32_FLOAT;  // R32?
			// depth_pool 0  ?
			// td->fsaa = "";  // auto
		}
		nd->mCustomIdentifier = "2-resolve";

		//  We need to "downsample/resolve" mrtDepthBuffer because the impact
		//  performance on Refractions is gigantic.
		nd->setNumTargetPass(1);  //*  targets
		td = nd->addTargetPass( "resolvedDepthBuffer" );
		td->setNumPasses(1);
		{
			auto* pq = static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
			pq->setAllLoadActions( LoadAction::DontCare );
			pq->mMaterialName = "Ogre/Resolve/1xFP32_Subsample0";
			pq->addQuadTextureSource( 0, "gBufferDepthBuffer" );  // input
			pq->mProfilingId = "Resolve Depth Buffer";
		}
		nd->setNumOutputChannels( 1 );  //  out
		nd->mapOutputChannel( 0, "resolvedDepthBuffer" );
	}
/* 3 ------------------------------  */
	{	nd = mgr->addNodeDefinition("SR3_Refract_Final");
		vNodes.push_back(nd);
		
		nd->addTextureSourceName("rtt_first", 0, TextureDefinitionBase::TEXTURE_INPUT);
		nd->addTextureSourceName("depthBuffer", 1, TextureDefinitionBase::TEXTURE_INPUT);
		nd->addTextureSourceName("depthBufferNoMsaa", 2, TextureDefinitionBase::TEXTURE_INPUT);
		nd->addTextureSourceName("rt_renderwindow", 3, TextureDefinitionBase::TEXTURE_INPUT);

		nd->setNumLocalTextureDefinitions(1);  //*  textures
		{
			auto* td = nd->addTextureDefinition( "rtt_final" );
			td->format = PFG_UNKNOWN;  // target_format
			td->fsaa = "";  // auto

			auto* rtv = nd->addRenderTextureView( "rtt_final" );  // rtv
			rtv->depthAttachment.textureName = "depthBuffer";
		}

		nd->mCustomIdentifier = "3-final";
		//if( pass->getParentNode()->getDefinition()->mCustomIdentifier == "CustomString" )

		nd->setNumTargetPass(2);  //*  targets

		//  Refracted  Fluids  ----
		td = nd->addTargetPass( "rtt_final" );
		td->setNumPasses(2);
		{
			auto* cp = static_cast<CompositorPassDepthCopyDef*>(td->addPass(Ogre::PASS_DEPTHCOPY));
			cp->setDepthTextureCopy("rtt_first", "rtt_final");

			ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
			ps->setAllLoadActions( LoadAction::Load );
            ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
			ps->mStoreActionDepth = StoreAction::DontCare;
            ps->mStoreActionStencil = StoreAction::DontCare;

			ps->mProfilingId = "Refractive Objects";
			ps->mIdentifier = 10002;
			ps->mFirstRQ = RQG_Fluid;  // 210
			ps->mLastRQ = RQG_Fluid+2;
			ps->setVisibilityMask(RV_Fluid);  // 0x00000002
			ps->mIncludeOverlays = false;
			
			switch (pSet->g.shadow_type)  // shadows
			{
			case Sh_None:  break;  // none
			case Sh_Depth: ps->mShadowNode = csShadow;  break;
			case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
			}
			ps->mShadowNodeRecalculation = SHADOW_NODE_REUSE;
			ps->setUseRefractions("depthBufferNoMsaa", "rtt_first");
		}

		//  rnd window  ----
		td = nd->addTargetPass( "rt_renderwindow" );
		td->setNumPasses(3);
		{
			auto* pq = static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
			pq->setAllLoadActions( LoadAction::DontCare );
			pq->mMaterialName = "Ogre/Copy/4xFP32";
			pq->addQuadTextureSource( 0, "rtt_final" );  // input
			pq->mProfilingId = "Copy to Window";

			//  ⏲️ Hud  --------
			{	ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
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
			}
			//  🎛️ Gui, add MyGUI pass  --------
			{	CompositorPassDef *pass;
				pass = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
				pass->mProfilingId = "Gui";  pass->mIdentifier = 99900;
				// if (!vr_mode)  // single gui, fullscreen
				{
					pass->mExecutionMask = 0x02;
					pass->mViewportModifierMask = 0x00;
				}
			}
		}
		//  workspace
		{
			// connect	RefractionsRenderingNode 1 RefractionsMsaaDepthResolve 0
			// connect	RefractionsRenderingNode 0 1 RefractionsRefractionsNode 0 1
			// connect	RefractionsMsaaDepthResolve 0 RefractionsRefractionsNode 2
			// connect_output RefractionsRefractionsNode 3
			CompositorWorkspaceDef *workDef = mgr->addWorkspaceDefinition( sNewWork );
			workDef->connect("SR3_Refract_first", 1, "SR3_DepthResolve", 0 );  // depthBuffer
			workDef->connect("SR3_Refract_first", 0, "SR3_Refract_Final", 0 );  // first
			workDef->connect("SR3_Refract_first", 1, "SR3_Refract_Final", 1 );  // depth
			workDef->connect("SR3_DepthResolve", 0, "SR3_Refract_Final", 2 );  // resolvedDepth
			workDef->connectExternal( 0, "SR3_Refract_Final", 3 );
			vWorkDefs.push_back(workDef);
		}
	}
	//-----------------------------------------------------------------------------------------
}else  //  node  Old  no refract, no depth
{
	{	nd = mgr->addNodeDefinition(sNewNode);
		vNodes.push_back(nd);
		
		//  render window input
		nd->addTextureSourceName("rt_wnd", 0, TextureDefinitionBase::TEXTURE_INPUT);

		nd->mCustomIdentifier = "old-1";
		//if( pass->getParentNode()->getDefinition()->mCustomIdentifier == "CustomString" )

		nd->setNumTargetPass(3);  //*  targets

		td = nd->addTargetPass( "rt_wnd" );
		{
			ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
			ps->setAllLoadActions( LoadAction::Clear );
			ps->setAllStoreActions( StoreAction::Store );

			ps->mProfilingId = "Main Render Old";
			ps->mIdentifier = 21002;
			// ps->mLastRQ = 199;  //RQG_Fluid-1  // all

			ps->mExecutionMask = 0xff;  // executed in all views/eyes
			ps->mViewportModifierMask = 0xff;  // affected by modifier, so renders to a portion of screen
			
			switch (pSet->g.shadow_type)  // shadows
			{
			case Sh_None:  break;  // none
			case Sh_Depth: ps->mShadowNode = csShadow;  break;
			case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
			}
		}
		//  ⏲️ Hud  --------
		{	ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
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
		}
		//  🎛️ Gui, add MyGUI pass  --------
		{	CompositorPassDef *pass;
			pass = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
			pass->mProfilingId = "Gui";  pass->mIdentifier = 99900;
			// if (!vr_mode)  // single gui, fullscreen
			{
				pass->mExecutionMask = 0x02;
				pass->mViewportModifierMask = 0x00;
			}
		}
		//  workspace
		{
			CompositorWorkspaceDef *workDef = mgr->addWorkspaceDefinition( sNewWork );
			workDef->connectExternal( 0, nd->getName(), 0 );
			vWorkDefs.push_back(workDef);
		}
	}
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
	
#ifndef SR_EDITOR  // game
	const int views = pSet->game.local_players;
	bool vr_mode = pSet->game.vr_mode;
#else
	const int views = 1;  // ed
	bool vr_mode = pSet->vr_mode;
#endif


	DestroyCompositor();  // ----

	//  🌒 shadows  ----
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
	CreateCubeReflect();


	//  🪄 Create Compositor  ----
	CreateCompositor();


	//  Render window external channels  ----
	CompositorChannelVec ext( mCubeReflTex ? 2 : 1 );
	ext[0] = mWindow->getTexture();
	if (mCubeReflTex)
		ext[1] = mCubeReflTex;  // 🔮


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
		// auto str = getWorkspace(1, 0);  // old
		auto str = "RefractionsWorkspaceMsaa";  // new ok
		// auto str = sNewWork;  // new todo ..
		
		const IdString wsName( str );
		auto ws = mgr->addWorkspace( mSceneMgr, ext, c->cam, wsName, true );  // in .compositor
		// ws->addListener(listener);
		vWorkspaces.push_back(ws);
		// LogO("C### Created Single workspaces: "+toStr(mWorkspaces.size()));

		// createShadowMapDebugOverlays();  //-
		// return ws;
	}
	LogO("C^## Done Compositor "+getWsInfo());
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
