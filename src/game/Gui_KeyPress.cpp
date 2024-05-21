#include "pch.h"
#include "BaseApp.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "settings.h"
#include "paths.h"
#include "game.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "GuiCom.h"
#include "GraphView.h"
#include "Slider.h"
#include "FollowCamera.h"
#include "CarModel.h"
#include "SoundMgr.h"
#include "SoundBaseMgr.h"
#include <MyGUI.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;

#define key(a)  SDL_SCANCODE_##a


//  ⌨️ Key Released
//-----------------------------------------------------------------------------------------------------------------------------
void App::keyReleased( const SDL_KeyboardEvent &arg )
{
	//  input
	mInputCtrl->keyReleased(arg);
	for (int i=0; i < MAX_Players; ++i)  mInputCtrlPlayer[i]->keyReleased(arg);

	if (bAssignKey)  return;

	switch (arg.keysym.scancode)
	{
	case key(LSHIFT):  case key(RSHIFT):  shift = false;  break;  // mods
	case key(LCTRL):   case key(RCTRL):   ctrl = false;   break;
	case key(LALT):    case key(RALT):    alt = false;    break;

	case key(DOWN):  keyDown = false;   break;
	case key(UP):    keyUp   = false;   break;
	case key(PAGEDOWN):  keyPgDown = false;   break;
	case key(PAGEUP):    keyPgUp   = false;   break;
	case key(KP_MULTIPLY):  keyMul = false;  break;
	case key(KP_DIVIDE):    keyDiv = false;  break;

	/*case key(F4):
	if (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
	{
		// Hot reload of Terra shaders
		Root *root = mGraphicsSystem->getRoot();
		HlmsManager *hlmsManager = root->getHlmsManager();

		Hlms *hlms = hlmsManager->getHlms( HLMS_USER3 );
		GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}   break;*/
	default:  break;
	}

	if (mGui && (isFocGui || isTweak()))
	{
		MyGUI::KeyCode kc = SDL2toGUIKey(arg.keysym.sym);
		MyGUI::InputManager::getInstance().injectKeyRelease(kc);
	}
}

#if 0
		// Hot reload of shaders.
		Ogre::Root *root = mGraphicsSystem->getRoot();
		Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

		Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_PBS );  // F1
		Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_UNLIT );  // F2
		Ogre::Hlms *hlms = hlmsManager->getComputeHlms();  // F3
		Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );

		// Force device reelection
		Ogre::Root *root = mGraphicsSystem->getRoot();
		root->getRenderSystem()->validateDevice( true );
#endif

//-----------------------------------------------------------------------------------------------------------
//  ⌨️ Key pressed
//-----------------------------------------------------------------------------------------------------------

void App::keyPressed(const SDL_KeyboardEvent &arg)
{	
	if (!mInputCtrl->keyPressed(arg))
		return;
	for (int i=0; i < MAX_Players; ++i)
	{
		if (!mInputCtrlPlayer[i]->keyPressed(arg))
			return;
	}

	SDL_Scancode skey = arg.keysym.scancode;
	bool tweak = isTweak();

	switch (skey)
	{
	case key(LSHIFT): case key(RSHIFT):  shift = true;  break;  // mods
	case key(LCTRL):  case key(RCTRL):   ctrl = true;   break;
	case key(LALT):   case key(RALT):    alt = true;    break;

	case key(DOWN):  keyDown  = true;   break;  // for gui lists
	case key(UP):    keyUp    = true;   break;
	case key(PAGEDOWN):  keyPgDown = true;  break;
	case key(PAGEUP):    keyPgUp   = true;  break;
	case key(KP_MULTIPLY):  keyMul = true;  break;  // tire edit
	case key(KP_DIVIDE):    keyDiv = true;  break;
	default:  break;
	}


	//  level 1  Main menu
	//-------------------------------------------------
	if (pSet->iMenu == MN1_Main && isFocGui)
	{
		switch (skey)
		{
		case key(UP):  case key(KP_8):
			pSet->yMain = (pSet->yMain - 1 + ciMainBtns) % ciMainBtns;
			gui->toggleGui(false);  return;

		case key(DOWN):  case key(KP_2):
			pSet->yMain = (pSet->yMain + 1) % ciMainBtns;
			gui->toggleGui(false);  return;

		case key(KP_ENTER):
		case key(RETURN):
			switch (pSet->yMain)
			{
			case Menu_Setup:    pSet->iMenu = MN1_Setup;  break;
			case Menu_Replays:  pSet->iMenu = MN_Replays;  break;
			case Menu_Help:     pSet->iMenu = MN_Help;  break;
			case Menu_Options:  pSet->iMenu = MN_Options;  break;
			}
			gui->toggleGui(false);  return;
		default:  break;
	}	}

	//  level 2  Setup menu
	//-------------------------------------------------
	if (pSet->iMenu == MN1_Setup && isFocGui)
	{
		switch (skey)
		{
		case key(UP):  case key(KP_8):
			pSet->ySetup = (pSet->ySetup - 1 + ciSetupBtns) % ciSetupBtns;
			gui->toggleGui(false);  return;

		case key(DOWN):  case key(KP_2):
			pSet->ySetup = (pSet->ySetup + 1) % ciSetupBtns;
			gui->toggleGui(false);  return;

		case key(BACKSPACE):    pSet->iMenu = MN1_Main;
			gui->toggleGui(false);  return;

		case key(KP_ENTER):
		case key(RETURN):
			switch (pSet->ySetup)
			{
			case Setup_Games:     pSet->iMenu = MN1_Games;  break;

			case Setup_HowToPlay: pSet->iMenu = MN_HowTo;  break;
			case Setup_Back:      pSet->iMenu = MN1_Main;  break;
			}
			gui->toggleGui(false);  return;
		default:  break;
	}	}

	//  level 3  Games menu
	//-------------------------------------------------
	if (pSet->iMenu == MN1_Games && isFocGui)
	{
		switch (skey)
		{
		case key(UP):  case key(KP_8):
			pSet->yGames = (pSet->yGames - 1 + ciGamesBtns) % ciGamesBtns;
			gui->toggleGui(false);  return;

		case key(DOWN):  case key(KP_2):
			pSet->yGames = (pSet->yGames + 1) % ciGamesBtns;
			gui->toggleGui(false);  return;

		case key(BACKSPACE):    pSet->iMenu = MN1_Setup;
			gui->toggleGui(false);  return;

		case key(KP_ENTER):
		case key(RETURN):
			switch (pSet->yGames)
			{
			case Games_Single:       gui->GuiShortcut(MN_Single, TAB_Track);  return;
			case Games_SplitScreen:  gui->GuiShortcut(MN_Single, TAB_Split);  return;  // 👥
			case Games_Multiplayer:  gui->GuiShortcut(MN_Single, TAB_Multi);  return;  // 📡

			case Games_Tutorial:   gui->GuiShortcut(MN_Tutorial, TAB_Champs);  return;
			case Games_Champ:      gui->GuiShortcut(MN_Champ,    TAB_Champs);  return;
			case Games_Challenge:  gui->GuiShortcut(MN_Chall,    TAB_Champs);  return;
			
			case Games_Collection: gui->GuiShortcut(MN_Collect, TAB_Champs);  break;
			case Games_Career:     gui->GuiShortcut(MN_Career,  TAB_Champs);  break;
			case Games_Back:       pSet->iMenu = MN1_Setup;  break;
			}
			gui->toggleGui(false);  return;
		default:  break;
	}	}


	//-------------------------------------------------
	//  Esc
	if (skey == key(ESCAPE))
	{
		if (pSet->escquit && !bAssignKey)
			Quit();
		else
			if (mWndHowTo->getVisible())
			{	pSet->iMenu = MN1_Setup;  gui->toggleGui(false);	}
			else if (mWndChampStage->getVisible())  ///  close champ wnds
				gui->btnChampStageStart(0);
			else if (mWndChallStage->getVisible())  ///  chall
				gui->btnChallStageStart(0);
			else
				gui->toggleGui(true);	// gui on/off
		return;
	}

	//  ctrl-F1  Welcome wnd
	if (skey == key(F1) && ctrl)
	{
		mWndWelcome->setVisible(true);
		return;
	}


	//************************************************************************************************************
	//  shortcut keys for gui access (alt-Q,C,S,G,V,.. )
	if (alt && !shift)
	{	EMenu no = ctrl ? MN_Single : MN_NoCng;  // ctrl sets game mode
		auto gm = ctrl ? Games_Single : -1;
		switch (skey)     //  alt- Shortcuts  🎛️
		{
			case key(Q):	gui->GuiShortcut(no, TAB_Track, -1, gm);  return;  // Q Track  🏞️
			case key(C):	gui->GuiShortcut(no, TAB_Car,   -1, gm);  return;  // C Car  🚗
			case key(W):	gui->GuiShortcut(no, TAB_Setup, -1, gm);  return;  // W Setup  🔩

			case key(G):	gui->GuiShortcut(MN_Single,   TAB_Split, -1, Games_SplitScreen);  return;  // G SplitScreen  👥
			case key(U):	gui->GuiShortcut(MN_Single,   TAB_Multi, -1, Games_Multiplayer);  return;  // U Multiplayer  📡

			case key(J):	gui->GuiShortcut(MN_Tutorial, TAB_Champs, -1, Games_Tutorial);	return;  // J Tutorials
			case key(H):	gui->GuiShortcut(MN_Champ,    TAB_Champs, -1, Games_Champ);		return;  // H Champs  🏆
			case key(L):	gui->GuiShortcut(MN_Chall,    TAB_Champs, -1, Games_Challenge);	return;  // L Challs  🥇

			case key(R):	gui->GuiShortcut(MN_Replays,  1);	return;		     // R Replays  📽️

			case key(S):	gui->GuiShortcut(MN_Options, TABo_Screen);	return;  // S Screen  🖥️
			case key(I):	gui->GuiShortcut(MN_Options, TABo_Input);	return;  // I Input  🕹️
			case key(V):	gui->GuiShortcut(MN_Options, TABo_View);	return;  // V View  ⏱️

			case key(A):	gui->GuiShortcut(MN_Options, TABo_Graphics);	return;  // A Graphics  📊
			//case key(E):	gui->GuiShortcut(MN_Options, TABo_Graphics,2);	return;  // E -Effects

			case key(T):	gui->GuiShortcut(MN_Options, TABo_Settings);	return;  // T Settings  ⚙️
			case key(O):	gui->GuiShortcut(MN_Options, TABo_Sound);	return;  // O Sound  🔊

			case key(F):	gui->GuiShortcut(MN_Materials, 1);  return;  // F Material editor  🔮
			case key(K):	gui->GuiShortcut(MN_Options, TABo_Tweak);  return;  // K -Tweak  🔧
			default:  break;
	}	}
	//............................................................................................................
	//>--  dev shortcuts, alt-shift - start Test Tracks  🏞️
	if (pSet->dev_keys && alt && shift && !mClient)
	{
		string t;  char c = 0;
		if (skey >= SDL_SCANCODE_1 && skey <= SDL_SCANCODE_0)
		{	c = skey - SDL_SCANCODE_1;  c+='1';  t = pSet->dev_tracks[c];  }
		else if (skey >= SDL_SCANCODE_A && skey <= SDL_SCANCODE_Z)
		{	c = skey - SDL_SCANCODE_A;  c+='A';  t = pSet->dev_tracks[c];  }

		if (!t.empty())
		{
			gui->BackFromChs();
			pSet->gui.track = t;  bPerfTest = false;
			pSet->gui.track_user = false;
			NewGame();  return;
		}
	}
	//............................................................................................................


	//  alt-Z  Tweak  (alt-shift-Z  save & reload)
	if (pSet->dev_keys)
	switch (skey)
	{
		case key(Z):
			if (alt)  gui->TweakToggle();   return;
		case key(INSERT):  // save .car
		case key(RETURN):
			if (mWndTweak->getVisible() &&
				gui->tabTweak->getIndexSelected() < 2)
			{	gui->TweakCarSave();  return;  }
			break;
		default:  break;
	}

	///  ⚫📉 Tire edit
	if (pSet->graphs_type == Gh_TireEdit || pSet->graphs_type == Gh_Tires4Edit && !tweak)
	{
		int& iCL = iEdTire==1 ? iCurLong : (iEdTire==0 ? iCurLat : iCurAlign);
		int iCnt = iEdTire==1 ? 11 : (iEdTire==0 ? 15 : 18);
		switch (skey)
		{
			case key(HOME):  case key(KP_7):  // mode long/lat
			if (ctrl)
				iTireLoad = 1-iTireLoad;
			else
				iEdTire = iEdTire==1 ? 0 : 1;  iUpdTireGr=1;  return;

			case key(END):  case key(KP_1):  // mode align
				iEdTire = iEdTire==2 ? 0 : 2;  iUpdTireGr=1;  return;

			case key(PAGEUP):  case key(KP_9):   // prev val
				iCL = (iCL-1 +iCnt)%iCnt;  iUpdTireGr=1;  return;

			case key(PAGEDOWN):  case key(KP_3):   // next val
				iCL = (iCL+1)%iCnt;  iUpdTireGr=1;  return;
			default:  break;
		}
	}


	//  not main menus
	//--------------------------------------------------------------------------------------------------------------
	#ifdef REVERB_BROWSER
	static int ii=0;
	#endif
	bool trkTab = mTabsGame->getVisible() && mTabsGame->getIndexSelected() == TAB_Track;
	//bool Fspc = isFocGui && trkTab && wf == gcom->edTrkFind;
	if (!tweak)
	{
		Widget* wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
		bool editFocus = wf && wf->getTypeName() == "EditBox";
		bool rpl = bRplPlay && !isFocGui;
		switch (skey)
		{
			case key(BACKSPACE):
				if (editFocus)
					break;
				if (rpl && mWndRpl)
				{	bRplWnd = !bRplWnd;  hud->Show();  return;  }  // 📽️ replay controls

				if (mWndHowTo->getVisible())
				{	pSet->iMenu = MN1_Setup;  gui->toggleGui(false);  return;  }
				else if (mWndChampStage->getVisible())	// 🏆 back from champs stage wnd
				{	gui->btnChampStageBack(0);  return;  }
				else if (mWndChallStage->getVisible())	// 🥇 chall
				{	gui->btnChallStageBack(0);  return;  }

				switch (pSet->iMenu)
				{
				case MN1_Main:  break;
				case MN1_Setup:  pSet->iMenu = MN1_Main;  break;
				
				case MN_Single: case MN_Tutorial: case MN_Champ:
				case MN_Chall:  case MN_Collect:  case MN_Career:
					pSet->iMenu = MN1_Games;  break;
				
				case MN_Replays:
					if (isFocGui)  pSet->iMenu = MN1_Main;
					break;
				case MN_Help: case MN_Options:
					pSet->iMenu = MN1_Main;  break;
				}
				gui->toggleGui(false);
				return;

			case key(P):	// 📽️ replay play/pause
				if (rpl)
				{	bRplPause = !bRplPause;  gui->UpdRplPlayBtn();
					return;  }
				break;

			case key(K):	// 📽️ replay car ofs
				if (rpl) {  --iRplCarOfs;  return;  }
				break;
			case key(L):
				if (rpl) {  ++iRplCarOfs;  return;  }
				break;
				
			case key(F):	//  🔍 focus on find edit
				if (ctrl && gcom->edTrkFind && (pSet->dev_keys || isFocGui && trkTab))
				{
					if (wf == gcom->edTrkFind)  // ctrl-F  twice to toggle filtering
					{	gcom->ckTrkFilter.Invert();  return;  }
					// if (pSet->dev_keys)
						// gui->GuiShortcut(Menu_Race, 1);	// Track tab
					MyGUI::InputManager::getInstance().resetKeyFocusWidget();
					MyGUI::InputManager::getInstance().setKeyFocusWidget(gcom->edTrkFind);
					return;
				}	break;
			
			
			#ifdef REVERB_BROWSER  // 🔉
			case key(1):
			{	--ii;  int s = pGame->snd->sound_mgr->mapReverbs.size();  if (ii < 0)  ii += s;
				auto it = pGame->snd->sound_mgr->mapReverbs.begin();
				for (int i=0; i<ii; ++i)  ++it;
				pGame->snd->sound_mgr->SetReverb((*it).first);
			}	break;
			case key(2):
			{	++ii;  int s = pGame->snd->sound_mgr->mapReverbs.size();  if (ii >= s)  ii -= s;
				auto it = pGame->snd->sound_mgr->mapReverbs.begin();
				for (int i=0; i<ii; ++i)  ++it;
				pGame->snd->sound_mgr->SetReverb((*it).first);
			}	break;
			#endif


			case key(F6):	//  🔝 Arrow
				if (shift)	gui->ckBeam.Invert(); else
				if (ctrl)	gui->ckArrow.Invert();
				else		gui->ckTrailShow.Invert();
				return;

			case key(F7):	//  ⏱️ Times
			//	if (alt)	gui->ckCarDbgBars.Invert(); else
			//	if (shift)	gui->ckOpponents.Invert(); else
				if (!ctrl)	gui->ckTimes.Invert();
				return;

			case key(F8):	//  🌍 Minimap
				if (alt)	gui->ckPaceShow.Invert(); else
				if (!shift)	gui->ckMinimap.Invert();
				return;

			case key(F9):	//  ⚫ car dbg
				if (ctrl)	gui->ckTireVis.Invert(); else
				if (alt)	gui->ckCarDbgSurf.Invert(); else
				if (shift)	gui->ckCarDbgTxt.Invert();
				else		gui->ckGraphs.Invert();
				return;

			case key(F11):	//  📈 Fps, profiler times
				if (shift)	gui->ckProfilerTxt.Invert(); else
				if (!ctrl)  gcom->nextFps();
				break;

			case key(F10):	//  blt debug
				if (ctrl)	gui->ckBulletDebug.Invert(); else
				if (alt)	gui->ckSoundInfo.Invert(); else
				if (shift)	gui->ckBltProfTxt.Invert();
				else		gui->ckWireframe.Invert();
				return;


			case key(KP_ENTER):
			case key(RETURN):		//  🏆 close champ wnds
				if (mWndChampStage->getVisible())
					gui->btnChampStageStart(0);
				else				//  🥇 chall
				if (mWndChallStage->getVisible())
					gui->btnChallStageStart(0);
				else				//  🏁 New game  after up/dn
				if (isFocGui)
					switch (pSet->iMenu)
					{
					case MN_Replays:	gui->btnRplLoad(0);  break;
					case MN_Single:
					case MN_Tutorial: case MN_Champ: case MN_Chall:	case MN_Collect:
						if (mWndGame->getVisible())
						switch (mTabsGame->getIndexSelected())
						{
						case TAB_Track:	 gui->changeTrack();  gui->btnNewGame(0);  break;
						case TAB_Car:	 gui->changeCar();    gui->btnNewGame(0);  break;
						case TAB_Split:	 gui->btnNewGame(0);  break;
						case TAB_Multi:	 gui->chatSendMsg();  break;
						case TAB_Champs:
							   if (gui->isCollectGui())  gui->btnCollectStart(0);
							else if (gui->isChallGui())  gui->btnChallStart(0);
							else  gui->btnChampStart(0);  break;
						}	break;
						/*if (mWndGame->getVisible() && gui->isCollectGui() &&
							mTabsGame->getIndexSelected() == TAB_Champs)
							gui->btnCollectStart(0);
						break;*/
				}
				else
				if (mClient && !isFocGui)  // show/hide players net wnd
				{	mWndNetEnd->setVisible(!mWndNetEnd->getVisible());  return;  }
				else
				if (!isFocGui)  // 🏁 show last lap results
				{	
					for (int i=0; i < carModels.size(); ++i)
					{	CarModel* cm = carModels[i];
						cm->updLap = true;  cm->fLapAlpha = 1.f;
				}	}
				break;

			case key(SPACE):		//  ◀️ toggle reversed
				if (isFocGui && mWndGame->getVisible())
				switch (mTabsGame->getIndexSelected())
				{
				case TAB_Track:  gui->ckReverse.Invert();  break;
				case TAB_Setup:
					if (pSet->paintAdj)
						gui->ckPaintNewLine.Invert();
					else  gui->btnPaintRandom(0);  break;
				}	break;
			
			case key(INSERT):		//  🖼️ toggle fullscreen preview
				if (gcom->imgPrv[2]->getVisible())
				{	gcom->imgPrv[2]->setVisible(false);
					gcom->imgTer[2]->setVisible(true);  gcom->imgMini[2]->setVisible(true);
				}else
				if (gcom->imgTer[2]->getVisible())
				{	gcom->imgTer[2]->setVisible(false);  gcom->imgMini[2]->setVisible(false);
				}else
				if (mWndGame->getVisible())
				{	switch (mTabsGame->getIndexSelected())
					{
					case TAB_Track: case TAB_Stage:
						gcom->imgPrv[2]->setVisible(true);  break;
				}	}
				break;
			default:  break;
		}
	}

	if (skey != key(RCTRL) && skey != key(LCTRL))
	if (mGui && (isFocGui || isTweak()))
	{
		MyGUI::KeyCode kc = SDL2toGUIKey(arg.keysym.sym);
		MyGUI::InputManager::getInstance().injectKeyPress(kc);
	}
}
#undef key


//----------------------------------------------------------------------------------------
//  ⌨️ Key actions
//----------------------------------------------------------------------------------------
void App::channelChanged(ICS::Channel *channel, float currentValue, float previousValue)
{
	if (currentValue != 1.f)
		return;

	#define action(a) (channel->getNumber() == a)
	int mnu = pSet->iMenu;

	//  Change Tabs
	//  🔧 Tweak
	if (mWndTweak->getVisible())
	{
		TabPtr tab = gui->tabTweak;
		if (!shift && tab->getIndexSelected() == 0)
			tab = gui->tabEdCar;  // car edit sections

		if (action(A_PrevTab) || action(A_PrevTab2)) {  // prev gui subtab
			int num = tab->getItemCount();
			tab->setIndexSelected( (tab->getIndexSelected() - 1 + num) % num );  }
		else if (action(A_NextTab) || action(A_NextTab2)) {  // next gui subtab
			int num = tab->getItemCount();
			tab->setIndexSelected( (tab->getIndexSelected() + 1) % num );  }

		if (tab == gui->tabEdCar)  // focus ed
		{	pSet->car_ed_tab = tab->getIndexSelected();
			MyGUI::InputManager::getInstance().resetKeyFocusWidget();
			MyGUI::InputManager::getInstance().setKeyFocusWidget(gui->edCar[tab->getIndexSelected()]);  }
	}
	//  change Gui tabs
	else if (isFocGui && !(mnu == MN1_Main || mnu == MN1_Setup))
	{
		MyGUI::TabPtr tab = 0;  MyGUI::TabControl* sub = 0;
		switch (mnu)
		{	case MN_Replays:  tab = mTabsRpl;  break;
			case MN_Help:     tab = mTabsHelp;  break;
			case MN_Options:  tab = mTabsOpts;  sub = gui->vSubTabsOpts[tab->getIndexSelected()];  break;
			default:          tab = mTabsGame;  sub = gui->vSubTabsGame[tab->getIndexSelected()];  break;
		}
		if (tab)
		if (alt)
		{	if (sub)  // prev, next subtab
			{	bool chng = false;
				if (action(A_PrevTab) || action(A_PrevTab2))
				{	int num = sub->getItemCount();  chng = true;
					sub->setIndexSelected( (sub->getIndexSelected() - 1 + num) % num );
				}else if (action(A_NextTab) || action(A_NextTab2))
				{	int num = sub->getItemCount();  chng = true;
						sub->setIndexSelected( (sub->getIndexSelected() + 1) % num );
				}
				if (chng && !sub->eventTabChangeSelect.empty())
					sub->eventTabChangeSelect(sub, sub->getIndexSelected());
		}	}
		else  // prev, next tab
		{	int num = tab->getItemCount()-1, i = tab->getIndexSelected(), n = 0;
			bool chng = false;
			if (action(A_PrevTab) || action(A_PrevTab2))
			{	do{  if (i==1)  i = num;  else  --i;  ++n;  }
				while (n < num && tab->getButtonWidthAt(i) == 1);
				chng = true;
			}else
			if (action(A_NextTab) || action(A_NextTab2))
			{	do{  if (i==num)  i = 1;  else  ++i;  ++n;  }
				while (n < num && tab->getButtonWidthAt(i) == 1);
				chng = true;
			}
			if (chng)
			{	tab->setIndexSelected(i);  gui->tabMainMenu(tab,i);  return;  }
		}
	}
	//  📉 change Graphs type
	else if (!isFocGui && pSet->show_graphs)
	{
		int& v = (int&)pSet->graphs_type;  int vo = v;
		if (action(A_PrevTab))  v = (v-1 + Gh_ALL) % Gh_ALL;
		if (action(A_NextTab))  v = (v+1) % Gh_ALL;
		if (vo != v)
		{
			gui->cmbGraphs->setIndexSelected(v);
			gui->comboGraphs(gui->cmbGraphs, v);
			if (v == 4)  iUpdTireGr = 1;  //upd now
		}
	}
	//----------------------------------------------------------------------------------------


	//  Gui on/off  or close wnds
	if (action(A_ShowOptions) && !alt)
	{
		Wnd wnd = mWndNetEnd;
		if (wnd && wnd->getVisible())  {  wnd->setVisible(false);  // hide netw end
			return;  }
		else
		{	wnd = mWndChampEnd;
			if (wnd && wnd->getVisible())  wnd->setVisible(false);  // hide champs end
			wnd = mWndChallEnd;
			if (wnd && wnd->getVisible())  wnd->setVisible(false);  // chall
			gui->toggleGui(true);
			return;
		}
	}

	//  🏁  New game - Reload   not in multiplayer
	if (action(A_RestartGame) && !mClient)
	{
		if (ctrl)  // ctrl-F5 start perf test
		{	gui->BackFromChs();
			pSet->gui.track = "Test10-FlatPerf";
			pSet->gui.track_user = false;
		}
		iPerfTestStage = PT_StartWait;
		NewGame(shift, ctrl);
		return;
	}

	//  🏁🔁  Reset game - fast (same track & cars)
	if (action(A_ResetGame) && !mClient)
	{
		for (int c=0; c < carModels.size(); ++c)
		{
			CarModel* cm = carModels[c];
			if (cm->pCar)
				cm->pCar->bResetPos = true;
			if (cm->iLoopLastCam != -1 && cm->fCam)
			{	cm->fCam->setCamera(cm->iLoopLastCam);  //o restore
				cm->iLoopLastCam = -1;  }
			cm->First();
			cm->ResetChecks();
			cm->iWonPlace = 0;  cm->iWonPlaceOld = 0;
			cm->iWonMsgTime = 0.f;
		}
		pGame->timer.Reset(-1);
		pGame->timer.pretime = mClient ? 2.0f : pSet->game.pre_time;  // same for all multi players

		iCollectedPrize = -2;
		carIdWin = 1;  //
		ghost.Clear();  replay.Clear();
	}
	
	//  todo: Screen shot
	/*if (action(A_Screenshot))
		mWindow->writeContentsToTimestampedFile(PATHS::Screenshots() + "/",
			pSet->screen_png ? ".png" : ".jpg");*/	
}
