#include "pch.h"
#include "Def_Str.h"
#include "CData.h"
#include "TracksXml.h"
#include "GuiCom.h"
#include "App.h"
#include "CGui.h"
#include "MultiList2.h"
#include <MyGUI.h>
using namespace std;  using std::vector;
using namespace Ogre;
using namespace MyGUI;


//  Prize const  * * *
const String
	CGui::clrPrize[4] = {"","#D0B050","#D8D8D8","#E8E050"},
	CGui::strPrize[4] = {"","#{Bronze}","#{Silver}","#{Gold}"};

//  lowering pass req
const int   CGui::ciAddPos[3]    = {4,2,0};
const float CGui::cfSubPoints[3] = {2.f,1.f,0.f};

const String CGui::StrPrize(int i)  //0..3
{
	return clrPrize[i] + TR(strPrize[i]);
}


//  chk all, reverse
void CGui::chkCh_All(Ck* ck)
{
	chkChampRev(ck);
}
void CGui::chkChampRev(Ck*)
{
	ChampsListUpdate();
	ChallsListUpdate();
}


///  🪟 upd Gui vis  games common  champs, challs, collects
//-----------------------------------------------------------------------------------------------
void CGui::UpdGamesTabVis()
{
	if (!liChamps || !tabChamp || !btStChamp || !btStCollect)  return;

	const bool gui = app->isFocGui;
	const int mnu = pSet->iMenu;
	bool game = mnu == MN_Single,   champ = mnu == MN_Champ,
		tutor = mnu == MN_Tutorial, chall = mnu == MN_Chall,
		collect = mnu == MN_Collect, career = mnu == MN_Career,
		chAny = tutor || champ || chall || collect || career,
		stages = tutor || champ || chall || career,
		setup = tutor || champ || collect,  //..
		gc = game || chAny,
		split = !chAny && pSet->yGames == Games_SplitScreen,
		multi = !chAny && pSet->yGames == Games_Multiplayer;

	btNewGameCar->setVisible(!chAny);
	imgTut->setVisible(tutor);    btStTut->setVisible(tutor);
	panCh->setColour(tutor ? Colour(0.9,0.8,0.7) : champ ? Colour(0.7,0.9,0.8) : Colour(0.77,0.75,0.92));

	tabChamp->setVisible(champ);  imgChamp->setVisible(champ);  btStChamp->setVisible(champ);
	liChamps->setVisible(champ || tutor);  liChamps->setColour(tutor ? Colour(0.85,0.8,0.75) : Colour(0.75,0.8,0.85));

	tabChall->setVisible(chall);  imgChall->setVisible(chall);  btStChall->setVisible(chall);
	liChalls->setVisible(chall);  liChalls->setColour(Colour(0.74,0.7,0.82));

	imgCollect->setVisible(collect);  btStCollect->setVisible(collect);
	tabCollect->setVisible(collect);
	liCollect->setVisible(collect);  liCollect->setColour(Colour(0.7,0.6,0.8));
	imgCareer->setVisible(career);

	// if (collect)  CollectListUpdate();  else
	// if (chall)  ChallsListUpdate();  else
	// 	ChampsListUpdate();

	//  img prv, detail
	for (int i=0; i < ImgTrk; ++i)
		imgTrk[i]->setVisible(0);
	imgTrkBig->setVisible(collect);
	txColDetail[0]->setVisible(collect);
	txColDetail[1]->setVisible(collect);

	//if (pSet->inRace == Race_Single)
	//	BackFromChs();
	
	updGameInfo();
	btChRestart->setVisible(false);


	///  update track tab, for champs wnd
	UString sCh =
		collect ? TR("#C080FF#{Collection}") : career ? TR("#FF8080#{Career}") :
		chall ? TR("#C0C0FF#{Challenge}") :
		champ ? TR("#B0FFB0#{Championship}") : TR("#FFC020#{Tutorial}");

	bool notMain = gui && !(mnu == MN1_Main || mnu == MN1_Setup || mnu == MN1_Games);
	bool vis = notMain && gc;
	app->mWndGame->setVisible(vis);
	if (vis)
	{
		UpdWndTitle();

		TabItem* t = app->mTabsGame->getItemAt(TAB_Champs);
		t->setCaption(sCh);
	}
	if (notMain && gc)  // show/hide tabs  track,stages,
	{
		Tab t = app->mTabsGame;  // from Game
		size_t id = t->getIndexSelected();
		t->setButtonWidthAt(TAB_Track, chAny ? 1 :-1);  if (id == TAB_Track && chAny)  t->setIndexSelected(TAB_Champs);
		t->setButtonWidthAt(TAB_Split, split ?-1 : 1);  if (id == TAB_Split && !split)  t->setIndexSelected(TAB_Champs);
		t->setButtonWidthAt(TAB_Multi, multi ?-1 : 1);  if (id == TAB_Multi && !multi)  t->setIndexSelected(TAB_Champs);
		
		t->setButtonWidthAt(TAB_Champs,chAny ?-1 : 1);  if (id == TAB_Champs && !chAny)  t->setIndexSelected(TAB_Track);
		t->setButtonWidthAt(TAB_Stages,stages?-1 : 1);  if (id == TAB_Stages && !stages) t->setIndexSelected(TAB_Track);
		t->setButtonWidthAt(TAB_Stage, chAny ?-1 : 1);  if (id == TAB_Stage  && !chAny)  t->setIndexSelected(TAB_Track);
		//  from Setup  0 paint  3 collision
		vSubTabsGame[TAB_Setup]->setButtonWidthAt(1, !setup ?-1 : 1);  // 1 Game 💨🔨⏪
		vSubTabsGame[TAB_Setup]->setButtonWidthAt(3, !setup ?-1 : 1);  // 2 Boost 💨
	}

}


//  info text
//-----------------------------------------------------------------------------------------------
void CGui::updGameInfo()
{
	if (!edGameInfo)  return;
	if (pSet->iMenu == MN1_Setup)
	{	edGameInfo->setCaption(
			pSet->ySetup == Setup_Difficulty ? TR("#{DifficultyHelp}") :
			pSet->ySetup == Setup_Simulation ? TR("#{SimulationHelp}") : "");
		return;
	}
	int g = pSet->yGames;
	bool game = g == Games_Single,  champ = g == Games_Champ,
		tutor = g == Games_Tutorial, chall = g == Games_Challenge,
		collect = g == Games_Collection;//, career = g == Games_Career;
	edGameInfo->setCaption(
		// career ? "" :
		collect ? TR("#{CollectInfo}") :
		chall ? TR("#{ChallInfo2}")+"\n"+TR("#{ChallInfo}") :
		tutor ? TR("#{TutorInfo}")+"\n"+TR("#{ChampInfo}") :
		champ ? TR("#{ChampInfo2}")+"\n"+TR("#{ChampInfo}") : "");

	// if (liChamps->getVisible() && liChamps->getIndexSelected() == -1)
	// 	return;
}
void CGui::btnGameInfo(WP)
{
	pSet->champ_info = !pSet->champ_info;
	updGameInfo();
	app->mWndGameInfo->setVisible(pSet->champ_info);
}


///  upd dim  games lists
//-----------------------------------------------------------------------------------------------
void CGui::updGamesListDim()
{
	const IntCoord& wi = app->mWndGame->getCoord();

	//  Champs  -----
	if (!liChamps)  return;
	int c,w;
	int sum = 0, cnt = liChamps->getColumnCount(), sw = 0;
	for (c=0; c < cnt; ++c)  sum += colCh[c];
	for (c=0; c < cnt; ++c)
	{
		w = c==cnt-1 ? 18 : float(colCh[c]) / sum * 0.76/*width*/ * wi.width * 0.97/*frame*/;
		liChamps->setColumnWidthAt(c, w);  sw += w;
	}
	int xt = 0.03*wi.width, yt = 0.09*wi.height;  // pos
	liChamps->setCoord(xt, yt, sw + 8/*frame*/, 0.40/*height*/*wi.height);
	liChamps->setVisible(isChampGui());


	//  Stages  -----
	if (!liStages)  return;
	sum = 0;  cnt = liStages->getColumnCount();  sw = 0;
	for (c=0; c < cnt; ++c)  sum += colSt[c];  sum += 43;//-
	for (c=0; c < cnt; ++c)
	{
		w = c==cnt-1 ? 18 : float(colSt[c]) / sum * 0.58/*width*/ * wi.width * 0.97/**/;
		liStages->setColumnWidthAt(c, w);  sw += w;
	}
	liStages->setCoord(xt, yt, sw + 8/**/, 0.50/*height*/*wi.height);
	liStages->setVisible(true);


	//  Challs  -----
	if (!liChalls)  return;
	sum = 0;  cnt = liChalls->getColumnCount();  sw = 0;
	for (c=0; c < cnt; ++c)  sum += colChL[c];
	for (c=0; c < cnt; ++c)
	{
		w = c==cnt-1 ? 18 : float(colChL[c]) / sum * 0.76/*width*/ * wi.width * 0.97/**/;
		liChalls->setColumnWidthAt(c, w);  sw += w;
	}
	liChalls->setCoord(xt, yt, sw + 8/**/, 0.40/*height*/*wi.height);
	liChalls->setVisible(isChallGui());


	//  Collect  -----
	if (!liCollect)  return;
	sum = 0;  cnt = liCollect->getColumnCount();  sw = 0;
	for (c=0; c < cnt; ++c)  sum += colCol[c];
	for (c=0; c < cnt; ++c)
	{
		w = c==cnt-1 ? 18 : float(colCol[c]) / sum * 0.76/*width*/ * wi.width * 0.97/**/;
		liCollect->setColumnWidthAt(c, w);  sw += w;
	}
	liCollect->setCoord(xt, yt, sw + 8/**/, 0.40/*height*/*wi.height);
	liCollect->setVisible(isCollectGui());
}
