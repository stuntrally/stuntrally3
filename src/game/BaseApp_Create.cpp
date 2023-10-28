#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "BaseApp.h"
#include "paths.h"
#include "settings.h"
#include "masterclient.hpp"
#include "gameclient.hpp"

#include "ICSInputControlSystem.h"
#include <OgreTimer.h>
#include <OgreWindow.h>

#include <MyGUI_Prerequest.h>
#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;
using namespace MyGUI;
// #define TEST_LOAD  // preview cams, no loading backgr


//  🕹️🆕 Create
//-------------------------------------------------------------------------------------
void BaseApp::CreateInputs()
{
	// mCursorManager = new SFO::SDLCursorManager();
	// onCursorChange(MyGUI::PointerManager::getInstance().getDefaultPointer());
	// mCursorManager->setEnabled(true);

	std::string file = PATHS::UserConfigDir() + "/input.xml";
	mInputCtrl = new ICS::InputControlSystem(file, true, mInputBindListner, NULL, 100);

	for (int j=0; j < SDL_NumJoysticks(); ++j)
		mInputCtrl->addJoystick(j);
	for (int i=0; i < MAX_Players; ++i)
	{
		file = PATHS::UserConfigDir() + "/input_p" + toStr(i) + ".xml";
		mInputCtrlPlayer[i] = new ICS::InputControlSystem(file, true, mInputBindListner, NULL, 100);
		for (int j=0; j < SDL_NumJoysticks(); ++j)
			mInputCtrlPlayer[i]->addJoystick(j);
	}

	// bSizeHUD = true;
	// bWindowResized = true;
	// mSplitMgr->Align();

	// mRoot->addFrameListener(this);
}


//  🌟 ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp()
	:mMasterClient(), mClient()
{
}

//  💥 dtor
//-------------------------------------------------------------------------------------
BaseApp::~BaseApp()
{
	DestroyGui();

	//  save inputs
	mInputCtrl->save(PATHS::UserConfigDir() + "/input.xml");
	delete mInputCtrl;
	for (int i=0; i < MAX_Players; ++i)
	{
		mInputCtrlPlayer[i]->save(PATHS::UserConfigDir() + "/input_p" + toStr(i) + ".xml");
		delete mInputCtrlPlayer[i];
	}
}


///  show / hide  Loading bar
//-------------------------------------------------------------------------------------
void BaseApp::LoadingOn()
{
	// if (!imgLoad)  return;
#ifndef TEST_LOAD
	// imgLoad->setVisible(true);
	imgBack->setVisible(true);
#endif
	bckLoad->setVisible(true);
}
void BaseApp::LoadingOff()
{
	// if (!imgLoad)  return;
	// imgLoad->setVisible(false);
	imgBack->setVisible(false);
	bckLoad->setVisible(false);
}


///  base Init Gui
//--------------------------------------------------------------------------------------------------------------
void BaseApp::baseInitGui()
{
	//  loading
	barSizeX = 580;  barHeight = 150;
	bckLoad = mGui->createWidget<ImageBox>("ImageBox",
		100,100, barSizeX +20,barHeight, Align::Default, "Pointer", "LoadBck");
	bckLoad->setImageTexture("loading_back.jpg");

	bckLoadBar = bckLoad->createWidget<ImageBox>("ImageBox",
		10,56, barSizeX,26, Align::Default, "LoadBckBar");
	bckLoadBar->setImageTexture("loading_bar2.jpg");
	bckLoadBar->setColour(Colour(0.5,0.5,0.5,1));

	barSizeY = 22;
	barLoad = bckLoadBar->createWidget<ImageBox>("ImageBox",
		0,2, 40,22, Align::Default, "LoadBar");
	barLoad->setImageTexture("loading_bar1.jpg");


	txLoadBig = bckLoad->createWidget<TextBox>("TextBox",
		10,8, barSizeX,40, Align::Default, "LoadTbig");
	txLoadBig->setFontName("font.big");
	txLoadBig->setTextColour(Colour(0.7,0.83,1));
	txLoadBig->setCaption(TR("#{LoadingDesc}"));

	txLoad = bckLoad->createWidget<TextBox>("TextBox",
		10,96, barSizeX,40, Align::Default, "LoadT");
	txLoad->setFontName("font.big");
	txLoad->setTextColour(Colour(0.65,0.78,1));
	txLoad->setCaption(TR("#{Initializing}..."));


	///  menu background image
	//  dont show for autoload and no loadingbackground
	// if (!(!pSet->loadingbackground && pSet->autostart))
	{
		imgBack = mGui->createWidget<ImageBox>("ImageBox",
			0,0, 800,600, Align::Default, "ToolTip","ImgBack");
		imgBack->setImageTexture("background2.jpg");
		#ifdef TEST_LOAD
		imgBack->setVisible(false);  //-
		#endif
	}

	///  loading background img  - not used
/*	imgLoad = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 800,600, Align::Default, "Pointer", "ImgLoad");
	#ifdef TEST_LOAD
	imgLoad->setVisible(false);  //-
	#else
	// imgLoad->setVisible(true);
	#endif
*/

	LogO("---- baseSizeGui");
	baseSizeGui();
}

void BaseApp::SetLoadingBar(float pecent)
{
	float p = pecent * 0.01f;
	int s = p * barSizeX; // w = p * 512.f;
	//barLoad->setImageCoord( IntCoord(512-w,0, w,64) );
	barLoad->setSize( s, barSizeY );
}


///  size gui (on resolution change)
//-------------------------------------------------------------------
void BaseApp::baseSizeGui()
{
	int sx = mWindow->getWidth(), sy = mWindow->getHeight() +30; //?
	bckLoad->setPosition(sx/2 - barSizeX/2, sy - barHeight -20);

	//imgBack->setCoord(0,0, sx, sy);
	//return;

	//  fit image to window, preserve aspect
	int ix = 1920, iy = 1200;  // get org img size ...
	int six, siy;  // sized to window
	int oix=0, oiy=0;  // offset pos
	float sa = float(sx)/sy, si = float(ix)/iy;  // aspects
	
	if (si >= sa)  // wider than screen
	{
		siy = sy;  six = si * siy;  // six/siy = si
		oix = (six - sx) / 2;
	}else
	{	six = sx;  siy = six / si;
		oiy = (siy - sy) / 2;
	}
	// imgLoad->setCoord(-oix, -oiy, six, siy);
	if (imgBack)
		imgBack->setCoord(-oix, -oiy, six, siy);
}
