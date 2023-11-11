#include "pch.h"
#include "par.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "GuiCom.h"
// #include "Def_Str.h"
// #include "CData.h"
#include "SceneXml.h"
#include "CScene.h"
#include "game.h"

#include "CarModel.h"
#include "Grass.h"
// #include "Road.h"
// #include "SoundMgr.h"
#include "Gui_Popup.h"
#include "Axes.h"
// #include "RenderBoxScene.h"
#include "GraphicsSystem.h"
#include "BtOgreExtras.h"
using namespace Ogre;


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
	for (int i = 0; i < MAX_Players; ++i)
		hud->arrow[i].Destroy(mSceneMgr);
	
	// for (int i=0; i < MAX_Players; ++i)
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
	scn->refl.DestroyFluids();
	scn->refl.DestroyRTT();
	scn->DestroyEmitters(true);
	scn->DestroyAllAtmo();

	scn->DestroyPace();
	scn->DestroyTrails();

	if (pGame)
		pGame->End();
	
	// delete[] blendMtr;  blendMtr = 0;


	LogO(">>>>>>>> Destroy SR ----");
	Destroy();
	LogO(">>>>>>>> Destroy SR done ----");
}


void App::updCarLightsBright()
{
	for (auto& c:carModels)
		c->updLightsBright();
}
