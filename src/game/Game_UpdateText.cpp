#include "pch.h"
#include "Def_Str.h"
#include "CGame.h"
#include "CScene.h"
#include "SceneXml.h"
#include "GraphicsSystem.h"

#include <OgreOverlayManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreAtmosphere2Npr.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;


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

	LogO("C--- create overlay");
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
		// Hot reload of shaders.
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
		// Force device reelection
		Ogre::Root *root = mGraphicsSystem->getRoot();
		root->getRenderSystem()->validateDevice( true );
	}

}
#endif
