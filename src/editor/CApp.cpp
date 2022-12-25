#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "pathmanager.h"
#include "CApp.h"
#include "CGui.h"
#include "GuiCom.h"
#include "CScene.h"
#include "Axes.h"
#include "Road.h"
// #include "WaterRTT.h"
// #include "RenderBoxScene.h"
#include "settings.h"
#include "GraphicsSystem.h"
#include <string>
#include <filesystem>
using namespace Ogre;
using namespace std;


const Ogre::String App::csBrShape[BRS_ALL] =
{ "Triangle", "Sinus", "Noise", "Noise2", "N-gon" };  // static


//  ctor
//----------------------------------------------------------------------------------------------------------------------
App::App()
{
	// pSet = pSet1;
	Axes::Init();
	
	mBrSize[0] = 16.f;	mBrSize[1] = 24.f;	mBrSize[2] = 16.f;	mBrSize[3] = 16.f;
	mBrIntens[0] = 20.f;mBrIntens[1] = 20.f;mBrIntens[2] = 20.f;mBrIntens[3] = 20.f;
	mBrPow[0] = 2.f;	mBrPow[1] = 2.f;	mBrPow[2] = 2.f;	mBrPow[3] = 2.f;
	mBrFq[0] = 1.f;		mBrFq[1] = 1.f;		mBrFq[2] = 1.f;		mBrFq[3] = 1.f;
	mBrNOf[0] = 0.f;	mBrNOf[1] = 0.f;	mBrNOf[2] = 0.f;	mBrNOf[3] = 0.f;
	mBrOct[0] = 5;		mBrOct[1] = 5;		mBrOct[2] = 5;		mBrOct[3] = 5;
	mBrShape[0] = BRS_Sinus;  mBrShape[1] = BRS_Sinus;
	mBrShape[2] = BRS_Sinus;  mBrShape[3] = BRS_Sinus;
}

//  load settings from default file
void App::LoadDefaultSet(SETTINGS* settings, string setFile)
{
	settings->Load(PATHMANAGER::GameConfigDir() + "/editor-default.cfg");
	settings->Save(setFile);
}

void App::Load()
{
	
	Ogre::Timer ti;
	setlocale(LC_NUMERIC, "C");

	//  Paths
	PATHMANAGER::Init();
	

	///  Load Settings
	//----------------------------------------------------------------
	pSet = new SETTINGS();
	string setFile = PATHMANAGER::SettingsFile();
	
	if (!PATHMANAGER::FileExists(setFile))
	{
		cerr << "Settings not found - loading defaults." << endl;
		LoadDefaultSet(pSet,setFile);
	}
	pSet->Load(setFile);  // LOAD
	if (pSet->version != SET_VER)  // loaded older, use default
	{
		cerr << "Settings found, but older version - loading defaults." << endl;
		std::filesystem::rename(setFile, PATHMANAGER::UserConfigDir() + "/editor_old.cfg");
		LoadDefaultSet(pSet,setFile);
		pSet->Load(setFile);  // LOAD
	}


	//  paths
	LogO(PATHMANAGER::info.str());


	///  Game start
	//----------------------------------------------------------------
	LogO(">>>> Init editor ----");

	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	updBrush();

	///  new
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
	mCamera = mGraphicsSystem->getCamera();
	mSceneMgr = mGraphicsSystem->getSceneManager();


	LoadData();  /// loads data xmls
}

void App::LoadData()
{
	Ogre::Timer ti;

	//  data xmls
	// pGame->ReloadSimData();  // need surfaces
	
	scn->data->Load(/*&pGame->surf_map*/0, 0);
	scn->sc->pFluidsXml = scn->data->fluids;
	scn->sc->pReverbsXml = scn->data->reverbs;

}

///  material factory setup
//---------------------------------------------------------------------------------------------------------------------------
/*void App::postInit()
{
	sh::OgrePlatform* platform = new sh::OgrePlatform("General", PATHMANAGER::Data() + "/" + "materials");
	platform->setCacheFolder(PATHMANAGER::ShaderDir());
}*/

App::~App()
{
	DestroyObjects(false);

	delete scn;

	// delete mFactory;  //!

	// gui->viewBox->destroy();
	// delete gui->viewBox;

	BltWorldDestroy();
	
	delete[] pBrFmask;  pBrFmask = 0;

	delete[] mBrushData;

	delete gcom;
	delete gui;
}
	

//  util
//---------------------------------------------------------------------------------------------------------------
ManualObject* App::Create2D(const String& mat, Real s, bool dyn)
{
	/*ManualObject* m = mSceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);
	m->estimateVertexCount(4);
	m->begin(mat, RenderOperation::OT_TRIANGLE_STRIP);
	m->position(-s,-s*asp, 0);  m->textureCoord(0, 1);
	m->position( s,-s*asp, 0);  m->textureCoord(1, 1);
	m->position(-s, s*asp, 0);  m->textureCoord(0, 0);
	m->position( s, s*asp, 0);  m->textureCoord(1, 0);
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RQG_Hud2);*/
	return 0;
}
