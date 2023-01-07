#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "AppGui.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindow.h>

#include <OgreTextureGpuManager.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsManager.h>

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
using namespace Ogre;


//  🔮 create cube Reflections tex,workspace
//-----------------------------------------------------------------------------------------
void AppGui::CreateCubeReflect()
{
	auto* rndSys = mRoot->getRenderSystem();
	auto* texMgr = rndSys->getTextureGpuManager();
	auto* mgr = mRoot->getCompositorManager2();

	// mIblQuality = IblLow;  //par..
	// mIblQuality = IblMedium;

	// A RenderTarget created with AllowAutomipmaps means the compositor still needs to
	// explicitly generate the mipmaps by calling generate_mipmaps.

	uint32 iblSpecularFlag = 0;
	if( rndSys->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM ) &&
		mIblQuality != MipmapsLowest )
	{
		iblSpecularFlag = TextureFlags::Uav | TextureFlags::Reinterpretable;
	}

	/*TextureGpu* reflTex = texMgr->findTextureNoThrow("DynamicCubemap");
	if (reflTex)
		texMgr->destroyTexture(reflTex);*/

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

	int mips = PixelFormatGpuUtils::getMaxMipmapCount( resolution );
	auto curmips = mCubeReflTex->getNumMipmaps();
	
	// if (curmips != mips)
	// if( mIblQuality != MipmapsLowest )
		// mCubeReflTex->setNumMipmaps( mips );  // no change-
		mCubeReflTex->setNumMipmaps( mips - 4u );  //par Limit max mipmap to 16x16
	
	mCubeReflTex->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
	mCubeReflTex->scheduleTransitionTo( GpuResidency::Resident );


	HlmsManager *hlmsMgr = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs *>( hlmsMgr->getHlms( HLMS_PBS ) ) );
	HlmsPbs *hlmsPbs = static_cast<HlmsPbs *>( hlmsMgr->getHlms( HLMS_PBS ) );
	// hlmsPbs->resetIblSpecMipmap( 0u );  // auto, no-
	hlmsPbs->resetIblSpecMipmap( mips - 4u );  //+

	//  Create camera used to render to cubemap reflections
	if (mCubeCamera)
	{	mSceneMgr->destroyCamera(mCubeCamera);  mCubeCamera = 0;  }
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
		// mCubeCamera->setFarClipDistance( 100 );  // todo: !
		mCubeCamera->setFarClipDistance( pSet->view_distance );
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
	CompositorChannelVec chan( 1 );
	chan[0] = mCubeReflTex;

	const String name( "SR3_ReflCubemap" );  // created from code
	if( !mgr->hasWorkspaceDefinition( name ) )
	{
		LogO("REFL ws add");
		auto* w = mgr->addWorkspaceDefinition( name );
		w->connectExternal( 0, idCubeNode, 0 );
	}

	auto* mWorkspace = mgr->addWorkspace(
		mSceneMgr, chan, mCubeCamera, name, true );
	mWorkspaces.push_back(mWorkspace);  //+
}
