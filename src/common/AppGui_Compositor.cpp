#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "App.h"
#include "GraphicsSystem.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>

#include <OgreTextureGpuManager.h>
// #include <OgrePixelFormatGpuUtils.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsManager.h>

// #include <OgreAtmosphere2Npr.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
using namespace Ogre;


//  🪄 Setup Compositor
//-----------------------------------------------------------------------------------
CompositorWorkspace* AppGui::SetupCompositor()
{
	LogO("C### setup Compositor");
	// We first create the Cubemap workspace and pass it to the final workspace
	// that does the real rendering.
	//
	// If in your application you need to create a workspace but don't have a cubemap yet,
	// you can either programatically modify the workspace definition (which is cumbersome)
	// or just pass a PF_NULL texture that works as a dud and barely consumes any memory.
	// See Tutorial_Terrain for an example of PF_NULL dud.

	auto* rndSys = mRoot->getRenderSystem();
	auto* texMgr = rndSys->getTextureGpuManager();
	auto* mgr = mRoot->getCompositorManager2();
	
	//  destroy old
	LogO("D### setup Compositor rem workspaces: "+ toStr(mWorkspaces.size()));
	for (auto ws : mWorkspaces)
	{
		// LogO("D### setup Compositor rem workspace");
		mgr->removeWorkspace( ws );
		// ws = 0;
	}
	mWorkspaces.clear();


	// A RenderTarget created with AllowAutomipmaps means the compositor still needs to
	// explicitly generate the mipmaps by calling generate_mipmaps.

	//  🔮 cube Reflection
	uint32 iblSpecularFlag = 0;
	if( rndSys->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM ) &&
		mIblQuality != MipmapsLowest )
	{
		iblSpecularFlag = TextureFlags::Uav | TextureFlags::Reinterpretable;
	}

	mCubeReflTex = texMgr->createOrRetrieveTexture( "DynamicCubemap",
		GpuPageOutStrategy::Discard,
		TextureFlags::RenderToTexture | TextureFlags::AllowAutomipmaps | iblSpecularFlag,
		TextureTypes::TypeCube );
	mCubeReflTex->scheduleTransitionTo( GpuResidency::OnStorage );

	uint32 resolution = 512u;  //?
	if( mIblQuality == MipmapsLowest || mIblQuality == IblMedium)
		resolution = 1024u;
	else if( mIblQuality == IblLow )
		resolution = 256u;
	else
		resolution = 512u;
	mCubeReflTex->setResolution( resolution, resolution );
	mCubeReflTex->setNumMipmaps( PixelFormatGpuUtils::getMaxMipmapCount( resolution ) );
	if( mIblQuality != MipmapsLowest )
	{
		// Limit max mipmap to 16x16 .. //** par
		// mCubeReflTex->setNumMipmaps( mDynamicCubemap->getNumMipmaps() - 4u );
		mCubeReflTex->setNumMipmaps( mCubeReflTex->getNumMipmaps() - 2u );  // -4u
		// mCubeReflTex->setNumMipmaps( mDynamicCubemap->getNumMipmaps() /*- 1u*/ );
	}
	mCubeReflTex->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
	mCubeReflTex->scheduleTransitionTo( GpuResidency::Resident );


	HlmsManager *hlmsMgr = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlmsMgr->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlmsMgr->getHlms( HLMS_PBS ) );
	hlmsPbs->resetIblSpecMipmap( 0u );  //..

	//  Create camera used to render to cubemap reflections
	if( !mCubeCamera )
	{
		mCubeCamera = mSceneMgr->createCamera( "CubemapCam", true, true );
		mCubeCamera->setFOVy( Degree(90) );  mCubeCamera->setAspectRatio( 1 );
		mCubeCamera->setFixedYawAxis( false );
		mCubeCamera->setPosition( 0, 1.0, 0 );  // upd in car
		mCubeCamera->setNearClipDistance( 0.1 );
		// mCubeCamera->setUseRenderingDistance(true);
		// mCubeCamera->_setRenderedRqs(0, 200);
		mCubeCamera->setVisibilityFlags( RV_MaskReflect );
		// mCubeCamera->setDefaultVisibilityFlags( RV_Sky);  //** set in cubemap_target
		//? mCubeCamera->setVrData(VrData
		mCubeCamera->setFarClipDistance( 20000 ); //300 );  // par  20000 needed for sky meh..
		// mCubeCamera->setShadowRenderingDistance( 300 );  // par?-
		// mCubeCamera->setCastShadows(true);
	}

	//  No need to tie RenderWindow's use of MSAA with cubemap's MSAA
	const IdString idCubeNode =
		//mWindow->getSampleDescription().isMultisample() ? "CubemapNodeMsaa" :
		"CubemapNode";  // never use MSAA for cubemap.
	{
		CompositorNodeDef *nodeDef = mgr->getNodeDefinitionNonConst( idCubeNode );
		const CompositorPassDefVec &passes =
			nodeDef->getTargetPass( nodeDef->getNumTargetPasses() - 1u )->getCompositorPasses();

		OGRE_ASSERT_HIGH( dynamic_cast<CompositorPassIblSpecularDef *>( passes.back() ) );
		CompositorPassIblSpecularDef *iblSpecPassDef =
			static_cast<CompositorPassIblSpecularDef *>( passes.back() );
		iblSpecPassDef->mForceMipmapFallback = mIblQuality == MipmapsLowest;
		iblSpecPassDef->mSamplesPerIteration = mIblQuality == IblLow ? 32.0f : mIblQuality == IblMedium ? 16.f : 128.0f;
		iblSpecPassDef->mSamplesSingleIterationFallback = iblSpecPassDef->mSamplesPerIteration;
	}


	//  🔮 Cubemap's compositor channels  ----
	CompositorChannelVec cubeExt( 1 );
	cubeExt[0] = mCubeReflTex;

	const String name( "SR3_ReflCubemap" );  // created from code
	if( !mgr->hasWorkspaceDefinition( name ) )
	{
		auto* w = mgr->addWorkspaceDefinition( name );
		w->connectExternal( 0, idCubeNode, 0 );
	}

	auto* mWorkspace = mgr->addWorkspace(
		mSceneMgr, cubeExt, mCubeCamera, name, true );
	mWorkspaces.push_back(mWorkspace);  //+?


	//  Render window external channels  ----
	CompositorChannelVec ext( 2 );
	ext[0] = mWindow->getTexture();
	ext[1] = mCubeReflTex;

	
	//  🎛️ Gui, add MyGUI pass
	CompositorNodeDef* node = mgr->getNodeDefinitionNonConst("SR3_Render");
	CompositorTargetDef* target = node->getTargetPass(0);
	target->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
	

	//  game
	const IdString wsName( "SR3_Workspace" );
#ifndef SR_EDITOR
	if (pSet->game.local_players > 1)
	{
		//  VR setup  ---- ---- ---- ----
		Camera* mCamera2 = mSceneMgr->createCamera( "Camera2" );

		mCamera2->setPosition( Vector3( 0, 51, 115 ) );
		// Look back along -Z
		mCamera2->lookAt( Vector3( 0, 0, 0 ) );
		mCamera2->setNearClipDistance( 0.2f );
		mCamera2->setFarClipDistance( 1000.0f );
		mCamera2->setAutoAspectRatio( true );


		Vector4 dims; //OffsetScale;

		Camera* mEyeCameras[2] = {mCamera, mCamera2 };  // temp

		dims = Vector4( 0.0f, 0.0f, 0.5f, 1.0f );
		CompositorWorkspace* mEyeWs[2];
		mEyeWs[0] =
			mgr->addWorkspace( mSceneMgr, ext,
				mEyeCameras[0], wsName,
				true, -1, 0, 0,
				dims, 0x01, 0x01 );

		dims = Vector4( 0.5f, 0.0f, 0.5f, 1.0f );
		mEyeWs[1] =
			mgr->addWorkspace( mSceneMgr, ext, 
				mEyeCameras[1], wsName,
				true, -1, 0, 0,
				dims, 0x02, 0x02 );

		mWorkspaces.push_back(mEyeWs[0]);
		mWorkspaces.push_back(mEyeWs[1]);

		LogO("C### Split Created workspaces: "+toStr(mWorkspaces.size()));
		return mEyeWs[0];
	}
	else  // single view
#endif
	{
		auto ws = mgr->addWorkspace( mSceneMgr, ext, mCamera, wsName, true );  // in .compositor
		mWorkspaces.push_back(ws);
		LogO("C### Single Created workspaces: "+toStr(mWorkspaces.size()));
		return ws;
	}
}
