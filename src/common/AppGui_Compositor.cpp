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


/*  Legend, marks for only:
	[]  Single 🖥️
	++  SplitScreen 👥
	S   SSAO 🕳️
	M   MSAA (FSAA)  antialiasing
	H   HDR ..


0 S  s0_ssao  [prepare] for ssao 🕳️
	tex rtt_ssao  \   clr0 unused
	tex depthHalf |   clr1  (all half size, no fsaa
	tex gNormals  } rtv
	tex gFog     /    clr2

	target rtt_ssao
		pass Scene  opaque only

	out0> gNormals
	out1> depthHalf
	out2> gFog


1 s1_first  [first]
 S	in0> gNormals
 S	in1> depthHalf
 S	in2> gFog
	tex depthBuffer \rtv
	tex rtt_first   /

	target rtt_first
		pass Scene  opaque,transp ⛰️
	out0> rtt_first
	out1> depthBuffer

2  s2_depth  [Depth Resolve]
	>in0 gBufferDB  <- depthBuffer
	tex resolvedDB

	target resolvedDB
	M	pass quad  resolve gBufferDB  \
	M-	pass copy  gBufferDB  / or
	out0> resolvedDB

3 s3_Final  [Final] 🪩
	>in0 rtt_first    
	>in1 depthBuffer \
	>in2 depthBufferNoMsaa  <- resolvedDB
	>in3 rtt_FullOut     or  [] rt_renderwindow
	tex rtt_final    /rtv depthBuffer

	target rtt_final
		pass copy    rtt_first to rtt_final
		pass Scene   🌊 Refractive Fluids
			refractions: depthBufferNoMsaa rtt_first

	target rtt_FullOut   or  [] rt_renderwindow
		pass quad  copy rtt_final to rtt_FullOut or wnd
	[]	pass scene  Hud
	[]	pass scene  Gui


5 ++  sCombine  [Combine]
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
	s0_ssao  = "SR3_0_prepare-", // 0 prepare 4 ssao
	s1_first = "SR3_1_First-",   // 1 new  scene  opaque & glass
	s2_depth = "SR3_2_depth-",   // 2 for refract  resolve / copy depth
	s3_Final = "SR3_3_Final-",   // 3 last
	sCombine = "SR3_Combine", sCombineWS = "SR3_Combine_WS";  // new in SplitScreen, last


//  Add Hud, Gui  ----------------
#ifdef SR_EDITOR
	#define nHudGui 3  // ed 1 more
#else
	#define nHudGui 2
#endif
void AppGui::AddHudGui(CompositorTargetDef* td)  // + 3 ed, + 2 game  pass
{
	//  ⏲️ Hud  --------
	auto* ps = AddScene(td);  // + scene Hud
	ps->setAllStoreActions( StoreAction::Store );
	ps->mProfilingId = "HUD";  ps->mIdentifier = 10007;

	ps->mFirstRQ = RQG_RoadMarkers;
	ps->mLastRQ = RQG_Hud3+1;
	ps->setVisibilityMask(RV_Hud + RV_Particles);

	//  🎛️ Gui, add MyGUI pass  --------
	auto* gui = td->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);  // + pass Gui
	gui->mProfilingId = "GUI";  gui->mIdentifier = 99900;

#ifdef SR_EDITOR
	//  🌍 ed Hud after Gui  --------
	ps = AddScene(td);  // + scene Hud 2
	ps->setAllStoreActions( StoreAction::Store );
	ps->mProfilingId = "HUD2";  ps->mIdentifier = 10007;

	ps->mFirstRQ = RQG_Hud4;
	ps->mLastRQ = RQG_Hud4+1;
	ps->setVisibilityMask(RV_Hud + RV_Particles);
#endif
}


//  🪄 Create Compositor  main render setup
//-----------------------------------------------------------------------------------------
TextureGpu* AppGui::CreateCompositor(int view, int splits, float width, float height)
{
	const auto inp = TextureDefinitionBase::TEXTURE_INPUT;
	//  cfg
	const bool refract = pSet->g.water_refract,
		ssao = pSet->g.ssao, hdr = pSet->g.hdr,
		lens = pSet->g.lens_flare, sunbeams = pSet->g.sunbeams,
		split = splits > 1,
		msaa = mWindow->getSampleDescription().isMultisample();

	//  log
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
	//  🪟 5 Combine last, final wnd
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
			td->setNumPasses( 1 + nHudGui );  //* passes

			//  render Window  ----
			{
				auto* pq = AddQuad(td);  // + quad
				pq->setAllLoadActions( LoadAction::DontCare );

				pq->mMaterialName = getSplitMtr(splits);

				for (int i=0; i < splits; ++i)
					pq->addQuadTextureSource( i, "rtt_FullIn"+toStr(i) );  // input
				pq->mProfilingId = "Combine to Window";

				//  ⏲️ Hud, 🎛️ Gui  --------
				AddHudGui(td);  // +
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

	//  node  🪩 New Refract  * * *
	//------------------------------------------------------------------------------------------------------------------------------------------
	if (refract)
	{
		if (split)
			rtt = AddSplitRTT(si, width, height);
		
	// 0  s0_ssao  Pre render  ------------------------------------------------------------
		//  own node, for more control of render RQG, no pipe glass, etc
		//  could be less Fps, renders more tris, but has no artifacts
		//  splitting render of pipe glass messed up depth of fluids
		if (ssao)
		{	nd = AddNode(s0_ssao+si);  //++ node
			
			nd->setNumLocalTextureDefinitions( 4 );  //* textures
			{
				auto* td = nd->addTextureDefinition( "rtt_ssao" );  // color not needed..
				td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  // half
				td->format = PFG_UNKNOWN;  td->fsaa = "1";  // off

				td = nd->addTextureDefinition( "depthHalf" );
				td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  // half
				td->format = PFG_D32_FLOAT;  td->fsaa = "1";  // d

				td = nd->addTextureDefinition( "gNormals" );
				td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  // half
				td->format = PFG_R10G10B10A2_UNORM;  td->fsaa = "1";  // n
				// td->textureFlags |= TextureFlags::MsaaExplicitResolve;  //-?

				td = nd->addTextureDefinition( "gFog" );
				td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  // half
				td->format = PFG_R16_FLOAT;  td->fsaa = "1";  // r  R32-

				AddRtv(nd, "rtt_ssao", "rtt_ssao", "depthHalf", "gNormals", "gFog");
				// AddRtv(nd, "rtt_ssao", "", "depthHalf", "gNormals", "gFog");  //bugs-
			}
			nd->mCustomIdentifier = "0-ssao-pre-"+si;
			
			nd->setNumTargetPass( 1 );  //* targets
			td = nd->addTargetPass( "rtt_ssao" );
			td->setNumPasses(1);  //* passes
			{
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Clear );
				ps->mClearColour[0] = ColourValue(0.2, 0.4, 0.6, 1.0);  // clr
				ps->mClearColour[1] = ColourValue(0.5, 0.5, 1.0, 1.0);  // norm

				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store;
				ps->mStoreActionStencil = StoreAction::DontCare;
				// ps->lod_update_list off

				ps->mProfilingId = "Pre Ssao-"+si;  // Opaque only, no pipe glass
				ps->mIdentifier = 10001;  //?
				ps->mFirstRQ = RQG_Sky+1;  // no sky
				ps->mLastRQ = RQG_Grass+1;  // RQG_Hud3+1;  trail?-
				ps->setVisibilityMask(
					RV_Terrain | RV_Road |
					RV_Vegetation | RV_VegetGrass | 
					RV_Objects | (refract ? 0 : RV_Fluid) |
					RV_Car | RV_CarGlass |  // | RV_Particles
					RV_Hud3D[plr]);  // trail
				
				ps->mGenNormalsGBuf = true;
				// no shadows
				ps->mEnableForwardPlus = 0;
			}
			nd->setNumOutputChannels( 3 );  //  out>  vvv
			nd->mapOutputChannel(0, "gNormals");
			nd->mapOutputChannel(1, "depthHalf");
			nd->mapOutputChannel(2, "gFog" );
		}

	// 1  s1_First  Render  ------------------------------------------------------------
		{	nd = AddNode(s1_first+si);  //++ node

			if (ssao)
			{	nd->addTextureSourceName("gNormals", 0, inp);  //  >in  ^^^
				nd->addTextureSourceName("depthHalf", 1, inp);
				nd->addTextureSourceName("gFog", 2, inp);
			}
			nd->setNumLocalTextureDefinitions( (ssao ? 4 : 0) + 2 );  //* textures
			{
				auto* td = nd->addTextureDefinition( "rtt_first" );  // + 2
				td->format = hdr ? PFG_RGBA16_FLOAT : PFG_UNKNOWN;  td->fsaa = "";
				// target_format  // PFG_RGBA8_UNORM_SRGB ^

				td = nd->addTextureDefinition( "depthBuffer" );
				td->format = PFG_D32_FLOAT;  td->fsaa = "";  // auto
				// td->textureFlags = TextureFlags::RenderToTexture;  //- no discard between frames

				AddRtv(nd, "rtt_first", "rtt_first", "depthBuffer"); //, ssao ? "gNormals" : "", ssao ? "gFog" : "");
				if (ssao)
				{
					td = nd->addTextureDefinition( "ssaoTexture" );
					td->widthFactor = 0.5f;  td->heightFactor = 0.5f;  //par HQ?, todo: 2nd ssao?
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
			
			nd->setNumTargetPass( (ssao ? 4 : 0) + 1 );  //* targets
			td = nd->addTargetPass( "rtt_first" );
			td->setNumPasses(1);  //* passes
			{
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Clear );
				ps->mClearColour[0] = ColourValue(0.2, 0.4, 0.6, 1.0);  // clr

				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store;
				ps->mStoreActionStencil = StoreAction::DontCare;
				// ps->lod_update_list off  //?

				ps->mProfilingId = "Render First-"+si;  // ⛰️ Opaque only
				ps->mIdentifier = 10001;
				ps->mLastRQ = RQG_PipeGlass;  // before
				ps->setVisibilityMask(RV_view);
				
				// ps->mExposedTextures.push_back("") mCubeReflTex  // todo.. add

				AddShadows(ps);  // shadows
			}

			if (ssao)  //  SSAO  + 4  ----------------
			{
				const String depthCopy = "depthHalf";

				td = nd->addTargetPass( "ssaoTexture" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::Clear );
					pq->mClearColour[0] = ColourValue::White;

					pq->mMaterialName = "SSAO/HS";  pq->mProfilingId = "SSAO 1 HS";
					pq->addQuadTextureSource( 0, depthCopy );  // input
					pq->addQuadTextureSource( 1, "gNormals" );
					pq->mFrustumCorners = CompositorPassQuadDef::VIEW_SPACE_CORNERS;
					// quad_normals  camera_far_corners_view_space^
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
					pq->addQuadTextureSource( 2, "gFog" );
					// pq->addQuadTextureSource( 0, "ssaoTexture" );  //* test noisy
					// pq->addQuadTextureSource( 1, "gNormals" );  //* test normals
				}
			}
			nd->setNumOutputChannels( 2 );  //  out>
			nd->mapOutputChannel(0, ssao ? "ssaoApply" : "rtt_first");
			nd->mapOutputChannel(1, "depthBuffer");
		}

	// 2  s2_depth  resolve / copy Depth  ------------------------------
		{	nd = AddNode(s2_depth+si);  //++ node
			
			nd->addTextureSourceName("gBufferDB", 0, inp);  //  >in

			nd->setNumLocalTextureDefinitions(1);  //* textures
			{
				auto* td = nd->addTextureDefinition( "resolvedDB" );
				td->format = PFG_R32_FLOAT;  td->fsaa = "1";  // D32 -> R32  off
				AddRtv(nd, "resolvedDB", "resolvedDB", "");
			}
			nd->mCustomIdentifier = "2-depth-"+si;

			nd->setNumTargetPass(1);  //* targets
			td = nd->addTargetPass( "resolvedDB" );
			td->setNumPasses(1);  //* passes
			{
				//  We need to "downsample/resolve" DepthBuffer because the impact
				//  performance on Refractions is gigantic
				if (msaa)
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );
					pq->setAllStoreActions( StoreAction::DontCare );
					pq->mStoreActionColour[0] = StoreAction::StoreOrResolve;

					pq->mMaterialName = "Ogre/Resolve/1xFP32_Subsample0";  pq->mProfilingId = "Depth Resolve";
					pq->addQuadTextureSource( 0, "gBufferDB" );  // input
				}
				else  // need to copy, no fsaa, since we write and read from this copy
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );
					pq->setAllStoreActions( StoreAction::DontCare );
					pq->mStoreActionColour[0] = StoreAction::StoreOrResolve;
					
					pq->mMaterialName = "Ogre/Copy/1xFP32";  pq->mProfilingId = "Depth copy";
					pq->addQuadTextureSource( 0, "gBufferDB" );  // input
				}
			}
			nd->setNumOutputChannels(1);  //  out>
			nd->mapOutputChannel(0, "resolvedDB");
		}

	// 3  s3_Final  🪩 Final Refractive  ------------------------------------------------------------
		{	nd = AddNode(s3_Final+si);  //++ node
			
			nd->addTextureSourceName("rtt_FullOut", 0, inp);  // or rt_renderwindow
			nd->addTextureSourceName("rrt_firstIn", 1, inp);  //  >in  ssaoApply or rtt_first
			nd->addTextureSourceName("depthBuffer", 2, inp);
			nd->addTextureSourceName("depthBufferNoMsaa", 3, inp);

			const int post = (lens ? 1 : 0) + (sunbeams ? 1 : 0);
			
			nd->setNumLocalTextureDefinitions( post + 1 );  //* textures
			{
				auto* td = nd->addTextureDefinition( "rtt_final" );
				td->format = PFG_UNKNOWN;  td->fsaa = "";  // target_format, auto
				AddRtv(nd, "rtt_final", "rtt_final", "depthBuffer");
			if (lens)
			{	auto* td = nd->addTextureDefinition( "rtt_lens" );
				td->format = PFG_UNKNOWN;  td->fsaa = "";
				AddRtv(nd, "rtt_lens", "rtt_lens", "depthBuffer");
			}
			if (sunbeams)
			{	auto* td = nd->addTextureDefinition( "rtt_beams" );
				td->format = PFG_UNKNOWN;  td->fsaa = "";
				AddRtv(nd, "rtt_beams", "rtt_beams", "depthBuffer");
			}	}

			nd->mCustomIdentifier = "3-Final-"+si;

			nd->setNumTargetPass( post + 2 );  //* targets

			//  🌊 Final  ----
			td = nd->addTargetPass( "rtt_final" );
			td->setNumPasses(3);  //* passes
			{
				//  Perform exact copy (MSAA-preserving) so we can continue rendering
				//  into rtt_final. Meanwhile rtt_first will be used for sampling refractions
				auto* cp = static_cast<CompositorPassDepthCopyDef*>(
					td->addPass(Ogre::PASS_DEPTHCOPY));  // + copy
				cp->setDepthTextureCopy("rrt_firstIn", "rtt_final");
				cp->mProfilingId = "copy first to final";

				//  🌊 Refracted  Fluids
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Load );
				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store; //DontCare;
				ps->mStoreActionStencil = StoreAction::DontCare;

				ps->mProfilingId = "Refractive Fluids";  ps->mIdentifier = 10002;
				ps->mFirstRQ = RQG_Fluid;  ps->mLastRQ = RQG_Refract+1;
				ps->setVisibilityMask(RV_Fluid);  // 🌊
				
				// ps->mEnableForwardPlus = 0;  //?
				AddShadows(ps);  // shadows
				ps->mShadowNodeRecalculation = SHADOW_NODE_REUSE;  //`
				ps->setUseRefractions("depthBufferNoMsaa", "rrt_firstIn");  // ~

				//  ⭕ glass pipes, car glass, 💭 particles, pacenotes
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Load );
				ps->mStoreActionColour[0] = StoreAction::StoreOrResolve;
				ps->mStoreActionDepth = StoreAction::Store; //DontCare;
				ps->mStoreActionStencil = StoreAction::DontCare;

				ps->mProfilingId = "Particles";  ps->mIdentifier = 10001;
				ps->mFirstRQ = RQG_PipeGlass;  ps->mLastRQ = RQG_Hud3+1;
				ps->setVisibilityMask(
					RV_CarGlass |
					RV_Road | RV_Vegetation |  // transparent only
					RV_Hud3D[plr] | RV_Particles );
				
				// ps->mEnableForwardPlus = 0;  //?
				AddShadows(ps);  // no shadows
				ps->mShadowNodeRecalculation = SHADOW_NODE_REUSE;  //`
			}
			String final = "rtt_final";  // last rtt


			//  🌅 Hdr  ----------------
			if (hdr)
			{
				// todo: add
			}

			//  🔆 Lens flare  ----------------
			if (lens)
			{
				td = nd->addTargetPass( "rtt_lens" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );
					
					pq->mMaterialName = "LensFlare";  pq->mProfilingId = "Lens Flare";  // input
					pq->addQuadTextureSource( 0, "depthBufferNoMsaa" );
					pq->addQuadTextureSource( 1, final );
				}
				final = "rtt_lens";
			}

			//  🌄 Sunbeams  ----------------
			if (sunbeams)
			{
				td = nd->addTargetPass( "rtt_beams" );
				td->setNumPasses( 1 );  //* passes
				{
					auto* pq = AddQuad(td);  // + quad
					pq->setAllLoadActions( LoadAction::DontCare );
					
					pq->mMaterialName = "SunBeams";  pq->mProfilingId = "Sunbeams";  // input
					pq->addQuadTextureSource( 0, "depthBufferNoMsaa" );
					pq->addQuadTextureSource( 1, final );
				}
				final = "rtt_beams";
			}

			//  render / Window  ----
			td = nd->addTargetPass( "rtt_FullOut" );
			td->setNumPasses( 1 + (!split ? nHudGui : 0) );  //* passes
			{
				auto* pq = AddQuad(td);  // + quad
				pq->setAllLoadActions( LoadAction::DontCare );
				
				pq->mMaterialName = "Ogre/Copy/4xFP32";  // needed? merge above-
				pq->mProfilingId = "Copy to Window";
				pq->addQuadTextureSource( 0, final );  // input

				//  ⏲️ Hud, 🎛️ Gui  --------
				if (!split)
					AddHudGui(td);  // +
			}
		}

		//  workspace  ----------------
		{
			wd = AddWork( sWork+si );
			wd->connectExternal( 0, s3_Final+si, 0 );  // rtt_FullOut  or  rt_renderwindow
			//  ssao pre render
			if (ssao)
			{	wd->connect( s0_ssao+si, 0, s1_first+si, 0 );  // gNormals
				wd->connect( s0_ssao+si, 1, s1_first+si, 1 );  // depthHalf
				wd->connect( s0_ssao+si, 2, s1_first+si, 2 );  // gFog
			}
			//  main
			wd->connect( s1_first+si, 0, s3_Final+si, 1 );  // rtt_first
			wd->connect( s1_first+si, 1, s3_Final+si, 2 );  // depthBuffer
			wd->connect( s1_first+si, 1, s2_depth+si, 0 );  // depthBuffer -> gBufferDB
			wd->connect( s2_depth+si, 0, s3_Final+si, 3 );  // resolvedDB -> depthBufferNoMsaa
		}
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	else  //  node  🖥️ Old  no refract, no depth, no effects  - - -
	{						// meh todo ssao w/o refract
		if (split)
			rtt = AddSplitRTT(si, width, height);

		{	nd = AddNode(sNode+si);  //++ node
			
			//  render window input
			nd->addTextureSourceName("rt_wnd", 0, inp);  // in  wnd

			nd->mCustomIdentifier = "old-1";

			nd->setNumTargetPass( 1 + (!split ? nHudGui : 0) );  //* targets

			td = nd->addTargetPass( "rt_wnd" );
			{
				ps = AddScene(td);  // + scene
				ps->setAllLoadActions( LoadAction::Clear );
				ps->setAllStoreActions( StoreAction::Store );

				ps->mProfilingId = "Render Old";  ps->mIdentifier = 21002;
				// ps->mLastRQ = 199;  //RQG_Fluid-1  // all

				AddShadows(ps);  // shadows
			}
			//  ⏲️ Hud, 🎛️ Gui  --------
			if (!split)
				AddHudGui(td);  // +

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
//  🪄 Setup Compositors All  🪟 splits etc
//-----------------------------------------------------------------------------------------
void AppGui::SetupCompositors()
{
	LogO("CC## Setup Compositor    "+getWsInfo());
	auto* mgr = mRoot->getCompositorManager2();
	const bool hdr = pSet->g.hdr;
	
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
		IdString wsName;
		if (!hdr)
		{
			CreateCompositor(-1, 1,  1.f, 1.f);
			wsName = sWork + "0";
		}
		else  // test hdr
		{
			CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
			RenderSystem *renderSystem = mRoot->getRenderSystem();
			const RenderSystemCapabilities *caps = renderSystem->getCapabilities();

			wsName = "HdrWorkspace";
			if (mWindow->isMultisample() && caps->hasCapability( RSC_EXPLICIT_FSAA_RESOLVE ))
				wsName = "HdrWorkspaceMsaa";
		}

		//  render Window external channels  ----
		CompositorChannelVec chWnd( mCubeReflTex ? 2 : 1 );
		chWnd[0] = mWindow->getTexture();
		if (mCubeReflTex)
			chWnd[1] = mCubeReflTex;  // 🔮

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
