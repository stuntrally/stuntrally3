#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "pathmanager.h"
#include "game.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "MultiList2.h"
#include "Slider.h"
#include "Gui_Popup.h"
#include "CData.h"
#include "TracksXml.h"

// #include <OgreRoot.h>
// #include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace MyGUI;
// using namespace Ogre;
using namespace std;


///  🎛️ Gui Init  🚗 Car
//---------------------------------------------------------------------------------------------------------------------

void CGui::InitGuiCar()
{
	//  for find defines
	Btn btn, bchk;  Cmb cmb;
	Slider* sl;  SV* sv;  Ck* ck;
	int i;  float f;

	//  🎨 car color
	sv= &svCarClrH;		sv->Init("CarClrH",     &f, 0.f, 1.f);  Sev(CarClr);
	sv= &svCarClrS;		sv->Init("CarClrS",     &f, 0.f, 1.f);  Sev(CarClr);
	sv= &svCarClrV;		sv->Init("CarClrV",     &f, 0.f, 1.f);  Sev(CarClr);
	sv= &svCarClrGloss;	sv->Init("CarClrGloss", &f, 0.f,   1.3f, 1.5f);  Sev(CarClr);
	sv= &svCarClrMetal;	sv->Init("CarClrMetal", &f, 0.f,   1.0f, 1.f );  Sev(CarClr);
	sv= &svCarClrRough;	sv->Init("CarClrRough", &f, 0.01f, 1.0f, 1.f );  Sev(CarClr);
	imgCarClr = fImg("ImgCarClr");
	UpdCarClrSld();


	///  Car tab  vel graph  ~~~
	graphV = fWP("VelGraph");  WP w;  ISubWidget* sw;
	w = graphV->createWidget<Widget>("PolygonalSkin", IntCoord(IntPoint(), graphV->getSize()), Align::Stretch);
	w->setColour(Colour(0.5f,0.6f,0.6f));
	sw = w->getSubWidgetMain();  graphVGrid = sw->castType<PolygonalSkin>();
	graphVGrid->setWidth(1.5f);
	w = graphV->createWidget<Widget>("PolygonalSkin", IntCoord(IntPoint(), graphV->getSize()), Align::Stretch);
	w->setColour(Colour(0.5f,0.7f,0.8f));
	sw = w->getSubWidgetMain();  graphVel = sw->castType<PolygonalSkin>();
	graphVel->setWidth(3.5f);

	//  setup sss graph
	graphS = fWP("SSSGraph");
	w = graphS->createWidget<Widget>("PolygonalSkin", IntCoord(IntPoint(), graphS->getSize()), Align::Stretch);
	w->setColour(Colour(0.5f,0.6f,0.6f));
	sw = w->getSubWidgetMain();  graphSGrid = sw->castType<PolygonalSkin>();
	graphSGrid->setWidth(1.5f);
	w = graphS->createWidget<Widget>("PolygonalSkin", IntCoord(IntPoint(), graphS->getSize()), Align::Stretch);
	w->setColour(Colour(0.5f,0.8f,0.8f));
	sw = w->getSubWidgetMain();  graphSSS = sw->castType<PolygonalSkin>();
	graphSSS->setWidth(3.5f);


	//  car setup  todo: for each player ..
	Chk("CarABS",  chkAbs, pSet->abs[0]);  bchAbs = bchk;
	Chk("CarTCS",  chkTcs, pSet->tcs[0]);  bchTcs = bchk;

	ck= &ckCarGear;		ck->Init("CarGear",		&pSet->autoshift);  Cev(Gear);
	ck= &ckCarRear;		ck->Init("CarRear",		&pSet->autorear);   Cev(Gear);
	ck= &ckCarRearInv;	ck->Init("CarRearThrInv",&pSet->rear_inv);  Cev(Gear);

	TabPtr tTires = fTab("tabCarTires");  Tev(tTires, TireSet);

	sv= &svSSSEffect;		sv->Init("SSSEffect",		&f, 0.f, 1.f);  Sev(SSS);
	sv= &svSSSVelFactor;	sv->Init("SSSVelFactor",	&f, 0.f, 2.f);	Sev(SSS);
	sv= &svSteerRangeSurf;	sv->Init("SteerRangeSurf",	&f, 0.3f, 1.3f);  Sev(SSS);
	sv= &svSteerRangeSim;	sv->Init("SteerRangeSim",	&f, 0.3f, 1.3f);  Sev(SSS);
	Btn("SSSReset", btnSSSReset);  Btn("SteerReset", btnSteerReset);
	SldUpd_TireSet();


	///  🚗 Car
	//------------------------------------------------------------
	Tbi tbc = fTbi("CarClrs");
	const int clrBtn = data->colors->v.size(), clrRow = data->colors->perRow, sx = data->colors->imgSize;
	for (i=0; i < clrBtn; ++i)
	{
		int x = i % clrRow, y = i / clrRow;
		Img img = tbc->createWidget<ImageBox>("ImageBox",
			12+x*sx, 102+y*sx, sx-1,sx-1, Align::Left, "carClr"+toStr(i));
		img->setImageTexture("white.png");
		gcom->setOrigPos(img, "GameWnd");

		const CarColor& cl = data->colors->v[i];
		float h = cl.hue, s = cl.sat, v = cl.val;
		
		Ogre::ColourValue c;  c.setHSB(1.f-h, s, v);
		img->setColour(Colour(c.r,c.g,c.b));
		img->eventMouseButtonClick += newDelegate(this, &CGui::imgBtnCarClr);
		img->setUserString("s", toStr(s));  img->setUserString("h", toStr(h));  img->setUserString("v", toStr(v));
		img->setUserString("g", toStr(cl.gloss));
		img->setUserString("m", toStr(cl.metal));  img->setUserString("r", toStr(cl.rough));
	}
	Btn("CarClrRandom", btnCarClrRandom);
	sv= &svNumLaps;  sv->Init("NumLaps",  &pSet->gui.num_laps, 1,10, 1.3f);  sv->DefaultI(2);

	//  car stats
	for (i=0; i < iCarSt; ++i)
	{	txCarStTxt[i] = fTxt("cst"+toStr(i));
		txCarStVals[i] = fTxt("csv"+toStr(i));
		barCarSt[i] = fImg("cb"+toStr(i));
	}
	txCarSpeed = fTxt("CarSpeed");  barCarSpeed = fImg("CarSpeedBar");
	txCarType = fTxt("CarType");  txCarYear = fTxt("CarYear");
	txCarRating = fTxt("CarRating");  txCarDiff = fTxt("CarDiff");  txCarWidth = fTxt("CarWidth");
	for (i=0; i < iDrvSt; ++i)
	{	auto s = toStr(i);
		txTrkDrivab[i] = fTxt("txTrkDrivab"+s);
		imgTrkDrivab[i] = fImg("imgTrkDrivab"+s);
	}
	txCarAuthor = fTxt("CarAuthor");  txTrackAuthor = fTxt("TrackAuthor");


	///  📃 Cars list
	//------------------------------------------------------------------------
	Tbi carTab = fTbi("TabCar");
	Mli2 li = carTab->createWidget<MultiList2>("MultiListBox",16,48,600,110, Align::Left | Align::VStretch);
	li->setColour(Colour(0.7,0.85,1.0));
	li->removeAllColumns();  int n=0;
	li->addColumn("#BBA8A8""Id", colCar[n++]);
	li->addColumn("#BBA8A8"+TR("#{Name}"), colCar[n++]);
	li->addColumn("#C0B0A0"">"/*+TR("#{CarSpeed}")*/, colCar[n++]);
	li->addColumn("#C0D0E0""*"/*TR("#{rating}")*/, colCar[n++]);
	li->addColumn("#C0C0E0""!"/*+TR("#{Difficulty}")*/, colCar[n++]);
	li->addColumn("#80A0E0""-"/*+TR("#{Road_Width}")*/, colCar[n++]);
	li->addColumn("#B0B0B0""o", colCar[n++]);
	li->addColumn("#B0B8C0""%", colCar[n++]);  // drivability
	//li->addColumn("#B0B8C0"+TR("#{CarYear}"), colCar[n++]);
	//li->addColumn("#C0C0E0"+TR("#{CarType}"), colCar[n++]);
	li->addColumn(" ", colCar[n++]);
	carList = li;

	FillCarList();  //once

	li->mSortColumnIndex = pSet->cars_sort;
	li->mSortUp = pSet->cars_sortup;
	Lev(carList, CarChng);

	CarListUpd(false);  //upd

	sListCar = pSet->gui.car[0];
	imgCar = fImg("CarImg");  carDesc = fEd("CarDesc");
	//listCarChng(carList,0);  // wrong size graph

	Btn("CarView1", btnCarView1);  Btn("CarView2", btnCarView2);



	///  ⚫ tire graphs
	//------------------------------------------------------------
	sv= &svTC_r;	sv->Init("TC_r",	&pSet->tc_r,  0.5f, 2.f, 1.5f, 1,4);	sv->DefaultF(1.f);
	sv= &svTC_xr;	sv->Init("TC_xr",	&pSet->tc_xr, 0.f, 5.f,  1.5f, 1,4);	sv->DefaultF(1.f);

	sv= &svTE_yf;	sv->Init("TE_yf",	&pSet->te_yf, 5000.f, 10000.f, 1.f, 0,4);	sv->DefaultF(7000.f);
	sv= &svTE_xfx;	sv->Init("TE_xfx",	&pSet->te_xfx, 4.f, 42.f,  2.f, 1,4);	sv->DefaultF(12.f);
	sv= &svTE_xfy;	sv->Init("TE_xfy",	&pSet->te_xfy, 90.f, 560.f, 2.f, 0,3);	sv->DefaultF(160.f);
	sv= &svTE_xpow;	sv->Init("TE_xpow",	&pSet->te_xf_pow, 0.5f, 2.f, 1.f, 1,3);	sv->DefaultF(1.f);

	ck= &ckTE_Common;		ck->Init("TE_Common",		&pSet->te_common);
	ck= &ckTE_Reference;	ck->Init("TE_Reference",	&pSet->te_reference);  //Cev(TEupd);


	///  🔧 Tweak, car edit
	//------------------------------------------------------------
	for (i=0; i < ciEdCar; ++i)
		edCar[i] = fEd("EdCar"+toStr(i));
	edPerfTest = fEd("TweakPerfTest");
	tabEdCar = fTab("TabEdCar");  Tev(tabEdCar, CarEdChng);  tabEdCar->setIndexSelected(pSet->car_ed_tab);

	tabTweak = fTab("TabTweak");  Tev(tabTweak, TweakChng);  tabTweak->setIndexSelected(pSet->tweak_tab);
	txtTweakPath = fTxt("TweakPath");
	Btn("TweakCarSave", btnTweakCarSave);

	edTweakCol = fEd("TweakEditCol");
	txtTweakPathCol = fTxt("TweakPathCol");
	Btn("TweakColSave", btnTweakColSave);


	///  tweak tires  ----
	Btn("TweakTireSave", btnTweakTireSave);
	txtTweakTire = fTxt("TweakTireTxtSaved");
	Edt(edTweakTireSet, "TweakTireSet", editTweakTireSet);

	liTwkTiresUser = fLi("TweakTiresUser");  Lev(liTwkTiresUser, TwkTiresUser);
	liTwkTiresOrig = fLi("TweakTiresOrig");  Lev(liTwkTiresOrig, TwkTiresOrig);
	Btn("TweakTireLoad",  btnTweakTireLoad);  Btn("TweakTireDelete", btnTweakTireDelete);
	Btn("TweakTireReset", btnTweakTireReset);


	///  tweak surfaces  ----
	liTwkSurfaces = fLi("TweakSurfaces");  Lev(liTwkSurfaces, TwkSurfaces);
	Btn("TweakSurfPickWh", btnTwkSurfPick);
	sv= &svSuFrict;		sv->Init("SuFrict",     &f, 0.f, 1.5f,  1.0f, 2,4);	sv->DefaultF(0.65f);
	sv= &svSuFrictX;	sv->Init("SuFrictX",    &f, 0.f, 1.5f,  1.0f, 2,4);	sv->DefaultF(1.f);
	sv= &svSuFrictY;	sv->Init("SuFrictY",    &f, 0.f, 1.5f,  1.0f, 2,4);	sv->DefaultF(1.f);
	sv= &svSuBumpWave;	sv->Init("SuBumpWave",	&f, 0.f, 50.f,  1.2f, 1,4);	sv->DefaultF(20.f);
	sv= &svSuBumpAmp;	sv->Init("SuBumpAmp",	&f, 0.f, 0.4f,  1.0f, 2,4);	sv->DefaultF(0.15f);
	sv= &svSuBumpWave2;	sv->Init("SuBumpWave2",	&f, 0.f, 50.f,  1.2f, 1,4);	sv->DefaultF(20.f);
	sv= &svSuBumpAmp2;	sv->Init("SuBumpAmp2",	&f, 0.f, 0.4f,  1.0f, 2,4);	sv->DefaultF(0.15f);
	sv= &svSuRollDrag;	sv->Init("SuRollDrag",	&f, 0.f, 200.f,  2.f, 0,3);	sv->DefaultF(60.f);
	sv= &svSuRollRes;	sv->Init("SuRollRes",	&f, 0.f, 200.f,  2.f, 0,3);	sv->DefaultF(1.f);
	//Btn("TweakSurfSave"

	Cmb(cmb, "CmbSuTire", comboSurfTire);  cmbSurfTire = cmb;
	Cmb(cmb, "CmbSuType", comboSurfType);  cmbSurfType = cmb;
	cmb->removeAllItems();
	for (i=0; i < TRACKSURFACE::NumTypes; ++i)
		cmb->addItem(csTRKsurf[i]);
}
