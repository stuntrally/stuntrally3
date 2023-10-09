#include "pch.h"
#include "enums.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CData.h"
#include "TracksXml.h"
#include "PresetsXml.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "MultiList2.h"
#include "Slider.h"
#include "paths.h"

#include <filesystem>
#include <MyGUI.h>
#include <MyGUI_InputManager.h>
#include <OgreTimer.h>
// #include <OgreWindow.h>
// #include "RenderBoxScene.h"
using namespace MyGUI;
// using namespace Ogre;
using Ogre::String;  using Ogre::StringUtil;
using namespace std;


///  Gui Init
//----------------------------------------------------------------------------------------------------------------------

void CGui::InitGui() 
{
	LogO(Ogre::String(":::# Init Gui ----"));
	mGui = app->mGui;
	gcom->mGui = mGui;
	Check::pGUI = mGui;  SliderValue::pGUI = mGui;
	Check::bGI = &bGI;   SliderValue::bGI = &bGI;

	if (!mGui)  return;
	Ogre::Timer ti;

	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");


	///  Load .layout files
	auto Load = [&](string file)
	{
		auto v = LayoutManager::getInstance().loadLayout(file + ".layout");
		app->vwGui.insert(app->vwGui.end(), v.begin(), v.end());
	};
	Load("Common");  Load("MaterialEditor");
	Load("Editor_Main");   Load("Editor");  Load("Editor_Track");
	Load("Editor_Utils");  Load("Editor_Tools");
	Load("Editor_Help");   Load("Editor_Options");


	//  🪟 main windows  ----
	app->mWndMain = fWnd("MainMenuWnd");
	app->mWndTrack = fWnd("TrackWnd");  app->mWndEdit = fWnd("EditorWnd");
	app->mWndOpts = fWnd("OptionsWnd"); app->mWndHelp = fWnd("HelpWnd");  // common
	app->mWndMaterials = fWnd("MaterialsWnd");
	
	app->mWndPick = fWnd("PickWnd");
	app->mWndTrkFilt = fWnd("TrackFilterWnd");

	//  tool wnds edit  ----
	const int y = 96;
	app->mWndCam =   fWnd("CamWnd");    app->mWndCam->setPosition(0,y);
	app->mWndStart = fWnd("StartWnd");  app->mWndStart->setPosition(0,y);
	app->mWndBrush = fWnd("BrushWnd");  app->mWndBrush->setPosition(0,y);

	app->mWndRoadCur =   fWnd("RoadCur");    app->mWndRoadCur->setPosition(0,y);
	app->mWndRoadStats = fWnd("RoadStats");  app->mWndRoadStats->setPosition(0,y+380);

	app->mWndFluids =    fWnd("FluidsWnd");   app->mWndFluids->setPosition(0,y);
	app->mWndObjects =   fWnd("ObjectsWnd");  app->mWndObjects->setPosition(0,y);
	app->mWndParticles = fWnd("ParticlesWnd"); app->mWndParticles->setPosition(0,y);


	//  for find defines
	Btn btn, bchk;
	Sl* sl;  SV* sv;  Ck* ck;

	
	//  Tabs  --------
	TabPtr tab,sub;
	fTabW("TabWndTrack");  app->mWndTabsTrack = tab;
	fTabW("TabWndEdit");   app->mWndTabsEdit = tab;
	fTabW("TabWndOpts");   app->mWndTabsOpts = tab;
	fTabW("TabWndHelp");   app->mWndTabsHelp = tab;
	fTabW("TabWndMat");    app->mWndTabsMat = tab;

	//  get sub tabs
	vSubTabsTrack.clear();
	size_t u;
	for (u=0; u < app->mWndTabsTrack->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsTrack->getItemAt(u));
		vSubTabsTrack.push_back(sub);
	}
	vSubTabsEdit.clear();
	for (u=0; u < app->mWndTabsEdit->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsEdit->getItemAt(u));
		vSubTabsEdit.push_back(sub);
	}
	vSubTabsHelp.clear();
	for (u=0; u < app->mWndTabsHelp->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsHelp->getItemAt(u));
		vSubTabsHelp.push_back(sub);
	}
	vSubTabsOpts.clear();
	for (u=0; u < app->mWndTabsOpts->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsOpts->getItemAt(u));
		vSubTabsOpts.push_back(sub);
	}
	vSubTabsMat.clear();
	for (u=0; u < app->mWndTabsMat->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsMat->getItemAt(u));
		vSubTabsMat.push_back(sub);
	}

	///  🎛️ Gui common init  ---
	InitMainMenu();
	gcom->GuiInitTooltip();
	gcom->GuiInitLang();

	gcom->GuiInitGraphics();
	gcom->InitGuiScreenRes();


	//app->mWndTabs->setIndexSelected(3);  //default*--
	gcom->ResizeOptWnd(); //?


	//  center mouse pos
	// app->mInputWrapper->setMouseVisible(app->bGuiFocus || !app->bMoveCam);
	gcom->GuiCenterMouse();
	
	//  hide all wnd  ---
	app->SetEdMode(ED_Deform);
	app->UpdEditWnds();
	app->mWndBrush->setVisible(false);
	

	#if 0  ///0 _tool_ fix video capture cursor-
	imgCur = mGui->createWidget<ImageBox>("ImageBox", 100,100, 32,32, Align::Default, "Pointer");
	imgCur->setImageTexture("pointer.png");
	imgCur->setVisible(true);
	#endif


	//  🛠️ tool window texts  ----------------------
	int i;
	for (i=0; i < RD_TXT/*MAX*/; ++i)
	{	String s = toStr(i);
		if (i < BR_TXT)  {
			brTxt[i] = fTxt("brTxt"+s);
			brVal[i] = fTxt("brVal"+s);
			brKey[i] = fTxt("brKey"+s);  }
		if (i < RD_TXT)  {
			rdTxt[i] = fTxt("rdTxt"+s);
			rdVal[i] = fTxt("rdVal"+s);
			rdKey[i] = fTxt("rdKey"+s);
			rdImg[i] = fImg("rdImg"+s);  }
		if (i < RDS_TXT)  {
			rdTxtSt[i] = fTxt("rdTxtSt"+s);
			rdValSt[i] = fTxt("rdValSt"+s);  }

		if (i < ST_TXT)   stTxt[i] = fTxt("stTxt"+s);
		if (i < FL_TXT)   flTxt[i] = fTxt("flTxt"+s);
		if (i < OBJ_TXT)  objTxt[i]= fTxt("objTxt"+s);
		if (i < EMT_TXT)  emtTxt[i]= fTxt("emtTxt"+s);
	}


	//  mode, status
	imgCam  = fImg("modeCam");  imgEdit = fImg("modeEdit");  imgGui  = fImg("modeGui");
	panStatus = fWP("panStatus");  txtStatus = fTxt("txtStatus");

	static float f=0.f;  i=0;  // temp vars
	static bool b=0;


	///  🖌️⛰️ brush presets   o o o o o o
	//------------------------------------------------------------------------
	scvBrushes = fScv("svBrushes");  imgBrCur = fImg("imgBrCur");
	
	ck= &ckBrushAdj;  ck->Init("chkBrushAdj", &pSet->br_adjust);  Cev(BrushAdj);
	panBrushAdj = fWP("panBrushAdj");  panBrushAdj->setVisible(pSet->br_adjust);
	Btn("BrushAdd", btnBrushAdd);  Btn("BrushDel", btnBrushDel);
	Btn("BrushesSave", btnBrushesSave);  Btn("BrushesLoad", btnBrushesLoad);  Btn("BrushesLoadDef", btnBrushesLoadDef);

	Btn("BrushRandom", btnBrushRandom);  Btn("BrushRandom2", btnBrushRandom2);

	//  load bushes
	auto usrB = PATHS::UserConfigDir()+"/brushes.", cfgB = PATHS::GameConfigDir()+"/brushes.";
	auto usrBr = usrB+"ini", usrPrv = usrB+"png";
	if (!PATHS::FileExists(usrBr))
		Copy(cfgB+"ini", usrBr);
	
	if (!PATHS::FileExists(usrPrv))
		Copy(cfgB+"png", usrPrv);
	// app->prvBrushes.Load(usrPrv,1);  // doesnt show

	btnBrushesLoad(0);
	// GuiShortcut(WND_Edit, TAB_Terrain,2);  // test, shows

	//  🖌️ brush params
	sv= &svBrSize;	sv->Init("BrSize",	&f, 2.f,BrushMaxSize, 2.f, 1,4);  sv->DefaultF(20.f);  //Sev(UpdBr);
	sv= &svBrForce;	sv->Init("BrForce",	&f, 0.1f,100.f, 1.f, 1,4);  sv->DefaultF(20.f);
	sv= &svBrPower;	sv->Init("BrPower",	&f, 0.02f,10.f, 4.f, 2,4);  sv->DefaultF(1.5f);  Sev(UpdBr);
	sv= &svBrShape;
	for (i=0; i < BRS_ALL; ++i)
		sv->strMap[i] = BrushesIni::csBrShape[i];
					sv->Init("BrShape",	&i, 0,BRS_ALL-1, 1);  sv->DefaultI(BRS_Noise);  Sev(UpdBr);
	sv= &svBrFreq;	sv->Init("BrFreq",	&f, 0.01f,2.f, 3.f, 3,5);  sv->DefaultF(0.2f);  Sev(UpdBr);
	sv= &svBrOct;	sv->Init("BrOct",	&i, 1,9, 1.f);  sv->DefaultI(3);  Sev(UpdBr);
	sv= &svBrOfsX;	sv->Init("BrOfsX",	&f,-20.f,20.f, 1.f, 2,4);  sv->DefaultF(2.f);  Sev(UpdBr);
	sv= &svBrOfsY;	sv->Init("BrOfsY",	&f,-10.f,10.f, 1.f, 2,4);  sv->DefaultF(2.f);  Sev(UpdBr);

	auto& brH = app->br[ED_Height];
	sv= &svBrTerH;  sv->Init("BrTerH",  &brH.height, -600.f,1500.f,  1.f, 1,3);  sv->DefaultF(20.f);
	sv= &svBrFilt;	sv->Init("BrFilt",	&f, 0.1f,8.f, 1.5f, 2,4);  sv->DefaultF(2.f);
	
	//  brush list par
	sv= &svBrZoom;	sv->Init("BrZoom",	&app->brSets.imgSize, 15,90, 1.f);  sv->DefaultI(48);  Sev(BrNewLine);
	Edt(edBrName, "BrName", editBrName);
	sv= &svBrRate;  sv->Init("BrRate",	&i,-2,4, 1.f);  sv->DefaultI(0);  Sev(BrNewLine);
	sv= &svBrNewLn; sv->Init("BrNewLn", &i,-2,1, 1.f);  sv->DefaultI(0);  Sev(BrNewLine);
	SldUpdBr();


	///  🎨 Color tool wnd  ----
	wndColor = fWnd("ColorWnd");											 //0.165
	sv= &svHue;		sv->Init("clrHue",	&f, 0.f,1.f, 1.f, 3,5);  sv->DefaultF(0.53f);  Sev(UpdClr);
	sv= &svSat;		sv->Init("clrSat",	&f, 0.f,1.f, 1.f, 3,5);  sv->DefaultF(0.1f);  Sev(UpdClr);
	sv= &svVal;		sv->Init("clrVal",	&f, 0.f,3.f, 2.f, 3,5);  sv->DefaultF(0.5f);  Sev(UpdClr);

	sv= &svAlp;		sv->Init("clrAlp",	&f, 0.f,2.f, 2.f, 3,5);  sv->DefaultF(1.f);   Sev(UpdClr);
	sv= &svNeg;		sv->Init("clrNeg",	&f, 0.f,1.f, 1.f, 3,5);  sv->DefaultF(0.f);   Sev(UpdClr);


	///  📊 Settings
	//------------------------------------------------------------------------
	ck= &ckMinimap;		ck->Init("Minimap",		&pSet->trackmap);  Cev(Minimap);
	sv= &svSizeMinimap;	sv->Init("SizeMinimap",	&pSet->size_minimap, 0.15f,2.f);  sv->DefaultF(0.55f);  Sev(SizeMinimap);

	sv= &svMiniNum;		sv->strMap[App::RT_ALL     ] = "ALL";
	sv->strMap[App::RT_RoadDens] = "Road Density";		sv->strMap[App::RT_RoadPrv ] = "Road Preview";  // todo: TR(..
	sv->strMap[App::RT_Terrain ] = "Terrain Preview";	sv->strMap[App::RT_View3D    ] = "Track View";
	sv->strMap[App::RT_PreView3D] = "Object View";
						sv->Init("MiniNum",		&pSet->num_mini,  0, App::RT_ALL-1);  sv->DefaultI(1);  Sev(MiniNum);

	// sv= &svTerUpd;		sv->Init("TerUpd",		&pSet->ter_skip,  0, 20);  sv->DefaultI(1);  //; todo: restore
	// sv= &svMiniUpd;		sv->Init("MiniUpd",		&pSet->mini_skip, 0, 20);  sv->DefaultI(4);

	sv= &svRoadUpd;		sv->Init("RoadUpd",		&pSet->road_skip, 0, 30);  sv->DefaultI(8);
	sv= &svSizeRoadP;	sv->Init("SizeRoadP",	&pSet->road_sphr, 0.1f,12.f);  sv->DefaultF(2.f);  Sev(SizeRoadP);

	//  📈 debug, other
	ck= &ckWireframe;	ck->Init("Wireframe",	&app->bWireframe);  Cev(Wireframe);
	ck= &ckCamPos;		ck->Init("CamPos",		&pSet->camPos);    Cev(CamPos);
	ck= &ckInputBar;	ck->Init("InputBar",	&pSet->inputBar);  Cev(InputBar);
	ck= &ckAllowSave;	ck->Init("AllowSave",	&pSet->allow_save);

	//  🎥 Camera set btns
	Btn("CamView1", btnSetCam);  Btn("CamView2", btnSetCam);
	Btn("CamView3", btnSetCam);  Btn("CamView4", btnSetCam);
	Btn("CamTop",   btnSetCam);
	Btn("CamLeft",  btnSetCam);  Btn("CamRight", btnSetCam);
	Btn("CamFront", btnSetCam);  Btn("CamBack",  btnSetCam);

	sv= &svCamSpeed;	sv->Init("CamSpeed",	&pSet->cam_speed, 0.1f,4.f);  sv->DefaultF(0.9f);
	sv= &svCamInert;	sv->Init("CamInert",	&pSet->cam_inert, 0.f, 1.f);  sv->DefaultF(0.4f);

	//  startup
	ck= &ckStartInMain;	ck->Init("StartInMain", &pSet->startInMain);
	ck= &ckAutoStart;	ck->Init("AutoStart",   &pSet->autostart);
	ck= &ckEscQuits;	ck->Init("EscQuits",    &pSet->escquit);
 // ck= &ckOgreDialog;	ck->Init("OgreDialog",  &pSet->ogre_dialog);  // todo: read cfg early
 // ck= &ckMouseCapture;ck->Init("MouseCapture",&pSet->mouse_capture);
 // ck= &ckScreenPng;   ck->Init("ScreenPng",   &pSet->screen_png);
	

	///  🌞 Sun
	//------------------------------------------------------------------------
	sv= &svSkyYaw;		sv->Init("SkyYaw",		&sc->skyYaw,  -180.f,180.f, 1.f, 1,4);  sv->DefaultF(0.f);  Sev(UpdSky);
	sv= &svSunPitch;	sv->Init("SunPitch",	&sc->ldPitch,    0.f,90.f,  1.f, 1,4);  sv->DefaultF(54.f);    Sev(UpdSun);
	sv= &svSunYaw;		sv->Init("SunYaw",		&sc->ldYaw,   -180.f,180.f, 1.f, 1,4);  sv->DefaultF(-123.f);  Sev(UpdSun);
	sv= &svRain1Rate;	sv->Init("Rain1Rate",	&sc->rainEmit,   0.f,90000.f, 2.5f);  sv->DefaultF(1000.f);
	sv= &svRain2Rate;	sv->Init("Rain2Rate",	&sc->rain2Emit,  0.f,90000.f, 2.5f);  sv->DefaultF(1000.f);
	//  🌫️ Fog
	sv= &svFogStart;	sv->Init("FogStart",	&sc->fogStart,   0.f,4000.f, 2.f, 0,3);  sv->DefaultF(100.f);  Sev(UpdFog);
	sv= &svFogEnd;		sv->Init("FogEnd",		&sc->fogEnd,     0.f,9000.f, 2.f, 0,3);  sv->DefaultF(600.f);  Sev(UpdFog);
	sv= &svFogHStart;	sv->Init("FogHStart",	&sc->fogHStart,  0.f,2000.f, 2.f, 0,3);  sv->DefaultF(0.f);    Sev(UpdFog);
	sv= &svFogHEnd;		sv->Init("FogHEnd",		&sc->fogHEnd,    0.f,2000.f, 2.f, 0,3);  sv->DefaultF(60.f);   Sev(UpdFog);
	sv= &svFogHeight;	sv->Init("FogHeight",	&sc->fogHeight, -200.f,200.f, 1.f, 1,4);  sv->DefaultF(-300.f);  Sev(UpdFog);
	sv= &svFogHDensity;	sv->Init("FogHDensity",	&sc->fogHDensity,  0.f,200.f, 2.f, 1,4);  sv->DefaultF(60.f);  Sev(UpdFog);
	sv= &svFogHDmg;		sv->Init("FogHDmg",		&sc->fHDamage,    0.f, 30.f, 1.5f, 1,3);  sv->DefaultF(0.f);
	//  toggle vis
	ck= &ckFog;			ck->Init("FogEnable",		&pSet->bFog);  Cev(Fog);
	ck= &ckWeather;		ck->Init("WeatherEnable",	&pSet->bWeather);
	ck= &ckEmitters;	ck->Init("EmittersEnable",	&pSet->bEmitters);  Cev(Emitters);

	//  🎨 clr imgs
	ImgB(clrAmb,  "ClrAmb",  btnClrSet);  ImgB(clrDiff, "ClrDiff",  btnClrSet);
	ImgB(clrSpec, "ClrSpec", btnClrSet);
	ImgB(clrFog,  "ClrFog",  btnClrSetA); ImgB(clrFog2, "ClrFog2",  btnClrSetA);
	ImgB(clrFogH, "ClrFogH", btnClrSetA);
	ImgB(clrTrail,"ClrTrail",btnClrSetA);


	///  ⛰️ Terrain
	//------------------------------------------------------------------------
	txTersCur = fTxt("TersCur");  txTersCur2 = fTxt("TersCur2");
	Btn("TersPrev",  btnTersPrev);  Btn("TersNext",  btnTersNext);
	Btn("TersPrev2", btnTersPrev);  Btn("TersNext2", btnTersNext);
	Btn("TersAdd",   btnTersAdd);   Btn("TersDel",   btnTersDel);

	imgTexDiff = fImg("TerImgDiff");
	Tab(tabsHmap, "TabHMapSize", tabHmap);  valHmapMB = fTxt("TerHmapMB");
	Btn("TerUpdateLight", btnUpdateLayers);

	//  diff clr
	sv= &svTerDiffR;  sv->Init("TerDiffR", &f, 0.f,2.f,  1.5f, 1,3);  sv->DefaultF(1.f);  //Sev(TerPar);
	sv= &svTerDiffG;  sv->Init("TerDiffG", &f, 0.f,2.f,  1.5f, 1,3);  sv->DefaultF(1.f);  //Sev(TerPar);
	sv= &svTerDiffB;  sv->Init("TerDiffB", &f, 0.f,2.f,  1.5f, 1,3);  sv->DefaultF(1.f);  //Sev(TerPar);
	//  emis, refl
	ck= &ckTerrainEmissive;	ck->Init("TerrainEmissive",	&b);
	sv= &svTerReflect;
	sv->strMap[-1] = TR("#{No}");  sv->strMap[0] = TR("#{auto}");  sv->strMap[1] = TR("#{Yes}");
						sv->Init("TerReflect", &i,  -2,1);  sv->DefaultI(0);  //Sev(TerPar);

	// sv= &svTerNormScale;  sv->Init("TerNormScale", &sc->td.normScale,  0.01f,3.f, 1.f, 1,3);  sv->DefaultF(1.f);  Sev(TerPar);
	// sv= &svTerSpecPow;    sv->Init("TerSpecPow",   &sc->td.specularPow,   0.2f,128.f,2.f, 1,4);  sv->DefaultF(32.f); Sev(TerPar);
	// sv= &svTerSpecPowEm;  sv->Init("TerSpecPowEm", &f, 0.5f,4.f,  1.f, 1,3);  sv->DefaultF(2.f);  Sev(TerPar);

	//  horiz, collis
	sv= &svTerHoriz;	sv->Init("TerHoriz",	&i,    0, 2);  sv->DefaultI(0);  Sev(TerHoriz);
	ck= &ckTerCollis;	ck->Init("TerCollis",	&b);   Cev(TerCollis);
	//  pos
	sv= &svTerPosX;		sv->Init("TerPosX",	&f, -2000.f,2000.f, 1.f, 2,5);  sv->DefaultF(0.f);  Sev(TerPosX);
	sv= &svTerPosZ;		sv->Init("TerPosZ",	&f, -2000.f,2000.f, 1.f, 2,5);  sv->DefaultF(0.f);  Sev(TerPosZ);
	//  borders []
	ck= &ckTerBL;	ck->Init("TerBL",	&b);   Cev(TerBL);
	ck= &ckTerBR;	ck->Init("TerBR",	&b);   Cev(TerBR);
	ck= &ckTerBF;	ck->Init("TerBF",	&b);   Cev(TerBF);
	ck= &ckTerBB;	ck->Init("TerBB",	&b);   Cev(TerBB);
	// SldUpd_Ter();  // no ter yet


	Btn("TerrainNew",    btnTerrainNew);   Btn("UpdateTerrain", btnUpdateLayers);
	Btn("TerrainGenAdd", btnTerGenerate);  Btn("TerrainGenSub", btnTerGenerate);    Btn("TerrainGenMul", btnTerGenerate);
	Btn("TerrainHalf",   btnTerrainHalf);  Btn("TerrainDouble", btnTerrainDouble);  Btn("TerrainMove",   btnTerrainMove);

	//  align
	Btn("AlignTerrain",  btnAlignTerToRoad);  Btn("AlignHorizon",  btnAlignHorizonToTer);
	
	sv= &svAH_BaseTer;	sv->Init("AH_BaseTer",	&pSet->ah_base_ter, 0,8);  sv->DefaultI(0);
	// sv= &svAH_Border;	sv->Init("AH_Border",	&pSet->ah_border, 0.f,160.f, 2.f, 2,4);  sv->DefaultF(10.f);
	sv= &svAH_Below;	sv->Init("AH_Below",	&pSet->ah_below, 0.f,160.f, 2.f, 2,4);  sv->DefaultF(10.f);
	

	///  generator  . . . . . . .
	sv= &svTerGenScale;	sv->Init("TerGenScale",	&pSet->gen_scale, 0.f,160.f, 2.f, 2,4);  sv->DefaultF(52.f);
	sv= &svTerGenOfsX;	sv->Init("TerGenOfsX",	&pSet->gen_ofsx, -12.f,12.f, 1.f, 3,5);  sv->DefaultF(0.14f);   Sev(TerGen);
	sv= &svTerGenOfsY;	sv->Init("TerGenOfsY",	&pSet->gen_ofsy, -12.f,12.f, 1.f, 3,5);  sv->DefaultF(-1.54f);  Sev(TerGen);

	sv= &svTerGenFreq;	sv->Init("TerGenFreq",	&pSet->gen_freq,   0.06f,3.f, 2.f, 3,5);  sv->DefaultF(0.914f); Sev(TerGen);
	sv= &svTerGenOct;	sv->Init("TerGenOct",	&pSet->gen_oct,    0, 9);                 sv->DefaultI(4);      Sev(TerGen);
	sv= &svTerGenPers;	sv->Init("TerGenPers",	&pSet->gen_persist,0.f, 0.7f, 1.f, 3,5);  sv->DefaultF(0.347f); Sev(TerGen);
	sv= &svTerGenPow;	sv->Init("TerGenPow",	&pSet->gen_pow,    0.f, 6.f,  2.f, 2,4);  sv->DefaultF(1.f);    Sev(TerGen);
	
	sv= &svTerGenMul;	sv->Init("TerGenMul",	&pSet->gen_mul,    0.f, 6.f,  2.f, 2,4);  sv->DefaultF(1.f);
	sv= &svTerGenOfsH;	sv->Init("TerGenOfsH",	&pSet->gen_ofsh,   0.f, 60.f, 2.f, 2,4);  sv->DefaultF(0.f);
	sv= &svTerGenRoadSm;sv->Init("TerGenRoadSm",&pSet->gen_roadsm, 0.f, 6.f,  1.f, 2,4);  sv->DefaultF(0.f);

	sv= &svTerGenAngMin;sv->Init("TerGenAngMin",&pSet->gen_terMinA, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(0.f);
	sv= &svTerGenAngMax;sv->Init("TerGenAngMax",&pSet->gen_terMaxA, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(90.f);
	sv= &svTerGenAngSm;	sv->Init("TerGenAngSm",	&pSet->gen_terSmA,  0.f,  90.f,  2.f, 1,4);  sv->DefaultF(10.f);
	sv= &svTerGenHMin;	sv->Init("TerGenHMin",	&pSet->gen_terMinH,-150.f,150.f, 1.f, 0,1);  sv->DefaultF(-300.f);
	sv= &svTerGenHMax;	sv->Init("TerGenHMax",	&pSet->gen_terMaxH,-150.f,150.f, 1.f, 0,1);  sv->DefaultF( 300.f);
	sv= &svTerGenHSm;	sv->Init("TerGenHSm",	&pSet->gen_terSmH,  0.f,  100.f, 2.f, 1,4);  sv->DefaultF(20.f);


	///  🏔️ Layers
	//------------------------------------------------------------------------
	ck= &ckTerLayOn;	ck->Init("TerLayOn",	&b);   Cev(TerLayOn);
	valTerLAll = fTxt("TerLayersAll");
	valTriplAll = fTxt("TerTriplAll");
	Tab(tabsTerLayers, "TabTerLay", tabTerLayer);
	Btn("UpdateLayers", btnUpdateLayers);

	ck= &ckTexNormAuto;	ck->Init("TexNormAuto",	&bTexNormAuto);
	ck= &ckTerLayTripl;	ck->Init("TerLayTripl",	&b);   Cev(TerLayTripl);
	ck= &ckDebugBlend;	ck->Init("DebugBlend",  &bDebugBlend);  Cev(DebugBlend);  // todo:
	dbgLclr = fImg("dbgTerLclr");

	sv= &svTerTriSize;	sv->Init("TerTriSize", &f, 0.4f, 20.f,  1.5f);  sv->DefaultF(1.0f);  Sev(TerTriSize);
	//  ter layer, blendmap
	sv= &svTerLScale;	sv->Init("TerLScale",  &f, 6.0f, 72.f,  2.f);  sv->DefaultF(8.f);  //Sev(TerLay);
	sv= &svTerLAngMin;  sv->Init("TerLAngMin", &f, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLAngMax;  sv->Init("TerLAngMax", &f, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(90.f);  Sev(TerLay);
	sv= &svTerLAngSm;   sv->Init("TerLAngSm",  &f, 0.f,  90.f,  2.f, 1,4);  sv->DefaultF(20.f);  Sev(TerLay);

	sv= &svTerLHMin;    sv->Init("TerLHMin",   &f,-500.f,950.f, 1.f, 0,2);  sv->DefaultF(-400.f);  Sev(TerLay);
	sv= &svTerLHMax;    sv->Init("TerLHMax",   &f,-500.f,950.f, 1.f, 0,2);  sv->DefaultF( 900.f);  Sev(TerLay);
	sv= &svTerLHSm;     sv->Init("TerLHSm",    &f, 0.f,  400.f, 2.f, 1,4);  sv->DefaultF(20.f);  Sev(TerLay);
	Btn("TerLmoveL", btnTerLmoveL);  Btn("TerLmoveR", btnTerLmoveR);

	//  🌀 noise
	ck= &ckTerLNOnly;   ck->Init("TerLNonly",  &b);   Cev(TerLNOnly);
	sv= &svTerLNoise;   sv->Init("TerLNoise",  &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLNprev;   sv->Init("TerLNprev",  &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLNnext2;  sv->Init("TerLNnext2", &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLDmg;     sv->Init("TerLDmg",    &f, 0.f, 6.f, 1.5f, 1,3);  sv->DefaultF(0.f);
	//  noise params
	for (i=0; i < 2; ++i)
	{	String s = toStr(i+1);
		sv= &svTerLN_Freq[i];  sv->Init("TerLNFreq"+s,  &f, 0.2f,300.f, 4.f, 2,4);  sv->DefaultF(30.f);  Sev(TerLay);
		sv= &svTerLN_Oct[i];   sv->Init("TerLNOct" +s,  &i, 1,5);                   sv->DefaultI(3);     Sev(TerLay);
		sv= &svTerLN_Pers[i];  sv->Init("TerLNPers"+s,  &f, 0.1f, 0.7f, 1.f, 3,5);  sv->DefaultF(0.3f);  Sev(TerLay);
		sv= &svTerLN_Pow[i];   sv->Init("TerLNPow" +s,  &f, 0.2f, 8.f,  2.f);       sv->DefaultF(1.f);   Sev(TerLay);
	}
	//  noise btns
	Btn("TerLNbtn1", radN1);  bRn1 = btn;  bRn1->setStateSelected(true);
	Btn("TerLNbtn2", radN2);  bRn2 = btn;
	for (i=0; i < 15; ++i)
	{	Btn("TerLN_"+toStr(i), btnNpreset);  }
	Btn("TerLNrandom", btnNrandom);
	Btn("TerLNswap", btnNswap);
	
	//  ⚫💭 particles
	sv= &svLDust;   sv->Init("LDust",  &f, 0.f,1.f);  sv->DefaultF(0.f);
	sv= &svLDustS;  sv->Init("LDustS", &f, 0.f,1.f);  sv->DefaultF(0.f);
	sv= &svLMud;    sv->Init("LMud",   &f, 0.f,1.f);  sv->DefaultF(0.f);
	sv= &svLSmoke;  sv->Init("LSmoke", &f, 0.f,1.f);  sv->DefaultF(0.f);
	// SldUpd_Surf();
	Cmb(cmbParDust, "CmbParDust", comboParDust);
	Cmb(cmbParMud,  "CmbParMud",  comboParDust);
	Cmb(cmbParSmoke,"CmbParSmoke",comboParDust);

	//  surface
	Cmb(cmbSurface, "Surface", comboSurface);  //1 txt-
	txtSuBumpWave = fTxt("SuBumpWave");   txtSuFrict  = fTxt("SuFrict");
	txtSuBumpAmp  = fTxt("SuBumpAmp");	  txtSurfTire = fTxt("SurfTire");
	txtSuRollDrag = fTxt("SuRollDrag");	  txtSurfType = fTxt("SurfType");
	// SldUpd_TerL();  // no ter yet

	
	///  🌳🪨 Vegetation
	//------------------------------------------------------------------------
	sv= &svGrassDens;	sv->Init("GrassDens",	&sc->densGrass, 0.f, 1.f, 2.f, 3,5);  sv->DefaultF(0.2f);
	sv= &svTreesDens;	sv->Init("TreesDens",	&sc->densTrees, 0.f, 3.f, 2.f, 2,4);  sv->DefaultF(0.3f);

	//? Ed(GrPage, editTrGr);  Ed(GrDist, editTrGr);
	//- Ed(TrPage, editTrGr);  Ed(TrDist, editTrGr);  Ed(TrImpDist, editTrGr);
	
	sv= &svTrRdDist;	sv->Init("TrRdDist",	&sc->trRdDist,     0,6);   sv->DefaultI(1);  /// todo: restore!
	sv= &svGrDensSmooth;sv->Init("GrDensSmooth",&sc->grDensSmooth, 0,10);  sv->DefaultI(3);

	//  veget models
	ck= &ckPgLayOn;		ck->Init("LTrEnabled",	&b);   Cev(PgLayOn);
	valLTrAll = fTxt("LTrAll");
	Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);
	Btn("UpdateVeget", btnUpdateVeget);

	sv= &svLTrDens;		sv->Init("LTrDens",		 &f, 0.f, 1.f, 2.f, 3,5);  sv->DefaultF(0.15f);
	
	sv= &svLTrRdDist;	sv->Init("LTrRdDist",	 &i, 0,20);  sv->DefaultI(0);
	sv= &svLTrRdDistMax;sv->Init("LTrRdDistMax", &i, 0,20);  sv->DefaultI(20);
	
	sv= &svLTrMinSc;	sv->Init("LTrMinSc",	 &f, 0.f,4.f, 3.f, 3,5);  sv->DefaultF(0.7f);  Sev(LTrSc);
	sv= &svLTrMaxSc;	sv->Init("LTrMaxSc",	 &f, 0.f,4.f, 3.f, 3,5);  sv->DefaultF(1.2f);  Sev(LTrSc);
	
	// sv= &svLTrWindFx;	sv->Init("LTrWindFx",	 &f, 0.f,12.f, 3.f, 3,5);  sv->DefaultF(0.5f);   /// todo: wind from presets.xml
	// sv= &svLTrWindFy;	sv->Init("LTrWindFy",	 &f, 0.f,0.4f, 3.f, 3,5);  sv->DefaultF(0.06f);  /// and  global gui params
	
	sv= &svLTrMaxTerAng;sv->Init("LTrMaxTerAng", &f, 0.f,90.f, 2.f, 1,4);  sv->DefaultF(30.f);

	sv= &svLTrMinTerH;	sv->Init("LTrMinTerH",	 &f,-60.f,60.f, 1.f, 1,4);  sv->DefaultF(-100.f);
	sv= &svLTrMaxTerH;	sv->Init("LTrMaxTerH",	 &f, 0.f,120.f, 1.f, 1,4);  sv->DefaultF( 100.f);
	sv= &svLTrFlDepth;	sv->Init("LTrFlDepth",	 &f, 0.f,5.f, 2.f, 1,4);  sv->DefaultF(0.f);
	SldUpd_PgL();  // real &f set here

	txVCnt = fTxt("LTrInfCnt");
	txVHmin = fTxt("LTrInfHmin");  txVHmax = fTxt("LTrInfHmax");
	txVWmin = fTxt("LTrInfWmin");  txVWmax = fTxt("LTrInfWmax");
	

	///  🌿 Grass
	//------------------------------------------------------------------------
	Ed(GrSwayDistr, editTrGr);  Ed(GrSwayLen, editTrGr);  Ed(GrSwaySpd, editTrGr);

	imgGrass = fImg("ImgGrass");  imgGrClr = fImg("ImgGrClr");
	//; Cmb(cmbGrassClr, "CmbGrClr", comboGrassClr);

	//  grass channels
	sv= &svGrChAngMin;	sv->Init("GrChMinA",	&f, 0.f,90.f, 1.f, 1,4);  sv->DefaultF(30.f);
	sv= &svGrChAngMax;	sv->Init("GrChMaxA",	&f, 0.f,90.f, 1.f, 1,4);  sv->DefaultF(30.f);
	sv= &svGrChAngSm;	sv->Init("GrChSmA",		&f, 0.f,50.f, 2.f, 1,4);  sv->DefaultF(20.f);
									  
	sv= &svGrChHMin;	sv->Init("GrChMinH",	&f,-60.f,60.f,  1.f, 1,4);  sv->DefaultF(-200.f);
	sv= &svGrChHMax;	sv->Init("GrChMaxH",	&f,  0.f,120.f, 1.f, 1,4);  sv->DefaultF( 200.f);
	sv= &svGrChHSm;		sv->Init("GrChSmH",		&f,  0.f,60.f,  2.f, 1,4);  sv->DefaultF(20.f);
	sv= &svGrChRdPow;	sv->Init("GrChRdPow",	&f, -8.f, 8.f,  1.f, 1,4);  sv->DefaultF(0.f);
	//  🌀 noise  // todo:
	sv= &svGrChNoise;	sv->Init("GrChNoise",	&f, 0.f,2.f,   1.f, 1,4);   sv->DefaultF(0.f);
	sv= &svGrChNfreq;	sv->Init("GrChNFreq",	&f, 1.f,300.f, 2.f, 1,3);   sv->DefaultF(30.f);
	sv= &svGrChNoct;	sv->Init("GrChNOct",	&i, 1,5);                   sv->DefaultI(3);
	sv= &svGrChNpers;	sv->Init("GrChNPers",	&f, 0.1f, 0.7f, 1.f, 3,5);  sv->DefaultF(0.3f);
	sv= &svGrChNpow;	sv->Init("GrChNPow",	&f, 0.2f, 8.f,  2.f);       sv->DefaultF(1.f);
	Tab(tabsGrChan, "GrChanTab", tabGrChan);
	SldUpd_GrChan();

	//  grass layers
	ck= &ckGrLayOn;		ck->Init("LGrEnabled",	&b);   Cev(GrLayOn);
	valLGrAll = fTxt("LGrAll");
	Tab(tabsGrLayers, "LGrLayTab", tabGrLayers);
	Btn("UpdateGrass", btnUpdateGrass);

	sv= &svGrMinX;	sv->Init("GrMinX",	&f, 0.5f,4.f, 1.5f);  sv->DefaultF(1.2f);
	sv= &svGrMaxX;	sv->Init("GrMaxX",	&f, 0.5f,4.1, 1.5f);  sv->DefaultF(1.6f);
	sv= &svGrMinY;	sv->Init("GrMinY",	&f, 0.5f,4.f, 1.5f);  sv->DefaultF(1.2f);
	sv= &svGrMaxY;	sv->Init("GrMaxY",	&f, 0.5f,4.f, 1.5f);  sv->DefaultF(1.6f);
	sv= &svGrChan;	sv->Init("LGrChan",	&i, 0,3);  sv->DefaultI(0);
	sv= &svLGrDens;	sv->Init("LGrDens",	&f, 0.001f,1.f, 2.f, 3,5);  sv->DefaultF(0.22f);
	SldUpd_GrL();

	
	///  🛣️ Road
	//------------------------------------------------------------------------
	txRoadsCur = fTxt("RoadsCur");
	Btn("RoadsPrev", btnRoadsPrev);  Btn("RoadsNext", btnRoadsNext);
	Btn("RoadsAdd", btnRoadsAdd);  Btn("RoadsDel", btnRoadsDel);

	sv= &svRdTcMul; 	sv->Init("RdTcMul", 	&f, 0.001f,0.3f, 1.5f, 3,5);  sv->DefaultF(0.1f);
	sv= &svRdTcMulW;	sv->Init("RdTcMulW",	&f, 0.005f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.1f);
	sv= &svRdTcMulP;	sv->Init("RdTcMulP",	&f, 0.005f,0.3f, 1.5f, 3,5);  sv->DefaultF(0.2f);
	sv= &svRdTcMulPW;	sv->Init("RdTcMulPW",	&f, 0.005f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.2f);
	sv= &svRdTcMulC;	sv->Init("RdTcMulC",	&f, 0.005f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.2f);

	sv= &svRdLenDim;	sv->Init("RdLenDim",	&f, 0.5f, 4.f, 1.5f, 2,4);  sv->DefaultF(1.f);
	sv= &svRdWidthSteps;sv->Init("RdWidthSteps",&i, 3,16, 1.5f);  sv->DefaultI(6);
	sv= &svRdPlsM;		sv->Init("RdPlsM",		&f, 1.f, 8.f, 1.5f, 1,3);  sv->DefaultF(1.f);
	sv= &svRdPwsM;		sv->Init("RdPwsM",		&f, 1.f, 8.f, 1.5f, 1,3);  sv->DefaultF(4.f);

	sv= &svRdColN;		sv->Init("RdColN",		&i, 3,16, 1.5f);  sv->DefaultI(4);
	sv= &svRdColR;		sv->Init("RdColR",		&f, 1.0f, 8.f, 1.5f, 2,4);  sv->DefaultF(2.f);

	sv= &svRdMergeLen;	sv->Init("RdMergeLen",	&f, 60.f, 2400.f, 2.f, 0,3);  sv->DefaultF(800.f);
	sv= &svRdLodPLen;	sv->Init("RdLodPLen",	&f, 10.f, 200.f,  2.f, 0,2);  sv->DefaultF(30.f);
	sv= &svRdVisDist;	sv->Init("RdVisDist",	&f, 100.f, 2400.f, 2.f, 0,3);  sv->DefaultF(600.f);
	sv= &svRdVisBehind;	sv->Init("RdVisBehind",	&f, 100.f, 2400.f, 2.f, 0,3);  sv->DefaultF(600.f);
	SldUpd_Road();
	
	Ed(RdHeightOfs, editRoad);
	Ed(RdSkirtLen, editRoad);  Ed(RdSkirtH, editRoad);
	

	///  🎮 Game  ------------------------------------
	sv= &svDamage;		sv->Init("DamageMul",	&sc->damageMul, 0.f,2.f, 1.f, 2,4);  sv->DefaultF(1.f);
	sv= &svWind;		sv->Init("WindAmt",		&sc->windAmt,  -6.f,6.f, 1.f, 2,5);  sv->DefaultF(0.f);
	sv= &svGravity;		sv->Init("Gravity",		&sc->gravity,   2.f,20.f, 1.5f, 2,4);  sv->DefaultF(9.81f);
	ck= &ckDenyReversed;	ck->Init("DenyReversed",	&sc->denyReversed);
	ck= &ckTiresAsphalt;	ck->Init("TiresAsphalt",	&sc->asphalt);
	ck= &ckNoWrongChks;		ck->Init("NoWrongChks",		&sc->noWrongChks);
	txtEdInfo = fTxt("EdInfo");
	
	
	///  🚦 Pacenotes  ------------------------------------
	sv= &svPaceShow;	sv->Init("PaceShow",	&pSet->pace_show,   0,5, 1.f);  sv->DefaultI(3);
	sv= &svPaceDist;	sv->Init("PaceDist",	&pSet->pace_dist,   20.f,2000.f, 2.f, 0,3);  sv->DefaultF(600.f);

	sv= &svPaceSize;	sv->Init("PaceSize",	&pSet->pace_size,   0.1f,2.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	sv= &svPaceNear;	sv->Init("PaceNear",	&pSet->pace_near,   0.1f,2.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	sv= &svPaceAlpha;	sv->Init("PaceAlpha",	&pSet->pace_alpha,  0.3f,1.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	slUpd_Pace(0);
	ck= &ckTrkReverse;	ck->Init("ReverseOn",	&pSet->trk_reverse);  Cev(TrkReverse);
	

	///  Surface
	ck= &ckRoad1Mtr;	ck->Init("Road1Mtr",	&sc->road1mtr);
	

	///  🛠️ Tools  ------------------------------------
	Btn("TrackCopySel", btnTrkCopySel);
	valTrkCpySel = fTxt("TrkCopySelName");

	Btn("CopySun", btnCopySun);				Btn("CopyTerHmap", btnCopyTerHmap);
	Btn("CopyTerLayers", btnCopyTerLayers);	Btn("CopyVeget", btnCopyVeget);
	Btn("CopyRoad", btnCopyRoad);			Btn("CopyRoadPars", btnCopyRoadPars);

	Btn("DeleteRoad", btnDeleteRoad);		Btn("DeleteFluids", btnDeleteFluids);
	Btn("DeleteObjects", btnDeleteObjects); Btn("DeleteParticles", btnDeleteParticles);

	sv= &svScaleAllMul;		sv->Init("ScaleAllMul",		&fScale,    0.5f,2.f, 1.5f);  sv->DefaultF(1.f);
	sv= &svScaleTerHMul;	sv->Init("ScaleTerHMul",	&fScaleTer, 0.5f,2.f, 1.5f);  sv->DefaultF(1.f);
	Btn("ScaleAll",  btnScaleAll);  Btn("ScaleTerH", btnScaleTerH);

	sv= &svAlignWidthAdd;	sv->Init("AlignWidthAdd",	&pSet->al_w_add,  0.f,20.f,1.f, 1,3);  sv->DefaultF(10.f);
	// sv= &svAlignWidthMul;	sv->Init("AlignWidthMul",	&pSet->al_w_mul,  1.f,4.f, 1.f, 2,4);  sv->DefaultF(1.f);
	sv= &svAlignSmooth;		sv->Init("AlignSmooth",		&pSet->al_smooth, 0.f,6.f, 1.f, 1,3);  sv->DefaultF(3.f);

	
	///  ⚠ Warnings  ------------------------------------
	edWarn = fEd("Warnings");
	txWarn = mGui->createWidget<TextBox>("TextBox", 300,20, 360,32, Align::Default, "Back");
	txWarn->setTextShadow(true);  txWarn->setTextShadowColour(Colour::Black);
	txWarn->setTextColour(Colour(1.0,0.4,0.2));  txWarn->setFontHeight(24);
	txWarn->setVisible(false);

	imgWarn = fImg("ImgWarn");  imgWarn->setVisible(false);
	imgInfo = fImg("ImgInfo");

	ck= &ckCheckSave;	ck->Init("CheckSave",	&pSet->check_save);
	ck= &ckCheckLoad;	ck->Init("CheckLoad",	&pSet->check_load);
	

	///  📃 Fill Combo boxes  . . . . . . .
	InitGuiLists();


	///  🏞️ Track
	//------------------------------------------------------------------------
	gcom->sListTrack = pSet->gui.track;  //! set last
	gcom->bListTrackU = pSet->gui.track_user;
	sCopyTrack = "";  //! none
	bCopyTrackU = 0;
	
	//  text desc
	gcom->panTrkDesc[0] = fWP("panTrkDesc0");
	Edt(gcom->trkDesc[0], "TrackDesc0", editTrkDescr);
	Edt(gcom->trkAdvice[0], "TrackAdvice0", editTrkAdvice);
	trkName = fEd("TrackName");
	if (trkName)  trkName->setCaption(pSet->gui.track);

	gcom->GuiInitTrack();
	
	//  btn  new, rename, delete
	Btn("TrackNew",		btnTrackNew);
	Btn("TrackRename",	btnTrackRename);
	Btn("TrackDelete",	btnTrackDel);

	Btn("OpenEdTut", btnEdTut);

    //  New game  ----
    for (i=0; i <= 1; ++i)
    {	Btn("NewGame"+toStr(i), btnNewGame);  }

	
	//  🔧 Tweak mtr  ---------------------
	gcom->GetTweakMtr();
	gcom->InitGuiTweakMtr();
	gcom->updTweakMtr();

	gcom->ChangeTrackView();
	

	///  🧊 3d view []  (veget models, objects)  todo:
	//--------------------------------------------
	//rndCanvas = mGUI->findWidget<Canvas>("CanVeget");  //?
	viewCanvas = app->mWndEdit->createWidget<Canvas>("Canvas", GetViewSize(), Align::Stretch);
	viewCanvas->setInheritsAlpha(false);
	viewCanvas->setPointer("hand");
	viewCanvas->setVisible(false);
	// viewBox->setCanvas(viewCanvas);
	// viewBox->setBackgroundColour(Colour(0.32,0.35,0.37,0.7));
	// viewBox->setAutoRotation(true);
	// viewBox->setMouseRotation(true);


	bGI = true;  // gui inited, gui events can now save vals

	LogO(String(":::* Time Init Gui: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


IntCoord CGui::GetViewSize()
{
	IntCoord ic = app->mWndEdit->getClientCoord();
	return IntCoord(ic.width*0.63f, ic.height*0.45f, ic.width*0.34f, ic.height*0.45f);
}
