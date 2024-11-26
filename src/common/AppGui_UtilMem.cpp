#include "pch.h"
#include "Def_Str.h"
#include "AppGui.h"
#include "GraphicsSystem.h"
#include <OgreLogManager.h>

#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreString.h>

#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsUnlitDatablock.h>

#include <OgreHlmsManager.h>
#include <OgreGpuProgramManager.h>
#include <OgreTextureGpuManager.h>
#include <OgrePixelFormatGpuUtils.h>
#include <Vao/OgreVaoManager.h>
#include <OgreTextureGpuManager.h>

using namespace Ogre;
using namespace std;


void AppGui::Quit()
{
	mGraphicsSystem->setQuit();
}

//  util
//-----------------------------------------------------------------------------------
template <typename T, size_t MaxNumTextures>
void AppGui::unloadTexturesFromUnusedMaterials(
	HlmsDatablock* datablock,
	std::set<TextureGpu*> &usedTex,
	std::set<TextureGpu*> &unusedTex )
{
	OGRE_ASSERT_HIGH( dynamic_cast<T *>( datablock ) );
	T *derivedDatablock = static_cast<T *>( datablock );

	for (size_t texUnit = 0; texUnit < MaxNumTextures; ++texUnit)
	{
		// Check each texture from the material
		TextureGpu *tex = derivedDatablock->getTexture( (uint8_t)texUnit );
		if (tex)
		{
			// If getLinkedRenderables is empty, then the material is not in use,
			// and thus so is potentially the texture
			if( !datablock->getLinkedRenderables().empty() )
				usedTex.insert( tex );
			else
				unusedTex.insert( tex );
		}
	}
}
//-----------------------------------------------------------------------------------
void AppGui::unloadTexturesFromUnusedMaterials()
{
	Root *root = mGraphicsSystem->getRoot();
	HlmsManager *hlmsManager = root->getHlmsManager();

	std::set<TextureGpu *> usedTex;
	std::set<TextureGpu *> unusedTex;

	// Check each material from each Hlms (except low level) to see if their material is
	// currently in use. If it's not, then its textures may be not either
	for (size_t i = HLMS_PBS; i < HLMS_MAX; ++i)
	{
		Hlms *hlms = hlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
		if (hlms)
		{
			const Hlms::HlmsDatablockMap &datablocks = hlms->getDatablockMap();

			auto itor = datablocks.begin();
			auto end = datablocks.end();
			while( itor != end )
			{
				String name = itor->second.name;
				if( !StringUtil::endsWith(name, "_TrueTypeFont", false))  // Gui font
				{
				if( i == HLMS_PBS )
					unloadTexturesFromUnusedMaterials<HlmsPbsDatablock, NUM_PBSM_TEXTURE_TYPES>(
						itor->second.datablock, usedTex, unusedTex );
				else if( i == HLMS_UNLIT )
					unloadTexturesFromUnusedMaterials<HlmsUnlitDatablock, NUM_UNLIT_TEXTURE_TYPES>(
						itor->second.datablock, usedTex, unusedTex );
				}
				++itor;
			}
		}
		// hlms->_clearShaderCache();
		// hlms->destroyAllDatablocks(); // todo: ?
	}

	// Unload all unused textures, unless they're also in the "usedTex" (a texture may be
	// set to a material that is currently unused, and also in another material in use)
	auto itor = unusedTex.begin();
	auto end = unusedTex.end();

	while( itor != end )
	{
		if( usedTex.find( *itor ) == usedTex.end() )
			( *itor )->scheduleTransitionTo( GpuResidency::OnStorage );
		++itor;
	}
}

//-----------------------------------------------------------------------------------
void AppGui::unloadUnusedTextures()
{
	RenderSystem *rs = mGraphicsSystem->getRoot()->getRenderSystem();
	auto* textureGpuManager = rs->getTextureGpuManager();

	const auto &ents = textureGpuManager->getEntries();
	for (auto itor = ents.begin(); itor != ents.end(); ++itor)
	{
		const TextureGpuManager::ResourceEntry &entry = itor->second;

		const auto& listeners =	entry.texture->getListeners();

		bool canBeUnloaded = true;

		auto itListener = listeners.begin();
		auto enListener = listeners.end();
		while( itListener != enListener )
		{
			HlmsDatablock *datablock = dynamic_cast<HlmsDatablock *>( *itListener );
			if( datablock )
				canBeUnloaded = false;

			canBeUnloaded &= ( *itListener )->shouldStayLoaded( entry.texture );
			++itListener;
		}
		String name = entry.texture->getNameStr();
		if( name == "DynamicCubemap"
			|| name == "PrvBrushes"
			|| StringUtil::endsWith(name, "_TrueTypeFont", false)  // Gui font
			|| entry.texture->getTextureType() != TextureTypes::Type2D //
			|| !entry.texture->hasAutomaticBatching()  //
			|| !entry.texture->isTexture()
			|| entry.texture->isRenderToTexture()
			|| entry.texture->isUav() //
			|| mapCommonTex.find(name) != mapCommonTex.end()
			// || name == "flare1.png"
			// || name == "brushes.png" //-
			)
		{
			// likely internal texture
			//LogO("unload skip: "+name);
			canBeUnloaded = false;
		}
		// else
		// 	LogO("unload: "+name);

		if( canBeUnloaded )
			entry.texture->scheduleTransitionTo( GpuResidency::OnStorage );
	}
}

//-----------------------------------------------------------------------------------
void AppGui::MinimizeMemory()
{
	LogO("DD== MinimizeMemory ====DD");
	// setTightMemoryBudget();
	unloadTexturesFromUnusedMaterials();
	unloadUnusedTextures();

	SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
	sceneManager->shrinkToFitMemoryPools();

	Root *root = mGraphicsSystem->getRoot();
	RenderSystem *renderSystem = root->getRenderSystem();
	VaoManager *vaoManager = renderSystem->getVaoManager();
	try
	{	vaoManager->cleanupEmptyPools();
	}
	catch( UnimplementedException & )
	{	}	// Ignore. Vulkan doesn't implement this (yet?).

	LoadCommonTex();  // !
}

#if 0
//-----------------------------------------------------------------------------------
void AppGui::setTightMemoryBudget()
{
	Root *root = mGraphicsSystem->getRoot();
	RenderSystem *renderSystem = root->getRenderSystem();

	TextureGpuManager *textureGpuManager = renderSystem->getTextureGpuManager();

	textureGpuManager->setStagingTextureMaxBudgetBytes( 8u * 1024u * 1024u );
	textureGpuManager->setWorkerThreadMaxPreloadBytes( 8u * 1024u * 1024u );
	textureGpuManager->setWorkerThreadMaxPerStagingTextureRequestBytes( 4u * 1024u * 1024u );

	TextureGpuManager::BudgetEntryVec budget;
	textureGpuManager->setWorkerThreadMinimumBudget( budget );

	mTightMemoryBudget = true;
}
//-----------------------------------------------------------------------------------
void AppGui::setRelaxedMemoryBudget()
{
	Root *root = mGraphicsSystem->getRoot();
	RenderSystem *renderSystem = root->getRenderSystem();

	TextureGpuManager *textureGpuManager = renderSystem->getTextureGpuManager();

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS && OGRE_PLATFORM != OGRE_PLATFORM_ANDROID && \
OGRE_ARCH_TYPE != OGRE_ARCHITECTURE_32
	textureGpuManager->setStagingTextureMaxBudgetBytes( 256u * 1024u * 1024u );
#else
	textureGpuManager->setStagingTextureMaxBudgetBytes( 128u * 1024u * 1024u );
#endif
	textureGpuManager->setWorkerThreadMaxPreloadBytes( 256u * 1024u * 1024u );
	textureGpuManager->setWorkerThreadMaxPerStagingTextureRequestBytes( 64u * 1024u * 1024u );

	// textureGpuManager->setWorkerThreadMinimumBudget( mDefaultBudget );

	mTightMemoryBudget = false;
}
#endif


//  textures
//-----------------------------------------------------------------------------------
void AppGui::InitCommonTex()
{
	auto& v = mapCommonTex;
#ifndef SR_EDITOR  // game
	v["background2.jpg"] = 1; //?
	v["boost.png"] = 1;
	v["flare1.png"] = 1;
	v["mud.png"] = 1;
	v["dust.png"] = 1;
	v["gauges4.png"] = 1;  //pSet->gauges_type
#endif
	v["gui_icons.png"] = 1;
	v["track_icons.png"] = 1;
	v["stuntrally-logo.jpg"] = 1;  // to not flash white
}

void AppGui::LoadCommonTex()
{
	// return;  // needed, "fixes white texture flashes"
	auto* texMgr = mRoot->getRenderSystem()->getTextureGpuManager();

	for (auto name : mapCommonTex)
		LoadTex(name.first, false);

	WaitForTex();
}

 void AppGui::LoadTex(Ogre::String fname, bool wait)
 {
	auto* texMgr = mRoot->getRenderSystem()->getTextureGpuManager();

	TextureGpu *tex = texMgr->createOrRetrieveTexture( fname,
		GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
	tex->scheduleTransitionTo( GpuResidency::Resident );

	if (wait)
		WaitForTex();
}

//  Wait for Tex load .. still bad
//  to fix white texture flashes
void AppGui::WaitForTex()
{
	auto* texMgr = mRoot->getRenderSystem()->getTextureGpuManager();
	texMgr->waitForStreamingCompletion();
#if 0  // crash on load next
	const int old = texMgr->getLoadRequestsCounter();
	mRoot->renderOneFrame();
	if (old != texMgr->getLoadRequestsCounter())
	{
		texMgr->waitForStreamingCompletion();
		mRoot->renderOneFrame();
	}
#endif
}
