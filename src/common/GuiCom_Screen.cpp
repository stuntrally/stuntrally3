#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "CGui.h"
#include "GuiCom.h"
#include "settings.h"
#include "App.h"

#include <SDL_video.h>
#include <OgreRoot.h>
#include <OgreWindow.h>
#include <OgreString.h>
#include "GraphicsSystem.h"

#include <MyGUI_Widget.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ListBox.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_Gui.h>
#include <MyGUI_Button.h>
#include <MyGUI_Window.h>
#include <MyGUI_ImageBox.h>
using namespace MyGUI;
using namespace Ogre;



//  🗜️🪟 resize all Options wnds
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::ResizeOptWnd()
{
	if (!app->mWndOpts)  return;

	const int wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();

	Real xm = 0.f, ym = GetGuiMargin(wy);  // margin
	ym += pSet->font_wnds;

	Real yo = (1.f - ym)*wy, xo = 4.f/3.f * yo;  // opt wnd size in pix
	ym = (wy - yo)*0.5f;  xm = (wx - xo)*0.5f;

	int xc = (wx - wy)*0.5f;
	imgPrv[2]->setCoord(xc, 0, wy, wy);  // fullscr prv
	imgTer[2]->setCoord(xc, 0, wy, wy);
	imgMini[2]->setCoord(xc, 0, wy, wy);  initMiniPos(2);

#ifndef SR_EDITOR  // game
	app->mWndGame->setCoord(xm, ym, xo, yo);
	app->mWndGameInfo->setCoord(xm, 0, xo, 0.15f*wy);  // info top
	app->gui->edGameInfo->setCoord(8, 8, xo -16, 0.15f*wy -16);

	app->mWndReplays->setCoord(xm, ym, xo, yo);
	app->mWndWelcome->setCoord(xm, ym, xo, yo);
	app->mWndHowTo->setCoord(xm, ym, xo, yo);
	//app->mWndTweak->setCoord(0, 86, xo/3, yo-ym);
#else  // editor
	app->mWndTrack->setCoord(xm, ym, xo, yo);
	app->mWndEdit->setCoord(xm, ym, xo, yo);
#endif  // both, common
	app->mWndTrkFilt->setCoord(xm, ym+50, xo, yo);
	app->mWndHelp->setCoord(xm, ym, xo, yo);
	app->mWndOpts->setCoord(xm, ym, xo, yo);
	app->mWndMaterials->setCoord(xm, ym, xo, yo);


	if (bnQuit)  //  reposition Quit btn
		bnQuit->setCoord(wx - 0.12*wx, 0, 0.12*wx + 2, /*0.05*wy*/36);

	//  ed mode, status  ._
	#ifdef SR_EDITOR
	IntCoord ic(0.f, wy - 0.06*wy, 0.06*wy, 0.06*wy);
	if (app->gui->imgCam)
	{	app->gui->imgCam->setCoord(ic);  app->gui->imgEdit->setCoord(ic);  app->gui->imgGui->setCoord(ic);  }
	if (app->gui->panStatus)
		app->gui->panStatus->setCoord(0.08*wy, wy - 0.035*wy, 0.15*wy, 0.035*wy);
	#endif

	updTrkListDim();
}

//  gui margin
float CGuiCom::GetGuiMargin(int wy)
{
	const int yN = 7;
	const Real yw[yN] = {400.f, 600.f, 720.f, 768.f, 960.f, 1024.f, 1050.f};
	//  how much empty space for screen y size, 0 = full window
	const Real yf[yN] = {0.0f,  0.0f,  0.05f, 0.1f,  0.1f,  0.f,  0.1f};
	float ym = 0.f;
	for (int i=0; i < yN; ++i)
		if (wy >= yw[i]-10)  ym = yf[i];
	return ym;
}


//  🖥️ Screen
//-----------------------------------------------------------------------------------------------------------

//  Resolution sort
struct ScrRes
{	int w,h;  String mode;  };

bool ResSort(const ScrRes& r1, const ScrRes& r2)
{
	return (r1.w <= r2.w) && (r1.h <= r2.h);
}

//  Aspect color 🌈
const static int iClrA = 4;
const static String clrAsp[iClrA] = {"#E0E0E0","#A0FFFF","#C0FFC0","#F0F060"};

int clrAspect(float asp, String* sa)
{
	int ca = 0;  *sa = "";
	     if (fabs(asp - 16.f/10.f) < 0.05f) {  ca = 1;  *sa = "  16:10";  }
	else if (fabs(asp - 16.f/ 9.f) < 0.05f) {  ca = 2;  *sa = "  16:9";  }
	else if (fabs(asp -  4.f/ 3.f) < 0.05f) {  ca = 3;  *sa = "  4:3";  }
	return ca;
}


///  🆕🖥️ Init Screen  Resolutions
//-----------------------------------------------------------------------------------------------------------
static bool fullscreen =1, vsync =1;
void CGuiCom::InitGuiScreenRes()
{
	//  get cur cfg val
	auto& cfg = app->mRoot->getRenderSystem()->getConfigOptions();

	//  chk
	fullscreen = cfg.find("Full Screen")->second.currentValue == "Yes",
	vsync = cfg.find("VSync")->second.currentValue == "Yes";

	Ck* ck;  static bool b;
	ck= &ckVidFullscr;  ck->Init("FullScreen", &fullscreen);  CevC(VidFullscr);
	ck= &ckVidVSync;	ck->Init("VSync",      &vsync);		CevC(VidVSync);

	//  AA  ----
	Cmb cmb;
	CmbC(cmb, "CmbAntiAliasing", cmbAntiAliasing);
	auto fsaa = cfg.find("FSAA")->second.currentValue;
	auto& vaa = cfg.find("FSAA")->second.possibleValues;
	int i = 0;
	for (auto& aa : vaa)
	{
		cmb->addItem(aa);
		if (fsaa == aa)
			cmb->setIndexSelected(i);
		++i;
	}

	CmbC(cmb, "CmbRendSys", cmbRenderSystem);
	auto& vrs = app->mRoot->getAvailableRenderers();
	for (auto& rs : vrs)
	{
		auto s = rs->getName();
		s = StringUtil::replaceAll(s, " Rendering Subsystem", "");
		cmb->addItem(s);
	}
	cmb->setIndexSelected(0);


	//  🖥️ video resolutions combobox
	//-------------------------------------------------
	CmbC(resList, "ResList", cmbScreenRes);
	
	//  get resolutions
	String modeSel = "";
	std::vector<ScrRes> vRes;

	const auto& cfgvm = cfg.find("Video Mode")->second;
	const auto& videoModes = cfgvm.possibleValues;
	for (const auto& vmd : videoModes)
	{
		String mode = vmd;
		StringUtil::trim(mode);
		if (StringUtil::match(mode, "*16-bit*"))  continue;  //skip-

		StringVector wh = StringUtil::split(mode, " x");  // get w x h
		int w = s2i(wh[0]), h = s2i(wh[1]);
		
		if (w >= 800 && h >= 600)  // min res
		{
			//  color aspect
			float asp = float(w)/h;
			String sa = "";
			int ca = clrAspect(asp, &sa);
			
			mode = clrAsp[ca] + toStr(w) + " x " + toStr(h) + sa;

			ScrRes res;  res.w = w;  res.h = h;  res.mode = mode;
			vRes.push_back(res);

			int ww = w - app->mWindow->getWidth(), hh = h - app->mWindow->getHeight();
			if (abs(ww) < 30 && abs(hh) < 50)  // window difference
				modeSel = mode;
		}
	}

	//  sort and add
	std::sort(vRes.begin(), vRes.end(), ResSort);
	int rr = (int)vRes.size()-1;
	for (int r=0; r <= rr; ++r)
		resList->addItem(vRes[rr-r].mode);  // bigger to lower

	//  sel current mode
	if (modeSel != "")
		resList->setIndexSelected(resList->findItemIndexWith(modeSel));

	Btn btn;
	BtnC("ResChange", btnResChng);

	ReInitGuiRSrest();
}
	
//  🆕🔁 recreate all other opts gui
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::ReInitGuiRSrest()
{
	//  destroy old
	for (auto& w : vwGuiRS)
		mGui->destroyWidget(w);
	vwGuiRS.clear();

	//  all rs cfg in ogre-next src:  optVideoMode.name = "Video Mode";
	auto skip = {"Video Mode", "Full Screen", "VSync", "FSAA", "sRGB Gamma Conversion"};
	int y = 2;
	Scv sv = fScv("RSrest");

	auto& cfg = app->mRoot->getRenderSystem()->getConfigOptions();
	for (auto& c : cfg)
	{
		//  skip, have own gui
		bool sk =0;
		for (auto& s : skip)
			if (c.first == s)
			{  sk =1;  continue;  }
		if (sk)  continue;
		
		//  text
		Txt txt = sv->createWidget<TextBox>("TextBox",
			0,y+2, 160, 22,  Align::Center, "RSt"+toStr(y));
		txt->setCaption(c.first);
		txt->setTextColour(Colour(0.5,0.7,0.9));
		setOrigPos(txt, "OptionsWnd");
		vwGuiRS.push_back(txt);

		//  combo
		Cmb cb = sv->createWidget<ComboBox>("ComboBox",
			170, y, 220, 22,  Align::Center, "RSc"+toStr(y));
		cb->setTextColour(Colour(0.6,0.8,1.0));
		cb->setColour(Colour(0.6,0.8,1.0));
		cb->setEditReadOnly(true);
		setOrigPos(cb, "OptionsWnd");
		vwGuiRS.push_back(cb);

		auto cur = c.second.currentValue;
		int i = 0;
		for (auto& cc : c.second.possibleValues)
		{
			cb->addItem(cc);
			if (cur == cc)
				cb->setIndexSelected(i);
			++i;
		}
		y += 24;
	}
	// if (app->gui->bGI)  // resize
		doSizeGUI(sv->getEnumerator());
}


//  🔁 events
//-----------------------------------------------------------------------------------------------------------
//  util
void CGuiCom::setRScfg(const Ogre::String& cfg, const Ogre::String& val)
{
	LogO(">>== RenderSystem change: "+cfg+" to: "+val);
	app->mRoot->getRenderSystem()->setConfigOption(cfg, val);
	app->mRoot->saveConfig();
}

void CGuiCom::chkVidFullscr(Ck* ck)
{
	setRScfg("Full Screen", fullscreen ? "Yes" : "No");
	/*
	auto* w = app->mGraphicsSystem->getRenderWindow();
	app->mGraphicsSystem->getRenderWindow()->requestFullscreenSwitch(
		fullscreen, fullscreen, 0, w->getWidth(), w->getHeight(),  // meh-
		0, 0);
		// w->getFrequencyNumerator(), w->getFrequencyDenominator());
	// *SDL_SetWindowFullscreen(app->mSDLWindow, pSet->fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	#ifndef SR_EDITOR
	// app->bRecreateHUD = true;
	#endif
	*/
}

void CGuiCom::chkVidVSync(Ck* ck)
{
	setRScfg("VSync", vsync ? "Yes" : "No");
	app->mGraphicsSystem->getRenderWindow()->setVSync(vsync, vsync ? 1 : 0);  // ok
}

void CGuiCom::cmbAntiAliasing(Cmb c, size_t id)
{
	auto fsaa = c->getItemNameAt(id);
	setRScfg("FSAA", fsaa);
	app->mGraphicsSystem->getRenderWindow()->setFsaa(fsaa);  // can be ignored by HW
}

void CGuiCom::cmbRenderSystem(Cmb c, size_t id)
{
	// String val = c->getItemNameAt(id).substr(7);
	// app->mRoot->setRenderSystem(app->mRoot->getAvailableRenderers()[id]);  // crash..
	// app->mRoot->saveConfig();
	// ReInitGuiRSrest();  // todo: ..
}


///  🔁🖥️ Change Resolution
//-----------------------------------------------------------------------------------------------------------
String CGuiCom::getScreenRes(int* x, int* y)
{
	Cmb c = resList;  if (!c)  return "";
	size_t id = c->getIndexSelected();
	if (id == MyGUI::ITEM_NONE)  return "";

	String mode = c->getItemNameAt(id).substr(7);
	StringVector wh = StringUtil::split(mode, " x");  // get w x h
	String w = wh[0], h = wh[1], sMode = w+" x "+h;

	if (x)  *x = s2i(w);  if (y)  *y = s2i(h);
	return sMode;
}

void CGuiCom::cmbScreenRes(Cmb c, size_t id)
{
	String sMode = getScreenRes();
	setRScfg("Video Mode", sMode);
}

void CGuiCom::btnResChng(WP)  // almost ok
{
	int x,y;
	String sMode = getScreenRes(&x, &y);
	setRScfg("Video Mode", sMode);

	auto* wnd = app->mGraphicsSystem->mSdlWindow;
	if (fullscreen)
	{
		SDL_DisplayMode mode;
		SDL_GetWindowDisplayMode(wnd, &mode);  mode.w = x;  mode.h = y;
		SDL_SetWindowDisplayMode(wnd, &mode);
		// SDL_SetWindowFullscreen(wnd, 0);
		SDL_SetWindowFullscreen(wnd, SDL_WINDOW_FULLSCREEN);
	}else
	{	// SDL_SetWindowFullscreen(wnd, 0);
		/**/Uint32 flags = SDL_GetWindowFlags(wnd);
		if (flags & SDL_WINDOW_MAXIMIZED)  // Can't change size of a maximized window
			SDL_RestoreWindow(wnd);/**/
		SDL_SetWindowSize(wnd, x,y);
	}
	app->mGraphicsSystem->getRenderWindow()->requestResolution(x, y);  // after?
/*	auto* wd = app->mGraphicsSystem->getRenderWindow();
	app->mGraphicsSystem->getRenderWindow()->requestFullscreenSwitch(
		fullscreen, fullscreen, 0, x, y,
		0, 0);	*/
	
	// todo: resize Gui, Hud ..!
}
