#include "pch.h"
#include "par.h"
#include "paths.h"
#include "Road.h"
#include "Def_Str.h"
#include "CData.h"
#include "SceneXml.h"
#include "CScene.h"
#include "Grass.h"
#include "GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "game.h"
#include "PaceNotes.h"
// #include "SoundMgr.h"
// #include "SoundBaseMgr.h"
#include "FollowCamera.h"
#include "CarModel.h"
#include "BtOgreExtras.h"
#include "gameclient.hpp"
#include "Terra.h"
#include "MainEntryPoints.h"
#include "HlmsPbs2.h"

#include <OgreCommon.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreSceneManager.h>
#include <OgreParticleSystem.h>
#include <OgreResourceGroupManager.h>
#include <OgreImage2.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreTextureGpuManager.h>
#include <Compositor/OgreCompositorWorkspace.h>

#include <MyGUI_TextBox.h>
#include <MyGUI_Window.h>
#include "MessageBox.h"
using namespace MyGUI;
using namespace Ogre;
using namespace std;


bool Args::Help()  // ❔
{
	if (has("?") || has("help"))
	{
		cout << "StuntRally3  command line Arguments help  ----\n";
		Common("");
		cout << "  check - Does a check for tracks.ini, championships.xml, challenges.xml, missing track ghosts, etc.\n";
		cout << "  \n";
		cout << "  convert - For new tracks. Convert ghosts to track's ghosts (less size and frames)\n";
		cout << "     Put original ghosts into  data/ghosts/original/*_ES.rpl\n";
		cout << "     ES car, normal sim, 1st lap, no boost, use rewind type: Go back time\n";
		cout << "     Time should be like in tracks.ini or less (last T= )\n";
		cout << "  \n";
		cout << "  ghosts - Test all vehicles points on all tracks, from all user ghosts. Needs many.\n";
		cout << "  trkghosts - Test all tracks ghosts, for sudden jumps, due to bad rewinds.\n";
		return 1;
	}
	return 0;
}


//  📄 Load Scene Data
//-------------------------------------------------------------------------------------
void App::LoadData()
{
	//  🖼️ prv tex  todo
	prvView.mgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	int k = 1024;
	prvView.Create(k,k,"PrvView");
	prvRoad.Create(k,k,"PrvRoad");
	 prvTer.Create(k,k,"PrvTer");
	prvStCh.Create(k,k,"PrvStCh");  // chs stage
	
	/*scn->roadDens.Create(k+1,k+1,"RoadDens");  // var size..
	
	mLoadingBar->loadTex.Create(1920,1200,"LoadingTex");*/

	
	//  restore camNums
	for (int i=0; i < MAX_Players; ++i)
		if (pSet->cam_view[i] >= 0)
			carsCamNum[i] = pSet->cam_view[i];

	Ogre::Timer ti;


	///  🧰 _Tool_ check tracks, champs, challs  ............
	auto& args = MainEntryPoints::args;
	// args.set("check");  // debug
	bool check = args.has("check");


	//  data xmls
	pGame->ReloadSimData();  // need surfaces
	
	scn->data->Load(&pGame->surf_map, check);
	scn->sc->pFluidsXml = scn->data->fluids;
	scn->sc->pReverbsXml = scn->data->reverbs;

	//  set fluids in Pbs2
	auto* mgr = mRoot->getHlmsManager();
	HlmsPbs2 *hlmsPbs2 = static_cast<HlmsPbs2 *>( mgr->getHlms( HLMS_PBS ) );
	hlmsPbs2->pFluidsXml = scn->sc->pFluidsXml;

	//  championships.xml, progress.xml
	gui->Ch_XmlLoad();


	//  rpl sizes
	ushort u(0x1020);
	struct SV{  std::vector<int> v;  };
	int sv = sizeof(SV), sr2 = sizeof(ReplayFrame2)-3*sv, wh2 = sizeof(RWheel);

	LogO(String("**** ReplayFrame size ") +toStr(sr2)+" + "+toStr(wh2)+" *4 (wh) = "+toStr(sr2+4*wh2));
	LogO(String("**** Replay test sizes: 12244: ") + toStr(sizeof(char))+","+toStr(sizeof(short))+
		","+toStr(sizeof(half))+","+toStr(sizeof(float))+","+toStr(sizeof(int))+"  vec: "+toStr(sv)+
		"   hi,lo 16,32: "+toStr(*((uchar*)&u+1))+","+toStr(*((uchar*)&u)));

	LogO(String(":::* Time load xmls: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();


	///  rpl test-
	bool quit = 0;
	#if 0
	std::string file = PATHS::Ghosts() + "/normal/TestC4-ow_V2.rpl";
	replay.LoadFile(file);  quit = 1;
	#endif
	

	///  🧰 _Tool_ convert to track's ghosts ..............
	// args.set("convert");  // test
	if (args.has("convert"))
	{	gui->ToolGhostsConv();  quit = 1;  }

	///  🧰 _Tool_ ghosts times .......
	if (args.has("ghosts"))
	{	gui->ToolGhosts();  quit = 1;  }

	///  _Tool_ test track's ghosts ..............
	if (args.has("trkghosts"))
	{	gui->ToolTestTrkGhosts();  quit = 1;  }

	///  🧰 _Tool_ presets .......
	// if (args.has("presets"))
	// {	gui->ToolPresets();  quit = 1;  }

	if (quit || check)
		exit(0);
	

	//  Gui  * * *
	if (pSet->startInMain)
		pSet->iMenu = MN1_Main;

	if (!pSet->autostart)
		isFocGui = true;

	pSet->gui.champ_num = -1;  //dont auto start old chs
	pSet->gui.chall_num = -1;

	bRplRec = pSet->rpl_rec;  // startup setting


	//  bullet Debug drawing
	//------------------------------------
	if (pSet->bltLines)
	{	dbgdraw = new BtOgre::DebugDrawer(
			mSceneMgr->getRootSceneNode(SCENE_STATIC),
			pGame->collision.world, mSceneMgr);
		pGame->collision.world->setDebugDrawer(dbgdraw);
		pGame->collision.world->getDebugDrawer()->setDebugMode(
			1 /*0xfe/*8+(1<<13)*/);
	}
}


//---------------------------------------------------------------------------------------------------------------
///  🏁 New Game
//---------------------------------------------------------------------------------------------------------------
void App::NewGame(bool force, bool perfTest)
{
	bPerfTest = perfTest;

	//  actual loading isn't done here
	isFocGui = false;
	if (gui->bGI)
		gui->toggleGui(false);  // hide gui
	// mWndNetEnd->setVisible(false);
 
	bLoading = true;  iLoad1stFrames = 0;
	carIdWin = 1;  iRplCarOfs = 0;
	iCollectedPrize = -2;

	//  wait until sim finishes
	while (bSimulating)
		Threads::Sleep( pSet->thread_sleep );

	bRplPlay = 0;  iRplSkip = 0;
	pSet->rpl_rec = bRplRec;  // changed only at new game
	gui->pChall = 0;
	
	
	if (!newGameRpl)  // if from replay, dont
	{
		///* 👥 copy game config from gui *
		pSet->game = pSet->gui;
		if (gPar.carPrv > 0)
			pSet->game.track = "Sav8-SlowFrenzy";
		
		//  👀 set VR, only 1 player
		pSet->game.vr_mode = pSet->vr_mode;
		if (pSet->game.vr_mode)
			pSet->game.local_players = 1;
		
		Ch_NewGame();

		if (mClient && mLobbyState != HOSTING)  // 📡 all but host
			gui->updateGameSet();  // override gameset params for networked game (from host gameset)
		if (mClient)  // for all, including host
			pSet->game.local_players = 1;
	}
	newGameRpl = false;

	///<>  same track
	dstTrk = force || oldTrack != pSet->game.track || oldTrkUser != pSet->gui.track_user;

	///  check if track exist ..
	if (!PATHS::FileExists(gcom->TrkDir()+"scene.xml"))
	{
		bLoading = false;  iLoad1stFrames = -2;
		LogO("TRACK doesn't exist !!");
		gui->BackFromChs();
		//toggleGui(true);  // show gui
		Message::createMessageBox("Message", TR("#{Track}"),
			TR("#{TrackNotFound}")+"\n" + pSet->game.track +
			(pSet->game.track_user ? " *"+TR("#{TweakUser}")+"*" :"") + "\nPath: " + gcom->TrkDir(),
			MessageBoxStyle::IconError | MessageBoxStyle::Ok);
		return;
	}	
	if (mWndRpl)     mWndRpl->setVisible(false);
	if (mWndRplTxt)  mWndRplTxt->setVisible(false);  // hide rpl ctrl

	LoadingOn();
	hud->Show(true);  // hide HUD
	//mFpsOverlay->hide();  // hide FPS
	hideMouse();

	curLoadState = 0;
}


/* *  Loading steps (in this order)  * */
//---------------------------------------------------------------------------------------------------------------

//  💥 Cleanup  1
void App::LoadCleanUp()
{
	LogO("DD-- LoadCleanUp ------DD");
	updMouse();

	DestroyGI();  // 🌄
	
	if (dstTrk)
	{
		scn->refl.DestroyFluids();
		scn->refl.DestroyRTT();

		DestroyObjects(true);   // 📦🏢
		DestroyCollects(true);  // 💎
		DestroyFields(true);    // 🎆
	}
	
	DestroyGraphs();
	hud->Destroy();
	
	//  hide hud arrow,beam,pace,trail
	bool morePlr = pSet->game.local_players > 1;
	bool rplRd = bRplPlay /*|| scn->road && scn->road->getNumPoints() < 2/**/;
	bool rplHide = bRplPlay && pSet->rpl_hideHudAids;

	bHideHudBeam = rplRd;
	bHideHudArr = rplRd /*|| morePlr*/;
	bool denyPace = gui->pChall && !gui->pChall->pacenotes;
	bHideHudPace = /*morePlr ||*/ denyPace || rplHide;  // todo: mplr ?
	bool denyTrail = gui->pChall && !gui->pChall->trail;
	bHideHudTrail = /*morePlr ||*/ denyTrail || rplHide;


	// rem old track
	if (dstTrk)
	{
		if (resTrk != "")
			ResourceGroupManager::getSingleton().removeResourceLocation(resTrk);
		LogO(">>>>----  Loading track: "+pSet->game.track + "\n");
		resTrk = gcom->TrkDir() + "objects";  // for roadDensity.png
		ResourceGroupManager::getSingleton().addResourceLocation(resTrk, "FileSystem");
	}

	//  Delete all cars
	for (int i=0; i < carModels.size(); ++i)
	{
		CarModel* c = carModels[i];
		if (c && c->fCam)
		{
			carsCamNum[i] = 
				c->iLoopLastCam != -1 ? c->iLoopLastCam +1 :  //o
				c->fCam->miCurrent +1;  // save which cam view
			if (i < MAX_Players)
				pSet->cam_view[i] = carsCamNum[i];
		}
		delete c;
	}
	carModels.clear();  //carPoses.clear();


	LogO("DD## Destroy All track --DD");
	if (dstTrk)
	{
		scn->DelRoadDens();
		scn->grass->Destroy();  // 🌿
		scn->DestroyVegets();    // 🌳🪨

		DestroyObjects(true);   // 📦🏢
		DestroyCollects(true);  // 💎
		DestroyFields(true);    // 🎆

		scn->DestroyRoads();    // 🛣️
		scn->DestroyTerrains(); // ⛰️
		//^ cars
		scn->refl.DestroyFluids();   // 💧
		scn->refl.DestroyRTT();      // 💧
		
		scn->DestroyEmitters(true);  // 🔥
		scn->DestroyAllAtmo();  // 🌦️
	}
	scn->DestroyTrails();


	///  destroy all  shouldn't be needed..
	LogO("#### Destroy All ----");
	if (dstTrk)
	{	// destroy all scenenodes
		mSceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();
		// MaterialManager::getSingleton().destroyAllResourcePools();
		mSceneMgr->destroyAllParticleSystems();
		// mSceneMgr->destroyAllRibbonTrails();
		// mSplitMgr->mGuiSceneMgr->destroyAllManualObjects(); //-
	}

	/*LogO("------  # Unload prev track res");
	MeshManager::getSingleton().unloadUnreferencedResources();
	sh::Factory::getInstance().unloadUnreferencedMaterials();
	Ogre::TextureManager::getSingleton().unloadUnreferencedResources();
	// TextureGpuManager::getEntries() Singleton().unloadUnreferencedResources();
	LogO("------  # Unload prev track res done");*/

	if (dstTrk)
	{
		MinimizeMemory();  // !

		//** todo: temp, split screen disables refract
		//  compositor goes bad, wont restore

		/*if (pSet->game.local_players > 1 &&
			pSet->g.water_refract)
		{	pSet->g.water_refract = false;  gcom->ckWaterRefract.Upd();  }
		*/
		SetupCompositors();  //+ ok
	}
}


//---------------------------------------------------------------------------------------------------------------

//  🚀 Game  2
void App::LoadGame()
{
	//  viewports
	// int numRplViews = std::max(1, std::min( int(replay.header.numPlayers), pSet->rpl_numViews ));
	// mSplitMgr->mNumViewports = bRplPlay ? numRplViews : pSet->game.local_players;  // set num players
	// mSplitMgr->Align();
	// mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);
	
	pGame->LeaveGame(dstTrk);

	if (gui->bReloadSim)  // after gui cmb
	{	gui->bReloadSim = false;
		pGame->ReloadSimData();

		static bool f1 = true;
		if (f1) {  f1 = false;
			gui->updSld_TwkSurf(0);  }
	}
	
	///<>  save old track
	oldTrack = pSet->game.track;  oldTrkUser = pSet->game.track_user;
	
	
	//  load scene.xml - default if not found
	//  need to know sc->asphalt before vdrift car load
	if (dstTrk)
	{
		scn->sc->surf_map = &pGame->surf_map;
		scn->sc->LoadXml(gcom->TrkDir()+"scene.xml");
		// pGame->track.asphalt = scn->sc->asphalt;  //*
		// pGame->track.sDefaultTire = scn->sc->asphalt ? "asphalt" : "gravel";  //*
		if (scn->sc->denyReversed)
			pSet->game.track_reversed = false;

		pGame->NewGameDoLoadTrack();
	}

	//  🔉 set normal reverb
	// pGame->snd->sound_mgr->SetReverb(scn->sc->revSet.normal);
	
	//  upd car abs,tcs,sss
	pGame->ProcessNewSettings();

		
	///  🚗 init car models
	///--------------------------------------------
	//  will create vdrift cars, actual car loading will be done later in LoadCar()
	//  this is just here because vdrift car has to be created first
	
	int numCars = mClient ? mClient->getPeerCount()+1 :  // 📡 networked
		pSet->game.local_players;  // or 👥 splitscreen
	int i;
	for (i = 0; i < numCars; ++i)
	{
		// This only handles one local player
		CarModel::eCarType et = CarModel::CT_LOCAL;
		int startId = i;
		std::string carName = pSet->game.car[std::min(MAX_Players-1,i)], nick = "";
		if (mClient)  // 📡
		{
			// Various places assume carModels[0] is local
			// so we swap 0 and local's id but preserve starting position
			if (i == 0)  startId = mClient->getId();
			else  et = CarModel::CT_REMOTE;

			if (i == mClient->getId())  startId = 0;
			if (i != 0)  carName = mClient->getPeer(startId).car;

			//  get nick name
			if (i == 0)  nick = pSet->nickname;
			else  nick = mClient->getPeer(startId).name;
		}

		//  need road looped here
		String sRd = gcom->PathListTrk() + "/road.xml";
		SplineRoad rd(pGame);  rd.LoadFile(sRd,false);
		bool loop = //rd.getNumPoints() < 2 ? false :
			!rd.isLooped && pSet->game.track_reversed ? true : false;
		
		CarModel* car = new CarModel(i, i, et, carName, &mCams[i], this);
		car->Load(startId, loop);
		carModels.push_back(car);
		
		if (nick != "")  // set remote nickname // 📡
		{	car->sDispName = nick;
			if (i != 0)  // not for local
				car->pNickTxt = hud->CreateNickText(i, car->sDispName);
		}
	}

	///  👻 ghost car - last in carModels
	///--------------------------------------------
	ghPlay.Clear();
	if (!bRplPlay/*|| pSet->rpl_show_ghost)*/ && pSet->rpl_ghost && !mClient)
	{
		std::string ghCar = pSet->game.car[0], orgCar = ghCar;
		ghPlay.LoadFile(gui->GetGhostFile(pSet->rpl_ghostother ? &ghCar : 0));
		isGhost2nd = ghCar != orgCar;
		
		//  always because ghplay can appear during play after best lap
		// 1st ghost = orgCar
		CarModel* c = new CarModel(i, MAX_Players, CarModel::CT_GHOST, orgCar, 0, this);
		c->Load(-1, false);
		c->pCar = (*carModels.begin())->pCar;  // based on 1st car
		carModels.push_back(c);

		//  2st ghost - other car
		if (isGhost2nd)
		{
			CarModel* c = new CarModel(i, MAX_Players, CarModel::CT_GHOST2, ghCar, 0, this);
			c->Load(-1, false);
			c->pCar = (*carModels.begin())->pCar;
			carModels.push_back(c);
		}
	}
	///  🏞️👻 track's ghost  . . .
	///--------------------------------------------
	ghTrk.Clear();  vTimeAtChks.clear();
	bool deny = gui->pChall && !gui->pChall->trk_ghost;
	if (!bRplPlay /*&& pSet->rpl_trackghost-*/ && !mClient && !pSet->game.track_user && !deny)
	{
		std::string sRev = pSet->game.track_reversed ? "_r" : "";
		std::string file = PATHS::TrkGhosts()+"/"+ pSet->game.track + sRev + ".gho";
		if (ghTrk.LoadFile(file))
		{
			CarModel* c = new CarModel(i, MAX_Players+1, CarModel::CT_TRACK, "ES", 0, this);
			c->Load(-1, false);
			c->pCar = (*carModels.begin())->pCar;  // based on 1st car
			carModels.push_back(c);
	}	}

	float pretime = mClient ? 2.0f : pSet->game.pre_time;  // same for all multi players
	if (bRplPlay)  pretime = 0.f;
	if (mClient)  // 📡
	{	pGame->timer.waiting = true;  //+
		pGame->timer.end_sim = false;
	}

	pGame->NewGameDoLoadMisc(pretime,
		scn->sc->ambientSnd, scn->sc->ambientVol);
}
//---------------------------------------------------------------------------------------------------------------


//  Scene
void App::AddListenerRnd2Tex()
{
	if (scn->refl.mWsListener)
	for (auto* ws : vWorkspaces)
		ws->addListener(scn->refl.mWsListener);
}

void App::LoadScene()  // 3
{
	//  ⛅ sun, fog, weather, sky
	if (dstTrk)
		scn->CreateAllAtmo();

	if (dstTrk)
		scn->CreateEmitters();  // 🔥


	//  💧 Fluids
	if (dstTrk)
	{
		scn->refl.CreateRTT();
		AddListenerRnd2Tex();
		scn->refl.CreateFluids();
	}

	if (dstTrk)
		pGame->collision.world->setGravity(btVector3(0.0, 0.0, -scn->sc->gravity));


	//  🔝 create arrows
	bool col = pSet->game.collect_num >= 0;
	bool deny = gui->pChall && !gui->pChall->chk_arr;
	bool road = scn->road && scn->road->getNumPoints() > 2;
	if (!bHideHudArr && !deny)
		for (int i=0; i < carModels.size(); ++i)
		if (!carModels[i]->isGhost())
		{
			if (!col && road)  // chk
				hud->arrChk[i].Create(mSceneMgr, pSet, i);
			
			if (col)
			for (int a = 0; a < MAX_ArrCol; ++a)
			{	//  collect
				hud->arrCol[i][a].node = carModels[i]->ndMain;
				hud->arrCol[i][a].Create(mSceneMgr, pSet, i);
				hud->arrCol[i][a].it->setDatablockOrMaterialName("collect_arrow");				
			}
		}
	
	//  win cups
	hud->cup[0].Create(mSceneMgr, pSet, 0, 0.9f, "cup_bronze.mesh", "");
	hud->cup[1].Create(mSceneMgr, pSet, 0, 1.0f, "cup_silver.mesh", "");
	hud->cup[2].Create(mSceneMgr, pSet, 0, 1.0f, "cup_gold.mesh", "");
}


//  🚗 Cars  4
//---------------------------------------------------------------------------------------------------------------
void App::LoadCar()
{
	//  🚗 Create all cars
	for (int i=0; i < carModels.size(); ++i)
	{
		CarModel* c = carModels[i];
		c->Create();

		///  challenge off abs,tcs
		if (gui->pChall && c->pCar)
		{
			if (!gui->pChall->abs)  c->pCar->dynamics.SetABS(false);
			if (!gui->pChall->tcs)  c->pCar->dynamics.SetTCS(false);
		}

		//  restore which cam view
		if (c->fCam && carsCamNum[i] != 0)
		{
			c->fCam->setCamera(carsCamNum[i] -1);
			
			//; int visMask = c->fCam->ca->mHideGlass ? RV_MaskAll-RV_CarGlass : RV_MaskAll;
			// for (auto vp : mSplitMgr->mViewports)
			// 	vp->setVisibilityMask(visMask);
		}
		iCurPoses[i] = 0;
	}
	if (!dstTrk)  // reset objects if same track
		pGame->bResetObj = true;
	
	
	///  📽️ Init Replay  header, once
	///----------------------------------
	ReplayHeader2& rh = replay.header, &gh = ghost.header;
	if (!bRplPlay)
	{
		replay.InitHeader(pSet->game.track.c_str(), pSet->game.track_user, !bRplPlay);
		rh.numPlayers = mClient ? (int)mClient->getPeerCount()+1 :
			pSet->game.local_players;  // 📡 networked or 👥 splitscreen
		replay.Clear();  replay.ClearCars();  // upd num plr
		rh.trees = pSet->game.trees;

		rh.networked = mClient ? 1 : 0;
		rh.num_laps = pSet->game.num_laps;
		rh.sim_mode = pSet->game.sim_mode;
	}
	rewind.Clear();
	
	//  👻 ghost header
	ghost.InitHeader(pSet->game.track.c_str(), pSet->game.track_user, !bRplPlay);
	gh.numPlayers = 1;  // ghost always 1 car
	ghost.Clear();  ghost.ClearCars();
	gh.cars[0] = pSet->game.car[0];  gh.numWh[0] = carModels[0]->numWheels;
	gh.networked = 0;  gh.num_laps = 1;
	gh.sim_mode = pSet->game.sim_mode;
	gh.trees = pSet->game.trees;

	//  fill other cars (names, nicks, colors)
	int p, pp = pSet->game.local_players;
	if (mClient)  // // 📡 networked, 0 is local car
		pp = (int)mClient->getPeerCount()+1;

	if (!bRplPlay)
	for (p=0; p < pp; ++p)
	{
		const CarModel* cm = carModels[p];
		rh.cars[p] = cm->sDirname;  rh.nicks[p] = cm->sDispName;
		rh.numWh[p] = cm->numWheels;
	}
	
	//  set carModel nicks from networked replay
	if (bRplPlay && rh.networked)
	for (p=0; p < pp; ++p)
	{
		CarModel* cm = carModels[p];
		cm->sDispName = rh.nicks[p];
		cm->pNickTxt = hud->CreateNickText(p, cm->sDispName);
	}
}
//---------------------------------------------------------------------------------------------------------------


//  ⛰️ Terrain  5
void App::LoadTerrain()
{
	if (dstTrk)
	{
		scn->CreateTerrains(1);  // common
		//** GetTerMtrIds();  // todo: get from blendmap tex ..
		scn->CreateBltTerrains();  // 1st ter only-  // todo: before cars..
	}

	//; for (int c=0; c < carModels.size(); ++c)
		// carModels[c]->terrain = scn->ter;
}

//  🛣️ Road  6
void App::LoadRoad()
{
	CreateRoads();   // dstTrk inside
		
	for (int i=0; i < MAX_Players; ++i)
	if (hud->arrChk[i].nodeRot)
		hud->arrChk[i].nodeRot->setVisible(pSet->check_arrow && !bHideHudArr);

	//  boost fuel at start  . . .
	//  based on road length
	float boost_start = std::min(pSet->game.boost_max, std::max(pSet->game.boost_min,
		scn->road->st.Length * 0.001f * pSet->game.boost_per_km));
		
	for (int i=0; i < carModels.size(); ++i)
	{	CAR* car = carModels[i]->pCar;
		if (car)
		{	car->dynamics.boostFuelStart = boost_start;
			car->dynamics.boostFuel = boost_start;
	}	}


	///  🏞️👻 Run track's ghost
	//  to get ⏱️ times at 🔵 checkpoints
	fLastTime = 1.f;
	if (!scn->road || ghTrk.GetTimeLength() < 1.f)  return;
	int ncs = scn->road->mChks.size();
	if (ncs == 0)  return;

	vTimeAtChks.resize(ncs);
	int i,c;  // clear
	for (c=0; c < ncs; ++c)
		vTimeAtChks[c] = 0.f;

	int si = ghTrk.frames.size();
	for (i=0; i < si; ++i)
	{
		const TrackFrame& tf = ghTrk.frames[i];  // car
		if (tf.time > fLastTime)
			fLastTime = tf.time;
		for (c=0; c < ncs; ++c)  // test if in any checkpoint
		{
			const CheckSphere& cs = scn->road->mChks[c];
			Vector3 pos(tf.pos[0],tf.pos[2],-tf.pos[1]);
			Real d2 = cs.pos.squaredDistance(pos);
			if (d2 < cs.r2)  // inside
			if (vTimeAtChks[c] == 0.f)
			{	vTimeAtChks[c] = tf.time;
				//LogO("Chk "+toStr(c)+" ti "+fToStr(tf.time,1,4));
		}	}
	}
}

//  📦🏢 Objects, 💎 Collects, 🎆 Fields  7
void App::LoadObjects()
{
	if (dstTrk)
	{
		CreateObjects();

		CreateCollects();

		CreateFields();
	}
}

//  🌳🪨 Vegetation  8
void App::LoadTrees()
{
	if (dstTrk)
	{
		scn->LoadRoadDens();
		scn->CreateVegets();
		scn->grass->Create(this);  // 🌿

        if (pSet->gi)
			InitGI();  // 🌄
	}	
		
	//  check for cars inside terrain ___
	for (int i=0; i < carModels.size(); ++i)
	{
		CAR* car = carModels[i]->pCar;
		if (car)
		{
			MATHVECTOR<float,3> pos = car->posAtStart;
			Vector3 stPos(pos[0],pos[2],-pos[1]);
			float yt = scn->getTerH(stPos.x, stPos.z),
				yd = stPos.y - yt - 0.5f;
			//todo: either sweep test car body, or world->CastRay x4 at wheels -for bridges, pipes
			//pGame->collision.world->;  //car->dynamics.chassis
			if (yd < 0.f)
				pos[2] += -yd + (pSet->game.sim_mode == "easy" ? -0.1f : 0.9f);
			car->SetPosition1(pos);
	}	}
}

//  🎥 Preload Views  9
//  look around, to see and
//  Preload all resources, ensure visible
void App::LoadView(int c)
{
	Quaternion q;  Vector3 p{0,0,0};  // stat pos-
	auto* ter = scn->ter;
	float t = sc->tds[0].fTerWorldSize*0.3;  // *0.5
	
	if (c == 8)
	{	//  top view full
		q.FromAngleAxis(Degree(-90), Vector3::UNIT_Z);
		p.y = ter->getHeight(p.x, p.z) + t*4;
		
		isFocGui = false;  // hide back gui
		gui->toggleGui(false);

		carModels[0]->SetPaint();  // wip 1st ...
	}
	else if (c < 4)
	{	//  rotate around, middle of track
		q.FromAngleAxis(Degree(90 * c), Vector3::UNIT_Y);
		p.y = ter->getHeight(0.f, 0.f) + 10.f;
	}else //if (c < 8)
	{	//  top view, 4 corners
		q.FromAngleAxis(Degree(-90), Vector3::UNIT_X);
		p.x = (c-4)/2 ? -t : t;
		p.z = (c-4)%2 ? -t : t;
		p.y = ter->getHeight(p.x, p.z) + t*2;  //par hmax-

		isFocGui = true;  // show gui, to load tex
		gui->toggleGui(false);
	}
	LogO(String("Preload cam: ")+toStr(c)+"  pos "+toStr(p));//-
	mCamera->setPosition(p);
	mCamera->setOrientation(q);

	mCamera->setFarClipDistance(100000.f);
	mCamera->setLodBias(3.0f);
	// ter->update(Vector3::UNIT_Z);

	// Threads::Sleep( 1000 );  // test
}

//  ⏱️ HUD etc  10 last
void App::LoadMisc()
{
	bool rev = pSet->game.track_reversed;	
	/**if (pGame && !pGame->cars.empty())  //todo: move this into gui track tab chg evt, for cur game type
		gcom->UpdGuiRdStats(scn->road, scn->sc, gcom->sListTrack,
			pGame->timer.GetBestLap(0, rev), rev, 0);  // current
	**/

	// SetTexWrap( HLMS_UNLIT, "FluidWater", false );  //**


	if (pSet->hud_on)
	{
		hud->Create();  //!-
		hud->Show();
		//- hud->Show(true);  // hide
	}
	
	// Camera settings
	for (auto car : carModels)
	{	car->First();
		if (car->fCam)
		{	//car->fCam->mTerrain = scn->ter;  //?
			//car->fCam->mWorld = &(pGame->collision);
	}	}
}


//  Performs a single loading step.  Actual loading procedure that gets called every frame during load.
//---------------------------------------------------------------------------------------------------------------
String App::cStrLoad[LS_ALL+1] = {
	"#{LS_CLEANUP}","#{LS_GAME}","#{LS_SCENE}","#{LS_CAR}",
	"#{LS_TER}","#{LS_ROAD}","#{LS_OBJS}","#{LS_TREES}",
	"0", "1", "2", "3", "4", "5", "6", "7", "8",
	"#{LS_MISC}","#{LS_MISC}"};

void App::NewGameDoLoad()
{
	auto& cur = curLoadState;
	if (cur == LS_ALL)
	{
		//  Loading finished
		bLoading = false;
		// #ifdef DEBUG  //todo: doesnt hide later, why?
		// LoadingOff();
		// #endif
		SetLoadingBar(100.f);

		//-  cars need update
		for (int i=0; i < carModels.size(); ++i)
		{	CarModel* cm = carModels[i];
			cm->updTimes = true;

			bool collect = pSet->game.collect_num >= 0;
			if (!collect)
			{	cm->updLap = true;  cm->fLapAlpha = 1.f;
		}	}

		//if (pSet->show_fps)
		//	mFpsOverlay->show();
		//.mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);

		//.ChampLoadEnd();
		/**if (mClient)
			boost::this_thread::sleep(boost::posix_time::milliseconds(
				3000 * mClient->getId()));  /**/  // Test loading synchronization
		//.bLoadingEnd = true;
		return;
	}

	//  Do the next loading step
	int perc = 0;
	int c = cur - LS_VIEW0;
	switch (cur)
	{
		case LS_CLEANUP:	LoadCleanUp();	perc = 2;	break;  // 💥
		case LS_GAME:		LoadGame();		perc = 8;	break;  // 
		case LS_SCENE:		LoadScene();	perc = 13;	break;  // ⛅🔥
		case LS_CAR:		LoadCar();		perc = 20;	break;  // 🚗🚗

		case LS_TERRAIN:	LoadTerrain();	perc = 32;	break;  // ⛰️🏔️⛰️
		case LS_ROAD:		LoadRoad();		perc = 45;	break;  // 🛣️📏🏛️⭕
		
		case LS_OBJECTS:	LoadObjects();	perc = 62;	break;  // 📦🏢 💎
		case LS_TREES:		LoadTrees();	perc = 75;	break;

		case LS_VIEW0: case LS_VIEW1: case LS_VIEW2: case LS_VIEW3:  // 🎥
		case LS_VIEW4: case LS_VIEW5: case LS_VIEW6: case LS_VIEW7: case LS_VIEW8:
		{
			if (pSet->li.front)
			for (auto& c : carModels)  // on/off lights, lag
				if (c->cType == CarModel::CT_LOCAL)
				{
				for (auto& l : c->lights)
				if (l.type != CarModel::LI_Under)
					l.li->setVisible(cur <= LS_VIEW2 ||  //..?
						cur >= LS_VIEW4 && cur <= LS_VIEW7);
				}
			LoadView(c);
			// perc = 75.f + (90.f-75.f) * c / 9.f;
			perc = 85;
		}	break;
		// todo: show cars, particles, gui

		case LS_MISC:		LoadMisc();		perc = 90;	break;  // ⏱️
	}

	//  Update bar,txt
	SetLoadingBar(perc);

	//  next loading step
	++cur;
	//  show next already
	if (cur >= LS_VIEW0 && cur <= LS_VIEW8)
		txLoad->setCaption(TR("#{LS_SCENE} ") /*+ toStr(c+1)*/);
	else
		txLoad->setCaption(TR(cStrLoad[cur]));
}


//---------------------------------------------------------------------------------------------------------------
///  🛣️ Road  * * * * * * * 
//---------------------------------------------------------------------------------------------------------------

void App::CreateRoads()
{
	///  road  ~ ~ ~
	SplineRoad*& road = scn->road;
	Cam* cam = &mCams[0];  // todo: lod cam-

	//  road
	if (dstTrk)
	{
		scn->DestroyRoads();
		CreateRoadsInt();
	}else
		road->mCamera = cam;  // upd


	//  🚦 pace ~ ~
	if (pSet->hud_on)
	{
		LogO("D--~ destroy Trail");
		scn->DestroyTrails();

		scn->DestroyPace();
		if (!bHideHudPace)
		{
			for (int i=0; i < carModels.size(); ++i)
			if (!carModels[i]->isGhost())
			{	scn->pace[i] = new PaceNotes(pSet);
				scn->pace[i]->player = i;
				scn->pace[i]->Setup(mSceneMgr, 
					carModels[i]->fCam->cam->cam, // &mCams[i],
					scn->ter, gui->mGui, mWindow);
		}	}
	}

	//  after road load we have iChk1 so set it for carModels
	for (int i=0; i < carModels.size(); ++i)
		carModels[i]->ResetChecks(true);

	if (dstTrk)
	{
		road->bCastShadow = pSet->g.shadow_type >= Sh_Depth;
		road->bRoadWFullCol = pSet->gui.collis_roadw;

		for (auto r : scn->roads)
		{
			r->scn = scn;
			r->RebuildRoadInt();
			r->SetChecks();  // 2nd, upd
		}
		scn->grid.Create();  
	}
	

	//  🚦 pace ~ ~
	for (int i=0; i < MAX_Players; ++i)
	if (scn->pace[i])
	{
		road->RebuildRoadPace();  //todo: load only..
		scn->pace[i]->Rebuild(road, scn->sc, pSet->game.track_reversed);
	}

	for (int i=0; i < carModels.size(); ++i)
	if (!carModels[i]->isGhost())
		scn->CreateTrail(cam, i, bHideHudTrail);
}


void App::CreateRoadsInt()
{
	Cam* cam = &mCams[0];  // todo: lod cam-

	//  get all road*.xml
	strlist lr;  string path = gcom->TrkDir();
	PATHS::DirList(path, lr, "xml");
	
	for (auto fname:lr)
	if (StringUtil::startsWith(fname,"road"))
	{
		int id = scn->roads.size();
		LogO("C~~R Creating road " + toStr(id) + " from: " + fname);
		scn->road = new SplineRoad(pGame);
		scn->road->Setup("", 0.7, scn, mSceneMgr, cam, id);
		scn->road->LoadFile(path + fname);
		scn->roads.push_back(scn->road);
	}

	scn->rdCur = 0;
	scn->road = scn->roads[scn->rdCur];
}
