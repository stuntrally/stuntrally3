#include "pch.h"
#include "paths.h"
#include "Road.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
#include "Axes.h"
#include "CScene.h"
// #include "SplineBase.h"
#include "Grass.h"
#include "GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "game.h"
#include "PaceNotes.h"
#include "SoundMgr.h"
#include "SoundBaseMgr.h"
#include "FollowCamera.h"
#include "CarModel.h"
#include "BtOgreExtras.h"
// #include "SplitScreen.h"
// #include "GraphView.h"
//; #include "gameclient.hpp"
#include "Terra.h"

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreSceneManager.h>
#include <OgreParticleSystem.h>
#include <OgreResourceGroupManager.h>
#include <OgreImage2.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreTextureGpuManager.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_Window.h>
#include "MessageBox.h"
using namespace MyGUI;
using namespace Ogre;
using namespace std;


//  📄 Load Scene Data
//-------------------------------------------------------------------------------------
void App::LoadData()
{
	//  prv tex
	/*int k = 1024;
	prvView.Create(k,k,"PrvView");
	prvRoad.Create(k,k,"PrvRoad");
	 prvTer.Create(k,k,"PrvTer");
	//  ch stage
	prvStCh.Create(k,k,"PrvStCh");
	
	scn->roadDens.Create(k+1,k+1,"RoadDens");
	
	///  ter lay tex
	for (int i=0; i < 6; ++i)
	{	String si = toStr(i);
		scn->texLayD[i].SetName("layD"+si);
		scn->texLayN[i].SetName("layN"+si);
	}
	
	mLoadingBar->loadTex.Create(1920,1200,"LoadingTex");*/

	//  tex fil
	// MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	// MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);
	
	//  restore camNums
	for (int i=0; i<4; ++i)
		if (pSet->cam_view[i] >= 0)
			carsCamNum[i] = pSet->cam_view[i];

	Ogre::Timer ti;


	//  data xmls
	pGame->ReloadSimData();  // need surfaces
	
	///  🧰 _Tool_ check champs and challs  ............
	bool check = 0;
	scn->data->Load(&pGame->surf_map, check);
	scn->sc->pFluidsXml = scn->data->fluids;
	scn->sc->pReverbsXml = scn->data->reverbs;

	//  championships.xml, progress.xml
	gui->Ch_XmlLoad();

	//  user.xml
	/*#if 0
	userXml.LoadXml(PATHS::UserConfigDir() + "/user.xml");
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{
		const TrackInfo& ti = data->tracks->trks[i];
		if (userXml.trkmap[ti.name]==0)
		{	// not found, add
			UserTrkInfo tu;  tu.name = ti.name;

			userXml.trks.push_back(tu);
			userXml.trkmap[ti.name] = userXml.trks.size();
	}	}
	userXml.SaveXml(PATHS::UserConfigDir() + "/user.xml");
	#endif*/

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
	#if 0
	std::string file = PATHS::Ghosts() + "/normal/TestC4-ow_V2.rpl";
	replay.LoadFile(file);  exit(0);
	#endif
	

	///  🧰 _Tool_ ghosts times .......
	#if 0
	gui->ToolGhosts();  exit(0);
	#endif

	///  _Tool_ convert to track's ghosts ..............
	#if 0
	gui->ToolGhostsConv();  exit(0);
	#endif

	///  _Tool_ test track's ghosts ..............
	#if 0
	gui->ToolTestTrkGhosts();  exit(0);
	#endif

	///  🧰 _Tool_ presets .......
	#if 0
	gui->ToolPresets();  exit(0);
	#endif


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
		//  👀 set VR, only 1 player
		pSet->game.vr_mode = pSet->vr_mode;
		if (pSet->game.vr_mode)
			pSet->game.local_players = 1;
		Ch_NewGame();

		/*if (mClient && mLobbyState != HOSTING)  // 📡 all but host
			gui->updateGameSet();  // override gameset params for networked game (from host gameset)
		if (mClient)  // for all, including host
			pSet->game.local_players = 1;*/
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
	
	if (dstTrk)
	{	scn->DestroyFluids();
		DestroyObjects(true);
	}
	
	DestroyGraphs();
	hud->Destroy();
	
	//  hide hud arrow,beam,pace,trail
	bool morePlr = pSet->game.local_players > 1;
	bool rplRd = bRplPlay /*|| scn->road && scn->road->getNumPoints() < 2/**/;
	bool rplHide = bRplPlay && pSet->rpl_hideHudAids;

	bHideHudBeam = rplRd;
	bHideHudArr = rplRd || morePlr;
	bool denyPace = gui->pChall && !gui->pChall->pacenotes;
	bHideHudPace = morePlr || denyPace || rplHide;  // todo: ? pace, trail for splitscreen
	bool denyTrail = gui->pChall && !gui->pChall->trail;
	bHideHudTrail = morePlr || denyTrail || rplHide;


	// rem old track
	if (dstTrk)
	{
		if (resTrk != "")
			ResourceGroupManager::getSingleton().removeResourceLocation(resTrk);
		LogO(">>>>----  Loading track: "+pSet->game.track);
		resTrk = gcom->TrkDir() + "objects";  // for roadDensity.png
		ResourceGroupManager::getSingleton().addResourceLocation(resTrk, "FileSystem");
	}

	hud->arrow.Destroy(mSceneMgr);


	//  Delete all cars
	for (int i=0; i < carModels.size(); ++i)
	{
		CarModel* c = carModels[i];
		if (c && c->fCam)
		{
			carsCamNum[i] = 
				c->iLoopLastCam != -1 ? c->iLoopLastCam +1 :  //o
				c->fCam->miCurrent +1;  // save which cam view
			if (i < 4)
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
		scn->DestroyTrees();    // 🌳🪨

		DestroyObjects(true);   // 📦
		scn->DestroyRoads();    // 🛣️
		scn->DestroyTerrain();  // ⛰️
		//^ cars
		scn->DestroyFluids();   // 💧
		scn->DestroyEmitters(true);  // 🔥
		scn->DestroyAllAtmo();  // 🌦️
	}
	scn->DestroyTrail();


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

	MinimizeMemory();  // todo: car refl ?

	SetupCompositor();  //+ ok
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

	/*if (gui->bReloadSim)  //; ?
	{	gui->bReloadSim = false;
		pGame->ReloadSimData();

		static bool f1 = true;
		if (f1) {  f1 = false;
			gui->updSld_TwkSurf(0);  }
	}*/
	
	///<>  save old track
	oldTrack = pSet->game.track;  oldTrkUser = pSet->game.track_user;
	
	
	//  load scene.xml - default if not found
	//  need to know sc->asphalt before vdrift car load
	if (dstTrk)
	{
		scn->sc->pGame = pGame;
		scn->sc->LoadXml(gcom->TrkDir()+"scene.xml");
		// pGame->track.asphalt = scn->sc->asphalt;  //*
		// pGame->track.sDefaultTire = scn->sc->asphalt ? "asphalt" : "gravel";  //*
		if (scn->sc->denyReversed)
			pSet->game.track_reversed = false;

		pGame->NewGameDoLoadTrack();
	}

	//  set normal reverb
	pGame->snd->sound_mgr->SetReverb(scn->sc->revSet.normal);
	
	//  upd car abs,tcs,sss
	pGame->ProcessNewSettings();

		
	///  🚗 init car models
	///--------------------------------------------
	//  will create vdrift cars, actual car loading will be done later in LoadCar()
	//  this is just here because vdrift car has to be created first
	
	int numCars = /*mClient ? mClient->getPeerCount()+1 :*/
		pSet->game.local_players;  // 📡 networked or 👥 splitscreen
	int i;
	for (i = 0; i < numCars; ++i)
	{
		// This only handles one local player
		CarModel::eCarType et = CarModel::CT_LOCAL;
		int startId = i;
		std::string carName = pSet->game.car[std::min(3,i)], nick = "";
		/*if (mClient)  // 📡
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
		}*/

		//  need road looped here
		String sRd = gcom->PathListTrk() + "/road.xml";
		SplineRoad rd(pGame);  rd.LoadFile(sRd,false);
		bool loop = //rd.getNumPoints() < 2 ? false :
			!rd.isLooped && pSet->game.track_reversed ? true : false;
		
		CarModel* car = new CarModel(i, i, et, carName, &mCams[i], this);
		car->Load(startId, loop);
		carModels.push_back(car);
		
		/*if (nick != "")  // set remote nickname // 📡
		{	car->sDispName = nick;
			if (i != 0)  // not for local
				car->pNickTxt = hud->CreateNickText(i, car->sDispName);
		}*/
	}

	///  👻 ghost car - last in carModels
	///--------------------------------------------
	ghPlay.Clear();
	if (!bRplPlay/*|| pSet->rpl_show_ghost)*/ && pSet->rpl_ghost /*&& !mClient*/)
	{
		std::string ghCar = pSet->game.car[0], orgCar = ghCar;
		ghPlay.LoadFile(gui->GetGhostFile(pSet->rpl_ghostother ? &ghCar : 0));
		isGhost2nd = ghCar != orgCar;
		
		//  always because ghplay can appear during play after best lap
		// 1st ghost = orgCar
		CarModel* c = new CarModel(i, 4, CarModel::CT_GHOST, orgCar, 0, this);
		c->Load(-1, false);
		c->pCar = (*carModels.begin())->pCar;  // based on 1st car
		carModels.push_back(c);

		//  2st ghost - other car
		if (isGhost2nd)
		{
			CarModel* c = new CarModel(i, 4, CarModel::CT_GHOST2, ghCar, 0, this);
			c->Load(-1, false);
			c->pCar = (*carModels.begin())->pCar;
			carModels.push_back(c);
		}
	}
	///  🏞️👻 track's ghost  . . .
	///--------------------------------------------
	ghTrk.Clear();  vTimeAtChks.clear();
	bool deny = gui->pChall && !gui->pChall->trk_ghost;
	if (!bRplPlay /*&& pSet->rpl_trackghost- && !mClient*/ && !pSet->game.track_user && !deny)
	{
		std::string sRev = pSet->game.track_reversed ? "_r" : "";
		std::string file = PATHS::TrkGhosts()+"/"+ pSet->game.track + sRev + ".gho";
		if (ghTrk.LoadFile(file))
		{
			CarModel* c = new CarModel(i, 5, CarModel::CT_TRACK, "ES", 0, this);
			c->Load(-1, false);
			c->pCar = (*carModels.begin())->pCar;  // based on 1st car
			carModels.push_back(c);
	}	}

	float pretime = /*mClient ? 2.0f :*/ pSet->game.pre_time;  // same for all multi players
	if (bRplPlay)  pretime = 0.f;
	/*if (mClient)  // 📡
	{	pGame->timer.waiting = true;  //+
		pGame->timer.end_sim = false;
	}*/

	pGame->NewGameDoLoadMisc(pretime);
}
//---------------------------------------------------------------------------------------------------------------


//  Scene
void App::LoadScene()  // 3
{
	//  ⛅ sun, fog, weather, sky
	if (dstTrk)
		scn->CreateAllAtmo();

	if (dstTrk)
		scn->CreateEmitters();  // 🔥


	//  💧 Fluids
	if (dstTrk)
		scn->CreateFluids();

	if (dstTrk)
		pGame->collision.world->setGravity(btVector3(0.0, 0.0, -scn->sc->gravity));


	//  🔝 checkpoint arrow
	bool deny = gui->pChall && !gui->pChall->chk_arr;
	if (!bHideHudArr && !deny)
		hud->arrow.Create(mSceneMgr, pSet);
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
		rh.numPlayers = /*mClient ? (int)mClient->getPeerCount()+1 :*/
			pSet->game.local_players;  // 📡 networked or 👥 splitscreen
		replay.Clear();  replay.ClearCars();  // upd num plr
		rh.trees = pSet->game.trees;

		rh.networked = 0; //; mClient ? 1 : 0;
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
	//; if (mClient)  // // 📡 networked, 0 is local car
		// pp = (int)mClient->getPeerCount()+1;

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
		//; cm->pNickTxt = hud->CreateNickText(p, cm->sDispName);
	}
}
//---------------------------------------------------------------------------------------------------------------


//  ⛰️ Terrain  5
void App::LoadTerrain()
{
	if (dstTrk)
	{
		scn->CreateTerrain(false);  // common
		//; GetTerMtrIds();  // todo: get from blendmap tex ..
		scn->CreateBltTerrain();  // todo: before cars..
	}

	//; for (int c=0; c < carModels.size(); ++c)
	// 	carModels[c]->terrain = scn->terrain;
}

//  🛣️ Road  6
void App::LoadRoad()
{
	CreateRoads();   // dstTrk inside
		
	if (hud->arrow.nodeRot)
		hud->arrow.nodeRot->setVisible(pSet->check_arrow && !bHideHudArr);

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

//  📦 Objects  7
void App::LoadObjects()
{
	if (dstTrk)
		CreateObjects();
}

//  🌳🪨 Vegetation  8
void App::LoadTrees()
{
	if (dstTrk)
	{
		scn->LoadRoadDens();
		scn->CreateTrees();
		scn->grass->Create();  // 🌿
	}	
		
	//  check for cars inside terrain ___
	if (scn->terrain)
	for (int i=0; i < carModels.size(); ++i)
	{
		CAR* car = carModels[i]->pCar;
		if (car)
		{
			MATHVECTOR<float,3> pos = car->posAtStart;
			Vector3 stPos(pos[0],pos[2],-pos[1]);
			float yt = scn->terrain->getHeight(stPos.x, stPos.z),
				yd = stPos.y - yt - 0.5f;
			//todo: either sweep test car body, or world->CastRay x4 at wheels -for bridges, pipes
			//pGame->collision.world->;  //car->dynamics.chassis
			if (yd < 0.f)
				pos[2] += -yd + (pSet->game.sim_mode == "easy" ? -0.1f : 0.9f);
			car->SetPosition1(pos);
	}	}
}

//  ⏱️ HUD etc
void App::LoadMisc()  // 9 last
{
	bool rev = pSet->game.track_reversed;	
	/**if (pGame && !pGame->cars.empty())  //todo: move this into gui track tab chg evt, for cur game type
		gcom->UpdGuiRdStats(scn->road, scn->sc, gcom->sListTrack,
			pGame->timer.GetBestLap(0, rev), rev, 0);  // current
	**/

	// SetTexWrap( HLMS_UNLIT, "FluidWater", false );  //**


	hud->Create();  //!-
	hud->Show();
	//- hud->Show(true);  // hide
	
	// Camera settings
	for (auto car : carModels)
	{	car->First();
		if (car->fCam)
		{	//; car->fCam->mTerrain = scn->mTerrainGroup;  //?
			//car->fCam->mWorld = &(pGame->collision);
	}	}
	
	/*if (dstTrk)  // drop this
	try
	{	TexturePtr tex = Ogre::TextureManager::getSingleton().getByName("waterDepth.png");
		if (tex)
			tex->reload();
	} catch(...)
	{	}*/
}


//  Performs a single loading step.  Actual loading procedure that gets called every frame during load.
//---------------------------------------------------------------------------------------------------------------
String App::cStrLoad[LS_ALL+1] = 
	{"LS_CLEANUP","LS_GAME","LS_SCENE","LS_CAR","LS_TER","LS_ROAD","LS_OBJS","LS_TREES","LS_MISC","LS_ALL"};

void App::NewGameDoLoad()
{
	if (curLoadState == LS_ALL)
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
			cm->updLap = true;  cm->fLapAlpha = 1.f;
		}

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
	switch (curLoadState)
	{
		case LS_CLEANUP:	LoadCleanUp();	perc = 3;	break;
		case LS_GAME:		LoadGame();		perc = 10;	break;
		case LS_SCENE:		LoadScene();	perc = 20;	break;
		case LS_CAR:		LoadCar();		perc = 30;	break;

		case LS_TERRAIN:	LoadTerrain();	perc = 40;	break;
		case LS_ROAD:		LoadRoad();		perc = 50;	break;
		case LS_OBJECTS:	LoadObjects();	perc = 60;	break;
		case LS_TREES:		LoadTrees();	perc = 70;	break;

		case LS_MISC:		LoadMisc();		perc = 80;	break;
	}

	//  Update bar,txt
	txLoad->setCaption(TR("#{"+cStrLoad[curLoadState]+"}"));
	SetLoadingBar(perc);

	//  next loading step
	++curLoadState;
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
	LogO("D--~ destroy Trail");
	scn->DestroyTrail();

	scn->DestroyPace();
	if (!bHideHudPace)
	{
		scn->pace = new PaceNotes(pSet);
		scn->pace->Setup(mSceneMgr, cam, scn->terrain, gui->mGui, mWindow);
	}/**/


	//  after road load we have iChk1 so set it for carModels
	for (int i=0; i < carModels.size(); ++i)
		carModels[i]->ResetChecks(true);

	if (dstTrk)
	{
		// scn->UpdPSSMMaterials();  ///+~-

		road->bCastShadow = pSet->shadow_type >= Sh_Depth;
		road->bRoadWFullCol = pSet->gui.collis_roadw;

		for (auto r : scn->roads)
		{
			r->mTerrain = scn->terrain;
			r->RebuildRoadInt();
			r->SetChecks();  // 2nd, upd
	}	}
	

	//  pace ~ ~
	if (scn->pace)
	{
		road->RebuildRoadPace();  //todo: load only..
		scn->pace->Rebuild(road, scn->sc, pSet->game.track_reversed);
	}/**/

	CreateTrail(cam);
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
		scn->road->Setup("", 0.7, scn->terrain, mSceneMgr, cam, id);
		scn->road->LoadFile(path + fname);
		scn->roads.push_back(scn->road);
	}

	scn->rdCur = 0;
	scn->road = scn->roads[scn->rdCur];
}


///  Trail ghost track  ~~--~-~--
//---------------------------------------------------------------------------------------------------------------

void App::CreateTrail(Cam* cam)
{
	// if (!pSet->trail_show)
		// return;  // fixme crash in replay--
	if (scn->trail)
		scn->DestroyTrail();

	//  load
	TrackGhost gho;
	int frames = 0;

	string sRev = pSet->game.track_reversed ? "_r" : "";
	string file = PATHS::TrkGhosts()+"/"+ pSet->game.track + sRev + ".gho";
	if (!PATHS::FileExists(file))
		LogO("Trail trk gho not found: "+file);
	else
	{	gho.LoadFile(file, 0);
		frames = gho.getNumFrames();
	}
	if (frames == 0 || !scn->terrain)  return;
	LogO("C--~ CreateTrail");


	//  setup trail road
	SplineRoad* tr = new SplineRoad(pGame);
	tr->Setup("", 0.7, scn->terrain, mSceneMgr, cam, 100);
	tr->sMtrRoad[0] = "trailRoad";  tr->bMtrRoadTer[0] = false;
	tr->type = RD_Trail;  tr->isLooped = false;

	//tr->g_LenDim0 = 2.f;  tr->g_iWidthDiv0 = 1;  //  high quality
	tr->g_LenDim0 = 3.f;  tr->g_iWidthDiv0 = 1;  //  ok
	//tr->g_LenDim0 = 4.f;  tr->g_iWidthDiv0 = 1;  //  low
	//  mergeLen affects Fps most
	tr->g_MergeLen = 200.f;  tr->g_LodPntLen = 30.f;  tr->bMerge = true;  // ok
	//tr->g_MergeLen = 100.f;  tr->g_LodPntLen = 20.f;  tr->bMerge = true;  // low
	tr->newP.onTer = false;  tr->newP.aType = AT_Both;

	//  params  add p
	const float dd = 5*5, al = 0.6f, down = 0.5f,  // high quality
	// const float dd = 10*10, al = 0.6f, down = 0.5f,  // quality  p each 10 m
	// const float dd = 20*20, al = 0.6f, down = 0.45f, // low, cuts loops etcs
		acc_sens = 30.f, acc_sensw = 45.f;  // color sensitivity


	//  chk vars
	int iNextChk = pSet->game.track_reversed ? scn->road->iChkId1Rev : scn->road->iChkId1;
	int iCk = 1;
	const bool rev = pSet->game.track_reversed;
	const int inc = (rev ? -1 : 1) * scn->road->iDir;
	const int ncs = scn->road->mChks.size();


	//  add points  - > - -
	Vector3 pos, old;  float tOld = 0.f, vOld = 0.f;

	for (int i=0; i < frames; ++i)
	{
		const TrackFrame& fr = gho.getFrame0(i);
		Axes::toOgre(pos, fr.pos);  pos.y -= down;
		if (i==0)  old = pos;

		//  if ghost car in next check, inc trail seg id
		const int np = tr->getNumPoints();

		const auto& ck = scn->road->mChks[iNextChk];
		if (pos.squaredDistance(ck.pos) < ck.r2)
		{
			//LogO(toStr(np) + " in ck "+toStr(iCk)+" r "+toStr(ck.r)+" nCk "+toStr(tr->newP.nCk));
			++iCk;
			iNextChk = (iNextChk + inc + ncs) % ncs;
		}

		float d = old.squaredDistance(pos);
		if (d > dd)
		{	d = sqrt(d);  // todo: on angle changes..

			float fa = scn->sc->GetDepthInFluids(pos);
			if (fa > 0.f)  pos.y += fa + 0.2f;  // above fluids

			const float t = fr.time, dt = t - tOld;
			const float vel = d / dt * 3.6f;  // kmh
			const float acc = (vel - vOld) / dt;

			//LogO(toStr(np) + " v " + fToStr(vel) + " a " + fToStr(acc)+" ic "+toStr(iCk));

			float a = max(-1.f, min(1.f, acc / acc_sens));
			float aw = max(-1.f, min(1.f, acc / acc_sensw));
			//tr->newP.width = 0.5f - a * 0.3f;  // par big
			tr->newP.width = 0.3f - aw * 0.15f;  // par sm
			tr->newP.pos = pos;
			tr->newP.nCk = iCk;

			tr->newP.mRoll = fr.steer/127.f * 10.f;
			// todo: get roll from car in road seg space?..
			//auto q = Axes::toOgre(fr.rot);
			//tr->newP.mRoll = q.getRoll().valueDegrees();  //-

			tr->newP.clr = a > 0.f ?
				Vector4(1.f - a, 1.f, 0.f, al) :  // accel clr
				Vector4(1.f + aw*aw*0.5f, 1.f + a, 0.f, al);  // brake

			if (vel < 600.f)  // fix end jmp err
				tr->Insert(INS_End);
			
			old = pos;  tOld = t;  vOld = vel;
	}	}

	tr->Rebuild(true);
	tr->RebuildRoadInt();
	scn->trail = tr;
	bool vis = !pSet->trail_show || bHideHudTrail;
	tr->SetVisTrail(!vis);
	// tr->SetVisTrail(false);
}
