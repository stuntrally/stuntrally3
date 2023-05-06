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


const Ogre::String App::csBrShape[BRS_ALL] =
{ "Triangle", "Sinus", "N-gon", "Noise", "Noise2" };  // static


//  🌟 ctor
//------------------------------------------------------------------------------------
App::App()
{
	// pSet = pSet1;
	Axes::Init();
	
	br[0].size = 16.f;   br[1].size = 24.f;   br[2].size = 16.f;   br[3].size = 16.f;
	br[0].intens = 20.f; br[1].intens = 20.f; br[2].intens = 20.f; br[3].intens = 20.f;
	br[0].power = 2.f;   br[1].power = 2.f;   br[2].power = 2.f;   br[3].power = 2.f;
	br[0].freq = 1.f;    br[1].freq = 1.f;    br[2].freq = 1.f;    br[3].freq = 1.f;
	br[0].nofs = 0.f;    br[1].nofs = 0.f;    br[2].nofs = 0.f;    br[3].nofs = 0.f;
	br[0].octaves = 5;   br[1].octaves = 5;	  br[2].octaves = 5;   br[3].octaves = 5;
	br[0].shape = BRS_Sinus;  br[1].shape = BRS_Sinus;
	br[2].shape = BRS_Sinus;  br[3].shape = BRS_Sinus;
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
