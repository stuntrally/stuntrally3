#include "pch.h"
#include "Def_Str.h"
#include "paths.h"
#include "CGame.h"
#include "CGui.h"
#include "CData.h"
#include "TracksXml.h"
#include "GuiCom.h"

#include "MultiList2.h"
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
	/*if (app->mMasterClient && valNetPassword->getVisible())
	{	uploadGameInfo();
		updateGameInfoGUI();  }*/
}

//  🏁 New Game
void CGui::btnNewGame(WP wp)
{
	if (app->mWndGame->getVisible() &&
		app->mWndTabsGame->getIndexSelected() < TAB_Champs  /*|| app->mClient*/)
		BackFromChs();  /// champ, back to single race
	
	bool force = false;
	if (wp)
	{	string s = wp->getName();
		s = s.substr(s.length()-1,1);
		bool force = s=="3" || s=="4";
	}
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
	const bool gui = app->isFocGui;
	const int mnu = pSet->iMenu;

	app->mWndMain->setVisible(gui && mnu == MN1_Main);
	app->mWndRace->setVisible(gui && mnu == MN1_Race);
	
	app->mWndHowTo->setVisible(  gui && mnu == MN_HowTo);
	app->mWndReplays->setVisible(gui && mnu == MN_Replays);
	app->mWndHelp->setVisible(   gui && mnu == MN_Help);
	app->mWndOpts->setVisible(   gui && mnu == MN_Options);
	if (!gui)  app->mWndTrkFilt->setVisible(false);

	if (!gui && gcom->imgPrv[2])  // hide fullscr prv
	{	gcom->imgPrv[2]->setVisible(false);
		gcom->imgTer[2]->setVisible(false);
		gcom->imgMini[2]->setVisible(false);
	}
	
	//  fill help editboxes from text files
	if (app->mWndHelp->getVisible() && loadReadme)
	{	loadReadme = false;
		FillHelpTxt();
	}

	///  update track tab, for champs wnd
	bool game = mnu == MN_Single,   champ = mnu == MN_Champ,
		tutor = mnu == MN_Tutorial, chall = mnu == MN_Chall,
		chAny = champ || tutor || chall, gc = game || chAny;
	
	UString sCh = tutor ? TR("#FFC020#{Tutorial}") :
		champ ? TR("#B0FFB0#{Championship}") : TR("#C0C0FF#{Challenge}");

	UpdChampTabVis();
	
	bool notMain = gui && !(mnu == MN1_Main || mnu == MN1_Race);
	bool vis = notMain && gc;
	app->mWndGame->setVisible(vis);
	if (vis)
	{	const static float clrs[4][3] = {
			// single      tutor          champ          chall
			{0.9,0.9,0.6}, {1.0,0.6,0.3}, {0.6,1.0,0.6}, {0.6,0.6,1.0} };
		const int c = tutor ? 1 : champ ? 2 : chall ? 3 : 0;
		app->mWndGame->setColour(Colour(clrs[c][0], clrs[c][1], clrs[c][2]));
		app->mWndGame->setCaption(chAny ? sCh : TR("#{SingleRace}"));
		
		TabItem* t = app->mWndTabsGame->getItemAt(TAB_Champs);
		t->setCaption(sCh);
	}
	if (notMain && gc)  // show hide champs,stages
	{
		Tab t = app->mWndTabsGame;
		size_t id = t->getIndexSelected();
		t->setButtonWidthAt(TAB_Track, chAny ? 1 :-1);  if (id == TAB_Track && chAny)  t->setIndexSelected(TAB_Champs);
		t->setButtonWidthAt(TAB_Split, chAny ? 1 :-1);  if (id == TAB_Split && chAny)  t->setIndexSelected(TAB_Champs);
		t->setButtonWidthAt(TAB_Multi, chAny ? 1 :-1);  if (id == TAB_Multi && chAny)  t->setIndexSelected(TAB_Champs);
		
		t->setButtonWidthAt(TAB_Champs,chAny ?-1 : 1);  if (id == TAB_Champs && !chAny)  t->setIndexSelected(TAB_Track);
		t->setButtonWidthAt(TAB_Stages,chAny ?-1 : 1);  if (id == TAB_Stages && !chAny)  t->setIndexSelected(TAB_Track);
		t->setButtonWidthAt(TAB_Stage, chAny ?-1 : 1);  if (id == TAB_Stage  && !chAny)  t->setIndexSelected(TAB_Track);
	}

	gcom->bnQuit->setVisible(gui);
	app->updMouse();
	if (!gui)  gcom->mToolTip->setVisible(false);

	for (int i=0; i < ciMainBtns; ++i)
		app->mWndMainPanels[i]->setVisible(pSet->yMain == i);
	for (int i=0; i < ciRaceBtns; ++i)
		app->mWndRacePanels[i]->setVisible(pSet->yRace == i);
		
	//  1st center mouse
	static bool first = true;
	if (gui && first)
	{	first = false;
		gcom->GuiCenterMouse();
	}
	// LogO(String(":::* Time Gui upd: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


//  🎛️ Gui Shortcut  ⌨️ alt-letters
//.......................................................................................
void CGui::GuiShortcut(EMenu menu, int tab, int subtab)
{
	if (subtab == -1 && (!app->isFocGui || pSet->iMenu != menu))
		subtab = -2;  // cancel subtab cycling

	app->isFocGui = true;
	pSet->iMenu = menu;
	
	TabPtr mWndTabs = 0;
	std::vector<TabControl*>* subt = 0;
	
	switch (menu)
	{	case MN_Replays:  mWndTabs = app->mWndTabsRpl;  break;
		case MN_Help:     mWndTabs = app->mWndTabsHelp;  break;
		case MN_Options:  mWndTabs = app->mWndTabsOpts;  subt = &vSubTabsOpts;  break;
		default:          mWndTabs = app->mWndTabsGame;  subt = &vSubTabsGame;  break;
	}
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

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
}

//  close netw end
/*void CGui::btnNetEndClose(WP)
{
	app->mWndNetEnd->setVisible(false);
	app->isFocGui = true;  // show back gui
	toggleGui(false);
}*/


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
		switch (app->mWndTabsGame->getIndexSelected())
		{
			case TAB_Track:  if (gcom->trkList->getItemCount() == 0)  return;
				gcom->listTrackChng(gcom->trkList,  LNext(gcom->trkList, rel, 11));  return;

			case TAB_Car:	 listCarChng(carList, LNext(carList, rel, 5));  return;
			case TAB_Champs:
				if (isChallGui())
				      listChallChng(liChalls, LNext(liChalls, rel, 8));
				else  listChampChng(liChamps, LNext(liChamps, rel, 8));
				return;
			case TAB_Stages: listStageChng(liStages, LNext(liStages, rel, 8));  return;
			case TAB_Stage:	 if (rel > 0)  btnStageNext(0);  else  btnStagePrev(0);  return;
		}
}


///  💫 Gui Update (frame start)  .,.,.,.,..,.,.,.,..,.,.,.,..,.,.,.,.
void CGui::GuiUpdate()
{
	gcom->UnfocusLists();

	/*if (gcom->bGuiReinit)  // after language change from combo
	{	gcom->bGuiReinit = false;

		mGui->destroyWidgets(app->vwGui);
		gcom->bnQuit=0; app->mWndOpts=0;  //todo: rest too..  delete, new gui; ?

		bGI = false;
		InitGui();

		app->bWindowResized = true;
		app->mWndTabsOpts->setIndexSelected(3);  // switch back to view tab
	}*/

		
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
	/*if (app->pGame->reloadSimDone)
	{	app->pGame->reloadSimDone = false;

		FillTweakLists();
		btnTweakTireLoad(0);  // load back
	}*/
}


//  🔗 open urls
//.......................................................................................
void CGui::btnWelcome(WP)
{
	app->mWndWelcome->setVisible(true);
}
void CGui::btnWebsite(WP)
{
	PATHS::OpenUrl("https://stuntrally.tuxfamily.org/");
}
void CGui::btnWiki(WP)
{
	PATHS::OpenUrl("https://stuntrally.tuxfamily.org/wiki/doku.php?id=features");
}
void CGui::btnWikiInput(WP)
{
	PATHS::OpenUrl("https://stuntrally.tuxfamily.org/wiki/doku.php?id=running#controllers");
}

void CGui::btnForum(WP)
{
	PATHS::OpenUrl("https://forum.freegamedev.net/viewforum.php?f=77");
}
void CGui::btnSources(WP)
{
	PATHS::OpenUrl("https://github.com/stuntrally/stuntrally");
}
void CGui::btnEdTut(WP)
{
	PATHS::OpenUrl("https://stuntrally.tuxfamily.org/wiki/doku.php?id=editor");
}
void CGui::btnTransl(WP)
{
	PATHS::OpenUrl("https://stuntrally.tuxfamily.org/wiki/doku.php?id=localization");
}
void CGui::btnDonations(WP)
{
	PATHS::OpenUrl("https://cryham.tuxfamily.org/donate/");
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
				if (s.substr(0,7) == "## Desc")
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
	ed = fEd("Contributing");  if (ed)  PathMd("Contributing.md", 1);

	ed = fEd("Credits");
	if (ed)
	{	string dir = PATHS::Data()+"/";
		String text = "", sep = "-------------------------------------------------------";

		auto Sep = [&](String title)
		{
			text += "\n#B0D0FF"+ sep+sep +"\n"+ "   " + title +"\n"+ sep+sep +"#D0D0D0\n";
		};
		auto ReadTxt = [&](string path)
		{
			path = dir + path;
			text += "\n#F0F0C0====  File: " + path + "#D0D0D0\n\n";
			std::ifstream fi(path.c_str());
			if (fi.good())
			{
				String s;
				while (getline(fi,s))
					text += s + "\n";
			}
		};
		auto ReadTxts = [&](string path)
		{
			strlist lo;
			PATHS::DirList(dir + path, lo, "txt");
			
			Sep(path);
			for (auto p:lo)
				ReadTxt(path +"/"+ p);
		};
		Sep("Vehicles");
		for (auto c:data->cars->cars)
			ReadTxt("cars/" + c.id + "/about.txt");

		ReadTxts("objects");  ReadTxts("objects2");  ReadTxts("objectsC");  ReadTxts("objects0");
		ReadTxts("skies");  ReadTxts("rocks");
		ReadTxts("grass");  ReadTxts("terrain");  ReadTxts("road");
		ReadTxts("trees");  ReadTxts("trees2");   ReadTxts("trees-old");  //todo: replace-
		ReadTxts("sounds");  ReadTxts("hud");
		//  particles-

		ed->setCaption(UString(text));
		ed->setVScrollPosition(0);
	}
}


//  🖼️ fullscr previews
void CGui::ImgPrvClk(WP)
{
	gcom->imgPrv[2]->setVisible(true);
}
void CGui::ImgTerClk(WP)
{
	gcom->imgTer[2]->setVisible(true);
	gcom->imgMini[2]->setVisible(true);
}
void CGui::ImgPrvClose(WP)
{
	gcom->imgPrv[2]->setVisible(false);
	gcom->imgTer[2]->setVisible(false);
	gcom->imgMini[2]->setVisible(false);
}
