#include "AppGui.h"
#include "settings.h"
#include "RenderConst.h"
#include "GraphicsSystem.h"

#include <OgreRoot.h>
#include <OgreWindow.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayManager.h>

#include <OgreHlmsManager.h>
#include "HlmsPbs2.h"
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

// #define USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS  //-


//-----------------------------------------------------------------------------------
void AppGui::setupESM()
{
	//  For ESM, setup the filter settings (radius and gaussian deviation).
	//  It controls how blurry the shadows will look.
	Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
	Ogre::HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();

	Ogre::uint8 R = 8;  // kernelRadius
	float gauss = 0.5f;  // gaussianDeviationFactor
	Ogre::uint16 K = 80;
	Ogre::HlmsComputeJob *job = 0;

	//  Setup compute shader filter (faster for large kernels; but
	//  beware of mobile hardware where compute shaders are slow)
	//  For reference large kernels means kernelRadius > 2 (approx)
	using namespace Demo;
	job = hlmsCompute->findComputeJob( "ESM/GaussianLogFilterH" );
	MiscUtils::setGaussianLogFilterParams( job, R, gauss, K );
	job = hlmsCompute->findComputeJob( "ESM/GaussianLogFilterV" );
	MiscUtils::setGaussianLogFilterParams( job, R, gauss, K );

	//  Setup pixel shader filter (faster for small kernels,
	//  also to use as a fallback on GPUs with no or slow compute shaders).
	MiscUtils::setGaussianLogFilterParams(
		"ESM/GaussianLogFilterH", R, gauss, K );
	MiscUtils::setGaussianLogFilterParams(
		"ESM/GaussianLogFilterV", R, gauss, K );
}

//  🆕 create PCF
//-----------------------------------------------------------------------------------
void AppGui::createPcfShadowNode()
{
	CompositorManager2 *mgr = mRoot->getCompositorManager2();
	RenderSystem *rs = mRoot->getRenderSystem();

	ShadowNodeHelper::ShadowParamVec spv;
	ShadowNodeHelper::ShadowParam sp;
	silent_memset( &sp, 0, sizeof(sp) );

	//  light 1, directional
	const uint32
		size1 = pSet->GetTexSize(pSet->shadow_size),
		size2 = size1 / 2u;  // par /?
	// SHADOWMAP_FOCUSED, SHADOWMAP_PLANEOPTIMAL,  // for 1
	sp.technique = pSet->shadow_count > 1 ?
		SHADOWMAP_PSSM : SHADOWMAP_FOCUSED;
	sp.numPssmSplits = std::min(4, std::max(2,
		pSet->shadow_count ));

	sp.resolution[0].x = size1;  sp.resolution[0].y = size1;
	for( size_t i = 1u; i < 4u; ++i )
	{	sp.resolution[i].x = size2;  sp.resolution[i].y = size2;  }

	sp.atlasStart[0].x = 0u;	sp.atlasStart[0].y = 0u;
	sp.atlasStart[1].x = 0u;	sp.atlasStart[1].y = size1;
	sp.atlasStart[2].x = size2;	sp.atlasStart[2].y = size1;

	sp.supportedLightTypes = 0u;
	sp.addLightType( Light::LT_DIRECTIONAL );
	spv.push_back( sp );


	if (pSet->car_lights)  // or >1 sun planets..
	{
		//  light 2  dir, spot or point
	#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
		sp.atlasId = 1;
	#endif
		sp.technique = SHADOWMAP_FOCUSED;
		sp.resolution[0].x = size1;  sp.resolution[0].y = size1;
		sp.atlasStart[0].x = 0u;
	#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
			sp.atlasStart[0].y = 0u;
	#else
			sp.atlasStart[0].y = size1 + size2;
	#endif

		sp.supportedLightTypes = 0u;
	#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
		sp.addLightType( Light::LT_DIRECTIONAL );
	#endif
		sp.addLightType( Light::LT_POINT );
		sp.addLightType( Light::LT_SPOTLIGHT );
		spv.push_back( sp );


		//  light 3  dir, spot or point
	#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
			sp.atlasStart[0].y = size1;
	#else
			sp.atlasStart[0].y = size1 + size2 + size1;
	#endif
		spv.push_back( sp );

	#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
		//  light 4  -
		sp.atlasStart[0].x = size1;  sp.atlasStart[0].y = 0u;
		spv.push_back( sp );

		//  light 5
		sp.atlasStart[0].x = size1;  sp.atlasStart[0].y = size1;
		spv.push_back( sp );
	#endif
	}

	ShadowNodeHelper::createShadowNodeWithSettings(
		mgr, rs->getCapabilities(), "ShadowMapFromCodeShadowNode", spv,
		false,  // useEsm
		size2,  // pointLight CubemapRes = 1024u,
		0.95f,  // pssmLambda = 0.95f,
		1.0f,   // splitPadding = 1.0f,
		0.125f, // splitBlend = 0.125f,
		0.313f, // splitFade = 0.313f,
		0,      // numStableSplits = 0,
		VisibilityFlags::RESERVED_VISIBILITY_FLAGS,  // visibilityMask
		1.5f,   // xyPadding = 1.5f,
		RQG_Sky,  //0u,     // firstRq = 0u,
		RQG_PipeGlass  //255u    // lastRq = 255u
	);
	mSceneMgr->setShadowDirectionalLightExtrusionDistance( pSet->shadow_dist );
	mSceneMgr->setShadowFarDistance( pSet->shadow_dist );
	// HlmsDatablock::mShadowConstantBias.
}

//  🆕 create ESM
//-----------------------------------------------------------------------------------
void AppGui::createEsmShadowNodes()
{
	CompositorManager2 *mgr = mRoot->getCompositorManager2();
	RenderSystem *rs = mRoot->getRenderSystem();

	ShadowNodeHelper::ShadowParamVec spv;
	ShadowNodeHelper::ShadowParam sp;
	silent_memset( &sp, 0, sizeof( sp ) );

	const uint32
		size1 = pSet->GetTexSize(pSet->shadow_size), size2 = size1 / 2u;  // par /?
	// SHADOWMAP_PLANEOPTIMAL,  // for 1?
	// SHADOWMAP_FOCUSED,
	sp.technique = SHADOWMAP_PSSM;
	sp.numPssmSplits = std::min(4, std::max(2, pSet->shadow_count )); // 3u;

	//  light 1  directional
	sp.technique = SHADOWMAP_PSSM;
	sp.numPssmSplits = 3u;
	sp.resolution[0].x = size2;  sp.resolution[0].y = size2;
	sp.resolution[1].x = size1;  sp.resolution[1].y = size1;
	sp.resolution[2].x = size2;  sp.resolution[2].y = size2;
	
	sp.atlasStart[0].x = 0u;     sp.atlasStart[0].y = 0u;
	sp.atlasStart[1].x = 0u;     sp.atlasStart[1].y = size2;
	sp.atlasStart[2].x = size2;  sp.atlasStart[2].y = 0u;

	sp.supportedLightTypes = 0u;
	sp.addLightType( Light::LT_DIRECTIONAL );
	spv.push_back( sp );

	//  light 2  dir, spot or point
#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	sp.atlasId = 1;
#endif
	sp.technique = SHADOWMAP_FOCUSED;
	sp.resolution[0].x = size2;
	sp.resolution[0].y = size2;
	sp.atlasStart[0].x = 0u;
	sp.atlasStart[0].y = size1 + size2;

	sp.supportedLightTypes = 0u;
	sp.addLightType( Light::LT_DIRECTIONAL );
	sp.addLightType( Light::LT_POINT );
	sp.addLightType( Light::LT_SPOTLIGHT );
	spv.push_back( sp );

	//  light 3  dir, spot or point
	sp.atlasStart[0].x = size2;
	spv.push_back( sp );

#ifdef USE_STATIC_BRANCHING_FOR_SHADOWMAP_LIGHTS
	//  light 4  -
	sp.atlasStart[0].x = size2;
	sp.atlasStart[0].y = 0u;
	spv.push_back( sp );

	//  light 5
	sp.atlasStart[0].x = size2;
	sp.atlasStart[0].y = size2;
	spv.push_back( sp );
#endif

	const auto *caps = rs->getCapabilities();
	RenderSystemCapabilities capsCopy = *caps;

	// Force the utility to create ESM shadow node with compute filters.
	// Otherwise it'd create using what's supported by the current GPU.
	capsCopy.setCapability( RSC_COMPUTE_PROGRAM );
	ShadowNodeHelper::createShadowNodeWithSettings(
		mgr, &capsCopy,
		"ShadowMapFromCodeEsmShadowNodeCompute", spv,
		true );

	// Force the utility to create ESM shadow node with graphics filters.
	// Otherwise it'd create using what's supported by the current GPU.
	capsCopy.unsetCapability( RSC_COMPUTE_PROGRAM );
	ShadowNodeHelper::createShadowNodeWithSettings(
		mgr, &capsCopy,
		"ShadowMapFromCodeEsmShadowNodePixelShader", spv,
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

	//  add Workspace
	LogO("++++ WS add:  Shadows, all: "+toStr(mgr->getNumWorkspaces()));
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
	assert( dynamic_cast<HlmsPbs2 *>( hlms ) );
	HlmsPbs2 *pbs = static_cast<HlmsPbs2 *>( hlms );
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
		return "ShadowMapFromCodeEsmShadowNodePixelShader";
	}else
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


#if 0  // debug,  didnt work anyway
//-----------------------------------------------------------------------------------
//  toggle-
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
	// mWorkspaces[0]..
	CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
	Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( HLMS_UNLIT );

	HlmsMacroblock macroblock;
	macroblock.mDepthCheck = false;
	HlmsBlendblock blendblock;

	bool isUsingEsm = false;
	{
		Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
		assert( dynamic_cast<HlmsPbs2 *>( hlms ) );
		HlmsPbs2 *pbs = static_cast<HlmsPbs2 *>( hlms );
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
			depthShadow = (HlmsUnlitDatablock *)hlmsUnlit->createDatablock(
				datablockName, datablockName, macroblock, blendblock, HlmsParamVec() );

		const auto *shadowTexDef = shadowNodeDef->getShadowTextureDefinition( i );

		TextureGpu *tex = shadowNode->getDefinedTexture( shadowTexDef->getTextureNameStr() );
		depthShadow->setTexture( 0, tex );

		//  If it's an UV atlas, then only display the relevant section.
		Matrix4 uvOffsetScale;
		uvOffsetScale.makeTransform(
			Vector3( shadowTexDef->uvOffset.x, shadowTexDef->uvOffset.y, 0.0f ),
			Vector3( shadowTexDef->uvLength.x, shadowTexDef->uvLength.y, 1.0f ),
			Quaternion::IDENTITY );
		depthShadow->setEnableAnimationMatrix( 0, true );
		depthShadow->setAnimationMatrix( 0, uvOffsetScale );
	}

	v1::OverlayManager &mgr = v1::OverlayManager::getSingleton();
	//  Create an overlay
	mDebugOverlayPSSM = mgr.create( "PSSM Overlays" );
	mDebugOverlaySpotlights = mgr.create( "Spotlight overlays" );

	for( int i = 0; i < 3; ++i )
	{
		//  Create a panel
		v1::OverlayContainer *panel =
			static_cast<v1::OverlayContainer *>( mgr.createOverlayElement(
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
			static_cast<v1::OverlayContainer *>( mgr.createOverlayElement(
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
	v1::OverlayManager &mgr = v1::OverlayManager::getSingleton();

	if( mDebugOverlayPSSM )
	{
		auto itor =	mDebugOverlayPSSM->get2DElementsIterator();
		while( itor.hasMoreElements() )
		{
			v1::OverlayContainer *panel = itor.getNext();
			mgr.destroyOverlayElement( panel );
		}
		mgr.destroy( mDebugOverlayPSSM );
		mDebugOverlayPSSM = 0;
	}

	if( mDebugOverlaySpotlights )
	{
		auto itor = mDebugOverlaySpotlights->get2DElementsIterator();
		while( itor.hasMoreElements() )
		{
			v1::OverlayContainer *panel = itor.getNext();
			mgr.destroyOverlayElement( panel );
		}
		mgr.destroy( mDebugOverlaySpotlights );
		mDebugOverlaySpotlights = 0;
	}
}
#endif
