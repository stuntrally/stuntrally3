#include "AppGui.h"
#include "GraphicsSystem.h"
#include "OgreWindow.h"
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayManager.h>

#include <OgreRoot.h>
#include <OgreHlmsManager.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlitDatablock.h>
#include <OgreHlmsCompute.h>

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorShadowNode.h>
#include <Compositor/OgreCompositorShadowNodeDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
#include "Utils/MiscUtils.h"
using namespace Ogre;
using namespace Demo;


void AppGui::setupESM()
{
	// For ESM, setup the filter settings (radius and gaussian deviation).
	// It controls how blurry the shadows will look.
	Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
	Ogre::HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();

	Ogre::uint8 kernelRadius = 8;
	float gaussianDeviationFactor = 0.5f;
	Ogre::uint16 K = 80;
	Ogre::HlmsComputeJob *job = 0;

	// Setup compute shader filter (faster for large kernels; but
	// beware of mobile hardware where compute shaders are slow)
	// For reference large kernels means kernelRadius > 2 (approx)
	job = hlmsCompute->findComputeJob( "ESM/GaussianLogFilterH" );
	MiscUtils::setGaussianLogFilterParams( job, kernelRadius, gaussianDeviationFactor, K );
	job = hlmsCompute->findComputeJob( "ESM/GaussianLogFilterV" );
	MiscUtils::setGaussianLogFilterParams( job, kernelRadius, gaussianDeviationFactor, K );

	// Setup pixel shader filter (faster for small kernels, also to use as a fallback
	// on GPUs that don't support compute shaders, or where compute shaders are slow).
	MiscUtils::setGaussianLogFilterParams( "ESM/GaussianLogFilterH", kernelRadius,
											gaussianDeviationFactor, K );
	MiscUtils::setGaussianLogFilterParams( "ESM/GaussianLogFilterV", kernelRadius,
											gaussianDeviationFactor, K );
}

//-----------------------------------------------------------------------------------
void AppGui::createPcfShadowNode()
{
	CompositorManager2 *mgr = mRoot->getCompositorManager2();
	RenderSystem *rs = mRoot->getRenderSystem();

	ShadowNodeHelper::ShadowParamVec shadowParams;

	ShadowNodeHelper::ShadowParam shadowParam;
	silent_memset( &shadowParam, 0, sizeof( shadowParam ) );

	// First light, directional
	// uint32 size1 = 8192u, size2 = 4096u;  //no
	uint32 size1 = 4096u, size2 = 2048u;
	// uint32 size1 = 2048u, size2 = 1024u;  //par..
	// uint32 size1 = 1024u, size2 = 512u;
	// uint32 size1 = 512u, size2 = 256u;
	// uint32 size1 = 256u, size2 = 128u;
	shadowParam.technique = SHADOWMAP_PSSM;
	shadowParam.numPssmSplits = 2u;

	shadowParam.resolution[0].x = size1;
	shadowParam.resolution[0].y = size1;
	for( size_t i = 1u; i < 4u; ++i )
	{
		shadowParam.resolution[i].x = size2;
		shadowParam.resolution[i].y = size2;
	}
	shadowParam.atlasStart[0].x = 0u;
	shadowParam.atlasStart[0].y = 0u;
	shadowParam.atlasStart[1].x = 0u;
	shadowParam.atlasStart[1].y = size1;
	shadowParam.atlasStart[2].x = size2;
	shadowParam.atlasStart[2].y = size1;

	shadowParam.supportedLightTypes = 0u;
	shadowParam.addLightType( Light::LT_DIRECTIONAL );
	shadowParams.push_back( shadowParam );

	// Second light, directional, spot or point
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	shadowParam.atlasId = 1;
#endif
	shadowParam.technique = SHADOWMAP_FOCUSED;
	shadowParam.resolution[0].x = size1;
	shadowParam.resolution[0].y = size1;
	shadowParam.atlasStart[0].x = 0u;
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	shadowParam.atlasStart[0].y = 0u;
#else
	shadowParam.atlasStart[0].y = size1 + size2;
#endif

	shadowParam.supportedLightTypes = 0u;
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	shadowParam.addLightType( Light::LT_DIRECTIONAL );
#endif
	shadowParam.addLightType( Light::LT_POINT );
	shadowParam.addLightType( Light::LT_SPOTLIGHT );
	shadowParams.push_back( shadowParam );

	// Third light, directional, spot or point
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	shadowParam.atlasStart[0].y = size1;
#else
	shadowParam.atlasStart[0].y = size1 + size2 + size1;
#endif
	shadowParams.push_back( shadowParam );

#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	// Fourth light
	shadowParam.atlasStart[0].x = size1;
	shadowParam.atlasStart[0].y = 0u;
	shadowParams.push_back( shadowParam );

	// Fifth light
	shadowParam.atlasStart[0].x = size1;
	shadowParam.atlasStart[0].y = size1;
	shadowParams.push_back( shadowParam );
#endif

	ShadowNodeHelper::createShadowNodeWithSettings(
		mgr, rs->getCapabilities(), "ShadowMapFromCodeShadowNode",
		shadowParams, false );
}

//-----------------------------------------------------------------------------------
void AppGui::createEsmShadowNodes()
{
	CompositorManager2 *mgr = mRoot->getCompositorManager2();
	RenderSystem *rs = mRoot->getRenderSystem();

	ShadowNodeHelper::ShadowParamVec shadowParams;

	ShadowNodeHelper::ShadowParam shadowParam;
	silent_memset( &shadowParam, 0, sizeof( shadowParam ) );

	// First light, directional
	shadowParam.technique = SHADOWMAP_PSSM;
	shadowParam.numPssmSplits = 3u;
	shadowParam.resolution[0].x = 1024u;
	shadowParam.resolution[0].y = 1024u;
	shadowParam.resolution[1].x = 2048u;
	shadowParam.resolution[1].y = 2048u;
	shadowParam.resolution[2].x = 1024u;
	shadowParam.resolution[2].y = 1024u;
	shadowParam.atlasStart[0].x = 0u;
	shadowParam.atlasStart[0].y = 0u;
	shadowParam.atlasStart[1].x = 0u;
	shadowParam.atlasStart[1].y = 1024u;
	shadowParam.atlasStart[2].x = 1024u;
	shadowParam.atlasStart[2].y = 0u;

	shadowParam.supportedLightTypes = 0u;
	shadowParam.addLightType( Light::LT_DIRECTIONAL );
	shadowParams.push_back( shadowParam );

	// Second light, directional, spot or point
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	shadowParam.atlasId = 1;
#endif
	shadowParam.technique = SHADOWMAP_FOCUSED;
	shadowParam.resolution[0].x = 1024u;
	shadowParam.resolution[0].y = 1024u;
	shadowParam.atlasStart[0].x = 0u;
	shadowParam.atlasStart[0].y = 2048u + 1024u;

	shadowParam.supportedLightTypes = 0u;
	shadowParam.addLightType( Light::LT_DIRECTIONAL );
	shadowParam.addLightType( Light::LT_POINT );
	shadowParam.addLightType( Light::LT_SPOTLIGHT );
	shadowParams.push_back( shadowParam );

	// Third light, directional, spot or point
	shadowParam.atlasStart[0].x = 1024u;
	shadowParams.push_back( shadowParam );

#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	// Fourth light
	shadowParam.atlasStart[0].x = 1024u;
	shadowParam.atlasStart[0].y = 0u;
	shadowParams.push_back( shadowParam );

	// Fifth light
	shadowParam.atlasStart[0].x = 1024u;
	shadowParam.atlasStart[0].y = 1024u;
	shadowParams.push_back( shadowParam );
#endif

	const auto *caps = rs->getCapabilities();
	RenderSystemCapabilities capsCopy = *caps;

	// Force the utility to create ESM shadow node with compute filters.
	// Otherwise it'd create using what's supported by the current GPU.
	capsCopy.setCapability( RSC_COMPUTE_PROGRAM );
	ShadowNodeHelper::createShadowNodeWithSettings(
		mgr, &capsCopy,
		"ShadowMapFromCodeEsmShadowNodeCompute", shadowParams,
		true );

	// Force the utility to create ESM shadow node with graphics filters.
	// Otherwise it'd create using what's supported by the current GPU.
	capsCopy.unsetCapability( RSC_COMPUTE_PROGRAM );
	ShadowNodeHelper::createShadowNodeWithSettings(
		mgr, &capsCopy,
		"ShadowMapFromCodeEsmShadowNodePixelShader", shadowParams,
		true );
}

//-----------------------------------------------------------------------------------
CompositorWorkspace *AppGui::setupShadowCompositor()
{
	CompositorManager2 *mgr = mRoot->getCompositorManager2();
	const String workspaceName( "ShadowMapFromCodeWorkspace" );

	if( !mgr->hasWorkspaceDefinition( workspaceName ) )
	{
		mgr->createBasicWorkspaceDef(
			workspaceName, ColourValue(0,0,0,0), IdString() );

		const String nodeDefName =
			"AutoGen " + IdString( workspaceName + "/Node" ).getReleaseText();
		CompositorNodeDef *nodeDef =
			mgr->getNodeDefinitionNonConst( nodeDefName );

		CompositorTargetDef *targetDef = nodeDef->getTargetPass( 0 );
		const CompositorPassDefVec &passes = targetDef->getCompositorPasses();

		assert( dynamic_cast<CompositorPassSceneDef *>( passes[0] ) );
		CompositorPassSceneDef *passSceneDef =
			static_cast<CompositorPassSceneDef *>( passes[0] );
		passSceneDef->mShadowNode = "ShadowMapFromCodeShadowNode";

		createPcfShadowNode();
		createEsmShadowNodes();
	}

	auto* mWorkspace = mgr->addWorkspace(
		mSceneMgr, mWindow->getTexture(),
		mCamera, "ShadowMapFromCodeWorkspace", true );
	return mWorkspace;
}

//-----------------------------------------------------------------------------------
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
void AppGui::registerHlms() override
{
	GraphicsSystem::registerHlms();
	Root *root = getRoot();
	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	assert( dynamic_cast<HlmsPbs *>( hlms ) );
	HlmsPbs *pbs = static_cast<HlmsPbs *>( hlms );
	if( pbs )
		pbs->setStaticBranchingLights( true );
}
#endif

//-----------------------------------------------------------------------------------
const char *AppGui::chooseEsmShadowNode()
{
	Root *root = mGraphicsSystem->getRoot();
	RenderSystem *rs = root->getRenderSystem();

	const auto *caps = rs->getCapabilities();
	bool hasCompute = caps->hasCapability( RSC_COMPUTE_PROGRAM );

	if( !hasCompute )
	{
		// There's no choice.
		return "ShadowMapFromCodeEsmShadowNodePixelShader";
	}
	else
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
		// On iOS, the A7 GPUs have slow compute shaders.
		DriverVersion driverVersion = caps->getDriverVersion();
		if( driverVersion.major == 1 )
			;
		return "ShadowMapFromCodeEsmShadowNodePixelShader";
#endif
		return "ShadowMapFromCodeEsmShadowNodeCompute";
	}
}

//-----------------------------------------------------------------------------------
void AppGui::setupShadowNode( bool forEsm )
{
	Root *root = mGraphicsSystem->getRoot();
	CompositorManager2 *mgr = root->getCompositorManager2();

	const String nodeDefName =
		"AutoGen " + IdString( "ShadowMapFromCodeWorkspace/Node" ).getReleaseText();
	CompositorNodeDef *nodeDef = mgr->getNodeDefinitionNonConst( nodeDefName );

	CompositorTargetDef *targetDef = nodeDef->getTargetPass( 0 );
	const CompositorPassDefVec &passes = targetDef->getCompositorPasses();

	assert( dynamic_cast<CompositorPassSceneDef *>( passes[0] ) );
	CompositorPassSceneDef *ps =
		static_cast<CompositorPassSceneDef *>( passes[0] );

	if( forEsm && ps->mShadowNode == "ShadowMapFromCodeShadowNode" )
	{
		destroyShadowMapDebugOverlays();
		mGraphicsSystem->stopCompositor();
		ps->mShadowNode = chooseEsmShadowNode();
		mGraphicsSystem->restartCompositor();
		createShadowMapDebugOverlays();
	}
	else if( !forEsm && ps->mShadowNode != "ShadowMapFromCodeShadowNode" )
	{
		destroyShadowMapDebugOverlays();
		mGraphicsSystem->stopCompositor();
		ps->mShadowNode = "ShadowMapFromCodeShadowNode";
		mGraphicsSystem->restartCompositor();
		createShadowMapDebugOverlays();
	}
}

//-----------------------------------------------------------------------------------
void AppGui::createShadowMapDebugOverlays()
{
	destroyShadowMapDebugOverlays();

	Root *root = mGraphicsSystem->getRoot();
	CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
	Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( HLMS_UNLIT );

	HlmsMacroblock macroblock;
	macroblock.mDepthCheck = false;
	HlmsBlendblock blendblock;

	bool isUsingEsm = false;
	{
		Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
		assert( dynamic_cast<HlmsPbs *>( hlms ) );
		HlmsPbs *pbs = static_cast<HlmsPbs *>( hlms );
		isUsingEsm = pbs->getShadowFilter() == HlmsPbs::ExponentialShadowMaps;
	}

	const String shadowNodeName =
		isUsingEsm ? chooseEsmShadowNode() : "ShadowMapFromCodeShadowNode";

	CompositorShadowNode *shadowNode = workspace->findShadowNode( shadowNodeName );
	const CompositorShadowNodeDef *shadowNodeDef = shadowNode->getDefinition();

	for( size_t i = 0u; i < 5u; ++i )
	{
		const String datablockName( "depthShadow" + StringConverter::toString( i ) );
		HlmsUnlitDatablock *depthShadow =
			(HlmsUnlitDatablock *)hlmsUnlit->getDatablock( datablockName );

		if( !depthShadow )
		{
			depthShadow = (HlmsUnlitDatablock *)hlmsUnlit->createDatablock(
				datablockName, datablockName, macroblock, blendblock, HlmsParamVec() );
		}

		const ShadowTextureDefinition *shadowTexDef =
			shadowNodeDef->getShadowTextureDefinition( i );

		TextureGpu *tex = shadowNode->getDefinedTexture( shadowTexDef->getTextureNameStr() );
		depthShadow->setTexture( 0, tex );

		// If it's an UV atlas, then only display the relevant section.
		Matrix4 uvOffsetScale;
		uvOffsetScale.makeTransform(
			Vector3( shadowTexDef->uvOffset.x, shadowTexDef->uvOffset.y, 0.0f ),
			Vector3( shadowTexDef->uvLength.x, shadowTexDef->uvLength.y, 1.0f ),
			Quaternion::IDENTITY );
		depthShadow->setEnableAnimationMatrix( 0, true );
		depthShadow->setAnimationMatrix( 0, uvOffsetScale );
	}

	v1::OverlayManager &overlayManager = v1::OverlayManager::getSingleton();
	// Create an overlay
	mDebugOverlayPSSM = overlayManager.create( "PSSM Overlays" );
	mDebugOverlaySpotlights = overlayManager.create( "Spotlight overlays" );

	for( int i = 0; i < 3; ++i )
	{
		// Create a panel
		v1::OverlayContainer *panel =
			static_cast<v1::OverlayContainer *>( overlayManager.createOverlayElement(
				"Panel", "PanelName" + StringConverter::toString( i ) ) );
		panel->setMetricsMode( v1::GMM_RELATIVE_ASPECT_ADJUSTED );
		panel->setPosition( 100 + Real( i ) * 1600, 10000 - 1600 );
		panel->setDimensions( 1500, 1500 );
		panel->setMaterialName( "depthShadow" + StringConverter::toString( i ) );
		mDebugOverlayPSSM->add2D( panel );
	}

	for( int i = 3; i < 5; ++i )
	{
		// Create a panel
		v1::OverlayContainer *panel =
			static_cast<v1::OverlayContainer *>( overlayManager.createOverlayElement(
				"Panel", "PanelName" + StringConverter::toString( i ) ) );
		panel->setMetricsMode( v1::GMM_RELATIVE_ASPECT_ADJUSTED );
		panel->setPosition( 100 + Real( i ) * 1600, 10000 - 1600 );
		panel->setDimensions( 1500, 1500 );
		panel->setMaterialName( "depthShadow" + StringConverter::toString( i ) );
		mDebugOverlaySpotlights->add2D( panel );
	}

	mDebugOverlayPSSM->show();
	mDebugOverlaySpotlights->show();
}

//-----------------------------------------------------------------------------------
void AppGui::destroyShadowMapDebugOverlays()
{
	v1::OverlayManager &overlayManager = v1::OverlayManager::getSingleton();

	if( mDebugOverlayPSSM )
	{
		auto itor =	mDebugOverlayPSSM->get2DElementsIterator();
		while( itor.hasMoreElements() )
		{
			v1::OverlayContainer *panel = itor.getNext();
			overlayManager.destroyOverlayElement( panel );
		}
		overlayManager.destroy( mDebugOverlayPSSM );
		mDebugOverlayPSSM = 0;
	}

	if( mDebugOverlaySpotlights )
	{
		auto itor = mDebugOverlaySpotlights->get2DElementsIterator();
		while( itor.hasMoreElements() )
		{
			v1::OverlayContainer *panel = itor.getNext();
			overlayManager.destroyOverlayElement( panel );
		}
		overlayManager.destroy( mDebugOverlaySpotlights );
		mDebugOverlaySpotlights = 0;
	}
}
