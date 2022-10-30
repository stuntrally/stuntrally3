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

#include <list>
#include <filesystem>


using namespace Demo;
using namespace Ogre;
using namespace std;


namespace Demo
{
	TerrainGame::TerrainGame()
		: TutorialGameState()
		, mPitch( 50.f * Math::PI / 180.f )  // par
		, mYaw( 102 * Math::PI / 180.f )
		//, mIblQuality( IblHigh )  // par
		//, mIblQuality( MipmapsLowest )
	{
		macroblockWire.mPolygonMode = PM_WIREFRAME;
		SetupVeget();
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
		//; pApp->mRoot = root;
		pGame->app = pApp;
		sc = pApp->scn->sc;

		pGame->ReloadSimData();

/*  for game.cfg
track = 
Test1-Flat
Test2-Asphalt
Test3-Bumps
Test4-TerrainBig

Test7-FluidsSmall
Test8-Objects
Test12-Snow

TestC6-Temp
TestC8-Align
TestC9-Jumps
TestC10-Pace
TestC12-SR
TestC13-Rivers

Jng13-Tropic
Jng20-JungleMaze
For12-HighPeaks
Tox2-AcidLakes

Jng25-CantorJungle
For18-MountCaro
Isl17-AdapterIslands
*/
		//  new game
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


		//  Light  ------------------------------------------------
		LogO("---- new light");
		mSunLight = sceneManager->createLight();
		SceneNode *lightNode = rootNode->createChildSceneNode();
		lightNode->attachObject( mSunLight );
		
		// mSunLight->setPowerScale( 1.0f );  // should be * 1..
		mSunLight->setPowerScale( Math::PI * 3 );  //** par! 1.5 2 3* 4
		mSunLight->setType( Light::LT_DIRECTIONAL );
		mSunLight->setDirection( Vector3( 0, -1, 0 ).normalisedCopy() );  //-

		//  ambient  set in update ..
		sceneManager->setAmbientLight(
			// ColourValue( 0.63f, 0.61f, 0.28f ) * 0.04f,
			// ColourValue( 0.52f, 0.63f, 0.76f ) * 0.04f,
			ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
			ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
			Vector3::UNIT_Y );

		//  atmosphere  ------------------------------------------------
	#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
		LogO("---- new Atmosphere");
		mGraphicsSystem->createAtmosphere( mSunLight );
		OGRE_ASSERT_HIGH( dynamic_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() ) );
		AtmosphereNpr *atmosphere = static_cast<AtmosphereNpr *>( sceneManager->getAtmosphere() );
		AtmosphereNpr::Preset p = atmosphere->getPreset();
		p.fogDensity = 0.0002f;  //** par
		p.densityCoeff = 0.27f;  //0.47f;
		p.densityDiffusion = 0.75f;  //2.0f;
		p.horizonLimit = 0.025f;
		// p.sunPower = 1.0f;
		// p.skyPower = 1.0f;
		p.skyColour = Vector3(0.234f, 0.57f, 1.0f);
		p.fogBreakMinBrightness = 0.25f;
		p.fogBreakFalloff = 0.1f;
		// p.linkedLightPower = Math::PI;
		// p.linkedSceneAmbientUpperPower = 0.1f * Math::PI;
		// p.linkedSceneAmbientLowerPower = 0.01f * Math::PI;
		p.envmapScale = 1.0f;
		atmosphere->setPreset( p );
	#endif

		//  camera  ------------------------------------------------
		// mCameraController = new CameraController( mGraphicsSystem, false );
		mGraphicsSystem->getCamera()->setFarClipDistance( 20000.f );  // par far

		camPos = Vector3(10.f, 11.f, 16.f );
		//camPos.y += mTerra->getHeightAt( camPos );
		mGraphicsSystem->getCamera()->setPosition( camPos );
		mGraphicsSystem->getCamera()->lookAt( camPos + Vector3(0.f, -1.6f, -2.f) );
		Vector3 objPos;


		LogO(">>>> Init SR ----");
		Init();
		LogO(">>>> Init SR done ----");

		//  Terrain  ------------------------------------------------
		// CreatePlane();  // fast
		// CreateTerrain();  // 5sec
		// CreateVeget();


		LogO("---- base createScene");

		TutorialGameState::createScene01();
	}


	//  Destroy
	//-----------------------------------------------------------------------------------
	void TerrainGame::destroyScene()
	{
		LogO("---- destroyScene");

		DestroyTerrain();
		DestroyPlane();

		LogO("---- base destroyScene");

		TutorialGameState::destroyScene();

		LogO(">>>> Destroy SR ----");
		Destroy();
		LogO(">>>> Destroy SR done ----");
	}

}
