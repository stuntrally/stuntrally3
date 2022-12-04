#include "pch.h"
#include "CGame.h"
#include "CHud.h"
// #include "CGui.h"
#include "Def_Str.h"
// #include "RenderConst.h"
// #include "GuiCom.h"
#include "CData.h"
#include "SceneXml.h"
#include "CScene.h"
#include "cardefs.h"
#include "game.h"

#include "CarModel.h"
// #include "Road.h"
// #include "SplitScreen.h"
// #include "WaterRTT.h"
// #include "MultiList2.h"
// #include "Gui_Popup.h"
#include "Axes.h"
// #include <OgreManualObject.h>
// #include <OgreTechnique.h>
// #include "../ogre/common/RenderBoxScene.h"
#include <thread>
using namespace Ogre;
using namespace std;


//  ctors  -----------------------------------------------
App::App(SETTINGS *settings, GAME *game)
	:pGame(game)
	// ,hud(0), gui(0), gcom(0)
	// , input(0)
	// ,mThread()
	, mTimer(0.f)
	// game
	,blendMtr(0), blendMapSize(513)
	,carIdWin(-1), iRplCarOfs(0)
	// other
	,newGameRpl(0), curLoadState(0), dstTrk(1)
	// ,bHideHudArr(0), bHideHudBeam(0), bHideHudPace(0), bHideHudTrail(0)
	,bRplPlay(0),bRplPause(0), bRplRec(0), bRplWnd(1)
	, iRplSkip(0)
	// ,iEdTire(0), iTireLoad(0), iCurLat(0),iCurLong(0),iCurAlign(0), iUpdTireGr(0)
	,fLastFrameDT(0.001f)
	// ,bPerfTest(0),iPerfTestStage(PT_StartWait)
	,isGhost2nd(0)
	,fLastTime(1.f)
{
	pSet = settings;
	pGame->collision.pApp = this;

	frm.resize(MAX_CARS);
	for (int i=0; i < MAX_CARS; ++i)
		iCurPoses[i] = 0;

	Axes::Init();

	resCar = "";  resTrk = "";  resDrv = "";
	oldTrack = "";  oldTrkUser = false;
	
	///  new
	scn = new CScene(this);
	data = scn->data;
	hud = new CHud(this);

	// gcom = new CGuiCom(this);
	// gui = new CGui(this);
	// gui->gcom = gcom;
	// hud->gui = gui;
	// gui->popup = new GuiPopup();
	// gui->viewBox = new wraps::RenderBoxScene();

	// mBindListner = gui;
	// input = new CInput(this);

	// if (pSet->multi_thr)
	mThread = new thread(&App::UpdThr, this);
}

void App::ShutDown()
{
	mShutDown = true;
	if (mThread->joinable())
		mThread->join();
	delete mThread;
}

App::~App()
{
	ShutDown();

	// gui->viewBox->destroy();
	// delete gui->viewBox;

	// delete gui->popup;
	// delete gcom;
	// delete gui;
	delete scn;

	delete hud;
	// delete input;
}


/*void App::postInit()
{
	SetFactoryDefaults();

	mSplitMgr->pApp = this;

	mFactory->setMaterialListener(this);
}
*/

void App::destroyScene()
{
	delete dbgdraw;  dbgdraw = 0;
	
	// scn->mWaterRTT->destroy();
	
	DestroyObjects(true);
	
	// for (int i=0; i < graphs.size(); ++i)
	// 	delete graphs[i];

	// for (int i=0; i<4; ++i)
		// pSet->cam_view[i] = carsCamNum[i];

	// Delete all cars
	for (auto& car : carModels)
	 	delete car;

	// carModels.clear();
	//carPoses.clear();
	
	// gcom->mToolTip = 0;  //?

	scn->DestroyRoads();

	scn->DestroyTrees();

	if (pGame)
		pGame->End();
	
	// delete[] blendMtr;  blendMtr = 0;

	// BaseApp::destroyScene();
}


//  simulation (2nd) thread
//---------------------------------------------------------------------------------------------------------------

void App::UpdThr()
{
	Ogre::Timer gtim;
	//#ifdef _WIN32
	//DWORD af = 2;
	//gtim.setOption("QueryAffinityMask", &af);
	//#endif
	gtim.reset();

	while (!mShutDown)
	{
		///  step Game  **

		double dt = double(gtim.getMicroseconds()) * 0.000001;
		gtim.reset();
		
		if (/*pSet->multi_thr == 1 &&*/ !bLoading && !mShutDown)
		{
			bSimulating = true;
			bool ret = pGame->OneLoop(dt);
			if (!ret)
				mShutDown = true;  //ShutDown();

			// DoNetworking();
			bSimulating = false;
		}
		this_thread::sleep_for(chrono::milliseconds(pSet->thread_sleep));
	}
}
