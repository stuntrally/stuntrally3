#include "pch.h"
#include "CGui.h"
#include "CHud.h"
#include "GuiCom.h"
#include "Gui_Popup.h"
#include "GraphicsSystem.h"
#include "OgreLogManager.h"
#include "OgreCamera.h"

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


//  load settings from default file
void App::LoadDefaultSet(SETTINGS* settings, string setFile)
{
	settings->Load(PATHMANAGER::GameConfigDir() + "/game-default.cfg");
	settings->Save(setFile);

	//  delete old keys.xml too
	string sKeys = PATHMANAGER::UserConfigDir() + "/keys.xml";
	if (std::filesystem::exists(sKeys))
		std::filesystem::rename(sKeys, PATHMANAGER::UserConfigDir() + "/keys_old.xml");
}


//  Init SR game
//-----------------------------------------------------------------------------------------------------------------------------
void App::Load()
{

	Ogre::Timer ti;
	setlocale(LC_NUMERIC, "C");

	//  Paths
	PATHMANAGER::Init();
	

	///  Load Settings
	//----------------------------------------------------------------
	pSet = new SETTINGS();
	string setFile = PATHMANAGER::SettingsFile();
	
	if (!PATHMANAGER::FileExists(setFile))
	{
		cerr << "Settings not found - loading defaults." << endl;
		LoadDefaultSet(pSet,setFile);
	}
	pSet->Load(setFile);  // LOAD
	if (pSet->version != SET_VER)  // loaded older, use default
	{
		cerr << "Settings found, but older version - loading defaults." << endl;
		std::filesystem::rename(setFile, PATHMANAGER::UserConfigDir() + "/game_old.cfg");
		LoadDefaultSet(pSet,setFile);
		pSet->Load(setFile);  // LOAD
	}


	//  paths
	LogO(PATHMANAGER::info.str());


	///  Game start
	//----------------------------------------------------------------
	LogO("@ @  Init game, sounds");

	pGame = new GAME(pSet);
	pGame->collision.pApp = this;
	pGame->Start();


	///  new
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
	mCamera = mGraphicsSystem->getCamera();
	mSceneMgr = mGraphicsSystem->getSceneManager();


	LoadData();  /// loads data xmls

	mThread = new thread(&App::UpdThr, this);
}

void App::Destroy()
{
	DestroyGui();

	delete pGame;
	delete pSet;
	// delete data;
	// delete hud;
	// rest in ~App()
}



//  Create
//-----------------------------------------------------------------------------------------------------------------------------
void App::createScene01()
{
	LogO(">>>> Init SR ----");
	
	//  SR cfg, xmls etc
	Load();
	
	createInputs();

	//  Gui Init
	baseInitGui(mGraphicsSystem);
	gui->mGui = mGui;
	gui->InitGui();


	mGraphicsSystem->mWorkspace = setupCompositor();


	SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
	SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );

	LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);

	LogO("---- createScene");
	RenderSystem *renderSystem = mGraphicsSystem->getRoot()->getRenderSystem();
	renderSystem->setMetricsRecordingEnabled( true );


	//  camera  ------------------------------------------------
	mGraphicsSystem->getCamera()->setFarClipDistance( 20000.f );  // par far
	// mGraphicsSystem->getCamera()->setFarClipDistance( pSet->view_distance );  // par far

	camPos = Vector3(10.f, 11.f, 16.f );
	//camPos.y += mTerra->getHeightAt( camPos );
	mGraphicsSystem->getCamera()->setPosition( camPos );
	mGraphicsSystem->getCamera()->lookAt( camPos + Vector3(0.f, -1.6f, -2.f) );
	Vector3 objPos;


	LogO(">>>> Init SR done ----");

	LogO("---- base createScene");

	TutorialGameState::createScene01();
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
