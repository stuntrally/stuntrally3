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

	Real yo = (1.f - ym)*wy, xo = 4.f/3.f * yo;  // opt wnd size in pix
	ym = (wy - yo)*0.5f;  xm = (wx - xo)*0.5f;

	int xc = (wx - wy)*0.5f;
	imgPrv[2]->setCoord(xc, 0, wy, wy);  // fullscr prv
	imgTer[2]->setCoord(xc, 0, wy, wy);
	imgMini[2]->setCoord(xc, 0, wy, wy);  initMiniPos(2);

#ifndef SR_EDITOR  // game
	app->mWndGame->setCoord(xm, ym, xo, yo);
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

///  🆕 init Screen Resolutions
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::InitGuiScreenRes()
{
return;  /// todo: restore..
	Ck* ck;
	// ck= &ckVidFullscr;  ck->Init("FullScreen", &pSet->fullscreen);  CevC(VidFullscr);
	// ck= &ckVidVSync;	ck->Init("VSync",      &pSet->vsync);		CevC(VidVSync);

	//  video resolutions combobox
	resList = fCmb("ResList");
	if (resList)
	{
		//  get resolutions
		String modeSel = "";
		std::vector<ScrRes> vRes;

		const StringVector& videoModes = app->mRoot->getRenderSystem()->getConfigOptions().find("Video Mode")->second.possibleValues;
		for (int i=0; i < videoModes.size(); ++i)
		{
			String mode = videoModes[i];
			StringUtil::trim(mode);
			if (StringUtil::match(mode, "*16-bit*"))  continue;  //skip ?DX

			StringVector wh = StringUtil::split(mode, " x");  // only resolution
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
	}
	// Btn btn;
	// BtnC("ResChange",btnResChng);
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

#if 0  // todo: ..
void CGuiCom::chkVidFullscr(Ck*)
{
	*SDL_SetWindowFullscreen(app->mSDLWindow, pSet->fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	#ifndef SR_EDITOR
	app->bRecreateHUD = true;
	#endif
}

void CGuiCom::chkVidVSync(Ck*)
{		
	app->mWindow->setVSyncEnabled(pSet->vsync);
}

void CGuiCom::cmbRenderSystem(CMB)
{
	pSet->rendersystem = wp->getItemNameAt(val);
}

void CGuiCom::cmbAntiAliasing(CMB)
{
	int v = s2i(wp->getItemNameAt(val));
	// if (app->gui->bGI)
		pSet->fsaa = v;
}

///  🔁🖥️ Change Resolution
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::btnResChng(WP)
{
	if (!resList)  return;
	if (resList->getIndexSelected() == MyGUI::ITEM_NONE)  return;
	String mode = resList->getItemNameAt(resList->getIndexSelected()).substr(7);

	StringVector wh = StringUtil::split(mode," x");  // only resolution
	int w = s2i(wh[0]), h = s2i(wh[1]);
	// pSet->windowx = w;
	// pSet->windowy = h;

	/*Uint32 flags = SDL_GetWindowFlags(app->mSDLWindow);
	if (flags & SDL_WINDOW_MAXIMIZED)  // Can't change size of a maximized window
		SDL_RestoreWindow(app->mSDLWindow);

	if (pSet->fullscreen)
	{
		SDL_DisplayMode mode;
		SDL_GetWindowDisplayMode(app->mSDLWindow, &mode);
		mode.w = pSet->windowx;
		mode.h = pSet->windowy;
		SDL_SetWindowDisplayMode(app->mSDLWindow, &mode);
		SDL_SetWindowFullscreen(app->mSDLWindow, 0);
		SDL_SetWindowFullscreen(app->mSDLWindow, SDL_WINDOW_FULLSCREEN);
	}else
		SDL_SetWindowSize(app->mSDLWindow, pSet->windowx, pSet->windowy);
	*/
}
#endif
