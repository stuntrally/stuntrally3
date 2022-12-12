#pragma once
#include "settings.h"
// #include "Gui_Def.h"
#include "BaseApp.h"
#include "Replay.h"
#include "ReplayTrk.h"
#include "cardefs.h"
#include "CarPosInfo.h"
// #include "networkcallbacks.hpp"
// #include "ICSChannelListener.h"
// #include "PreviewTex.h"
#include <thread>
//#include "OgreTextureGpu.h"
#include "OgreHlmsDatablock.h"
#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Ogre {  class SceneNode;  class SceneManager;  class TextureGpu;  class Root;
	class Terra;  class HlmsPbsTerraShadows;  class HlmsDatablock;  }
namespace BtOgre  {  class DebugDrawer;  }
class Scene;  class CScene;  class CData;  class CInput;  class GraphView;
class SETTINGS;  class GAME;  class CHud;  class CGui;  class CGuiCom;
namespace MyGUI {  class Gui;  class Ogre2Platform;  }


enum IblQuality  // reflections
{
	MipmapsLowest,
	IblLow,
	IblHigh
};


class App : public BaseApp,	//public ICS::ChannelListener,
 	public TutorialGameState
{
	//  vars
	Ogre::Vector3 camPos;

	//  input  temp  ----
	int mArrows[11] = {0,0,0,0,0,0,0,0,0,0},
		mKeys[4] = {0,0,0,0};  // sun keys
	int param = 0;  // to adjust
	bool left = false, right = false;  // arrows
public:	
	bool shift = false, ctrl = false;
private:
	int idTrack = 0, idCar = 0;

	//  wireframe
	Ogre::HlmsMacroblock macroblockWire;
	bool wireTerrain = false;

	//  fps overlay
	void generateDebugText();
	Ogre::String generateFpsDebugText();


public:
	//  Gui
	// MyGUI::Gui* mGui = 0;
	// MyGUI::Ogre2Platform* mPlatform = 0;

	void InitGui();//, DestroyGui();

	//  SR
	void LoadDefaultSet(SETTINGS* settings, std::string setFile);
	void Init(), Load();
	void Destroy();


	//  main
	void createScene01() override;
	void destroyScene() override;

	void update( float timeSinceLast ) override;

	//  events
	void keyPressed( const SDL_KeyboardEvent &arg ) override;
	void keyReleased( const SDL_KeyboardEvent &arg ) override;
public:


	//  reflection cube  ----
	Ogre::Camera *mCubeCamera = 0;
	Ogre::TextureGpu *mDynamicCubemap = 0;
protected:
	Ogre::CompositorWorkspace *mDynamicCubemapWorkspace = 0;

	IblQuality mIblQuality = IblLow;  // par in ctor
public:		
	Ogre::CompositorWorkspace *setupCompositor();


	//  terrain  ----
public:
	Ogre::Terra *mTerra = 0;
	void CreateTerrain(), DestroyTerrain();
protected:
	Ogre::String mtrName;
	Ogre::SceneNode *nodeTerrain = 0;
	// Listener to make PBS objects also be affected by terrain's shadows
	Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows = 0;


	//  util mtr, mem
	template <typename T, size_t MaxNumTextures>
	void unloadTexturesFromUnusedMaterials( Ogre::HlmsDatablock *datablock,
											std::set<Ogre::TextureGpu *> &usedTex,
											std::set<Ogre::TextureGpu *> &unusedTex );
	void unloadTexturesFromUnusedMaterials();
	void unloadUnusedTextures();
public:
	void minimizeMemory();
	// void setTightMemoryBudget();
	// void setRelaxedMemoryBudget();


public:
	//  ctor
	App();
	virtual ~App();
	void ShutDown();

	CScene* scn =0;
	CData* data =0;
	Scene* sc = 0;
	
	GAME* pGame =0;


	//  BaseApp init
	Ogre::Root* mRoot =0;
	Ogre::Camera* mCamera =0;
	Ogre::SceneManager* mSceneMgr =0;
	
	bool bLoading =0, bLoadingEnd =0, bSimulating =0;  int iLoad1stFrames =0;
	
	//  has to be in baseApp for camera mouse move
	typedef std::vector<class CarModel*> CarModels;
	CarModels carModels;
	
	///  Game Cars Data
	//  new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][MAX_CARS];  // max 16cars
	int iCurPoses[MAX_CARS];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	
	void newPoses(float time), newPerfTest(float time);  // vdrift
	void updatePoses(float time);  // ogre
	void UpdThr();
	bool mShutDown = false;
	std::thread* mThread = 0;  // 2nd thread for simulation

	//  replay - full, saved by user
	//  ghost - saved on best lap
	//  ghplay - ghost ride replay, loaded if was on disk, replaced when new
	Replay2 replay, ghost, ghplay;
	Rewind rewind;  // to take car back in time (after crash etc.)
	TrackGhost ghtrk;  //  ghtrk - track's ghost

	std::vector<ReplayFrame2> frm;  //size:16  //  frm - used when playing replay for hud and sounds

	bool isGhost2nd;  // if present (ghost but from other car)
	std::vector<float> vTimeAtChks;  // track ghost's times at road checkpoints
	float fLastTime;  // thk ghost total time
		

	// virtual bool frameStart(Ogre::Real time);  //;void DoNetworking();
	// virtual bool frameEnd(Ogre::Real time);
	float fLastFrameDT = 0.01f;

	// bool isTweakTab();
		
	BtOgre::DebugDrawer *dbgdraw = 0;  /// blt dbg


	///  HUD
	CHud* hud = 0;
	bool bSizeHUD = true;


	///  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar(), CreateRoads(), CreateRoadsInt(), CreateTrail(Ogre::Camera* cam);
	void CreateObjects(), DestroyObjects(bool clear), ResetObjects();

	void NewGame(bool force=false);
	void NewGameDoLoad();

	void LoadData();
	bool newGameRpl =0;

	bool dstTrk =0;  // destroy track, false if same
	Ogre::String oldTrack;  bool oldTrkUser =0;
	Ogre::String TrkDir();

	
	//  Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TERRAIN, LS_ROAD, LS_OBJECTS, LS_TREES, LS_MISC, LS_ALL };
	static Ogre::String cStrLoad[LS_ALL+1];
	int curLoadState = 0;
	std::map<int, std::string> loadingStates;

	float mTimer;  // wind,water


	//  mtr from ter  . . . 
	int blendMapSize;
	std::vector<char> blendMtr;
	//; void GetTerMtrIds();


	// void recreateReflections();  // call after refl_mode changed

	// virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);


	//  Input  ----
	/// todo: temp
	float inputs[14] = {0,0,0,0};

	// CInput* input =0;

	// virtual bool keyPressed(const SDL_KeyboardEvent &arg);
	// void channelChanged(ICS::Channel *channel, float currentValue, float previousValue);


	///  Gui
	//-----------------------------------------------------------------
	CGui* gui =0;
	CGuiCom* gcom =0;

	// PreviewTex prvView,prvRoad,prvTer;  // track tab
	// PreviewTex prvStCh;  // champ,chall stage view

	// bool bHideHudBeam;  // hides beam when replay or no road
	// bool bHideHudArr;	// hides arrow when replay,splitscreen
	// bool bHideHudPace;  // hides pacenotes when same or deny by challenge
	// bool bHideHudTrail; // hides trail if denied by challenge
	
	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iRplCarOfs, iRplSkip;

	//  race pos
	int GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints=0);
	float GetCarTimeMul(const std::string& car, const std::string& sim_mode);

	void Ch_NewGame();


	///  graphs
	// std::vector<GraphView*> graphs;
	// void CreateGraphs(), DestroyGraphs();
	// void UpdateGraphs(), GraphsNewVals();

	///* tire edit */
	// const static int TireNG;
	// int iEdTire, iTireLoad, iCurLat,iCurLong,iCurAlign, iUpdTireGr;

	///  car perf test
	// bool bPerfTest;  EPerfTest iPerfTestStage;
	// void PerfLogVel(class CAR* pCar, float time);
};
