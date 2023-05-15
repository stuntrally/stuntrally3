#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "paths.h"
#include "CApp.h"
#include "CGui.h"
#include "GuiCom.h"
#include "CScene.h"
#include "Axes.h"
#include "Road.h"
#include "TracksXml.h"
// #include "RenderBoxScene.h"
#include "settings.h"
#include "GraphicsSystem.h"
#include <string>
#include <filesystem>
using namespace Ogre;
using namespace std;


//  🌟 ctor
//------------------------------------------------------------------------------------
App::App()
{
	Axes::Init();
	
	br[0].size = 16.f;   br[1].size = 24.f;   br[2].size = 16.f;   br[3].size = 16.f;
	for (int i=0; i<4; ++i)
	{
		br[i].intens = 20.f;  br[i].power = 2.f;
		br[i].freq = 1.f;  br[i].octaves = 5;  br[i].offset = 0.f;
		br[i].shape = BRS_Sinus;
		br[i].height = 10.f;  br[i].filter = 2.f;
	}
}


//  🌟🌟 Init SR editor
//------------------------------------------------------------------------------------
void App::Load()
{
	Ogre::Timer ti;

	LoadSettings();


	LogO(">>>> Init editor ----");

	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	updBrush();

	
	///  new  ----
	scn = new CScene(this);

	gcom = new CGuiCom(this);
	gcom->mGui = mGui;
	gcom->sc = scn->sc;

	gui = new CGui(this);
	// gui->viewBox = new wraps::RenderBoxScene();
	gui->gcom = gcom;
	gui->sc = scn->sc;
	gui->scn = scn;
	gui->data = scn->data;

	// pGame->app = this;
	// sc = scn->sc;

	// pGame->ReloadSimData();


	mRoot = mGraphicsSystem->getRoot();
	mWindow = mGraphicsSystem->getRenderWindow();
	mSceneMgr = mGraphicsSystem->getSceneManager();
	mCamera = mGraphicsSystem->getCamera();


	LoadData();  /// loads data xmls
}

void App::LoadData()
{
	// Ogre::Timer ti;

	//  data xmls
	// pGame->ReloadSimData();  // need surfaces
	
	scn->data->Load(/*&pGame->surf_map*/0, 0);
	scn->sc->pFluidsXml = scn->data->fluids;
	scn->sc->pReverbsXml = scn->data->reverbs;

}

//  💥💥 dtor
//------------------------------------------------------------------------------------
App::~App()
{
	///+  save settings
	SaveCam();
	pSet->Save(PATHS::SettingsFile(1));
	scn->data->user->SaveXml(PATHS::UserConfigDir() + "/tracks.xml");

	DestroyObjects(false);

	delete scn;

	// gui->viewBox->destroy();
	// delete gui->viewBox;

	BltWorldDestroy();
	
	delete[] pBrFmask;  pBrFmask = 0;

	delete[] mBrushData;

	delete gcom;
	delete gui;
}
