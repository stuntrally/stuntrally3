#include "pch.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "GuiCom.h"
#include "Def_Str.h"
// #include "RenderConst.h"
// #include "GuiCom.h"
#include "CData.h"
#include "SceneXml.h"
#include "CScene.h"
#include "cardefs.h"
#include "game.h"

#include "CarModel.h"
#include "CInput.h"
// #include "Road.h"
// #include "SplitScreen.h"
// #include "WaterRTT.h"
// #include "MultiList2.h"
#include "Gui_Popup.h"
#include "Axes.h"
// #include <OgreManualObject.h>
// #include <OgreTechnique.h>
// #include "RenderBoxScene.h"
#include <thread>
using namespace Ogre;
using namespace std;


//  🌟 ctor
//-----------------------------------------------
App::App()
	:fLastFrameDT(0.001f)
	// , mIblQuality( IblHigh )  // 45 fps-  // par
	// , mIblQuality( IblMedium )
	// , mIblQuality( MipmapsLowest )  // very low, no mips-
	// ,mThread()
	// ,iEdTire(0), iTireLoad(0), iCurLat(0),iCurLong(0),iCurAlign(0), iUpdTireGr(0)
	// ,bPerfTest(0),iPerfTestStage(PT_StartWait)
{
	macroblockWire.mPolygonMode = PM_WIREFRAME;

	frm.resize(MAX_CARS);
	for (int i=0; i < MAX_CARS; ++i)
		iCurPoses[i] = 0;

	Axes::Init();

	resCar = "";  resTrk = "";  resDrv = "";
	oldTrack = "";  oldTrkUser = false;
	
	// Load();  later in createScene01
}

//  💥 dtor
//-----------------------------------------------
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

	delete gui->popup;
	delete gcom;
	delete gui;
	delete scn;

	delete hud;
	delete input;
}


void App::destroyScene()
{
	LogO("---- destroyScene");
	
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

	DestroyTerrain();

	LogO(">>>>>>>> Destroy SR ----");
	Destroy();
	LogO(">>>>>>>> Destroy SR done ----");
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
		
		if (!bLoading && !mShutDown && pGame)
		{
			bSimulating = true;
			bool ret = pGame->OneLoop(dt);
			if (!ret)
				mShutDown = true;  //ShutDown();

			// DoNetworking();
			bSimulating = false;
		}
		this_thread::sleep_for(chrono::milliseconds(
			pSet ? pSet->thread_sleep : 100));
	}
}
