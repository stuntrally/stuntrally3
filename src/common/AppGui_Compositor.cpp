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
#include <OgreDepthBuffer.h>
#include <OgrePixelFormatGpu.h>
#include <OgreRenderPassDescriptor.h>

#include <Compositor/Pass/OgreCompositorPassDef.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopy.h>
#include <Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
using namespace Ogre;


/*  Legend:   [] Single 🖥️   ++ SplitScreen 👥

1 s1_first  [Refract_first]
	tex depthBuffer \rtv
	tex rtt_first   /
	target rtt_first
		pass Scene  opaque,transp
	out0> rtt_first
	out1> depthBuffer

2 s2_depth  [DepthResolve]
	>in0 gBufferDB  <- depthBuffer
	tex resolvedDB
	target resolvedDB
		pass quad  resolve gBufferDB
	out0> resolvedDB

3 s3_Final  [Refract_Final]
	>in0 rtt_first    
	>in1 depthBuffer \
	>in2 depthBufferNoMsaa  <- resolvedDB
	>in3 rtt_FullOut     or  [] rt_renderwindow
	tex rtt_final    /rtv depthBuffer
	target rtt_final
		pass copy    rtt_first to rtt_final
		pass Scene   Refractive Fluids
			refractions: depthBufferNoMsaa rtt_first
	target rtt_FullOut   or  [] rt_renderwindow
		pass quad  copy rtt_final to rtt_FullOut or wnd
	[]	pass scene  Hud
	[]	pass scene  Gui

5 ++ sCombine  [Combine]
	>in0 rt_renderwindow
	>in1 rtt_FullIn1  <- rtt_FullOut 1
	>in2 rtt_FullIn2  <- rtt_FullOut 2  etc more plrs
	target rt_renderwindow
		pass quad copy  rtt_FullIn to wnd
	++	pass Hud
	++	pass Gui
*/

const String
	sNode = "SR3_New", sWork = "SR3_New_WS",  // Old, no refract
	s1_first = "SR3_1_Refract_first-",  // new Refract nodes
	s2_depth = "SR3_2_DepthResolve-",
	s3_Final = "SR3_3_Refract_Final-",
	sCombine = "SR3_Combine", sCombineWS = "SR3_Combine_WS";  // new in SplitScreen, last


//  util methods
//-----------------------------------------------------------------------------------------

void AppGui::AddShadows(CompositorPassSceneDef* ps)
{
	switch (pSet->g.shadow_type)  // shadows
	{
	case Sh_None:  break;  // none
	case Sh_Depth: ps->mShadowNode = csShadow;  break;
	case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
	}
}

String AppGui::getSplitMtr(int splits)
{
	String mtr;
#ifndef SR_EDITOR
	bool hor = !pSet->split_vertically;
	const String ss = "SplitScreen_";
	switch (splits)
	{
	case 2:  mtr = ss + (hor ? "2h" : "2v");  break;
	case 3:  mtr = ss + (hor ? "3h" : "3v");  break;
	case 4:  mtr = ss + "4";  break;
	case 5:  mtr = ss + (hor ? "5h" : "5v");  break;
	case 6:  mtr = ss + "6";  break;
	}
#endif
	return mtr;
}

//  add NodeDef
CompositorNodeDef* AppGui::AddNode(String name)
{
	auto* mgr = mRoot->getCompositorManager2();
	auto* nd = mgr->addNodeDefinition(name);
	vNodes.push_back(nd);
	LogO("C+-* Add Node " + name);
	return nd;
};
//  add Workspace
CompositorWorkspaceDef* AppGui::AddWork(String name)
{
	auto* mgr = mRoot->getCompositorManager2();
	auto* wd = mgr->addWorkspaceDefinition(name);
	vWorkDefs.push_back(wd);
	LogO("C+-# Add Work " + name);
	return wd;
};

//  add pass  Quad
CompositorPassQuadDef* AppGui::AddQuad(CompositorTargetDef* td)
{
	return static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
}

//  add pass  Scene
CompositorPassSceneDef* AppGui::AddScene(CompositorTargetDef* td)
{
	return static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
}


//  add Rtv
RenderTargetViewDef* AppGui::AddRtv(Ogre::CompositorNodeDef* nd,
	String name, String color, String depth, String color2)
{
	auto* rtv = nd->addRenderTextureView( name );
	rtv->colourAttachments.clear();
	RenderTargetViewEntry at;
	if (!color.empty())
	{	at.textureName = color;
		rtv->colourAttachments.push_back(at);
	}
	if (!color2.empty())
	{	at.textureName = color2;
		rtv->colourAttachments.push_back(at);
	}
	if (!depth.empty())
		rtv->depthAttachment.textureName = depth;
	// rtv->depthBufferId = 0;  // ignored if ^ set
	return rtv;
}


//  🖼️ create  Texture & RTT
//  final 1 player's view, for SplitScreen combine
//-----------------------------------------------------------------------------------------
TextureGpu* AppGui::AddSplitRTT(String id, float width, float height)
{
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	//  Tex
	auto* tex = texMgr->createTexture( "GTex" + id,
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
	auto* rtt = texMgr->createTexture( "GRtt" + id,
		GpuPageOutStrategy::Discard, flags, TextureTypes::Type2D );
	rtt->copyParametersFrom( tex );
	rtt->scheduleTransitionTo( GpuResidency::Resident );
	rtt->_setDepthBufferDefaults( DepthBuffer::POOL_DEFAULT, false, PFG_D32_FLOAT );  //?
	vRtt.push_back(rtt);
	return rtt;
}

//  Add Hud, Gui
void AppGui::AddHudGui(CompositorTargetDef* td)  // + 2 pass
{
	//  ⏲️ Hud  --------
	auto* ps = AddScene(td);  // + scene Hud
	ps->setAllStoreActions( StoreAction::Store );
	ps->mProfilingId = "HUD";  ps->mIdentifier = 10007;

	ps->mFirstRQ = RQG_RoadMarkers;  // 220
	ps->mLastRQ = RQG_Hud3+1;  // 223
	ps->setVisibilityMask(RV_Hud + RV_Particles);

	//  🎛️ Gui, add MyGUI pass  --------
	auto* gui = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);  // + pass Gui
	gui->mProfilingId = "GUI";  gui->mIdentifier = 99900;
}


//  🪄 Create Compositor  main render setup
//-----------------------------------------------------------------------------------------
TextureGpu* AppGui::CreateCompositor(int view, int splits, float width, float height)
{
	const auto inp = TextureDefinitionBase::TEXTURE_INPUT;
	//  log
	// pSet->g.water_refract = 1;  //! test
	const bool refract = pSet->g.water_refract, ssao = pSet->ssao;
	const bool split = splits > 1;

	LogO("CC+# Create Compositor   "+getWsInfo());
	if (view >= 50)
		LogO("CC=# Combine screen "+toStr(view));
	else
	if (split)
		LogO("CC+# Split screen "+toStr(view));
	else
		LogO("CC-# Single screen "+toStr(view));
	
	LogO("CC+# view dim: "+fToStr(width)+" "+fToStr(height));

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


#ifndef SR_EDITOR  // game
	//  Combine last, final wnd
	//  sum all splits (player views) + add one Hud, Gui
	//------------------------------------------------------------------------------------------------------------------------------------------
	if (view >= 50)
	{
		{	nd = AddNode(sCombine);  //++ node
			
			nd->addTextureSourceName("rt_renderwindow", 0, inp);  // in  wnd
			for (int i=0; i < splits; ++i)
				nd->addTextureSourceName("rtt_FullIn"+toStr(i), i+1, inp);

			nd->mCustomIdentifier = "50-Combine";

			nd->setNumTargetPass(1);  //* targets
			td = nd->addTargetPass( "rt_renderwindow" );
			td->setNumPasses( 1 + 2 );  //* passes

			//  render Window  ----
			{
				auto* pq = AddQuad(td);  // + quad
				pq->setAllLoadActions( LoadAction::DontCare );

				pq->mMaterialName = getSplitMtr(splits);

				for (int i=0; i < splits; ++i)
					pq->addQuadTextureSource( i, "rtt_FullIn"+toStr(i) );  // input
				pq->mProfilingId = "Combine to Window";

				//  ⏲️ Hud, 🎛️ Gui  --------
				AddHudGui(td);  // + 2
			}
		}
		//  workspace Full last
		{
			wd = AddWork( sCombineWS );
			wd->connectExternal( 0, sCombine, 0 );  // wnd
			for (int i=0; i < splits; ++i)
				wd->connectExternal( i+1, sCombine, i+1 );
		}
		return rtt;  // 0
	}
#endif

	//  node  New Refract  * * *
	//------------------------------------------------------------------------------------------------------------------------------------------
	if (refract)
	{
		if (split)
			rtt = AddSplitRTT(si, width, height);
		
		/* 1  s1_First  Render  -------------------- */
		{	nd = AddNode(s1_first+si);  //++ node
			
			nd->setNumLocalTextureDefinitions( (ssao ? 6 : 0) + 2 );  //* textures
			{
				auto* td = nd->addTextureDefinition( "rtt_first" );  // + 2
				td->format = PFG_UNKNOWN;  td->fsaa = "";  // target_format  // PFG_RGBA8_UNORM_SRGB

				td = nd->addTextureDefinition( "depthBuffer" );
				td->format = PFG_D32_FLOAT;  td->fsaa = "";  // auto
				// td->depthBufferFormat = PFG_D32_FLOAT;  //-
				// td->preferDepthTexture = 1;
				// td->textureFlags = TextureFlags::RenderToTexture;  //- no discard between frames

				if (ssao)  // ssao + 6
				{	td = nd->addTextureDefinition( "gNormals" );
					td->format = PFG_R10G10B10A2_UNORM;  td->fsaa = "";  // auto
					//td->textureFlags = TextureFlags::RenderToTexture | TextureFlags::MsaaExplicitResolve;
					td->textureFlags |= TextureFlags::MsaaExplicitResolve;
				}
				auto* rtv = AddRtv(nd, "rtt_first", "rtt_first", "depthBuffer", ssao ? "gNormals" : "");
				if (ssao)
				{
					td = nd->addTextureDefinition( "depthCopy" );
					td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  // half
					td->format = PFG_D32_FLOAT;  td->fsaa = "1";  // off
					AddRtv(nd, "depthCopy", "", "depthCopy");

					td = nd->addTextureDefinition( "ssaoTexture" );
					td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  //par HQ..
					td->format = PFG_R16_FLOAT;  td->fsaa = "1";  // off
					AddRtv(nd, "ssaoTexture", "ssaoTexture");
					// td->depthBufferId = 0;  //no- depth_pool 0

					td = nd->addTextureDefinition( "blurHoriz" );
					td->format = PFG_R16_FLOAT;  td->fsaa = "1";
					AddRtv(nd, "blurHoriz", "blurHoriz");

					td = nd->addTextureDefinition( "blurVertical" );
					td->format = PFG_R16_FLOAT;  td->fsaa = "1";
					AddRtv(nd, "blurVertical", "blurVertical");

					td = nd->addTextureDefinition( "ssaoApply" );
					td->format = PFG_UNKNOWN;  td->fsaa = "";  // target_format
					AddRtv(nd, "ssaoApply", "ssaoApply", "depthBuffer");
				}
			}
			nd->mCustomIdentifier = "1-first-"+si;
			
			nd->setNumTargetPass( (ssao ? 5 : 0) + 1 );  //* targets
			td = nd->addTargetPass( "rtt_first" );
			td->setNumPasses(1);  //* passes
			{
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Clear );
				ps->mClearColour[0] = ColourValue(0.2, 0.4, 0.6, 1.0);  // clr
				if (ssao)
					ps->mClearColour[1] = ColourValue(0.5, 0.5, 1.0, 1.0);  // norm

				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store;
				ps->mStoreActionStencil = StoreAction::DontCare;

				ps->mProfilingId = "Render First-"+si;  // "Opaque + Regular Transparents"
				ps->mIdentifier = 10001;
				// ps->mFirstRQ = 1; //RQG_Terrain;  // todo: sky last
				ps->mLastRQ = RQG_Weather;  // todo: where are car flares?..
				// ps->mLastRQ = RQG_RoadBlend+1; //RQG_Fluid-1; //199
				ps->setVisibilityMask(RV_view);
				
				// ps->mExposedTextures.push_back("") mCubeReflTex  // todo.. add
				if (ssao)
					ps->mGenNormalsGBuf = true;
				AddShadows(ps);  // shadows
			}

			if (ssao)  //  SSAO  + 5
			{
				const String depthCopy = "depthCopy";
				td = nd->addTargetPass( depthCopy );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );

					pq->mMaterialName = "Ogre/Depth/DownscaleMax";  pq->mProfilingId = "Depth Downscale";
					pq->addQuadTextureSource( 0, "depthBuffer" );
				}

				td = nd->addTargetPass( "ssaoTexture" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::Clear );
					pq->mClearColour[0] = ColourValue::White;

					pq->mMaterialName = "SSAO/HS";  pq->mProfilingId = "SSAO 1 HS";
					pq->addQuadTextureSource( 0, depthCopy );  // input
					pq->addQuadTextureSource( 1, "gNormals" );
					pq->mFrustumCorners = CompositorPassQuadDef::VIEW_SPACE_CORNERS;  // quad_normals  camera_far_corners_view_space
				}

				td = nd->addTargetPass( "blurHoriz" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );

					pq->mMaterialName = "SSAO/BlurH";  pq->mProfilingId = "SSAO 2 BlurH";
					pq->addQuadTextureSource( 0, "ssaoTexture" );  // input
					pq->addQuadTextureSource( 1, depthCopy );
				}

				td = nd->addTargetPass( "blurVertical" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );

					pq->mMaterialName = "SSAO/BlurV";  pq->mProfilingId = "SSAO 3 BlurV";
					pq->addQuadTextureSource( 0, "blurHoriz" );  // input
					pq->addQuadTextureSource( 1, depthCopy );
				}

				td = nd->addTargetPass( "ssaoApply" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );
					
					pq->mMaterialName = "SSAO/Apply";  pq->mProfilingId = "SSAO Apply";  // input
					pq->addQuadTextureSource( 0, "blurVertical" );
					pq->addQuadTextureSource( 1, "rtt_first" );
					// pq->addQuadTextureSource( 0, "ssaoTexture" );  //* test noisy
					// pq->addQuadTextureSource( 1, "gNormals" );  //* test normals
				}
			}
			nd->setNumOutputChannels( 2 );  //  out>
			nd->mapOutputChannel(0, ssao ? "ssaoApply" : "rtt_first");
			nd->mapOutputChannel(1, "depthBuffer");
		}

		/* 2  s2_depth  resolve  ---------- */
		{	nd = AddNode(s2_depth+si);  //++ node
			
			nd->addTextureSourceName("gBufferDB", 0, inp);  //  >in

			nd->setNumLocalTextureDefinitions(1);  //* textures
			{
				auto* td = nd->addTextureDefinition( "resolvedDB" );
				td->format = PFG_R32_FLOAT;  // D32 -> R32
				td->fsaa = "1";  // off

				AddRtv(nd, "resolvedDB", "resolvedDB", "");
			}
			nd->mCustomIdentifier = "2-depth-"+si;

			//  We need to "downsample/resolve" DepthBuffer because the impact
			//  performance on Refractions is gigantic
			nd->setNumTargetPass(1);  //* targets
			td = nd->addTargetPass( "resolvedDB" );
			td->setNumPasses(1);  //* passes
			{
				auto* pq = AddQuad(td);  // + quad
				pq->setAllLoadActions( LoadAction::DontCare );
				pq->setAllStoreActions( StoreAction::DontCare );
				pq->mStoreActionColour[0] = StoreAction::StoreOrResolve;

				pq->mMaterialName = "Ogre/Resolve/1xFP32_Subsample0";  pq->mProfilingId = "Depth Resolve";
				pq->addQuadTextureSource( 0, "gBufferDB" );  // input quad
			}
			nd->setNumOutputChannels(1);  //  out>
			nd->mapOutputChannel(0, "resolvedDB");
		}

		/* 3  s3_Final  Refractive  ------------------------------  */
		{	nd = AddNode(s3_Final+si);  //++ node
			
			nd->addTextureSourceName("rtt_FullOut", 0, inp);  // or rt_renderwindow
			nd->addTextureSourceName("rrt_firstIn", 1, inp);  //  >in  ssaoApply or rtt_first
			nd->addTextureSourceName("depthBuffer", 2, inp);
			nd->addTextureSourceName("depthBufferNoMsaa", 3, inp);

			nd->setNumLocalTextureDefinitions( 1 );  //* textures
			{
				auto* td = nd->addTextureDefinition( "rtt_final" );
				td->format = PFG_UNKNOWN;  td->fsaa = "";  // target_format, auto
				AddRtv(nd, "rtt_final", "rtt_final", "depthBuffer");  //-, ssao ? "gNormals" : "");
			}

			nd->mCustomIdentifier = "3-Final-"+si;

			nd->setNumTargetPass( 2 );  //* targets

			//  🌊 Refracted  Fluids  ----
			td = nd->addTargetPass( "rtt_final" );
			td->setNumPasses(2);  //* passes
			{
				//  Perform exact copy (MSAA-preserving) so we can continue rendering
				//  into rtt_final. Meanwhile rtt_first will be used for sampling refractions
				auto* cp = static_cast<CompositorPassDepthCopyDef*>(
					td->addPass(Ogre::PASS_DEPTHCOPY));  // + copy
				cp->setDepthTextureCopy("rrt_firstIn", "rtt_final");

				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Load );
				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store; //DontCare;
				ps->mStoreActionStencil = StoreAction::DontCare;

				ps->mProfilingId = "Refractive Fluids";  ps->mIdentifier = 10002;
				ps->mFirstRQ = RQG_Fluid;  ps->mLastRQ = RQG_Refract+1;  // 210, 211
				ps->setVisibilityMask(RV_Fluid);  // 0x00000002
				
				AddShadows(ps);  // shadows
				ps->mShadowNodeRecalculation = SHADOW_NODE_REUSE;  // `
				ps->setUseRefractions("depthBufferNoMsaa", "rrt_firstIn");  // ~
			}

			//  render / Window  ----
			td = nd->addTargetPass( "rtt_FullOut" );
			td->setNumPasses( (!split ? 2 : 0) + 1 );  //* passes
			{
				auto* pq = AddQuad(td);  // + quad
				pq->setAllLoadActions( LoadAction::DontCare );
				
				pq->mMaterialName = "Ogre/Copy/4xFP32";
				pq->mProfilingId = "Copy to Window";
				pq->addQuadTextureSource( 0, "rtt_final" );  // input

				//  ⏲️ Hud, 🎛️ Gui  --------
				if (!split)
					AddHudGui(td);  // + 2
			}
		}

		//  workspace
		{
			wd = AddWork( sWork+si );
			wd->connectExternal( 0, s3_Final+si, 0 );  // rtt_FullOut  or  rt_renderwindow
			wd->connect( s1_first+si, 0, s3_Final+si, 1 );  // rtt_first
			wd->connect( s1_first+si, 1, s3_Final+si, 2 );  // depthBuffer
			wd->connect( s1_first+si, 1, s2_depth+si, 0 );  // depthBuffer
			wd->connect( s2_depth+si, 0, s3_Final+si, 3 );  // resolvedDepth -> depthBufferNoMsaa
		}
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	else  //  node  Old  no refract, no depth  - - -  // todo old split screen ..
	{
		if (split)
			rtt = AddSplitRTT(si, width, height);

		{	nd = AddNode(sNode+si);  //++ node
			
			//  render window input
			nd->addTextureSourceName("rt_wnd", 0, inp);  // in  wnd

			nd->mCustomIdentifier = "old-1";
			//if( pass->getParentNode()->getDefinition()->mCustomIdentifier == "CustomString" )

			nd->setNumTargetPass( (!split ? 2 : 0) + 1 );  //* targets

			td = nd->addTargetPass( "rt_wnd" );
			{
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Clear );
				ps->setAllStoreActions( StoreAction::Store );

				ps->mProfilingId = "Render Old";
				ps->mIdentifier = 21002;
				// ps->mLastRQ = 199;  //RQG_Fluid-1  // all

				AddShadows(ps);  // shadows
			}
			//  ⏲️ Hud, 🎛️ Gui  --------
			if (!split)
				AddHudGui(td);  // + 2

			//  workspace
			{
				wd = AddWork(sWork+si);
				wd->connectExternal(0, nd->getName(), 0);  // rt_renderwindow
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
	auto* mgr = mRoot->getCompositorManager2();
	
#ifndef SR_EDITOR  // game
	const int views = pSet->game.local_players;
	// bool vr_mode = pSet->game.vr_mode;  // not used

	LogO("CC## views:  "+toStr(views) + 
		(!(views==2 || views==3 || views==5) ? "" :
		 !pSet->split_vertically ? "  horiz" : "  vert"));
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
	//......................................................................................................................................
	if (views > 1)
	{
		for (int i = 0; i < views; ++i)
		{
			bool f1 = i > 0;

			auto c = CreateCamera( "Player" + toStr(i), 0, Vector3::ZERO, Vector3::NEGATIVE_UNIT_Z );
			if (i==0)  mCamera = c->cam;
			
			auto& d = mDims[i];
			d.SetDim(views, !pSet->split_vertically, i);

			//  🪄 Create 1  ----
			auto* rtt = CreateCompositor(i, views, d.width0, d.height0);

			//  external output to rtt
			CompositorChannelVec chOne(1);  // mCubeReflTex ? 2 : 1 );
			chOne[0] = rtt;  // todo: add cubemap(s) refl..

			const IdString wsName( sWork + toStr(i+1) );
			auto* wOne = mgr->addWorkspace( mSceneMgr, chOne,  c->cam, wsName, true, 0 /*1st*/ );
			vWorkspaces.push_back(wOne);
		}

		//  🪄 Combine []  ----
		CreateCompositor(50, views, 1.f, 1.f);

			auto c = CreateCamera( "PlayerW", 0, Vector3(0,150,0), Vector3(0,0,0) );
			mCamera = c->cam;	// fake cam

		//  external channels  ----
		CompositorChannelVec chAll( 1 + views );
		chAll[0] = mWindow->getTexture();
		for (int i=0; i < views; ++i)
			chAll[i+1] = vRtt[i];  // RTTs player views

		const IdString wsNameC( "SR3_Combine_WS" );
		auto* wAll = mgr->addWorkspace( mSceneMgr, chAll,  c->cam, wsNameC, true, -1 /*last*/ );
		vWorkspaces.push_back(wAll);
	}
	else
#endif
  	//  🖥️ Single View  --------
	//......................................................................................................................................
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

		const IdString wsName( sWork + "0" );
		auto ws = mgr->addWorkspace( mSceneMgr, chWnd,  c->cam, wsName, true );
		// ws->addListener(listener);
		vWorkspaces.push_back(ws);
	}
#if 0	//  OLD meh
	else
	{	//  👀 VR mode  ---- ----  todo use OpenVR
	//......................................................................................................................................
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
