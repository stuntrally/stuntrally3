#include "pch.h"
#include "Def_Str.h"
#include "CGui.h"
#include "CHud.h"
#include "GuiCom.h"
#include "Gui_Popup.h"
#include "GraphicsSystem.h"
#include "MainEntryPoints.h"

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

#include <OgreLogManager.h>
#include <OgreCamera.h>
using namespace Ogre;
using namespace std;


//  🌟🌟 Init SR game
//-----------------------------------------------------------------------------------------------------------------------------
void App::Load()
{
	Ogre::Timer ti;

	LoadSettings();


	//  📡 Helper for testing networked games on 1 computer
	//  use number > 0 in command parameter,  adds it to nick, port and own ogre.log
	auto& args = MainEntryPoints::args;

	int num = -1;
	LogO("A--- Cmd Line Arguments: "+toStr(args.all.size()));
	LogO("A--- exe path: "+args.all[0]);
	
	if (args.all.size() > 1)
	{
		LogO("A--- Argument1: "+args.all[1]);
		num = Ogre::StringConverter::parseInt(args.all[1]);
	}
	if (num > 0)
	{
		pSet->net_local_plr = num;
		pSet->local_port += num;
		pSet->nickname += Ogre::StringConverter::toString(num);
	}


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
	// fixme ..
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


	// mGraphicsSystem->mWorkspace =
	SetupCompositor();
	// mCamera->setFarClipDistance( pSet->view_distance );
	// mCamera->setLodBias( pSet->lod_bias );


	LogO(">>>>>>>> Init SR done ----");

	CreateDebugTextOverlay();

	//  auto load
	if (pSet->autostart)
		NewGame();
}
