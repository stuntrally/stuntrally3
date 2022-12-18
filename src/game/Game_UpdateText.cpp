#include "pch.h"
#include "Def_Str.h"
#include "CGame.h"
#include "GraphicsSystem.h"
#include <OgreLogManager.h>

#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindow.h>

#include <OgreFrameStats.h>
#include <OgreTextureGpuManager.h>
#include "Vao/OgreVaoManager.h"

#include <OgreOverlayManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreTextAreaOverlayElement.h>

#include <OgreAtmosphere2Npr.h>

#include "game.h"  // snd
#include "SoundMgr.h"
#include "CarModel.h"
#include "CScene.h"
#include "SceneXml.h"
#include "TracksXml.h"
#include "CData.h"

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;


//  stat colors
const int iClrVal = 8;
const String sClrVal[iClrVal] =  // long
	{"#A0C0FF", "#20A0FF", "#10FFFF", "#10FF10", "#FFFF20", "#FFC010", "#FF8010", "#FF1010"};

const String clrVal(int i)
{	return sClrVal[std::max(0, std::min(iClrVal-1, i))];  }

String cvsF(float v, float grn, float red, int precision=2, int width=4)
{
	int c = 4.f * (v - grn) / (red - grn) + 3.f /*grn id*/;
	return clrVal(c) + fToStr(v, precision, width);
}
String cvsI(int v, int grn, int red, int width=4)
{
	int c = 4.f * (v - grn) / (red - grn) + 3.f /*grn id*/;
	return clrVal(c) + iToStr(v, width);
}


//  Fps stats
//------------------------------------------------------------------------------------------------
void App::updFpsText()
{
	RenderSystem *rs = mGraphicsSystem->getRoot()->getRenderSystem();
	const RenderingMetrics& rm = rs->getMetrics();  //** fps
	const FrameStats *st = mGraphicsSystem->getRoot()->getFrameStats();

	const float fps = st->getAvgFps(), tris = rm.mFaceCount/1000000.f, mem = getGPUmem();
	const int draw = rm.mDrawCount,
		inst = rm.mInstanceCount, vgt = scn->cntAll,
		gui = MyGUI::Ogre2RenderManager::getInstance().getBatchCount();

	String txt;
	txt += cvsF( fps,  59.f, 30.f, fps >= 100.f ? 0 : 1,4) + "  ";
	txt += cvsF( tris, 1.f, 3.f, 2,4) + "m ";  //txt += "v " + toStr( rm.mVertexCount/1000 ) + "  ";
	txt += cvsI( draw, 50, 180, 3) + "\n";

	txt += "#A0C0A0 Vgt ";
	txt += cvsI( inst, 1000, 8000, 5) + " ";  //txt += "b " + toStr( rm.mBatchCount, 0);
	txt += cvsI( vgt, 3000, 15000, 5) + "\n";

	txt += "#B0B0B0 Gui " + cvsI( gui, 5, 30, 2) + " ";
	txt += cvsF( mem,  500.f, 1300.f, 0,4) + "M\n";

	//  test colors
	/*for (int i = 15; i < 60; i+=5)
		txt += cvsF( i, 30.f, 50.f, 0,2) + " ";
	txt += "\n";/**/

	txFps->setCaption(txt);
}

//  GPU RAM mem
float App::getGPUmem()
{
	RenderSystem *rs = mGraphicsSystem->getRoot()->getRenderSystem();
	size_t freeB, capacityB;  // B = Bytes
	bool bIncludesTex;  // Textures
	VaoManager::MemoryStatsEntryVec memStats;
	VaoManager *vaoManager = rs->getVaoManager();
	vaoManager->getMemoryStats( memStats, capacityB, freeB, 0, bIncludesTex );

	TextureGpuManager *mgr = rs->getTextureGpuManager();
	size_t texBCpu, texBGpu, usedStagingTexB, availStagingTexB;
	mgr->getMemoryStats(
		texBCpu, texBGpu, usedStagingTexB, availStagingTexB );

	// Don't count texture memory twice if it's already included in VaoManager
	if( bIncludesTex )
		capacityB -= texBGpu + usedStagingTexB + availStagingTexB;

	const size_t BtoMB = 1024u * 1024u;
	/*char tmpBuffer[256];
	LwString text( Ogre::LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );
	text.clear();
	text.a( "\n\nGPU buffer pools (meshes, const, texture, indirect & uav buffers): ",
			( Ogre::uint32 )( ( capacityB - freeB ) / BtoMB ), "/",
			( Ogre::uint32 )( capacityB / BtoMB ), " MB" );
	outText += text.c_str();
	text.clear();
	text.a(
		"\nGPU StagingTexs. In use: ", ( Ogre::uint32 )( usedStagingTexB / BtoMB ),
		" MB. Available: ", ( Ogre::uint32 )( availStagingTexB / BtoMB ),
		" MB. Total:",
		( Ogre::uint32 )( ( usedStagingTexB + availStagingTexB ) / BtoMB ) );
	outText += text.c_str();*/

	const size_t totalB =
		capacityB + texBGpu + usedStagingTexB + availStagingTexB;

	/*text.clear();
	text.a( "\nGPU Textures:\t", ( Ogre::uint32 )( textureBGpu / BtoMB ), " MB" );
	text.a( "\nCPU Textures:\t", ( Ogre::uint32 )( textureBCpu / BtoMB ), " MB" );
	text.a( "\nTotal GPU:\t", ( Ogre::uint32 )( totalBNeeded / BtoMB ), " MB" );
	outText += text.c_str();*/

	return float( totalB / BtoMB ); // MB
	//return float( 1.f/1024.f * totalB / BtoMB);  // GB
}

//  text overlay-
//------------------------------------------------------------------------------------------------
void App::updDebugText()
{
	if (!mDebugText)  return;

	String txt;

	if (mDisplayOverlay)
	{
		txt = "F1 toggle help\n";
		txt += "Reload shaders:\n"
				"Ctrl+F1 PBS  Ctrl+F2 Unlit  Ctrl+F3 Compute  Ctrl+F4 Terra\n\n";
		txt += "R terrain wireframe\n";
		txt += "K next Sky\n\n";
		
		Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
		txt += "\n\nPos:  " + fToStr( camPos.x, 1) +"  "+ fToStr( camPos.y, 1) +"  "+ fToStr( camPos.z, 1) + "\n\n";
		txt += "\n- + Pitch  " + fToStr( sc->ldPitch, 1,4 );
		txt += "\n/ * Yaw    " + fToStr( sc->ldYaw, 1,4 );

		txt += "\n^ v Param  " + fToStr( param, 0 );
		
		SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
		Atmosphere2Npr *atmo = static_cast<Atmosphere2Npr*>( sceneManager->getAtmosphere() );
		Atmosphere2Npr::Preset p = atmo->getPreset();
		
		txt += "\n< > ";  const int d = 3;
		switch (param)
		{
		// p.fogHcolor.xyz  fogHparams.xy
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

	if (pGame)  // CAR text
	{
		/*int num = pGame->cars.size();
		if (num != 1)
			txt += "cars " + toStr(num) + "\n";*/
		
		/*for (const CAR* car : pGame->cars)
		{
			txt += "\ngear  " + iToStr(car->GetGear()) + "\nrpm  " + iToStr(car->GetEngineRPM(),4)
				+ "\nkm/h " + fToStr(car->GetSpeedometer()*3.6f, 0) +"\n";
				// todo: + "trk  " + fToStr(carModels[0]->trackPercent, 0) + "%";
		}
		for (const auto* cm : carModels)
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


//-----------------------------------------------------------------------------------
void App::CreateDebugTextOverlay()
{
	return;  // todo: move to Gui..

	LogO("---- create overlay");
	Ogre::v1::OverlayManager &mgr = Ogre::v1::OverlayManager::getSingleton();
	Ogre::v1::Overlay *overlay = mgr.create( "DebugText" );

	Ogre::v1::OverlayContainer *panel = static_cast<Ogre::v1::OverlayContainer*>(
		mgr.createOverlayElement("Panel", "DebugPanel"));
	mDebugText = static_cast<Ogre::v1::TextAreaOverlayElement*>(
		mgr.createOverlayElement( "TextArea", "DebugText" ) );
	mDebugText->setFontName( "DebugFont" );
	mDebugText->setCharHeight( 0.022f );

	mDebugTextShadow = static_cast<Ogre::v1::TextAreaOverlayElement*>(
		mgr.createOverlayElement( "TextArea", "0DebugTextShadow" ) );
	mDebugTextShadow->setFontName( "DebugFont" );
	mDebugTextShadow->setCharHeight( 0.022f );
	mDebugTextShadow->setColour( Ogre::ColourValue::Black );
	mDebugTextShadow->setPosition( 0.001f, 0.001f );

	panel->addChild( mDebugTextShadow );
	panel->addChild( mDebugText );
	overlay->add2D( panel );
	overlay->show();
}


#if 0
//-----------------------------------------------------------------------------------
void keyReleased( const SDL_KeyboardEvent &arg )
{
	if( arg.keysym.scancode == SDL_SCANCODE_F1 && (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) == 0 )
	{
		mDisplayHelpMode = !mDisplayHelpMode;
	}
	else if( arg.keysym.scancode == SDL_SCANCODE_F1 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
	{
		//Hot reload of shaders.
		Ogre::Root *root = mGraphicsSystem->getRoot();
		Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

		Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_PBS );  // F1
		Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_UNLIT );  // F2
		Ogre::Hlms *hlms = hlmsManager->getComputeHlms();  // F3
		Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}
	else if( arg.keysym.scancode == SDL_SCANCODE_F5 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
	{
		//Force device reelection
		Ogre::Root *root = mGraphicsSystem->getRoot();
		root->getRenderSystem()->validateDevice( true );
	}

}
#endif
