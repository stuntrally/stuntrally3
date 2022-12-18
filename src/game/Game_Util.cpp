#include "pch.h"
#include "CGame.h"
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
#include "Vao/OgreVaoManager.h"

using namespace Ogre;
using namespace std;


//  util
//-----------------------------------------------------------------------------------
template <typename T, size_t MaxNumTextures>
void App::unloadTexturesFromUnusedMaterials( HlmsDatablock *datablock,
											std::set<TextureGpu *> &usedTex,
											std::set<TextureGpu *> &unusedTex )
{
	OGRE_ASSERT_HIGH( dynamic_cast<T *>( datablock ) );
	T *derivedDatablock = static_cast<T *>( datablock );

	for( size_t texUnit = 0; texUnit < MaxNumTextures; ++texUnit )
	{
		// Check each texture from the material
		TextureGpu *tex = derivedDatablock->getTexture( (uint8_t)texUnit );
		if( tex )
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
void App::unloadTexturesFromUnusedMaterials()
{
	Root *root = mGraphicsSystem->getRoot();
	HlmsManager *hlmsManager = root->getHlmsManager();

	std::set<TextureGpu *> usedTex;
	std::set<TextureGpu *> unusedTex;

	// Check each material from each Hlms (except low level) to see if their material is
	// currently in use. If it's not, then its textures may be not either
	for( size_t i = HLMS_PBS; i < HLMS_MAX; ++i )
	{
		Hlms *hlms = hlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
		if( hlms )
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
				{
					unloadTexturesFromUnusedMaterials<HlmsPbsDatablock, NUM_PBSM_TEXTURE_TYPES>(
						itor->second.datablock, usedTex, unusedTex );
				}
				else if( i == HLMS_UNLIT )
				{
					unloadTexturesFromUnusedMaterials<HlmsUnlitDatablock, NUM_UNLIT_TEXTURE_TYPES>(
						itor->second.datablock, usedTex, unusedTex );
				}
				}
				++itor;
			}
		}
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
void App::unloadUnusedTextures()
{
	RenderSystem *renderSystem = mGraphicsSystem->getRoot()->getRenderSystem();

	auto* textureGpuManager = renderSystem->getTextureGpuManager();

	const auto &entries = textureGpuManager->getEntries();

	auto itor = entries.begin();
	auto end = entries.end();
	while( itor != end )
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
		if( StringUtil::endsWith(name, "_TrueTypeFont", false) ||  // Gui font
			entry.texture->getTextureType() != TextureTypes::Type2D ||
			!entry.texture->hasAutomaticBatching() || !entry.texture->isTexture() ||
			entry.texture->isRenderToTexture() || entry.texture->isUav() )
		{
			// likely internal texture
			// LogO("Skip: "+name);
			canBeUnloaded = false;
		}

		if( canBeUnloaded )
			entry.texture->scheduleTransitionTo( GpuResidency::OnStorage );
		++itor;
	}
}

//-----------------------------------------------------------------------------------
void App::MinimizeMemory()
{
	LogO("#### MinimizeMemory");
	// setTightMemoryBudget();
	unloadTexturesFromUnusedMaterials();
	unloadUnusedTextures();

	SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
	sceneManager->shrinkToFitMemoryPools();

	Root *root = mGraphicsSystem->getRoot();
	RenderSystem *renderSystem = root->getRenderSystem();
	VaoManager *vaoManager = renderSystem->getVaoManager();
	vaoManager->cleanupEmptyPools();
}

#if 0
//-----------------------------------------------------------------------------------
void App::setTightMemoryBudget()
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
void App::setRelaxedMemoryBudget()
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


//  util wrap tex
//-----------------------------------------------------------------------------------
void App::SetTexWrap(Ogre::HlmsTypes type, Ogre::String name, bool wrap)
{
	HlmsSamplerblock sampler;
	sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
	sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sampler.mMaxAnisotropy = pSet->anisotropy;
	auto w = wrap ? TAM_WRAP : TAM_CLAMP;
	sampler.mU = w;  sampler.mV = w;  sampler.mW = w;

	Hlms *hlms = mRoot->getHlmsManager()->getHlms( type );
	if (type == HLMS_PBS)
	{	HlmsPbsDatablock *db = static_cast<HlmsPbsDatablock*>(hlms->getDatablock( name ));
		if (db)  db->setSamplerblock( PBSM_DIFFUSE, sampler );
	}
	else if (type == HLMS_UNLIT)
	{	HlmsUnlitDatablock *db = static_cast<HlmsUnlitDatablock*>(hlms->getDatablock( name ));
		if (db)  db->setSamplerblock( PBSM_DIFFUSE, sampler );
	}
}

//  util wireframe
void App::SetWireframe(Ogre::HlmsTypes type, bool wire)
{
	HlmsMacroblock mb;
	mb.mPolygonMode = wire ? PM_WIREFRAME : PM_SOLID;

	Hlms *hlms = mRoot->getHlmsManager()->getHlms( type );

	const auto &dbs = hlms->getDatablockMap();
	for (auto it = dbs.begin(); it != dbs.end(); ++it)
	{
		// String name = itor->second.name;
		// if( !StringUtil::endsWith(name, "_TrueTypeFont", false))  // Gui font
		auto db = it->second.datablock;
		if (db)  db->setMacroblock( mb );
	}
}
