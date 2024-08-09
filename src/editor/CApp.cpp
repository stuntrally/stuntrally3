#include "pch.h"
#include "enums.h"
#include "Def_Str.h"
#include "CData.h"
#include "CApp.h"
#include "CGui.h"
#include "GuiCom.h"
#include "CScene.h"
#include "Axes.h"
#include "TracksXml.h"

#include "GraphicsSystem.h"
#include "MainEntryPoints.h"
#include <string>
#include <filesystem>
using namespace Ogre;
using namespace std;


//  🌟 ctor
//------------------------------------------------------------------------------------
App::App()
{
	Axes::Init();
	
	for (int i = ED_Deform; i <= ED_Filter; ++i)
	{
		br[i].size = 20.f;
		br[i].intens = 20.f;  br[i].power = 2.f;
		br[i].freq = 1.f;  br[i].octaves = 5;
		br[i].offsetX = 0.f;  br[i].offsetY = 1.f;
		br[i].shape = BRS_Sinus;
		br[i].height = 10.f;  br[i].filter = 2.f;
	}
	colNew.pos.y = 1.f;  // new ofs h
}


//  🌟🌟 Init SR editor
//------------------------------------------------------------------------------------
void App::Load()
{
	Ogre::Timer ti;

	//  log args
	auto& args = MainEntryPoints::args;
	LogO("A--- Cmd Line Arguments: "+toStr(args.all.size()));
	LogO("A--- exe path: "+args.all[0]);
	
	if (args.all.size() > 1)
		LogO("A--- Argument1: "+args.all[1]);


	LogO(">>>> Init editor ----");

	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	updBrush();

	
	///  new  ----
	scn = new CScene(this);

	gcom = new CGuiCom(this);
	gcom->mGui = mGui;
	gcom->sc = scn->sc;
	scn->sc->gcom = gcom;

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
	delete scn;  scn = 0;

	BltWorldDestroy();
	
	delete[] pBrFmask;  pBrFmask = 0;
	delete[] mBrushData;  mBrushData = 0;

	delete gcom;  gcom = 0;
	delete gui;  gui = 0;
}
