#include "pch.h"
#include "RenderConst.h"
#include "CGame.h"

#include "CameraController.h"
#include "GraphicsSystem.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreWindow.h"

#include "OgreTextureGpuManager.h"
#include "OgrePixelFormatGpuUtils.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"

// #include "OgreAtmosphere2Npr.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h"

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;


//-----------------------------------------------------------------------------------
CompositorWorkspace *App::setupCompositor()
{
	// We first create the Cubemap workspace and pass it to the final workspace
	// that does the real rendering.
	//
	// If in your application you need to create a workspace but don't have a cubemap yet,
	// you can either programatically modify the workspace definition (which is cumbersome)
	// or just pass a PF_NULL texture that works as a dud and barely consumes any memory.
	// See Tutorial_Terrain for an example of PF_NULL dud.

	Root *root = mGraphicsSystem->getRoot();
	SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
	Window *renderWindow = mGraphicsSystem->getRenderWindow();
	Camera *camera = mGraphicsSystem->getCamera();
	CompositorManager2 *compositorManager = root->getCompositorManager2();

	if( mDynamicCubemapWorkspace )
	{
		compositorManager->removeWorkspace( mDynamicCubemapWorkspace );
		mDynamicCubemapWorkspace = 0;
	}

	uint32 iblSpecularFlag = 0;
	if( root->getRenderSystem()->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM ) &&
		mIblQuality != MipmapsLowest )
	{
		iblSpecularFlag = TextureFlags::Uav | TextureFlags::Reinterpretable;
	}

	// A RenderTarget created with AllowAutomipmaps means the compositor still needs to
	// explicitly generate the mipmaps by calling generate_mipmaps. It's just an API
	// hint to tell the GPU we will be using the mipmaps auto generation routines.
	TextureGpuManager *textureManager = root->getRenderSystem()->getTextureGpuManager();
	mDynamicCubemap = textureManager->createOrRetrieveTexture( "DynamicCubemap",
		GpuPageOutStrategy::Discard,
		TextureFlags::RenderToTexture | TextureFlags::AllowAutomipmaps | iblSpecularFlag,
		TextureTypes::TypeCube );
	mDynamicCubemap->scheduleTransitionTo( GpuResidency::OnStorage );

	uint32 resolution = 512u;
	if( mIblQuality == MipmapsLowest )
		resolution = 1024u;
	else if( mIblQuality == IblLow )
		resolution = 256u;
	else
		resolution = 512u;
	mDynamicCubemap->setResolution( resolution, resolution );
	mDynamicCubemap->setNumMipmaps( PixelFormatGpuUtils::getMaxMipmapCount( resolution ) );
	if( mIblQuality != MipmapsLowest )
	{
		// Limit max mipmap to 16x16
		mDynamicCubemap->setNumMipmaps( mDynamicCubemap->getNumMipmaps() - 4u );
	}
	mDynamicCubemap->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
	mDynamicCubemap->scheduleTransitionTo( GpuResidency::Resident );


	Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
	assert( dynamic_cast<Ogre::HlmsPbs *>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
	Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs *>( hlmsManager->getHlms( Ogre::HLMS_PBS ) );
	hlmsPbs->resetIblSpecMipmap( 0u );

	// Create the camera used to render to our cubemap
	if( !mCubeCamera )
	{
		mCubeCamera = sceneManager->createCamera( "CubeMapCamera", true, true );
		mCubeCamera->setFOVy( Degree(90) );  mCubeCamera->setAspectRatio( 1 );
		mCubeCamera->setFixedYawAxis( false );
		mCubeCamera->setPosition( 0, 1.0, 0 );  // upd in car
		mCubeCamera->setNearClipDistance( 0.1 );
		// mCubeCamera->setUseRenderingDistance(true);
		// mCubeCamera->_setRenderedRqs(0, 200);
		mCubeCamera->setVisibilityFlags( RV_MaskReflect );
		// mCubeCamera->setDefaultVisibilityFlags( RV_Sky);  //** set in cubemap_target

		mCubeCamera->setFarClipDistance( 20000 ); //300 );  // par  20000 needed for sky meh..
		// mCubeCamera->setShadowRenderingDistance( 100 );  // par?-
		//mCubeCamera->setCastShadows(false);
	}

	// No need to tie RenderWindow's use of MSAA with cubemap's MSAA. Could never use MSAA for cubemap.
	const IdString cubemapRendererNode = /*renderWindow->getSampleDescription().isMultisample()
		? "CubemapRendererNodeMsaa" :*/ "CubemapRendererNode";
	{
		CompositorNodeDef *nodeDef = compositorManager->getNodeDefinitionNonConst( cubemapRendererNode );
		const CompositorPassDefVec &passes =
			nodeDef->getTargetPass( nodeDef->getNumTargetPasses() - 1u )->getCompositorPasses();

		OGRE_ASSERT_HIGH( dynamic_cast<CompositorPassIblSpecularDef *>( passes.back() ) );
		CompositorPassIblSpecularDef *iblSpecPassDef =
			static_cast<CompositorPassIblSpecularDef *>( passes.back() );
		iblSpecPassDef->mForceMipmapFallback = mIblQuality == MipmapsLowest;
		iblSpecPassDef->mSamplesPerIteration = mIblQuality == IblLow ? 32.0f : 128.0f;
		iblSpecPassDef->mSamplesSingleIterationFallback = iblSpecPassDef->mSamplesPerIteration;
	}


	//  Setup the cubemap's compositor
	CompositorChannelVec cubemapExternalChannels( 1 );
	cubemapExternalChannels[0] = mDynamicCubemap;

	const Ogre::String workspaceName( "Tutorial_DynamicCubemap_cubemap" );  // created from code
	if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
	{
		CompositorWorkspaceDef *workspaceDef = compositorManager->addWorkspaceDefinition( workspaceName );
		workspaceDef->connectExternal( 0, cubemapRendererNode, 0 );
	}

	mDynamicCubemapWorkspace = compositorManager->addWorkspace(
		sceneManager, cubemapExternalChannels, mCubeCamera,
		workspaceName, true );


	//  Now setup the regular Render window
	CompositorChannelVec externalChannels( 2 );
	externalChannels[0] = renderWindow->getTexture();
	externalChannels[1] = mDynamicCubemap;

	
	//  Gui, add MyGUI pass
	CompositorNodeDef * def = compositorManager->getNodeDefinitionNonConst("Tutorial_TerrainRenderingNode");
	CompositorTargetDef *tar = def->getTargetPass(0);
	tar->addPass(PASS_CUSTOM, MyGUI::OgreCompositorPassProvider::mPassId);
	

	CompositorWorkspace* wrk = compositorManager->addWorkspace(
		sceneManager, externalChannels, camera,
		"Tutorial_TerrainWorkspace", true );  // in .compositor
	return wrk;
}
