#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "SDL_scancode.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreWindow.h"
#include "OgreFrameStats.h"

#include "Terra/Terra.h"
#include "OgreItem.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreTextureGpuManager.h"
#include "OgrePixelFormatGpuUtils.h"
#include "mathvector.h"
#include "quaternion.h"

#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
#    include "OgreAtmosphereNpr.h"
#endif

//  SR
#include "pathmanager.h"
#include "settings.h"
#include "CGame.h"
#include "game.h"
#include "CarModel.h"
#include "CScene.h"
#include "CData.h"
#include "TracksXml.h"

#include <list>
#include <filesystem>


using namespace Demo;
using namespace Ogre;
using namespace std;


namespace Demo
{
	TerrainGame::TerrainGame()
		: TutorialGameState()
		//, mIblQuality( IblHigh )  // par
		//, mIblQuality( MipmapsLowest )
	{
		macroblockWire.mPolygonMode = PM_WIREFRAME;
	}


	//  load settings from default file
	void TerrainGame::LoadDefaultSet(SETTINGS* settings, string setFile)
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
	void TerrainGame::Init()
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
		pGame = new GAME(pSet);
		pGame->Start();

		pApp = new App(pSet, pGame);
		pApp->mainApp = this;
		pGame->app = pApp;
		sc = pApp->scn->sc;

		pGame->ReloadSimData();

		//  new game
		pApp->mRoot = mGraphicsSystem->getRoot();
		pApp->mCamera = mGraphicsSystem->getCamera();
		pApp->mSceneMgr = mGraphicsSystem->getSceneManager();
		pApp->mDynamicCubemap = mDynamicCubemap;

		pApp->CreateScene();  /// New
		
	}

	void TerrainGame::Destroy()
	{
		if (pGame)
			pGame->End();
	
		delete pApp;
		delete pGame;
		delete pSet;
	}


	
	//  Create
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::createScene01()
	{

		mGraphicsSystem->mWorkspace = setupCompositor();

		SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
		SceneNode *rootNode = sceneManager->getRootSceneNode( SCENE_STATIC );

		LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);

		LogO("---- createScene");
		Root *root = mGraphicsSystem->getRoot();
		RenderSystem *renderSystem = root->getRenderSystem();
		renderSystem->setMetricsRecordingEnabled( true );


		//  camera  ------------------------------------------------
		mGraphicsSystem->getCamera()->setFarClipDistance( 20000.f );  // par far
		// mGraphicsSystem->getCamera()->setFarClipDistance( pSet->view_distance );  // par far

		camPos = Vector3(10.f, 11.f, 16.f );
		//camPos.y += mTerra->getHeightAt( camPos );
		mGraphicsSystem->getCamera()->setPosition( camPos );
		mGraphicsSystem->getCamera()->lookAt( camPos + Vector3(0.f, -1.6f, -2.f) );
		Vector3 objPos;


		LogO(">>>> Init SR ----");
		Init();
		LogO(">>>> Init SR done ----");

		//  find cur id
		const auto* data = pApp->scn->data;
		idTrack = data->tracks->trkmap[pSet->gui.track] -1;
		idCar = data->cars->carmap[pSet->gui.car[0]] -1;


		LogO("---- base createScene");

		TutorialGameState::createScene01();
	}


	//  Destroy
	//-----------------------------------------------------------------------------------
	void TerrainGame::destroyScene()
	{
		LogO("---- destroyScene");

		DestroyTerrain();

		LogO("---- base destroyScene");

		TutorialGameState::destroyScene();

		LogO(">>>> Destroy SR ----");
		Destroy();
		LogO(">>>> Destroy SR done ----");
	}

}

/*  for game.cfg
Test7-FluidsSmall
Test8-Objects
TestC6-Temp
	good
Jng25-CantorJungle  +
Mos6-TaraMosses  ~
Des17-NileCity  +

	errors: // fixme
Grc8-SlopeCity - model? crash
Sur3-Chaotic  -
Isl17-AdapterIslands  -

Atm9-SchwabAutumn  -5m! tris trees
Vlc4-Spikeland  -ter fps low
*/
