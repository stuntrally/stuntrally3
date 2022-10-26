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

#ifdef OGRE_BUILD_COMPONENT_ATMOSPHERE
#    include "OgreAtmosphereNpr.h"
#endif

//  SR
#include "pathmanager.h"
#include "settings.h"
#include "CGame.h"
#include "game.h"

#include <list>
#include <filesystem>


using namespace Demo;
using namespace Ogre;
using namespace std;


namespace Demo
{
    TerrainGame::TerrainGame( const String &helpDescription )
        : TutorialGameState( helpDescription )
        , mPitch( 50.f * Math::PI / 180.f )  // par
        , mYaw( 102 * Math::PI / 180.f )
        //, mIblQuality( IblHigh )  // par
        , mIblQuality( MipmapsLowest )
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
        settings = new SETTINGS();
        string setFile = PATHMANAGER::SettingsFile();
        
        if (!PATHMANAGER::FileExists(setFile))
        {
            cerr << "Settings not found - loading defaults." << endl;
            LoadDefaultSet(settings,setFile);
        }
        settings->Load(setFile);  // LOAD
        if (settings->version != SET_VER)  // loaded older, use default
        {
            cerr << "Settings found, but older version - loading defaults." << endl;
            std::filesystem::rename(setFile, PATHMANAGER::UserConfigDir() + "/game_old.cfg");
            LoadDefaultSet(settings,setFile);
            settings->Load(setFile);  // LOAD
        }


        //  paths
        LogO(PATHMANAGER::info.str());


        ///  Game start
        //----------------------------------------------------------------
        pGame = new GAME(settings);
        pGame->Start();

        pApp = new App(settings, pGame);
        //; pApp->mRoot = root;
        pGame->app = pApp;


    	pApp->NewGame(true);

        // pGame->Tick();
    }

    void TerrainGame::Destroy()
    {
        if (pGame)
		    pGame->End();
	
        delete pApp;
        delete pGame;
        delete settings;
    }


    
    //  Create
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::createScene01()
    {
        LogO(">>>> Init SR ----");
        Init();
        LogO(">>>> Init SR done ----");


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
        
        mSunLight->setPowerScale( Math::PI * 3 );  //** par! 1.5 2 3* 4  should be * 1..
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
        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 100000.f );  // par far

        //camPos = Vector3(-10.f, 80.f, 10.f );
        //camPos = Vector3(-2005.f, 40.f, -929.f);
        camPos = Vector3(-52.f, mTerra ? 735.f : 60.f, mTerra ? 975.f : 517.f);
        //camPos.y += mTerra->getHeightAt( camPos );
        mGraphicsSystem->getCamera()->setPosition( camPos );
        mGraphicsSystem->getCamera()->lookAt( camPos + Vector3(0.f, -0.5f, -1.f) );
        Vector3 objPos;


        //  Terrain  ------------------------------------------------
        CreatePlane();  // fast
        // CreateTerrain();  // 5sec
        // CreateVeget();


        LogO("---- tutorial createScene");

        TutorialGameState::createScene01();
    }


    //  Destroy
    //-----------------------------------------------------------------------------------
    void TerrainGame::destroyScene()
    {
        LogO("---- destroyScene");

        DestroyTerrain();
        DestroyPlane();

        LogO("---- tutorial destroyScene");

        TutorialGameState::destroyScene();

        LogO(">>>> Destroy SR ----");
        Destroy();
        LogO(">>>> Destroy SR done ----");
    }

}
