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
#include "Grass.h"
#include "Road.h"
// #include "SplitScreen.h"
// #include "WaterRTT.h"
// #include "MultiList2.h"
#include "Gui_Popup.h"
#include "Axes.h"
// #include <OgreManualObject.h>
// #include <OgreTechnique.h>
// #include "RenderBoxScene.h"
#include "GraphicsSystem.h"
#include "BtOgreExtras.h"
#include <thread>
using namespace Ogre;
using namespace std;


//  🌟 ctor
//-----------------------------------------------
App::App()
{
	frm.resize(MAX_CARS);
	for (int i=0; i < MAX_CARS; ++i)
		iCurPoses[i] = 0;

	Axes::Init();

	// Load();  later in createScene01
}

//  💥 dtor
//-----------------------------------------------
void App::ShutDown()
{
	Quit();
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

//  💥 Destroy  LoadCleanUp()
//----------------------------------------------------------------------------------
void App::destroyScene()
{
	LogO("DD-- destroyScene ------DD");
	
	delete dbgdraw;  dbgdraw = 0;
	
	// scn->mWaterRTT->destroy();
	
	DestroyGraphs();
	hud->Destroy();
	hud->arrow.Destroy(mSceneMgr);
	
	// for (int i=0; i<4; ++i)
		// pSet->cam_view[i] = carsCamNum[i];

	//  Delete all cars
	for (auto& car : carModels)
	 	delete car;
	carModels.clear();
	//carPoses.clear();
	
	// gcom->mToolTip = 0;  //?

	scn->DelRoadDens();
	scn->grass->Destroy();
	scn->DestroyTrees();
	DestroyObjects(true);
	scn->DestroyRoads();
	scn->DestroyTerrains();
	scn->DestroyFluids();
	scn->DestroyEmitters(true);
	scn->DestroyAllAtmo();

	scn->DestroyTrail();

	if (pGame)
		pGame->End();
	
	// delete[] blendMtr;  blendMtr = 0;

	// BaseApp::destroyScene();

	LogO(">>>>>>>> Destroy SR ----");
	Destroy();
	LogO(">>>>>>>> Destroy SR done ----");
}


//  💫 Simulation  2nd thread
//----------------------------------------------------------------------------------
void App::UpdThr()
{
	Ogre::Timer gtim;
	//#ifdef _WIN32
	//DWORD af = 2;
	//gtim.setOption("QueryAffinityMask", &af);
	//#endif
	gtim.reset();

	while (!mGraphicsSystem->getQuit())
	{
		///  step Game  **

		double dt = double(gtim.getMicroseconds()) * 0.000001;
		gtim.reset();
		
		if (!bLoading && !mGraphicsSystem->getQuit() && pGame)
		{
			bSimulating = true;
			bool ret = pGame->OneLoop(dt);
			if (!ret)
				mGraphicsSystem->setQuit();

			// DoNetworking();
			bSimulating = false;
		}
		this_thread::sleep_for(chrono::milliseconds(
			pSet ? pSet->thread_sleep : 100));
	}
}
