#include "pch.h"
#include "CGui.h"
#include "CHud.h"
#include "GuiCom.h"
#include "Gui_Popup.h"
#include "GraphicsSystem.h"
#include <OgreLogManager.h>
#include <OgreCamera.h>

//  SR
#include "settings.h"
#include "CGame.h"
#include "game.h"
#include "CScene.h"
#include "CData.h"
#include "TracksXml.h"
#include "CInput.h"

#include <list>
#include <filesystem>
#include <SDL_joystick.h>

using namespace Ogre;
using namespace std;


//  🌟🌟 Init SR game
//-----------------------------------------------------------------------------------------------------------------------------
void App::Load()
{
	Ogre::Timer ti;

	LoadSettings();

	//  Enable joystick events
	LogO("C::J Init open joysticks ----");

	SDL_JoystickEventState(SDL_ENABLE);
	//  Open all available joysticks.  TODO: open them when they are required-
	for (int i=0; i<SDL_NumJoysticks(); ++i)
	{
		SDL_Joystick* js = SDL_JoystickOpen(i);
		if (js)
		{
			mJoysticks.push_back(js);
			const char* s = SDL_JoystickName(js);
			int axes = SDL_JoystickNumAxes(js);
			int btns = SDL_JoystickNumButtons(js);
			//SDL_JoystickNumBalls SDL_JoystickNumHats
			LogO(String("<Joystick> name: ")+s+"  axes: "+toStr(axes)+"  buttons: "+toStr(btns));
		}
	}

	///  Game start
	//----------------------------------------------------------------
	LogO("@@@@ Init game, sounds ----");

	pGame = new GAME(pSet);
	pGame->collision.pApp = this;
	pGame->Start();


	///  new  ----
	scn = new CScene(this);
	data = scn->data;
	hud = new CHud(this);

	gcom = new CGuiCom(this);
	gui = new CGui(this);
	gui->gcom = gcom;
	// hud->gui = gui;
	gui->popup = new GuiPopup();
	// gui->viewBox = new wraps::RenderBoxScene();

	mInputBindListner = gui;
	input = new CInput(this);

	pGame->app = this;
	sc = scn->sc;

	pGame->ReloadSimData();


	mRoot = mGraphicsSystem->getRoot();
	mWindow = mGraphicsSystem->getRenderWindow();
	mSceneMgr = mGraphicsSystem->getSceneManager();
	// mCamera = mGraphicsSystem->getCamera();


	LoadData();  /// loads data xmls

	mThread = new thread(&App::UpdThr, this);
}

//  💥💥
void App::Destroy()
{
	DestroyGui();

	//  close joysticks
	LogO("D::J Close joysticks");
	for (auto* js : mJoysticks)
		SDL_JoystickClose(js);
	mJoysticks.clear();

	delete pGame;
	delete pSet;
	// delete data;
	// delete hud;
	// rest in ~App()
}



//  🌟 Create
//-----------------------------------------------------------------------------------------------------------------------------
void App::createScene01()
{
	LogO(">>>>>>>> Init SR --------");
	
	//  SR cfg, xmls etc
	Ogre::Timer ti;

	Load();
	
	CreateInputs();

	LogO(String(":::* Time load xmls: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");


	//  Gui Init
	InitAppGui();
	gcom->CreateFonts();
	baseInitGui();
	gui->InitGui();


	mGraphicsSystem->mWorkspace = SetupCompositor();
	// mCamera->setFarClipDistance( pSet->view_distance );
	// mCamera->setLodBias( pSet->lod_bias );


	LogO(">>>>>>>> Init SR done ----");

	CreateDebugTextOverlay();

	//  auto load
	if (pSet->autostart)
		NewGame();
}

//-----------------------------------------------------------------------------------
/*
	bad look, add emissive:  Tox  Uni  Vlc
-62 Tropic  -75 HighPeaks  tripl  60 Aus3-Canyon  ter spec-
	clr:
103 Des SandStorm  65 Vlc1-Dark  66 Isl5-Shore  121 Isl-StuntIsl
110 Atm5-Twister  186 Sur4-BlueWinter  

	bad veh look:
ES n,gls  BV blk inter  U8 floor
*/
