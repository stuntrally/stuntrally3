#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreWindow.h"

#include "OgreFrameStats.h"
#include "OgreTextAreaOverlayElement.h"
#include "OgreTextureGpuManager.h"
#include "Vao/OgreVaoManager.h"

#include "OgreAtmosphere2Npr.h"

#include "game.h"  // snd
#include "SoundMgr.h"
#include "CGame.h"
#include "CarModel.h"
#include "CScene.h"
#include "SceneXml.h"
#include "TracksXml.h"
#include "CData.h"

#include "Def_Str.h"
using namespace Demo;
using namespace Ogre;


namespace Demo
{

	String TerrainGame::generateFpsDebugText()
	{
		//  fps stats  ------------------------------------------------
		RenderSystem *rs = mGraphicsSystem->getRoot()->getRenderSystem();
		const RenderingMetrics& rm = rs->getMetrics();  //** fps
		const FrameStats *st = mGraphicsSystem->getRoot()->getFrameStats();

		String txt = iToStr( (int)st->getAvgFps(), 4) +
			"  "+ fToStr( rm.mFaceCount/1000000.f, 2) +
			//" v " + toStr( rm.mVertexCount/1000 ) +
			"m  " + iToStr( rm.mDrawCount, 2) + " " + iToStr( rm.mInstanceCount, 4);
			//" b " + toStr( rm.mBatchCount, 0);
	#if 1
		//  mem
		VaoManager::MemoryStatsEntryVec memoryStats;
		size_t freeBytes;
		size_t capacityBytes;
		bool bIncludesTextures;
		VaoManager *vaoManager = rs->getVaoManager();
		vaoManager->getMemoryStats( memoryStats, capacityBytes, freeBytes, 0, bIncludesTextures );

		TextureGpuManager *textureGpuManager = rs->getTextureGpuManager();
		size_t textureBytesCpu, textureBytesGpu, usedStagingTextureBytes, availableStagingTextureBytes;
		textureGpuManager->getMemoryStats( textureBytesCpu, textureBytesGpu, usedStagingTextureBytes,
										availableStagingTextureBytes );

		// Don't count texture memory twice if it's already included in VaoManager
		if( bIncludesTextures )
			capacityBytes -= textureBytesGpu + usedStagingTextureBytes + availableStagingTextureBytes;

		const size_t bytesToMb = 1024u * 1024u;
		/*char tmpBuffer[256];
		LwString text( Ogre::LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );
		text.clear();
		text.a( "\n\nGPU buffer pools (meshes, const, texture, indirect & uav buffers): ",
				( Ogre::uint32 )( ( capacityBytes - freeBytes ) / bytesToMb ), "/",
				( Ogre::uint32 )( capacityBytes / bytesToMb ), " MB" );
		outText += text.c_str();
		text.clear();
		text.a(
			"\nGPU StagingTextures. In use: ", ( Ogre::uint32 )( usedStagingTextureBytes / bytesToMb ),
			" MB. Available: ", ( Ogre::uint32 )( availableStagingTextureBytes / bytesToMb ),
			" MB. Total:",
			( Ogre::uint32 )( ( usedStagingTextureBytes + availableStagingTextureBytes ) / bytesToMb ) );
		outText += text.c_str();*/

		const size_t totalBytesNeeded =
			capacityBytes + textureBytesGpu + usedStagingTextureBytes + availableStagingTextureBytes;

		/*text.clear();
		text.a( "\nGPU Textures:\t", ( Ogre::uint32 )( textureBytesGpu / bytesToMb ), " MB" );
		text.a( "\nCPU Textures:\t", ( Ogre::uint32 )( textureBytesCpu / bytesToMb ), " MB" );
		text.a( "\nTotal GPU:\t", ( Ogre::uint32 )( totalBytesNeeded / bytesToMb ), " MB" );
		outText += text.c_str();*/

		txt += "  " + toStr( totalBytesNeeded / bytesToMb ) + " MB\n";
	#endif
		return txt;
	}


	//  text
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::generateDebugText()
	{
		String txt;

		if( mDisplayHelpMode == 0 )
		{
			txt = "F1 toggle help\n";
			txt += "Reload shaders:\n"
					"Ctrl+F1 PBS  Ctrl+F2 Unlit  Ctrl+F3 Compute  Ctrl+F4 Terra\n\n";
			txt += "R terrain wireframe\n";
			txt += "K next Sky\n\n";
			
			Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
			txt += "\n\nPos:  " + fToStr( camPos.x, 1) +"  "+ fToStr( camPos.y, 1) +"  "+ fToStr( camPos.z, 1) + "\n\n";
		}
		else if( mDisplayHelpMode == 1 )
		{
			txt = generateFpsDebugText();

			txt += "Veget " + iToStr(pApp->scn->cntAll, 5);
			txt += "\n\n- + Pitch  " + fToStr( sc->ldPitch, 1,4 );
			txt +=   "\n/ * Yaw    " + fToStr( sc->ldYaw, 1,4 );

			txt += "\n^ v Param  " + fToStr( param, 0 );
			
			SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
			Atmosphere2Npr *atmo = static_cast<Atmosphere2Npr*>( sceneManager->getAtmosphere() );
			Atmosphere2Npr::Preset p = atmo->getPreset();
			
			txt += "\n< > ";  const int d = 3;
			switch (param)
			{
			// p.fogHcolor.xyz
			case 0:   txt += "Fog  " + fToStr( p.fogDensity, 5 );  break;
			case 1:   txt += "density coeff  " + fToStr( p.densityCoeff, d );  break;
			case 2:   txt += "density diffusion  " + fToStr( p.densityDiffusion, d );  break;
			case 3:   txt += "horizon limit  " + fToStr( p.horizonLimit, d );  break;
			case 4:   txt += "Sun Power  " + fToStr( p.sunPower, d );  break;
			case 5:   txt += "sky Power  " + fToStr( p.skyPower, d );  break;
			case 6:   txt += "sky Colour   Red  " + fToStr( p.skyColour.x, d );  break;
			case 7:   txt += "sky Colour Green  " + fToStr( p.skyColour.y, d );  break;
			case 8:   txt += "sky Colour  Blue  " + fToStr( p.skyColour.z, d );  break;
			case 9:   txt += "fog break MinBright  " + fToStr( p.fogBreakMinBrightness, d );  break;
			case 10:  txt += "fog break Falloff  " + fToStr( p.fogBreakFalloff, d );  break;
			case 11:  txt += "linked LightPower  " + fToStr( p.linkedLightPower, d );  break;
			case 12:  txt += "ambient UpperPower  " + fToStr( p.linkedSceneAmbientUpperPower, d );  break;
			case 13:  txt += "ambient LowerPower  " + fToStr( p.linkedSceneAmbientLowerPower, d );  break;
			case 14:  txt += "envmap Scale  " + fToStr( p.envmapScale, d );  break;
			}
		}
		txt += "\n\n";

		const auto* data = pApp->scn->data;
		txt += /*"Track: " +*/ //toStr(idTrack)
			toStr(data->tracks->trks[idTrack].n) +" "+
		 	data->tracks->trks[idTrack].name +"\n";
		txt += /*"  Car: " +*/ toStr(idCar+1) +"  "+
			data->cars->cars[idCar].id + " " +
			data->cars->cars[idCar].name +"\n\n";

		if (pApp->bLoading)
		{
			txt += "Loading... " + toStr(pApp->curLoadState) +" / "+
				toStr(App::LS_ALL) +"  "+ pApp->cStrLoad[pApp->curLoadState];
		}
		else  //-
		if (pGame)  // CAR text
		{
			int num = pGame->cars.size();
			if (num != 1)
				txt += "cars " + toStr(num) + "\n";
			
			for (const CAR* car : pGame->cars)
			{
				auto pos = car->dynamics.GetPosition();
				// txt += "pos  " + fToStr(pos[0],2) + "  " + fToStr(pos[1],2) + "  " + fToStr(pos[2],2) +"\n";
				txt += "\ngear  " + iToStr(car->GetGear()) + "\nrpm  " + iToStr(car->GetEngineRPM(),4)
					+ "\nkm/h " + fToStr(car->GetSpeedometer()*3.6f, 0) +"\n";
			}
			/*for (const auto* cm : pApp->carModels)
			{
				if (cm->fCam)
					txt += "cam  " + toStr(cm->fCam->camPosFinal) + "  " + cm->fCam->sName +"\n";
				else
					txt += "no cam \n";
			}*/
		}

		mDebugText->setCaption( txt );
		mDebugTextShadow->setCaption( txt );
	}

}
