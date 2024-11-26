#include "pch.h"
#include "Def_Str.h"
#include "GraphicsSystem.h"
#include "GameState.h"
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
	#include "SdlInputHandler.h"
#endif
#include <OgreException.h>
#include <OgreLogManager.h>

#include <OgreTextureGpuManager.h>

#include <OgreArchiveManager.h>
#include <OgreHlmsDiskCache.h>

#include <OgrePlatformInformation.h>
#include "System/Android/AndroidSystems.h"
#include <fstream>

#if OGRE_USE_SDL2
	#include <SDL_syswm.h>
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
	#include "OSX/macUtils.h"
	#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
		#include "System/iOS/iOSUtils.h"
	#else
		#include "System/OSX/OSXUtils.h"
	#endif
#endif
using namespace Ogre;


//  📄 Cache
//-----------------------------------------------------------------------------------
const static String texFile = "textureData.json";
const static String shaderFile = "shaderCode.cache";

//  textures data
void GraphicsSystem::loadTextureCache()
{
#if !OGRE_NO_JSON
	ArchiveManager &archiveManager = ArchiveManager::getSingleton();
	Archive *rwAccessFolderArchive =
		archiveManager.load( mCacheFolder, "FileSystem", true );
	try
	{
		if( rwAccessFolderArchive->exists( texFile ) )
		{
			DataStreamPtr stream = rwAccessFolderArchive->open( texFile );
			std::vector<char> fileData;
			fileData.resize( stream->size() + 1 );
			if( !fileData.empty() )
			{
				stream->read( &fileData[0], stream->size() );
				fileData.back() = '\0';  // add just in case
				TextureGpuManager *textureManager =
					mRoot->getRenderSystem()->getTextureGpuManager();
				textureManager->importTextureMetadataCache( stream->getName(), &fileData[0], false );
			}
		}else
			LogManager::getSingleton().logMessage(
				"[INFO] Texture cache not found at " + mCacheFolder +"/"+ texFile);
	}
	catch( Exception &e )
	{
		LogManager::getSingleton().logMessage( e.getFullDescription() );
	}

	archiveManager.unload( rwAccessFolderArchive );
#endif
}

//-----------------------------------------------------------------------------------
void GraphicsSystem::saveTextureCache()
{
	if( mRoot->getRenderSystem() )
	{
		TextureGpuManager *textureManager = mRoot->getRenderSystem()->getTextureGpuManager();
		if( textureManager )
		{
			String jsonString;
			textureManager->exportTextureMetadataCache( jsonString );
			const String path = mCacheFolder + "/" + texFile;
			std::ofstream file( path.c_str(), std::ios::binary | std::ios::out );
			if( file.is_open() )
				file.write( jsonString.c_str(), static_cast<std::streamsize>( jsonString.size() ) );
			file.close();
		}
	}
}

//  hlms, shaders
//-----------------------------------------------------------------------------------
void GraphicsSystem::loadHlmsDiskCache()
{
	if( !mUseMicrocodeCache && !mUseHlmsDiskCache )
		return;

	HlmsManager *hlmsManager = mRoot->getHlmsManager();
	HlmsDiskCache diskCache( hlmsManager );

	ArchiveManager &archiveManager = ArchiveManager::getSingleton();

	Archive *arch =
		archiveManager.load( mCacheFolder, "FileSystem", true );

	if( mUseMicrocodeCache )
	{
		//  Make sure the microcode cache is enabled
		GpuProgramManager::getSingleton().setSaveMicrocodesToCache( true );
		if( arch->exists( shaderFile ) )
		{
			DataStreamPtr file = arch->open( shaderFile );
			GpuProgramManager::getSingleton().loadMicrocodeCache( file );
		}
	}

	if( mUseHlmsDiskCache )
	{
		for (size_t i = HLMS_LOW_LEVEL + 1u; i < HLMS_MAX; ++i)
		{
			Hlms *hlms = hlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
			if( hlms )
			{
				String filename = "hlmsCache" + StringConverter::toString( i ) + ".bin";
				try
				{
					if( arch->exists( filename ) )
					{
						DataStreamPtr diskCacheFile = arch->open( filename );
						diskCache.loadFrom( diskCacheFile );
						diskCache.applyTo( hlms );
					}
				}
				catch( Exception& )
				{
					LogManager::getSingleton().logMessage(
						"Error loading cache from " + mCacheFolder + "/" +
						filename + "! If you have issues, try deleting the file "
						"and restarting the app" );
				}
		}	}
	}
	archiveManager.unload( mCacheFolder );
}

//--------------------------------------------------------------------------------------------------------------------------------
void GraphicsSystem::saveHlmsDiskCache()
{
	if( mRoot->getRenderSystem() && GpuProgramManager::getSingletonPtr() &&
		(mUseMicrocodeCache || mUseHlmsDiskCache) )
	{
		HlmsManager *hlmsManager = mRoot->getHlmsManager();
		HlmsDiskCache diskCache( hlmsManager );

		ArchiveManager &archMgr = ArchiveManager::getSingleton();
		Archive *arch = archMgr.load( mCacheFolder, "FileSystem", false );

		if( mUseHlmsDiskCache )
		{
			for (size_t i = HLMS_LOW_LEVEL + 1u; i < HLMS_MAX; ++i)
			{
				Hlms *hlms = hlmsManager->getHlms( static_cast<HlmsTypes>( i ) );
				if( hlms )
				{
					diskCache.copyFrom( hlms );

					DataStreamPtr diskCacheFile =
						arch->create( "hlmsCache" +
							StringConverter::toString( i ) + ".bin" );
					diskCache.saveTo( diskCacheFile );
			}	}
		}

		if( GpuProgramManager::getSingleton().isCacheDirty() && mUseMicrocodeCache )
		{
			DataStreamPtr file = arch->create( shaderFile );
			GpuProgramManager::getSingleton().saveMicrocodeCache( file );
		}
		archMgr.unload( mCacheFolder );
	}
}
