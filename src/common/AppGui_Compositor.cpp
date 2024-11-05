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


/*  legend:   // Single   ++ Splitscreen

1 SR3_Refract_first
	tex depthBuffer \rtv
	tex rtt_first   /
	target rtt_first
		pass Scene  opaque,transp
	out0> rtt_first
	out1> depthBuffer

2 SR3_DepthResolve
	>in0 gBufferDB  <- depthBuffer
	tex resolvedDB
	target resolvedDB
		pass quad  resolve gBufferDB
	out0> resolvedDB

3 SR3_Refract_Final
	>in0 rtt_first
	>in1 depthBuffer
	>in2 depthBufferNoMsaa  <- resolvedDB
	>in3 rtt_FullOut  // rt_renderwindow
	tex rtt_final  rtv depthBuffer
	target rtt_final
		pass copy  rtt_first to rtt_final
		pass Scene  Refractive Fluids
			UseRefractions  depthBufferNoMsaa rtt_first
	target rtt_FullOut or // rt_renderwindow
		pass quad  copy rtt_final to rtt_FullOut or wnd
	//	pass scene  Hud
	//	pass scene  Gui

++ 5 SR3_Combine
	>in0 rt_renderwindow
	>in1 rtt_FullIn1  <- rtt_FullOut 1
	>in2 rtt_FullIn2  <- rtt_FullOut 2  ..more plr
	target rt_renderwindow
		pass quad copy  rtt_FullIn to wnd
	++	pass Hud
	++	pass Gui
*/

const String sNewNode = "SR3_New";
const String sNewWork = "SR3_New_WS";


//  🪄 Create Compositor  main render setup
//-----------------------------------------------------------------------------------------
TextureGpu* AppGui::CreateCompositor(int view, int splits, float width, float height)
{
	//  log
	pSet->g.water_refract = 1;  //! test
	const bool refract = pSet->g.water_refract;
	const bool split = splits > 1;

	LogO("CC+# Create Compositor   "+getWsInfo());
	if (view >= 50)
		LogO("CC=# Combine screen "+toStr(view));
	else
	if (split)
		LogO("CC+# Split screen "+toStr(view));
	else
		LogO("CC-# Single screen "+toStr(view));

	//  var
	auto* mgr = mRoot->getCompositorManager2();
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	
	CompositorNodeDef* nd =0;  CompositorTargetDef* td =0;
	CompositorPassSceneDef *ps =0;  CompositorWorkspaceDef *wd = 0;
	TextureGpu* tex =0, *rtt =0;

	//  const
	const String si = toStr(view+1);
	const int plr = view < 0 ? 0 : view;
	const uint32 RV_view = // vis mask, split screen
		RV_Hud3D[plr] + (refract ? RV_MaskAll - RV_Fluid : RV_MaskAll);
	// const Real wx = 1.f, wy = 1.f;
	// const Real wx = 0.5f, wy = 1.f;

	//  Add
	auto AddNode = [&](String name)	{
		nd = mgr->addNodeDefinition(name);
		vNodes.push_back(nd);  LogO("C+-* Add Node " + name);
	};
	auto AddWork = [&](String name)	{
		wd = mgr->addWorkspaceDefinition(name);
		vWorkDefs.push_back(wd);  LogO("C+-# Add Work " + name);
	};

	//-----------------------------------------------------------------------------------------
	if (view >= 50)  //  Combine last, final wnd
	//  sum all splits (player views) + add one Hud, Gui
	{
		/* 50 ---------- */
		{	AddNode("SR3_Combine");
			
			nd->addTextureSourceName("rt_renderwindow", 0, TextureDefinitionBase::TEXTURE_INPUT);
			nd->addTextureSourceName("rtt_FullIn1", 1, TextureDefinitionBase::TEXTURE_INPUT);
			nd->addTextureSourceName("rtt_FullIn2", 2, TextureDefinitionBase::TEXTURE_INPUT);

			nd->mCustomIdentifier = "50-combine";

			nd->setNumTargetPass(1);  //* targets
			td = nd->addTargetPass( "rt_renderwindow" );
			td->setNumPasses(3);  //* passes
			//  render Window  ----
			{
				auto* pq = static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
				// ps->setAllLoadActions( LoadAction::Clear );
				pq->setAllLoadActions( LoadAction::DontCare );
				// ps->setAllStoreActions( StoreAction::Store );

				pq->mMaterialName = "Splitscreen";
				pq->addQuadTextureSource( 0, "rtt_FullIn1" );  // input
				pq->addQuadTextureSource( 1, "rtt_FullIn2" );
				//  more ..
				pq->mProfilingId = "Combine to Window";

				//  ⏲️ Hud  --------
				{	ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
					ps->setAllStoreActions( StoreAction::Store );
					ps->mProfilingId = "HUD";  ps->mIdentifier = 10007;

					ps->mFirstRQ = 220;  // RQG_RoadMarkers
					ps->mLastRQ = 230;  // RQG_Hud3
					ps->setVisibilityMask(RV_Hud + RV_Particles);
				}
				//  🎛️ Gui, add MyGUI pass  --------
				{	auto* gui = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
					gui->mProfilingId = "GUI";  gui->mIdentifier = 99900;
			}	}
		}
		//  workspace Full last
		{
			AddWork( "SR3_Combine_WS" );
			wd->connectExternal( 0, "SR3_Combine", 0 );
			wd->connectExternal( 1, "SR3_Combine", 1 );
			wd->connectExternal( 2, "SR3_Combine", 2 );
			// more ..
		}
		return rtt;  // 0
	}


	//  node  New Refract  * * *
	//-----------------------------------------------------------------------------------------
	if (refract)
	{
		//  🖼️ texture & rtt  for splitscreen
		if (split)
		{
			tex = texMgr->createTexture( "GTex" + si,
				GpuPageOutStrategy::SaveToSystemRam,
				TextureFlags::ManualTexture, TextureTypes::Type2D );
			
			tex->setResolution(  // dims
				mWindow->getWidth() * width, mWindow->getHeight() * height);
			tex->scheduleTransitionTo( GpuResidency::OnStorage );
			tex->setNumMipmaps(1);  // none

			tex->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
			tex->scheduleTransitionTo( GpuResidency::Resident );
			vTex.push_back(tex);

			//  RTT
			auto flags = TextureFlags::RenderToTexture;
			rtt = texMgr->createTexture( "GRtt" + si,
				GpuPageOutStrategy::Discard, flags, TextureTypes::Type2D );
			rtt->copyParametersFrom( tex );
			rtt->scheduleTransitionTo( GpuResidency::Resident );
			rtt->_setDepthBufferDefaults( DepthBuffer::POOL_DEFAULT, false, PFG_D32_FLOAT );  //?
			vRtt.push_back(rtt);
		}

		/* 1 -------------------- */
		{	AddNode("SR3_Refract_first-"+si);
						
			nd->setNumLocalTextureDefinitions(2);  //* textures
			{
				auto* td = nd->addTextureDefinition( "depthBuffer" );
				// td->depthBufferFormat = PFG_D32_FLOAT;
				// td->widthFactor = wx;  td->heightFactor = wy;
				td->format = PFG_D32_FLOAT;
				td->fsaa = "";  // auto
				// td->preferDepthTexture = 1;
				// td->depthBufferId = 0;
				// td->textureFlags = TextureFlags::RenderToTexture;  //- no discard between frames

				td = nd->addTextureDefinition( "rtt_first" );
				td->format = PFG_UNKNOWN;  //PFG_RGBA8_UNORM_SRGB;  // target_format
				td->fsaa = "";  // auto

				auto* rtv = nd->addRenderTextureView( "rtt_first" );  // rtv
				RenderTargetViewEntry at;
				at.textureName = "rtt_first";
				rtv->colourAttachments.push_back(at);
				rtv->depthAttachment.textureName = "depthBuffer";
				// rtv->depthBufferId = 0;  // ignored if ^ set
			}
			nd->mCustomIdentifier = "1-first-"+si;
			
			nd->setNumTargetPass(1);  //* targets
			td = nd->addTargetPass( "rtt_first" );
			td->setNumPasses(1);  //* passes
			{
				ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
				ps->setAllLoadActions( LoadAction::Clear );
				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store;
				ps->mStoreActionStencil = StoreAction::DontCare;

				ps->mProfilingId = "Render First-"+si;  // "Opaque + Regular Transparents"
				ps->mIdentifier = 10001;
				ps->mLastRQ = 199;  //RQG_Fluid-1
				ps->setVisibilityMask(RV_view);
				
				switch (pSet->g.shadow_type)  // shadows
				{
				case Sh_None:  break;  // none
				case Sh_Depth: ps->mShadowNode = csShadow;  break;
				case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
				}
			}
			nd->setNumOutputChannels(2);  //  out
			nd->mapOutputChannel(0, "rtt_first");
			nd->mapOutputChannel(1, "depthBuffer");
		}

		/* 2 ---------- */
		{	AddNode("SR3_DepthResolve-"+si);
			
			nd->addTextureSourceName("gBufferDB", 0, TextureDefinitionBase::TEXTURE_INPUT);

			nd->setNumLocalTextureDefinitions(1);  //* textures
			{
				auto* td = nd->addTextureDefinition( "resolvedDB" );
				td->format = PFG_R32_FLOAT;  // D32 -> R32
				td->fsaa = 1;  // off

				auto* rtv = nd->addRenderTextureView( "resolvedDB" );  // rtv
				RenderTargetViewEntry at;
				at.textureName = "resolvedDB";
				rtv->colourAttachments.push_back(at);
			}
			nd->mCustomIdentifier = "2-resolve-"+si;

			//  We need to "downsample/resolve" DepthBuffer because the impact
			//  performance on Refractions is gigantic
			nd->setNumTargetPass(1);  //* targets
			td = nd->addTargetPass( "resolvedDB" );
			td->setNumPasses(1);  //* passes
			{
				auto* pq = static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
				pq->setAllLoadActions( LoadAction::DontCare );
				pq->setAllStoreActions( StoreAction::DontCare );
				pq->mStoreActionColour[0] = StoreAction::StoreOrResolve;

				pq->mMaterialName = "Ogre/Resolve/1xFP32_Subsample0";
				pq->addQuadTextureSource( 0, "gBufferDB" );  // input
				pq->mProfilingId = "Resolve Depth Buffer";
			}
			nd->setNumOutputChannels(1);  //  out
			nd->mapOutputChannel(0, "resolvedDB");
		}

		/* 3 ------------------------------  */
		{	AddNode("SR3_Refract_Final-"+si);
			
			nd->addTextureSourceName("rtt_first", 0, TextureDefinitionBase::TEXTURE_INPUT);
			nd->addTextureSourceName("depthBuffer", 1, TextureDefinitionBase::TEXTURE_INPUT);
			nd->addTextureSourceName("depthBufferNoMsaa", 2, TextureDefinitionBase::TEXTURE_INPUT);
			nd->addTextureSourceName("rtt_FullOut", 3, TextureDefinitionBase::TEXTURE_INPUT);  // or rt_renderwindow

			nd->setNumLocalTextureDefinitions(1);  //* textures
			{
				auto* td = nd->addTextureDefinition( "rtt_final" );
				td->format = PFG_UNKNOWN;  // target_format, default
				td->fsaa = "";  // auto

				auto* rtv = nd->addRenderTextureView( "rtt_final" );  // rtv
				RenderTargetViewEntry at;
				at.textureName = "rtt_final";
				rtv->colourAttachments.push_back(at);
				rtv->depthAttachment.textureName = "depthBuffer";
			}

			nd->mCustomIdentifier = "3-final-"+si;

			nd->setNumTargetPass(2);  //* targets

			//  🌊 Refracted  Fluids  ----
			td = nd->addTargetPass( "rtt_final" );
			td->setNumPasses(2);  //* passes
			{
				//  Perform exact copy (MSAA-preserving) so we can continue rendering
				//  into rtt_final. Meanwhile rtt_first will be used for sampling refractions
				auto* cp = static_cast<CompositorPassDepthCopyDef*>(td->addPass(Ogre::PASS_DEPTHCOPY));
				cp->setDepthTextureCopy("rtt_first", "rtt_final");  //*

				ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
				ps->setAllLoadActions( LoadAction::Load );
				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store; //DontCare;
				ps->mStoreActionStencil = StoreAction::DontCare;

				ps->mProfilingId = "Refractive Fluids";  ps->mIdentifier = 10002;
				ps->mFirstRQ = RQG_Fluid;  // 210
				ps->mLastRQ = RQG_Fluid+2;
				ps->setVisibilityMask(RV_Fluid);  // 0x00000002
				
				switch (pSet->g.shadow_type)  // shadows
				{
				case Sh_None:  break;  // none
				case Sh_Depth: ps->mShadowNode = csShadow;  break;
				case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
				}
				ps->mShadowNodeRecalculation = SHADOW_NODE_REUSE;
				ps->setUseRefractions("depthBufferNoMsaa", "rtt_first");  //~
			}

			//  render //Window  ----
			td = nd->addTargetPass( "rtt_FullOut" );
			td->setNumPasses( split ? 1 : 3 );  //* passes
			{
				auto* pq = static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
				pq->setAllLoadActions( LoadAction::DontCare );
				pq->mMaterialName = "Ogre/Copy/4xFP32";
				pq->addQuadTextureSource( 0, "rtt_final" );  // input
				pq->mProfilingId = "Copy to Window";

				//  ⏲️ Hud  --------
				if (!split)
				{	ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
					ps->setAllStoreActions( StoreAction::Store );
					ps->mProfilingId = "HUD";  ps->mIdentifier = 10007;

					ps->mFirstRQ = 220;  // RQG_RoadMarkers
					ps->mLastRQ = 230;  // RQG_Hud3
					ps->setVisibilityMask(RV_Hud + RV_Particles);
				
				//  🎛️ Gui, add MyGUI pass  --------
					auto* gui = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
					gui->mProfilingId = "GUI";  gui->mIdentifier = 99900;
				}
			}
		}

		//  workspace
		{
			AddWork( sNewWork+si );
			wd->connect("SR3_Refract_first-"+si, 0, "SR3_Refract_Final-"+si, 0 );  // rtt_first
			wd->connect("SR3_Refract_first-"+si, 1, "SR3_Refract_Final-"+si, 1 );  // depthBuffer
			wd->connect("SR3_Refract_first-"+si, 1, "SR3_DepthResolve-"+si,  0 );  // depthBuffer
			wd->connect("SR3_DepthResolve-"+si,  0, "SR3_Refract_Final-"+si, 2 );  // resolvedDepth -> depthBufferNoMsaa
			wd->connectExternal( 0, "SR3_Refract_Final-"+si, 3 );  // rtt_FullOut  or  rt_renderwindow
		}
	}
	//-----------------------------------------------------------------------------------------
	else  //  node  Old  no refract, no depth  - - -  // todo old split screen ..
	{
		{	AddNode(sNewNode);
			
			//  render window input
			nd->addTextureSourceName("rt_wnd", 0, TextureDefinitionBase::TEXTURE_INPUT);

			nd->mCustomIdentifier = "old-1";
			//if( pass->getParentNode()->getDefinition()->mCustomIdentifier == "CustomString" )

			nd->setNumTargetPass(3);  //* targets

			td = nd->addTargetPass( "rt_wnd" );
			{
				ps = static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
				ps->setAllLoadActions( LoadAction::Clear );
				ps->setAllStoreActions( StoreAction::Store );

				ps->mProfilingId = "Render Old";
				ps->mIdentifier = 21002;
				// ps->mLastRQ = 199;  //RQG_Fluid-1  // all

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
				ps->mProfilingId = "HUD";  ps->mIdentifier = 10007;

				ps->mFirstRQ = 220;
				ps->mLastRQ = 230;
				ps->setVisibilityMask(RV_Hud + RV_Particles);
			}
			//  🎛️ Gui, add MyGUI pass  --------
			{	CompositorPassDef *pass;
				pass = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
				pass->mProfilingId = "GUI";  pass->mIdentifier = 99900;
			}
			//  workspace
			{
				AddWork( sNewWork+si );
				wd->connectExternal( 0, nd->getName(), 0 );  // rt_renderwindow
			}
		}
	}
	return rtt;
}

//-----------------------------------------------------------------------------------------
//  🪄 Setup Compositors All
//-----------------------------------------------------------------------------------------
void AppGui::SetupCompositors()
{
	LogO("CC## Setup Compositor    "+getWsInfo());
	auto* rndSys = mRoot->getRenderSystem();
	auto* texMgr = rndSys->getTextureGpuManager();
	auto* mgr = mRoot->getCompositorManager2();
	
#ifndef SR_EDITOR  // game
	const int views = pSet->game.local_players;
	// bool vr_mode = pSet->game.vr_mode;  // not used
#else
	const int views = 1;  // ed
	// bool vr_mode = pSet->vr_mode;
#endif

	DestroyCompositors();  // 💥 ----


	//  🌒 Shadows  ----
	bool esm = pSet->g.shadow_type == 2;
	if (pSet->g.shadow_type > 0)
	{
		if (esm)  setupESM();
		createPcfShadowNode();
		if (esm)  createEsmShadowNodes();  // fixme..
		// createShadowMapDebugOverlays();  //-
	}


	//  🔮 create Reflections  ----
	CreateCubeReflect();


	//  🖥️ Viewports  ----
	DestroyCameras();  // 💥🎥

	for (int i = 0; i < MAX_Players; ++i)
		mDims[i].Default();


#ifndef SR_EDITOR
	//  👥 Split Screen   ---- ---- ---- ----
	//.....................................................................................
	if (views > 1)
	{
		for (int i = 0; i < views; ++i)
		{
			bool f1 = i > 0;

			auto c = CreateCamera( "Player" + toStr(i), 0, Vector3::ZERO, Vector3::NEGATIVE_UNIT_Z );
			if (i==0)
				mCamera = c->cam;
			
			auto& d = mDims[i];
			d.SetDim(views, !pSet->split_vertically, i);

			//  🪄 Create 1  ----
			auto* rtt = CreateCompositor(i, views, d.width0, d.height0);

			//  external output to rtt
			CompositorChannelVec chOne(1);  // mCubeReflTex ? 2 : 1 );
			chOne[0] = rtt;  // todo: add cubemap(s) refl..

			const IdString wsName( sNewWork + toStr(i+1) );
			auto* wOne = mgr->addWorkspace( mSceneMgr, chOne,  c->cam, wsName, true, 0 /*1st*/ );
			vWorkspaces.push_back(wOne);
		}

		//  🪄 Combine []  ----
		CreateCompositor(50, views, 1.f, 1.f);

			auto c = CreateCamera( "PlayerW", 0, Vector3(0,150,0), Vector3(0,0,0) );
			mCamera = c->cam;	// fake cam

		//  Render window external channels  ----
		CompositorChannelVec chAll( 3 /*+ views*/ );
		chAll[0] = mWindow->getTexture();
		chAll[1] = vRtt[0];  // RTTs player views
		chAll[2] = vRtt[1];
		// for (int i=0; i < views; ++i)  //  todo more..
		// 	chAll[i+1] = vRtt[i];  // RTTs player views

		const IdString wsNameC( "SR3_Combine_WS" );
		auto* wAll = mgr->addWorkspace( mSceneMgr, chAll,  c->cam, wsNameC, true, -1 /*last*/ );
		vWorkspaces.push_back(wAll);
	}
	else
#endif
  	//  🖥️ Single View  --------
	//.....................................................................................
	// if (!vr_mode)
	{
		auto c = CreateCamera( "Player", 0, Vector3(0,150,0), Vector3(0,0,0) );
		mCamera = c->cam;

		//  🪄 Create []  ----
		CreateCompositor(-1, 1,  1.f, 1.f);

		//  render Window external channels  ----
		CompositorChannelVec chWnd( mCubeReflTex ? 2 : 1 );
		chWnd[0] = mWindow->getTexture();
		if (mCubeReflTex)
			chWnd[1] = mCubeReflTex;  // 🔮

		const IdString wsName( sNewWork + "0" );
		auto ws = mgr->addWorkspace( mSceneMgr, chWnd,  c->cam, wsName, true );
		// ws->addListener(listener);
		vWorkspaces.push_back(ws);
	}
#if 0
	else
	{	//  👀 VR mode  ---- ----  meh, todo use OpenVR
		//.................................................................................
		const Real eyeSide = 0.5f, eyeFocus = 0.45f, eyeZ = -10.f;  // dist

		AddGuiShadows(vr_mode, 0, true);

		auto camL = CreateCamera( "EyeL", 0,
			Vector3(-eyeSide, 0.f,0.f), Vector3(eyeFocus * -eyeSide, 0.f, eyeZ) );
		auto camR = CreateCamera( "EyeR", camL->nd,
			Vector3( eyeSide, 0.f,0.f), Vector3(eyeFocus *  eyeSide, 0.f, eyeZ) );
		mCamera = camL->cam;

		//  add Workspace
		LogO("--++ WS add:  VR Eye L, all: "+toStr(mgr->getNumWorkspaces()));
		auto str = getWorkspace(1, 0);
		const IdString wsName( str );
		auto* ws1 = mgr->addWorkspace( mSceneMgr, ext,
				camL->cam, wsName,
				true, -1, 0, 0,
				Vector4( 0.0f, 0.0f, 0.5f, 1.0f ),
				0x01, 0x01 );
		vWorkspaces.push_back(ws1);

		LogO("--++ WS add:  VR Eye R, all: "+toStr(mgr->getNumWorkspaces()));
		auto* ws2 = mgr->addWorkspace( mSceneMgr, ext, 
				camR->cam, wsName,
				true, -1, 0, 0,
				Vector4( 0.5f, 0.0f, 0.5f, 1.0f ),
				0x02, 0x02 );
		vWorkspaces.push_back(ws2);
	}
#endif
	LogO("CC## Done Compositor === "+getWsInfo());
}



//  💥 destroy Compositor
//-----------------------------------------------------------------------------------------
void AppGui::DestroyCompositors()
{
	LogO("DD## Destroy Compositor  "+getWsInfo());
	auto* mgr = mRoot->getCompositorManager2();
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();

	for (auto* ws : vWorkspaces)
	{
		mgr->removeWorkspace( ws );
	}
	vWorkspaces.clear();
			
	if (mgr->hasShadowNodeDefinition(csShadow))
		mgr->removeShadowNodeDefinition(csShadow);

	for (auto* rt : vRtt)
		texMgr->destroyTexture( rt );
	vRtt.clear();
	for (auto* tx : vTex)
		texMgr->destroyTexture( tx );
	vTex.clear();

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


//  💥🎥 Destroy Camera
//--------------------------------------------------------------------------------
void AppGui::DestroyCameras()
{
	LogO("DD## destroy Cameras-");
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
	//  VR-  Use one node to control both cameras
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


//  log util  ----
String AppGui::getWsInfo()
{
	auto* mgr = mRoot->getCompositorManager2();
	return " worksp: "+ toStr(vWorkspaces.size())+" ("+toStr(mgr->getNumWorkspaces())
		+ ") workdef: "+ toStr(vWorkDefs.size())
		+ " node: "+ toStr(vNodes.size())
		+ " rtt: "+ toStr(vRtt.size());
}
