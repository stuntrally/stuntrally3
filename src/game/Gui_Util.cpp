#include "pch.h"
#include "Def_Str.h"
#include "paths.h"
#include "BaseApp.h"
#include "CGame.h"
#include "CGui.h"
#include "CHud.h"
#include "CData.h"
#include "TracksXml.h"
#include "GuiCom.h"
#include "game.h"
// #include "SoundMgr.h"
#include "gameclient.hpp"

#include "MultiList2.h"
#include "settings.h"
#include <OgreTimer.h>
#include <MyGUI.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


//  🏞️ track
void CGui::changeTrack()
{
	pSet->gui.track = gcom->sListTrack;
	pSet->gui.track_user = gcom->bListTrackU;
							//_ only for host..
	if (app->mMasterClient && valNetPassword->getVisible())
	{	uploadGameInfo();
		updateGameInfoGUI();  }
}

//  🏁 New Game
void CGui::btnNewGame(WP wp)
{
	if (app->mWndGame->getVisible() &&
		app->mTabsGame->getIndexSelected() < TAB_Champs  || app->mClient)
		BackFromChs();  /// champ, back to single race
	
	/*bool force = false;
	if (wp)
	{	string s = wp->getName();
		s = s.substr(s.length()-1,1);
		bool force = s=="3" || s=="4";
	}*/
	bool force = 1;  //; fixme 0 on same track and game type
	app->NewGame(force);  app->isFocGui = false;  // off gui

	app->mWndOpts->setVisible(app->isFocGui);
	app->mWndRpl->setVisible(false);  app->mWndRplTxt->setVisible(false);//
	gcom->bnQuit->setVisible(app->isFocGui);
	
	app->updMouse();
	gcom->mToolTip->setVisible(false);
}
void CGui::btnNewGameStart(WP wp)
{
	changeTrack();
	btnNewGame(wp);
}


//  🪟🪧 Gui windows vis
//-----------------------------------------------------------------------------------------------------------
void CGui::toggleGui(bool toggle)
{
	Ogre::Timer ti;
	if (toggle)
		app->isFocGui = !app->isFocGui;
	
	// 🔉  if (pGame && pGame->snd)
	pGame->snd->getSoundManager()->setMasterVolume(app->isFocGui ? 0.0 : pSet->vol_master);

	const bool gui = app->isFocGui;
	const int mnu = pSet->iMenu;

	app->mWMainMenu->setVisible( gui && mnu == MN1_Main);  // main
	app->mWMainSetup->setVisible(gui && mnu == MN1_Setup);
	app->mWMainGames->setVisible(gui && mnu == MN1_Games);

	app->mWndHowTo->setVisible(  gui && mnu == MN_HowTo);
	app->mWndReplays->setVisible(gui && mnu == MN_Replays);
	
	app->mWndHelp->setVisible(     gui && mnu == MN_Help);
	app->mWndOpts->setVisible(     gui && mnu == MN_Options);
	app->mWndMaterials->setVisible(gui && mnu == MN_Materials);
	
	if (!gui)  app->mWndTrkFilt->setVisible(false);

	//  info
	bool v = pSet->champ_info && gui && (mnu == MN1_Games || mnu == MN1_Setup);
	app->mWndGameInfo->setVisible(v);
	updGameInfo();
	
	CarListUpd();  // filter, back
	
	for (int i=0; i < 3; ++i)
		hud->cup[i].Hide();


	//  hide fullscr prv
	if (!gui && gcom->imgPrv[2])
	{	gcom->imgPrv[2]->setVisible(false);
		gcom->imgTer[2]->setVisible(false);
		gcom->imgMini[2]->setVisible(false);
	}
	
	//  fill help editboxes from text files
	if (app->mWndHelp->getVisible() && loadReadme)
	{	loadReadme = false;
		FillHelpTxt();
	}

	UpdGamesTabVis();  // games

	//  quit
	gcom->bnQuit->setVisible(gui);
	app->updMouse();
	if (!gui)  gcom->mToolTip->setVisible(false);

	//  menu
	for (int i=0; i < ciMainBtns; ++i)
		app->mMainPanels[i]->setVisible(pSet->yMain == i);
	for (int i=0; i < ciSetupBtns; ++i)
		app->mMainSetupPanels[i]->setVisible(pSet->ySetup == i);
	app->updPanGames();
		
	//  1st center mouse
	static bool first = true;
	if (gui && first)
	{	first = false;
		gcom->GuiCenterMouse();
	}
	// LogO(String(":::* Time Gui upd: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}

void CGui::tabGame(Tab, size_t)  // main game wnd tab change
{
	//UpdWndTitle();
}

//  🪟 game window title and color
//.......................................................................................
void CGui::UpdWndTitle()
{
	const int mnu = pSet->iMenu;
	if (mnu < MN_Single && mnu > MN_Career)
		return;  // not set
	
	bool game = mnu == MN_Single,   champ = mnu == MN_Champ,
		tutor = mnu == MN_Tutorial, chall = mnu == MN_Chall,
		collect = mnu == MN_Collect, career = mnu == MN_Career,
		chAny = tutor || champ || chall || collect || career,
		gc = game || chAny;
	
	Tab t = app->mTabsGame;
	size_t id = t->getIndexSelected();

	int mplr = app->mClient ? app->mClient->getPeerCount()+1 :
		id == TAB_Multi ? 1 : 0;  // 📡 networked
	int plr = app->pSet->gui.local_players;  // 👥 splitscreen
	
	Colour c;  UString s;  // title
	if (mplr)
	{	c = Colour(0.2,0.6,1.0);
		s = TR("#80C0FF#{Multiplayer} - #{Players}: ") + toStr(mplr);
	}
	else if (career)
	{	c = Colour(1.0,0.6,0.6);
		s = TR("#FFA0A0#{Career}");
	}
	else if (collect)
	{	c = plr == 1 ? Colour(0.9,0.7,1.0) : Colour(0.8,0.6,1.0);
		s = plr == 1 ? TR("#D0A0FF#{Collection}") :
			TR("#E0C0FF#{Collection} - #{SplitScreen} - #{Players}: ") + toStr(plr);
	}
	else if (chall)
	{	c = Colour(0.6,0.6,1.0);
		s = TR("#C0C0FF#{Challenge}");
	}
	else if (champ)
	{	c = Colour(0.5,1.0,0.5);
		s = TR("#A0FFA0#{Championship}");
	}
	else if (tutor)
	{	c = Colour(1.0,0.6,0.3);
		s = TR("#FFC020#{Tutorial}");
	}else
	{	c = plr == 1 ? Colour(0.9,0.9,0.5) : Colour(0.5,1.0,1.0);
		s = plr == 1 ? TR("#F0F050#{SingleRace}") :
			TR("#60F0F0#{SplitScreen} - #{Players}: ") + toStr(plr);
	}
	app->mWndGame->setColour(c);
	app->mWndGame->setCaption(s);
}


//  🎛️ Gui Shortcut  ⌨️ alt-letters, main menu btns too
//.......................................................................................
void CGui::GuiShortcut(EMenu menu, int tab, int subtab, int game)
{
	if (subtab == -1 && (!app->isFocGui || (menu != MN_NoCng && pSet->iMenu != menu)))
		subtab = -2;  // cancel subtab cycling  ?^

	app->isFocGui = true;
	if (menu != MN_NoCng)  // chg menu
	{
		pSet->iMenu = menu;
		
		if (pSet->iMenu == MN_Single)  // set num plr
		{
			if (tab == TAB_Track || tab == TAB_Champs)
			{	SetNumPlayers(1);   app->updPanGames(game);  }
			if (tab == TAB_Split)
			{	SetNumPlayers(-2);  app->updPanGames(game);  }  // 👥
			if (tab == TAB_Multi)
			{	SetNumPlayers(1);   app->updPanGames(game);  }  // 📡
	}	}

	TabPtr tabs = 0;
	std::vector<TabControl*>* subt = 0;
	
	switch (pSet->iMenu)
	{	case MN_Replays:   tabs = app->mTabsRpl;  break;
		case MN_Help:      tabs = app->mTabsHelp;  break;
		case MN_Options:   tabs = app->mTabsOpts;  subt = &vSubTabsOpts;  break;
		case MN_Materials: tabs = app->mTabsMat;   subt = &vSubTabsMat;  break;
		default:           tabs = app->mTabsGame;  subt = &vSubTabsGame;  break;
	}

	if (tab < 0)
	{	toggleGui(false);  return;  }

	size_t t = tabs->getIndexSelected();
	tabs->setIndexSelected(tab);
	toggleGui(false);

	if (!subt)  return;
	TabControl* tc = (*subt)[tab];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{
		int i = (tc->getIndexSelected() +
			(app->shift ? -1+cnt : 1) ) % cnt;
		if (tc->getButtonWidthAt(i) > 10)  // if tab visible
			tc->setIndexSelected(i);
	}
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
	
	if (!tc->eventTabChangeSelect.empty())
		tc->eventTabChangeSelect(tc, tc->getIndexSelected());
	// UpdWndTitle();
	toggleGui(false);
}

//  close netw end
void CGui::btnNetEndClose(WP)
{
	app->mWndNetEnd->setVisible(false);
	app->isFocGui = true;  // show back gui
	toggleGui(false);
}


//  ⛓️ utility
//---------------------------------------------------------------------------------------------------------------------

//  📃 next/prev in list by key
int CGui::LNext(Mli2 lp, int rel, int ofs)
{
	int cnt = (int)lp->getItemCount();
	if (cnt==0)  return 0;
	if (cnt==1 || abs(rel) >= cnt)
	{	lp->setIndexSelected(0);  return 1;  }

	int i = (int)lp->getIndexSelected();
	//int i = std::max(0, std::min((int)cnt-1, i+rel ));
	i = (i + rel + cnt) % cnt;  // cycle
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-ofs));  // center
	return i;
}
int CGui::LNext(Mli lp, int rel)
{
	size_t cnt = lp->getItemCount();
	if (cnt==0 || abs(rel) >= cnt)  return 0;

	int i = std::max(0, std::min((int)cnt-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	return i;
}
int CGui::LNext(Li lp, int rel, int ofs)
{
	size_t cnt = lp->getItemCount();
	if (cnt==0 || abs(rel) >= cnt)  return 0;
	
	int i = std::max(0, std::min((int)cnt-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-ofs));  // center
	return i;
}

void CGui::LNext(int rel)
{
	if (!app->isFocGui /*|| pSet->isMain*/)  return;
	if (pSet->iMenu == MN_Replays)
		listRplChng(rplList,  LNext(rplList, rel, 11));
	else
	if (app->mWndGame->getVisible())
		switch (app->mTabsGame->getIndexSelected())
		{
			case TAB_Track:  if (gcom->trkList->getItemCount() == 0)  return;
				gcom->listTrackChng(gcom->trkList,  LNext(gcom->trkList, rel, 11));  return;

			case TAB_Car:	 listCarChng(carList, LNext(carList, rel, 5));  return;
			case TAB_Champs:
				if (isCollectGui())
			    	listCollectChng(liCollect, LNext(liCollect, rel, 8));
				else if (isChallGui())
				      listChallChng(liChalls, LNext(liChalls, rel, 8));
				else  listChampChng(liChamps, LNext(liChamps, rel, 8));
				return;
			case TAB_Stages: listStageChng(liStages, LNext(liStages, rel, 8));  return;
			case TAB_Stage:	 if (rel > 0)  btnStageNext(0);  else  btnStagePrev(0);  return;

			case TAB_Setup:  // Paints
				auto& c = pSet->car_clr;
				auto& v = data->paints->vf;
				int si = v.size();
				c = (c + rel + si) % si;
				pSet->gui.clr[iCurCar] = v[c];
				UpdPaintSld();
				return;
		}
}


///  💫 Gui Update (frame start)  .,.,.,.,..,.,.,.,..,.,.,.,..,.,.,.,.
void CGui::GuiUpdate()
{
	gcom->UnfocusLists();

	if (gcom->bGuiReinit)  // after language change from combo  fixme
	{	gcom->bGuiReinit = false;

		mGui->destroyWidgets(app->vwGui);
		gcom->bnQuit=0; app->mWndOpts=0;  //todo: rest too..  delete, new gui; ?

		bGI = false;
		InitGui();

		app->bWindowResized = true;
		app->mTabsOpts->setIndexSelected(3);  // switch back to view tab
	}

		
	//  sort trk list
	gcom->SortTrkList();

	//  sort car list
	if (gcom->needSort(carList))
	{
		pSet->cars_sort = carList->mSortColumnIndex;
		pSet->cars_sortup = carList->mSortUp;
		CarListUpd(false);
	}

	//  upd tweak tire save
	if (app->pGame->reloadSimDone)  // todo:?
	{	app->pGame->reloadSimDone = false;

		FillTweakLists();
		btnTweakTireLoad(0);  // load back
	}
}


//  🔗 open urls
//.......................................................................................
//  start ed exe
void CGui::btnTrackEditor(WP)
{
	auto path = PATHS::Bin();
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	PATHS::OpenBin(path + "\\SR-Editor3.exe", path);
#else
	PATHS::OpenBin(path + "/sr-editor3", path);
#endif
}

void CGui::btnWelcome(WP)
{
	app->mWndWelcome->setVisible(true);
}

void CGui::Url(const string& url)
{
	edOpenUrl->setCaption(url);
	PATHS::OpenUrl(url);
}
void CGui::btnWebsite(WP)
{
	Url("https://stuntrally.tuxfamily.org/");
}
void CGui::btnWiki(WP)
{
	Url("https://github.com/stuntrally/stuntrally3/blob/main/docs/_menu.md");
}
void CGui::btnWikiInput(WP)
{
	Url("https://github.com/stuntrally/stuntrally3/blob/main/docs/Running.md#controllers");
}

void CGui::btnOpenChat(WP)
{
	Url("https://discord.gg/TywnXxAtR6");
}
void CGui::btnForum(WP)
{
	Url("https://groups.f-hub.org/stunt-rally/");
}
void CGui::btnSources(WP)
{
	Url("https://github.com/stuntrally/stuntrally3");
}
void CGui::btnEdTut(WP)
{
	Url("https://github.com/stuntrally/stuntrally3/blob/main/docs/Editor.md");
}
void CGui::btnTransl(WP)
{
	Url("https://github.com/stuntrally/stuntrally3/blob/main/docs/Localization.md");
}
void CGui::btnDonations(WP)
{
	Url("https://cryham.tuxfamily.org/donate/");
}


//  📝 read and fill help texts
//.......................................................................................
void CGui::FillHelpTxt()
{
	Ed ed;  string path;
	
	auto ReadMd = [&](bool top)
	{
		std::ifstream fi(path.c_str());
		bool ok = top;  // skip
		if (fi.good())
		{
			String text = "", s;
			while (getline(fi,s))
			{
				if (s.substr(0,8) == "## Links")
					ok = true;  // start

				bool ch = !s.empty() && s[0]=='#';  // chapters
				s = StringUtil::replaceAll(s, "#", "");  // headers
				s = StringUtil::replaceAll(s, "**", "");  // bold
				if (ch)
				 	s = "#B0D0FF"+s+"#C8D0D8";
				
				if (ok)  //s.substr(0,2) != "![" && s.substr(0,2) != "[!")  // no imgs, bagdes
					text += s + "\n";
			}
			ed->setCaption(UString(text));
			ed->setVScrollPosition(0);
	}	};

	auto PathMd = [&](string file, bool top)
	{	path = PATHS::Data() +"/../"+ file;
		if (!PATHS::FileExists(path))
			path = PATHS::Data() +"/"+ file;
		if (!PATHS::FileExists(path))
			LogO("Can't locate file: " + file);
		ReadMd(top);
	};

	ed = fEd("Readme");        if (ed)  PathMd("Readme.md", 0);
	ed = fEd("Changes");       if (ed)  PathMd("docs/Changelog.md", 1);
	ed = fEd("Contributing");  if (ed)  PathMd("docs/Contributing.md", 1);

	ed = fEd("Credits");
	if (ed)
	{	//  data paths
		string dir[3] = {PATHS::Data()+"/", PATHS::Models()+"/", PATHS::Textures()+"/"};
		String text = "", sep = "-------------------------------------------------------";

		auto Sep = [&](String title)
		{
			text += "\n#B0D0FF"+ sep+sep +"\n"+ "   " + title +"\n"+ sep+sep +"#D0D0D0\n";
		};
		auto ReadTxt = [&](string path)
		{
			text += "\n#F0F0C0====  File: " + path + "#D0D0D0\n\n";
			std::ifstream fi(path.c_str());
			if (fi.good())
			{
				String s;
				while (getline(fi,s))
					text += s + "\n";
			}
		};
		auto ReadTxts = [&](string path, int d=1)
		{
			path = dir[d] + path;
			strlist lo;
			PATHS::DirList(path, lo, "txt");
			
			Sep(path);
			for (auto p : lo)
				if (p[0] == '_')
					ReadTxt(path +"/"+ p);
		};
		Sep("Vehicles");
		for (auto c : data->cars->cars)
			ReadTxt(dir[0] + "cars/" + c.id + "/about.txt");

		ReadTxts("objects");  ReadTxts("objects2");  ReadTxts("objectsC");  ReadTxts("objects0");
		ReadTxts("obstacles");  ReadTxts("rocks");  ReadTxts("rockshex");
		ReadTxts("gems");
		
		ReadTxts("skies",2);  
		ReadTxts("grass",2);  ReadTxts("terrain",2);  ReadTxts("road",2);

		ReadTxts("trees");  ReadTxts("trees2");   ReadTxts("trees-old");

		ReadTxts("sounds",0);  ReadTxts("hud",0);
		ReadTxts("particles",0);

		ed->setCaption(UString(text));
		ed->setVScrollPosition(0);
	}
}
