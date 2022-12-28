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

using namespace Ogre;
using namespace std;


//  🌟🌟 Init SR game
//-----------------------------------------------------------------------------------------------------------------------------
void App::Load()
{
	Ogre::Timer ti;

	LoadSettings();


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
	mCamera = mGraphicsSystem->getCamera();


	LoadData();  /// loads data xmls

	mThread = new thread(&App::UpdThr, this);
}

//  💥💥
void App::Destroy()
{
	DestroyGui();

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
	Load();
	
	CreateInputs();


	//  Gui Init
	InitAppGui();
	baseInitGui();
	gui->mGui = mGui;
	gui->InitGui();


	mGraphicsSystem->mWorkspace = SetupCompositor();

	mCamera->setFarClipDistance( pSet->view_distance );


	LogO(">>>>>>>> Init SR done ----");

	CreateDebugTextOverlay();

	//  auto load
	if (pSet->autostart)
		NewGame();
}

//-----------------------------------------------------------------------------------
/*
	good scn +
Aln Cry Atm  Aus Blk Can Des  Fin For Grc  Jng Mos  Mud Sav  Spc Stn Sur  Wnt

	bad scn -  // fixme look
Mrs- all  Tox- all  Uni- all  Vlc emis
-62 Tropic  -75 HighPeaks  tripl  60 Aus3-Canyon  ter spec-
clr:
103 Des SandStorm  65 Vlc1-Dark  66 Isl5-Shore  121 Isl-StuntIsl
110 Atm5-Twister  186 Sur4-BlueWinter  

	bad cars
3B uv,clr  ES n,gls  OT TW wh-  U8 O gls-

	low fps
167 Taiga  grass trees
191 SandyMountain
	border 60fps
128 Beauty
 62 Tropic
 84 JungleCanyons
 54 Grc Long
114 Fin Lakes
 14 Cross

*/
