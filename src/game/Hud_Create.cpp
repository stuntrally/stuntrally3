#include "pch.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
#include "RenderConst.h"
#include "CGame.h"
#include "CHud.h"
// #include "CGui.h"
#include "GuiCom.h"
#include "game.h"
#include "settings.h"
#include "Road.h"
#include "CarModel.h"
#include "GraphicsSystem.h"
// #include "SplitScreen.h"
#include "HudRenderable.h"

#include <OgreWindow.h>
#include <OgreCommon.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgreHlmsCommon.h>
#include <OgreSceneManager.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;
using namespace MyGUI;


///---------------------------------------------------------------------------------------------------------------
///  ⏱️ HUD create
///---------------------------------------------------------------------------------------------------------------

void CHud::Create()
{
	if (!pSet->hud_on)  return;
	// LogO("C--- Create Hud");
	//Destroy();  //
	if (app->carModels.size() == 0)  return;

	Ogre::Timer ti;

	SceneManager* scm = app->mSceneMgr;
	if (hud[0].txVel || hud[0].txTimes)
		LogO("C--! Create Hud: exists !");


	//  minimap from road img
	int plr = pSet->game.local_players;
	LogO("C--- Create Hud  plrs= "+toStr(plr));
	asp = 1.f;

	///  reload mini textures
	auto& resMgr = ResourceGroupManager::getSingleton();
	auto* texMgr = app->mGraphicsSystem->mRoot->getRenderSystem()->getTextureGpuManager();

	String path = app->bRplPlay ? 
		app->gcom->PathListTrkPrv(app->replay.header.track_user, app->replay.header.track) :
		app->gcom->PathListTrkPrv(pSet->game.track_user, pSet->game.track);
	const String sRoad = "road.png", sTer = "terrain.jpg", sGrp = "TrkMini";
	resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
	resMgr.unloadResourceGroup(sGrp);
	resMgr.initialiseResourceGroup(sGrp,false);

	Scene* sc = app->scn->sc;
	// {	try {  texMgr.unload(sRoad);  texMgr.load(sRoad, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
	// 	try {  texMgr.unload(sTer);   texMgr.load(sTer,  sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
	// }

	//if (terrain)
	int cnt = std::min(6/**/, (int)app->carModels.size() );  // others
	#ifdef DEBUG
	// assert(plr <= hud.size());
	//assert(cnt <= hud[0].moPos.size());
	#endif
	int y=1200; //off 0

	
	//  🔺 car pos tris (form all cars on all viewports)
	SceneNode* rt = scm->getRootSceneNode();
	// asp = 1.f;  //_temp
	hrPos = new HudRenderable("hud/CarPos", scm,
		// 0.f, true,true, 1.f,Vector2(1,1), RV_Hud,RQG_Hud3, plr * 6);
		// OT_TRIANGLE_LIST, true,true, RV_Hud,RQG_Hud3, plr * 3);  // ok 1
		OT_TRIANGLE_LIST, true,true, RV_Hud,RQG_Hud3, plr * cnt);  // each plr all cars (incl. ghosts)
	ndPos = rt->createChildSceneNode();
	ndPos->attachObject(hrPos);

	// const auto hudTimes = "hud.times", hudText = "hud.text";
	const auto hudText = "hud.text", hudTimes = hudText;  // same


	//  for each car
	//--------------------------------------------------------------------------------------------------------------
	for (int c=0; c < plr; ++c)
	{
		String s = toStr(c);
		Hud& h = hud[c];
		CarModel* cm = app->carModels[c];

		//  dims  ----
		const int wx = app->mWindow->getWidth(),
				  wy = app->mWindow->getHeight() +20;  //+? why
		const float asp = float(wx) / float(wy);

		const float wt = sc->tds[0].fTerWorldSize*0.5;  // 1st ter-
		minX = -wt;  minY = -wt;  maxX = wt;  maxY = wt;
		
		const float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
		const float size = std::max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;

		//  🌍 change minimap image  -----------
		String sMini = "hud/minimap";
	#if 0
		/*MaterialPtr mm = MaterialManager::getSingleton().getByName(sMat);
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)  tus->setTextureName( sRoad );
		tus = pass->getTextureUnitState(2);
		if (tus)  tus->setTextureName( sTer );*/
		UpdMiniTer();
	#endif
		
		h.hrMap = new HudRenderable(sMini, scm,
			OT_TRIANGLE_LIST, true, false, RV_Hud,RQG_Hud1, 1);
		h.ndMap = rt->createChildSceneNode();  h.ndMap->attachObject(h.hrMap);
		h.ndMap->setVisible(pSet->trackmap);


		//  ⏲️ gauges  -----------
		String st = toStr(pSet->gauges_type);
		int q = cm->hasRpm() ? 4 : 2;

		h.hrGauges = new HudRenderable("hud_"+st, scm,
			OT_TRIANGLE_LIST, true, false, RV_Hud,RQG_Hud1, q);
		h.ndGauges = rt->createChildSceneNode();  h.ndGauges->attachObject(h.hrGauges);
		h.ndGauges->setVisible(pSet->show_gauges);


		//  gear  text  ----
		float f = pSet->font_hud * getHudScale();

		h.parent = app->mGui->createWidget<Widget>("", 0,0,	wx, wy,
			Align::Left,"Back","main"+s);

		if (cm->vType == V_Car)
		{
			h.bckGear = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 79*f,102*f, Align::Left, "IGear"+s);
			h.bckGear->setImageTexture("back_gear.png");
			h.bckGear->setAlpha(0.7f);  //par

			h.txGear = h.parent->createWidget<TextBox>("TextBox",
				0,y, 65*f,90*f, Align::Left, "Gear"+s);  h.txGear->setVisible(false);
			h.txGear->setFontName("DigGear");
			h.txGear->setFontHeight(106 * f);  //par..
			h.txGear->setTextShadowColour(Colour::Black);  h.txGear->setTextShadow(true);
		}
		
		//  vel  km/h  ----
		h.bckVel = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 184*f,84*f, Align::Left, "IVel"+s);
		h.bckVel->setImageTexture("back_gear.png");
		h.bckVel->setAlpha(0.7f);
		
		// h.txVel = h.bckVel->createWidget<TextBox>("TextBox",
			// 10,5, 360,96, Align::Right, "Vel"+s);  h.txVel->setVisible(false);
		h.txVel = h.parent->createWidget<TextBox>("TextBox",
			0,y, 160*f,72*f, Align::Right, "Vel"+s);  h.txVel->setVisible(false);
		h.txVel->setFontName("DigGear");
		h.txVel->setFontHeight(72 * f);  //par
		//h.txVel->setInheritsAlpha(false);
		h.txVel->setTextShadowColour(Colour::Black);  h.txVel->setTextShadow(true);


		//  💨 boost
		if (cm->vType != V_Sphere)
		{
			h.txBFuel = h.parent->createWidget<TextBox>("TextBox",
				0,y, 120,60, Align::Right, "Fuel"+s);  h.txBFuel->setVisible(false);
			h.txBFuel->setTextAlign(Align::Right|Align::VCenter);
			h.txBFuel->setFontName("DigGearSeg");
			h.txBFuel->setFontHeight(44);
			h.txBFuel->setTextColour(Colour(0.6,0.8,1.0));  //h.txBFuel->setTextShadow(true);

			h.icoBFuel = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 40,40, Align::Left, "IFuel"+s);  //h.icoBFuel->setVisible(false);
			h.icoBFuel->setImageTexture("gui_icons.png");
			//if (pSet->game.boost_type == 3)
			h.icoBFuel->setImageCoord(IntCoord(512,0,128,128));

			//h.icoBInf = h.parent->createWidget<ImageBox>("ImageBox",
			//	0,y, 40,40, Align::Right, "IInf"+s);
			//h.icoBInf->setImageTexture("gui_icons.png");
			//h.icoBInf->setImageCoord(IntCoord(512,768,128,128));
		}

		//  🔨 damage %
		if (pSet->game.damage_type > 0)
		{
			h.imgDamage = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 130,46, Align::Left, "PDmg"+s);
			h.imgDamage->setImageTexture("menu2.png");
			
			h.txDamage = h.parent->createWidget<TextBox>("TextBox",
				0,y, 120,40, Align::Right, "Dmg"+s);  //h.txDamage->setVisible(false);
			h.txDamage->setTextAlign(Align::Right|Align::VCenter);
			h.txDamage->setFontName("hud.replay");  //h.txDamage->setFontHeight(44);
			h.txDamage->setTextColour(Colour(0.7,0.7,0.7));  h.txDamage->setTextShadow(true);

			h.icoDamage = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 40,40, Align::Left, "IDmg"+s);  //h.icoDamage->setVisible(false);
			h.icoDamage->setImageTexture("gui_icons.png");
			if (pSet->game.damage_type == 1)
				h.icoDamage->setImageCoord(IntCoord(512,256,128,128));
			else
				h.icoDamage->setImageCoord(IntCoord(640,384,128,128));
		}
		
		//  ⏪ rewind <<
	#if 0
		h.txRewind = h.parent->createWidget<TextBox>("TextBox",
			0,y, 240,80, Align::Right, "Rew"+s);  //h.txRewind->setVisible(false);
		h.txRewind->setFontName("DigGear");  h.txRewind->setFontHeight(64);
		h.txRewind->setTextColour(Colour(0.9,0.7,1.0));
		h.txRewind->setCaption("3.0");

		h.icoRewind = h.parent->createWidget<ImageBox>("ImageBox",
			200,180, 40,40, Align::Left, "IRew"+s);  //h.icoRewind->setVisible(false);
		h.icoRewind->setImageTexture("gui_icons.png");
		h.icoRewind->setImageCoord(IntCoord(512,384,128,128));
	#endif


		///  ⏱️ times text  ----------------------
		const float t = pSet->font_times * f;  // scale
		h.bckTimes = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 320/*420*/*t,220*t, Align::Left, "TimP"+s);  h.bckTimes->setVisible(false);
		h.bckTimes->setColour(Colour(0.1,0.1,0.1));
		h.bckTimes->setAlpha(0.4f);  //par
		h.bckTimes->setImageTexture("back_times.png");

		h.txTimTxt = h.parent->createWidget<TextBox>("TextBox",
			0,y, 170*t,260*t, Align::Left, "TimT"+s);
		h.txTimTxt->setFontName(hudTimes);
		h.txTimTxt->setFontHeight(32*t);
		h.txTimTxt->setTextShadowColour(Colour::Black);  h.txTimTxt->setTextShadow(true);
		h.txTimTxt->setInheritsAlpha(false);
		
		if (pSet->game.collect_num >= 0)
			h.txTimTxt->setCaption(
				"#80B0D0"+TR("#{Prize}") +
				"\n#C080FF"+TR("#{Collection}") +
				"\n#A0E0E0"+TR("#{TBTime}") +
				"\n#80E0E0"+TR("#{TBBest}") );
		else
			h.txTimTxt->setCaption(
				(pSet->game.hasLaps() ? String("#90D0C0")+TR("#{TBLap}") : "")+
				"\n#A0E0E0"+TR("#{TBTime}") +
				"\n#70D070"+TR("#{Track}") +
				"\n#C0C030"+TR("#{TBPosition}") +
				"\n#F0C050"+TR("#{TBPoints}") +
				"\n#C8A898"+TR("#{Progress}") );/*%*/

		h.txTimes = h.parent->createWidget<TextBox>("TextBox",
			0,y, 240*t,260*t, Align::Left, "Tim"+s);
		h.txTimes->setInheritsAlpha(false);
		h.txTimes->setFontName(hudTimes);
		h.txTimes->setFontHeight(32*t);
		h.txTimes->setTextShadow(true);

		h.txCollect = h.parent->createWidget<TextBox>("TextBox",
			0,y, 240*t,260*t, Align::Left, "Tim"+s);
		h.txCollect->setInheritsAlpha(false);
		h.txCollect->setFontName(hudTimes);
		h.txCollect->setFontHeight(26*t);
		h.txCollect->setTextShadow(true);


		///  🏁 lap results  ----------------------
		h.bckLap = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 420*t,240*t, Align::Left, "LapP"+s);  h.bckLap->setVisible(false);
		h.bckLap->setColour(Colour(0.4,0.4,0.4));
		h.bckLap->setAlpha(0.5f);
		h.bckLap->setImageTexture("back_times.png");

		h.txLapTxt = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120*t,300*t, Align::Left, "LapT"+s);
		h.txLapTxt->setFontName(hudTimes);
		h.txLapTxt->setFontHeight(32*t);
		h.txLapTxt->setTextShadow(true);
		h.txLapTxt->setInheritsAlpha(false);
		h.txLapTxt->setCaption(//String("\n")+
			//(hasLaps ? String("#D0F8F0")+TR("#{TBLap}") : "")+
			"\n#80C0F0"+TR("#{TBLast}") +
			"\n#80E0E0"+TR("#{TBBest}") +
			"\n#70D070"+TR("#{Track}") +
			"\n#C0C030"+TR("#{TBPosition}") +
			"\n#F0C050"+TR("#{TBPoints}") );/**/
		h.txLapTxt->setVisible(false);

		h.txLap = h.parent->createWidget<TextBox>("TextBox",
			0,y, 230*t,320*t, Align::Left, "Lap"+s);
		h.txLap->setInheritsAlpha(false);
		h.txLap->setFontName(hudTimes);
		h.txLap->setFontHeight(32*t);
		h.txLap->setTextShadow(true);
		h.txLap->setVisible(false);


		//  ❌ wrong chk warning  -----------
		h.bckWarn = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 500,60, Align::Left, "WarnB"+s);  h.bckWarn->setVisible(false);
		h.bckWarn->setImageTexture("back_times.png");

		h.txWarn = h.bckWarn->createWidget<TextBox>("TextBox",
			0,0, 500,60, Align::Center, "WarnT"+s);
		h.txWarn->setFontName(hudText);  h.txWarn->setTextShadow(true);
		h.txWarn->setTextColour(Colour(1,0.3,0));  h.txWarn->setTextAlign(Align::Center);
		h.txWarn->setCaption(TR("#{WrongChk}"));

		//  🥇 win place  -----------
		h.bckPlace = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 400,60, Align::Left, "PlcB"+s);  h.bckPlace->setVisible(false);
		h.bckPlace->setImageTexture("back_times.png");

		h.txPlace = h.bckPlace->createWidget<TextBox>("TextBox",
			0,0, 400,60, Align::Left, "PlcT"+s);
		h.txPlace->setFontName(hudText);  h.txPlace->setTextShadow(true);
		h.txPlace->setTextAlign(Align::Center);


		//  🏁 start countdown
		h.txCountdown = h.parent->createWidget<TextBox>("TextBox",
			0,y, 200,120, Align::Left, "CntT"+s);  h.txCountdown->setVisible(false);
		h.txCountdown->setFontName("DigGear");  h.txCountdown->setTextShadow(true);
		h.txCountdown->setTextColour(Colour(0.8,0.9,1));  h.txCountdown->setTextAlign(Align::Center);

		//  abs, tcs
		h.txAbs = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,60, Align::Left, "AbsT"+s);
		h.txAbs->setFontName(hudText);  h.txAbs->setTextShadow(true);
		h.txAbs->setCaption("ABS");  h.txAbs->setTextColour(Colour(1,1,0.6));

		h.txTcs = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,60, Align::Left, "TcsT"+s);
		h.txTcs->setFontName(hudText);  h.txTcs->setTextShadow(true);
		h.txTcs->setCaption("TCS");  h.txTcs->setTextColour(Colour(0.6,1,1));

		//  🎥 camera name
		h.txCam = h.parent->createWidget<TextBox>("TextBox",
			0,0, 300,30, Align::Left, "CamT"+s);
		h.txCam->setFontName(hudText);  h.txCam->setTextShadow(true);
		h.txCam->setTextColour(Colour(0.65,0.85,0.85));

		//  todo:  input bars, accel G's  --------------
		/*Img bar = tabitem->createWidget<ImageBox>("ImageBox",
			x2 + (twosided ? 0 : 64), y+4, twosided ? 128 : 64, 16, Align::Default,
			"bar_" + toStr(i) + "_" + sPlr);
		gcom->setOrigPos(bar, "OptionsWnd");
		bar->setUserData(*it);
		bar->setImageTexture(String("input_bar.png"));  bar->setImageCoord(IntCoord(0,0,128,16));
		*/
	}


	//  🎥 camera move info text
	txCamInfo = app->mGui->createWidget<TextBox>("TextBox",
		0,y, 1200,100, Align::Left, "Back", "CamIT");  txCamInfo->setVisible(false);
	txCamInfo->setFontName(hudText);  txCamInfo->setTextShadow(true);
	txCamInfo->setTextColour(Colour(0.8,0.9,0.9));


	//  💬 chat msg  -----------
	bckMsg = app->mGui->createWidget<ImageBox>("ImageBox",
		0,y, 600,80, Align::Left, "Back", "MsgB");  bckMsg->setVisible(false);
	bckMsg->setAlpha(0.9f);
	bckMsg->setColour(Colour(0.5,0.5,0.5));
	bckMsg->setImageTexture("back_times.png");

	txMsg = bckMsg->createWidget<TextBox>("TextBox",
		16,10, 800,80, Align::Left, "PlcT");
	txMsg->setInheritsAlpha(false);
	txMsg->setFontName(hudText);  txMsg->setTextShadow(true);
	txMsg->setTextColour(Colour(0.95,0.95,1.0));


	///  tex
	resMgr.removeResourceLocation(path, sGrp);
	
	//-  cars need update
	for (int i=0; i < app->carModels.size(); ++i)
	{	CarModel* cm = app->carModels[i];
		cm->updTimes = true;
		cm->updLap = true;  cm->fLapAlpha = 1.f;
	}

	///  ⚫ tire vis circles  + + + +
	asp = float(app->mWindow->getWidth()) / float(app->mWindow->getHeight());

	if (pSet->car_tirevis)
	{	SceneNode* rt = scm->getRootSceneNode();
		for (int i=0; i < MAX_TireVis; ++i)
		{
			hrTireVis[i] = new HudRenderable("hud/line", scm,
				OT_LINE_LIST, false, true, RV_Hud,RQG_Hud1,
				2 + 3*3 + 5*5 + 33);
 
			ndTireVis[i] = rt->createChildSceneNode();  ndTireVis[i]->attachObject(hrTireVis[i]);
			ndTireVis[i]->setPosition((i%2 ? 1.f :-1.f) * 0.13f - 0.7f,
									  (i/2 ?-1.f : 1.f) * 0.22f - 0.6f, 0.f);
			const Real s = 0.06f;  // par
			ndTireVis[i]->setScale(s, s*asp, 1.f);
			// ndTireVis[i]->setVisible(pSet->car_tirevis);  //-
			ndTireVis[i]->_getFullTransformUpdated();  //?
	}	}


	//  debug texts car  == == ==
	auto DbgTxt = [](Txt txt)
	{
		txt->setFontName("hud.fps");  txt->setTextShadow(true);
		txt->setTextColour(Colour(0.95,0.95,1.0));  txt->setTextShadowColour(Colour::Black);
		//txt->setVisible(false);  //?
	};
	for (int i=0; i < 4; ++i)
	{
		auto txt = app->mGui->createWidget<TextBox>("TextBox",
			i*270,80, 600,1100, Align::Left, "Back");
		DbgTxt(txt);  txDbgCar[i] = txt;
	}

	//  debug surfaces  ==
	auto txt = app->mGui->createWidget<TextBox>("TextBox",
		20,280, 800,300, Align::Left, "Back");
	DbgTxt(txt);  txDbgSurf = txt;

	//  profiler times  ::
	txt = app->mGui->createWidget<TextBox>("TextBox",
		280,80, 400,300, Align::Left, "Back");
	DbgTxt(txt);  txDbgProfTim = txt;
	
	//  profiler bullet  ;;;
	txt = app->mGui->createWidget<TextBox>("TextBox",
		320,80, 1400,1000, Align::Left, "Back");
	DbgTxt(txt);  txDbgProfBlt = txt;


	// Show();  //_
	app->bSizeHUD = true;
	// Size();
	
	LogO(":::* Time Create Hud: "+fToStr(ti.getMilliseconds(),0,3)+" ms");

	app->CreateGraphs();  //
}



//  💥 HUD destroy
//---------------------------------------------------------------------------------------------------------------
CHud::Hud::Hud()
{
	vMiniPos.resize(6);
}

void CHud::Destroy()
{
	if (!pSet->hud_on)  return;
	LogO("D--- Destroy Hud");
	SceneManager* scm = app->mSceneMgr;

	#define Dest2(nd)  {  if (nd) {  scm->destroySceneNode(nd);  nd=0;  }  }
	#define Del(hr)  {  delete hr;  hr =0;  }
	#define Dest(w)  if (w) {  app->mGui->destroyWidget(w);  w = 0;  }

	int i,c;  // per player
	for (c=0; c < hud.size(); ++c)
	{	Hud& h = hud[c];

		//  🌍⏲️ map, gauges
		Dest2(h.ndMap)  Del(h.hrMap)
		Dest2(h.ndGauges)  Del(h.hrGauges)
		//  vel, rpm,
		Dest(h.txGear) Dest(h.bckGear)
		Dest(h.txVel)  Dest(h.bckVel)
		Dest(h.txAbs)  Dest(h.txTcs)  Dest(h.txCam)
		//  💨 boost
		Dest(h.txBFuel)  Dest(h.txDamage)  Dest(h.txRewind)  Dest(h.imgDamage)
		Dest(h.icoBFuel)  Dest(h.icoBInf)  Dest(h.icoDamage)  Dest(h.icoRewind)

		//  🏁 times, lap
		Dest(h.txTimTxt)  Dest(h.txTimes)  Dest(h.bckTimes)
		Dest(h.txLapTxt)  Dest(h.txLap)  Dest(h.bckLap)
		h.sTimes = "";  h.sLap = "";
		
		//  ❌🥇 warn, msg plr
		Dest(h.txWarn)  Dest(h.bckWarn)
		Dest(h.txPlace)  Dest(h.bckPlace)
		Dest(h.txCountdown)
		Dest(h.parent)
	}
	//  🔧 debug
	for (int i=0; i < 4; ++i)
		Dest(txDbgCar[i]);
	Dest(txDbgSurf);
	Dest(txDbgProfTim);  Dest(txDbgProfBlt);

	//  global
	Dest2(ndPos)  Del(hrPos)
	Dest(txMsg)  Dest(bckMsg)
	Dest(txCamInfo)
	
	for (i=0; i < MAX_TireVis; ++i)
	{	Dest2(ndTireVis[i])  Del(hrTireVis[i])  }

	//  arrows  ----
	for (int i=0; i < MAX_Players; ++i)
	{
		arrChk[i].Destroy(scm);
		for (int a = 0; a < MAX_ArrCol; ++a)
			arrCol[i][a].Destroy(scm);
	}
	//  win cups
	for (int i=0; i < 3; ++i)
		cup[i].Destroy(scm);
}
