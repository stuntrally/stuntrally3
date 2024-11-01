#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "AppGui.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindow.h>

#include <OgreTextureGpuManager.h>
#include "HlmsPbs2.h"
#include <OgreHlmsManager.h>

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
using namespace Ogre;


//  🔮 re/create cube Reflections tex,workspace
//-----------------------------------------------------------------------------------------
void AppGui::CreateCubeReflect()
{
	LogO("C:## CreateCubeReflect "+getWsInfo());
	iReflStart = 0;
	auto* rndSys = mRoot->getRenderSystem();
	auto* texMgr = rndSys->getTextureGpuManager();
	auto* mgr = mRoot->getCompositorManager2();
	bool lowest = false;  // todo: par?

	//  A RenderTarget created with AllowAutomipmaps means the compositor still needs to
	//  explicitly generate the mipmaps by calling generate_mipmaps
	uint32 iblSpecularFlag = 0;
	if( rndSys->getCapabilities()->hasCapability( RSC_COMPUTE_PROGRAM )
		&& !lowest )
	{
		iblSpecularFlag = TextureFlags::Uav | TextureFlags::Reinterpretable;
	}

	/*TextureGpu* reflTex = texMgr->findTextureNoThrow("DynamicCubemap");
	if (reflTex)
		texMgr->destroyTexture(reflTex);*/  //?-

	//  🖼️ create rtt tex
	mCubeReflTex = texMgr->createOrRetrieveTexture( "DynamicCubemap",
		GpuPageOutStrategy::Discard,
		TextureFlags::RenderToTexture | TextureFlags::AllowAutomipmaps | iblSpecularFlag,
		TextureTypes::TypeCube );
	mCubeReflTex->scheduleTransitionTo( GpuResidency::OnStorage );

	uint32 size = cTexSizes[pSet->g.refl_size];  // from set
	mCubeReflTex->setResolution( size, size );

	int mips = PixelFormatGpuUtils::getMaxMipmapCount( size );
	auto curmips = mCubeReflTex->getNumMipmaps();
	
	// if (curmips != mips)
	// if( !lowest )
		// mCubeReflTex->setNumMipmaps( mips );  // no change-
	mCubeReflTex->setNumMipmaps( mips - 4u );  //par Limit max mipmap to 16x16
	
	mCubeReflTex->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
	mCubeReflTex->scheduleTransitionTo( GpuResidency::Resident );

	//  ibl mips
	HlmsManager *hlmsMgr = mRoot->getHlmsManager();
	assert( dynamic_cast<HlmsPbs2 *>( hlmsMgr->getHlms( HLMS_PBS ) ) );
	HlmsPbs2 *hlmsPbs2 = static_cast<HlmsPbs2 *>( hlmsMgr->getHlms( HLMS_PBS ) );
	// hlmsPbs2->resetIblSpecMipmap( 0u );  // auto, no-
	hlmsPbs2->resetIblSpecMipmap( mips - 4u );  //+


	//  🎥 create camera  used to render to cubemap
	if (mCubeCamera)
	{	mSceneMgr->destroyCamera(mCubeCamera);  mCubeCamera = 0;  }

	mCubeCamera = mSceneMgr->createCamera( "CubemapCam", true, true );
	mCubeCamera->setFOVy( Degree(90) );  mCubeCamera->setAspectRatio( 1 );
	mCubeCamera->setFixedYawAxis( false );
	mCubeCamera->setPosition( 0, 1.0, 0 );  // upd in car
	mCubeCamera->setNearClipDistance( 0.1 );
	
	// mCubeCamera->setUseRenderingDistance(true);
	// mCubeCamera->_setRenderedRqs(0, 200);
	mCubeCamera->setVisibilityFlags( RV_MaskReflect );
	// mCubeCamera->setDefaultVisibilityFlags( RV_Sky );  //** set in cubemap_target
	//? mCubeCamera->setVrData(VrData

	//  📊 graphics params 1 :
	// mCubeCamera->setFarClipDistance( pSet->view_distance );
	mCubeCamera->setFarClipDistance( pSet->g.refl_dist );  // todo: 2nd sky!
	mCubeCamera->setLodBias( pSet->g.refl_lod );  // par..
	// mCubeCamera->setShadowRenderingDistance( 300 );  // par ?-
	// mCubeCamera->setCastShadows(true);


	//  🪄 node def
	//  No need to tie RenderWindow's use of MSAA with cubemap's MSAA
	const IdString idCubeNode =
		//mWindow->getSampleDescription().isMultisample() ? "CubemapNodeMsaa" :
		"CubemapNode";  // never use MSAA for cubemap

	CompositorNodeDef *nodeDef = mgr->getNodeDefinitionNonConst( idCubeNode );
	const CompositorPassDefVec &passes =
		nodeDef->getTargetPass( nodeDef->getNumTargetPasses() - 1u )->getCompositorPasses();

	OGRE_ASSERT_HIGH( dynamic_cast<CompositorPassIblSpecularDef *>( passes.back() ) );
	CompositorPassIblSpecularDef *iblPassDef =
		static_cast<CompositorPassIblSpecularDef *>( passes.back() );
	
	//  📊 graphics params 2 :
	iblPassDef->mForceMipmapFallback = lowest; //mIblQuality == MipmapsLowest;  // todo: par?
	float ibl = 1u << size_t(pSet->g.refl_ibl);
	LogO("IBL: "+fToStr(ibl));
	iblPassDef->mSamplesPerIteration = ibl;  // mIblQuality == IblLow ? 32.0f : 128.0f;
	iblPassDef->mSamplesSingleIterationFallback = iblPassDef->mSamplesPerIteration;


	//  face skip mask  ----
	const size_t num = nodeDef->getNumTargetPasses();
	assert( num == 6u && "Target face passes not 6" );
	// LogO("Refl target passes: "+toStr(numTargetPasses));

	for (size_t faceIdx = 0u; faceIdx < num; ++faceIdx)
	{
		const auto* targetDef = nodeDef->getTargetPass( faceIdx );
		const auto& passDefs = targetDef->getCompositorPasses();

		for (auto* passDef : passDefs)
			passDef->mExecutionMask = 1u << faceIdx;
	}


	//  🔮 Cubemap's compositor channels  ----
	CompositorChannelVec chan( 1 );
	chan[0] = mCubeReflTex;

	const String name( "SR3_ReflCubemap" );  // created from code
	if (mgr->hasWorkspaceDefinition(name))
		mgr->removeWorkspaceDefinition(name);
	{
		auto* w = mgr->addWorkspaceDefinition( name );
		w->connectExternal( 0, idCubeNode, 0 );
	}

	//  add Workspace
	auto* ws = mgr->addWorkspace(
		mSceneMgr, chan, mCubeCamera, name, false );  // manual update
	vWorkspaces.push_back(ws);  //+ to destroy
	LogO("--++ WS add:  Reflect Cube, "+getWsInfo());

	wsCubeRefl = ws;
	iReflSkip = 0;
}


void AppGui::ApplyReflect()
{
	for (auto* db2 : vDbRefl)
	{
		db2->setTexture( PBSM_REFLECTION, mCubeReflTex );
	}
}


//  🔮💫 update  each frame try
void AppGui::UpdCubeRefl()
{
	if (!wsCubeRefl)  return;
	++iReflStart;
	if (iReflStart < 4)  return;  //? crash in cullPhase01 w/o

	//  skip whole update
	if (iReflSkip++ < pSet->g.refl_skip)
		return;
	iReflSkip = 0;


	//  upd faces mask  ----
	static uint8_t iFace = 0;
	uint8_t mask = 0;

	auto addFace = [&]()
	{
		if (iFace >= 6)
			iFace = 0;
		mask += 1u << iFace;
		++iFace;
	};

	if (pSet->g.refl_faces == 6)
		wsCubeRefl->setExecutionMask( 0xFF );
	else
	{	for (int i=0; i < pSet->g.refl_faces; ++i)
			addFace();
		wsCubeRefl->setExecutionMask(mask);
	}

	//  update, render cube face(s)
	wsCubeRefl->_beginUpdate( true );
	wsCubeRefl->_update();
	wsCubeRefl->_endUpdate( true );

	ApplyReflect();  //**
}
