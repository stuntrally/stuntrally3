#include "pch.h"
#include "Def_Str.h"
#include "CData.h"
#include "settings.h"
#include "TracksXml.h"
#include "GuiCom.h"
#include "CScene.h"
#include "paths.h"
#include "game.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "MultiList2.h"
#include "SoundMgr.h"
// #include <OgreTextureManager.h>
#include <MyGUI.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///
void CGui::BackFromChs()
{
	pSet->gui.champ_num = -1;
	pSet->gui.chall_num = -1;
	pSet->gui.collect_num = -1;
	pChall = 0;
	CarListUpd();  // off filtering
}

bool CGui::isChampGui()
{
	return pSet->iMenu == MN_Champ || pSet->iMenu == MN_Tutorial;
}
bool CGui::isChallGui()
{
	return pSet->iMenu == MN_Chall;
}

void CGui::tabChallType(Tab wp, size_t id)
{
	pSet->chall_type = id;
	ChallsListUpdate();
}

#define MAX_CHL_TYP 11


///  🥇📃 Challenges list  fill
//----------------------------------------------------------------------------------------------------------------------
void CGui::ChallsListUpdate()
{
	const int all = data->chall->all.size();

	std::vector<int> vIds[MAX_CHL_TYP];  // cur diff ids for chall [types]
	for (int id = 0; id < all; ++id)
	{
		const Chall& chl = data->chall->all[id];
		if (pSet->ch_all || (
			chl.diff >= pSet->difficulty &&
			chl.diff <= pSet->difficulty+1))  vIds[chl.type].emplace_back(id);
	}

	int cntCur = vIds[pSet->chall_type].size();

	//  hide empty tabs  ----
	const int tabs = tabChall->getItemCount();
	for (int t = 0; t < tabs; ++t)
	{
		int cnt = vIds[t].size();

		if (t == tabs-1 && !pSet->dev_keys)  // hide Test
			cnt = 0;

		tabChall->setButtonWidthAt(t, cnt == 0 ? 1 : -1);

		//  if none visible, set 1st nonempty
		if (cntCur == 0 && cnt > 0)
		{
			pSet->chall_type = t;  cntCur = cnt;
			tabChall->setIndexSelected(t);
	}	}

	FillChallsList(vIds[pSet->chall_type]);
}


void CGui::FillChallsList(std::vector<int> vIds)
{
	const char clrCh[MAX_CHL_TYP][8] = {
	//  0 Rally  1 Scenery  2 Endurance  3 Chase  4 Stunts  5 Extreme  6 Space    7 Special  8 Vehicle  9 Hover    10 Test
		"#A0D0FF","#80FF80","#C0FF60", "#FFC060","#FF8080","#C0A0E0",  "#7070F0", "#60B0FF", "#E0A0C0", "#F0C0F0", "#909090" };

	liChalls->removeAllItems();
	const int p = pSet->gui.champ_rev ? 1 : 0;

	size_t sel = ITEM_NONE;
 	for (int i : vIds)
	{
		const Chall& chl = data->chall->all[i];
		const ProgressChall& pc = progressL[p].chs[i];
		const int ntrks = pc.trks.size(), ct = pc.curTrack;
		const String& clr = clrCh[chl.type];
		//String cars = data->carsXml.colormap[chl.ci->type];  if (cars.length() != 7)  clr = "#C0D0E0";
		
		liChalls->addItem(""/*clr+ toStr(n/10)+toStr(n%10)*/, i+1);  int l = liChalls->getItemCount()-1;
		liChalls->setSubItemNameAt(1,l, clr+ chl.nameGui.c_str());
		liChalls->setSubItemNameAt(2,l, gcom->clrsDiff[chl.diff]+ TR("#{Diff"+toStr(chl.diff)+"}"));
		liChalls->setSubItemNameAt(3,l, chl.cars.GetStr(data->cars));
		
		liChalls->setSubItemNameAt(4,l, gcom->clrsDiff[std::min(8,ntrks*2/3+1)]+ iToStr(ntrks,3));
		liChalls->setSubItemNameAt(5,l, gcom->clrsDiff[std::min(8,int(chl.time/3.f/60.f))]+ StrTime2(chl.time));
		liChalls->setSubItemNameAt(6,l, ct == 0 || ct == ntrks ? "" :
			clr+ fToStr(100.f * ct / ntrks,0,3)+" %");

		liChalls->setSubItemNameAt(7,l, " "+ StrPrize(pc.fin+1));
		liChalls->setSubItemNameAt(8,l, clr+ (pc.fin >= 0 ? fToStr(pc.avgPoints,1,5) : ""));
		if (i == pSet->gui.chall_num)
			sel = l;
	}
	liChalls->setIndexSelected(sel);
}


///  Challenges list  sel changed,  fill Stages list
//----------------------------------------------------------------------------------------------------------------------
void CGui::listChallChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE || liChalls->getItemCount() == 0)  return;

	int nch = *liChalls->getItemDataAt<int>(id)-1;
	if (nch < 0 || nch >= data->chall->all.size())  {  LogO("Error chall sel > size.");  return;  }

	CarListUpd();  // filter car list  todo: off

	//  clear
	liStages->removeAllItems();
	for (int i=0; i < ImgTrk; ++i)
		imgTrk[i]->setVisible(0);
	imgTrkBig->setVisible(0);

	//  fill stages
	int n = 1, p = pSet->gui.champ_rev ? 1 : 0;
	const Chall& ch = data->chall->all[nch];
	int ntrks = ch.trks.size();
	for (int i=0; i < ntrks; ++i,++n)
	{
		const ChallTrack& trk = ch.trks[i];
		float po = progressL[p].chs[nch].trks[i].points;
		
		StageListAdd(n, trk.name, trk.laps, po > 0.f ? "#E0F0FF"+fToStr(po,1,3) : "");
		if (i < ImgTrk)
		{	imgTrk[i]->setImageTexture(trk.name+".jpg");
			imgTrk[i]->setVisible(1);
	}	}
	edChDesc->setCaption(ch.descr);
	txtChName->setCaption(ch.nameGui);

	UpdChallDetail(nch);
}


//  chall allows car
bool CGui::IsChallCar(String name)
{
	if (!liChalls || liChalls->getIndexSelected()==ITEM_NONE)  return true;

	int id = *liChalls->getItemDataAt<int>(liChalls->getIndexSelected())-1;

	return data->chall->all[id].cars.Allows(data->cars, name);
}


///  chall start
//---------------------------------------------------------------------
void CGui::btnChallStart(WP)
{
	if (liChalls->getIndexSelected()==ITEM_NONE)  return;
	pSet->gui.champ_num = -1;
	pSet->gui.chall_num = *liChalls->getItemDataAt<int>(liChalls->getIndexSelected())-1;
	pSet->gui.collect_num = -1;

	//  if already finished, restart - will loose progress and scores ..
	int chId = pSet->gui.chall_num, p = pSet->game.champ_rev ? 1 : 0;
	LogO("|] Starting chall: "+toStr(chId)+(p?" rev":""));
	ProgressChall& pc = progressL[p].chs[chId];
	if (pc.curTrack == pc.trks.size())
	{
		LogO("|] Was at 100%, restarting progress.");
		pc.curTrack = 0;  //pc.score = 0.f;
	}
	// change btn caption to start/continue/restart ?..

	btnNewGame(0);
}

///  stage start / end
//----------------------------------------------------------------------------------------------------------------------
void CGui::btnChallStageStart(WP)
{
	//  check if chall ended
	int chId = pSet->game.chall_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChall& pc = progressL[p].chs[chId];
	const Chall& ch = data->chall->all[chId];
	bool last = pc.curTrack == ch.trks.size();

	LogO("|] This was stage " + toStr(pc.curTrack) + "/" + toStr(ch.trks.size()) + " btn");
	if (last)
	{	//  show end window, todo: start particles..
		app->mWndChallStage->setVisible(false);
		app->mWndChallEnd->setVisible(true);
		
		///  sounds  🔉
		if (iChSnd < 0)
			pGame->snd_fail->start();
		else
		{	pGame->snd_win[iChSnd]->start();
			hud->cup[iChSnd].ShowCup();
		}
		return;
	}

	bool finished = pGame->timer.GetLastLap(0) > 0.f;  //?-
	if (finished)
	{
		LogO("|] Loading next stage.");
		app->mWndChallStage->setVisible(false);
		btnNewGame(0);
	}else
	{
		LogO("|] Starting stage.");
		app->mWndChallStage->setVisible(false);
		app->updMouse();
		pGame->pause = false;
		pGame->timer.waiting = false;
		pGame->timer.end_sim = false;
	}
}

//  stage back
void CGui::btnChallStageBack(WP)
{
	app->mWndChallStage->setVisible(false);
	app->isFocGui = true;  // show back gui
	toggleGui(false);
}

//  chall end
void CGui::btnChallEndClose(WP)
{
	app->mWndChallEnd->setVisible(false);
	app->isFocGui = true;  // show back gui
	toggleGui(false);
}


///  save progressL and update it on gui
void CGui::ProgressSaveChall(bool upgGui)
{
	progressL[0].SaveXml(PATHS::UserConfigDir() + "/progressL.xml");
	progressL[1].SaveXml(PATHS::UserConfigDir() + "/progressL_rev.xml");
	if (!upgGui)
		return;
	ChallsListUpdate();
	listChallChng(liChalls, liChalls->getIndexSelected());
	FillGameStats();
}


///  challenge advance logic
//  caution: called from GAME, 2nd thread, no Ogre stuff here
//----------------------------------------------------------------------------------------------------------------------
void CGui::ChallengeAdvance(float timeCur/*total*/)
{
	int chId = pSet->game.chall_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChall& pc = progressL[p].chs[chId];
	ProgressTrackL& pt = pc.trks[pc.curTrack];
	const Chall& ch = data->chall->all[chId];
	const ChallTrack& trk = ch.trks[pc.curTrack];
	LogO("|] --- Chall end: " + ch.name);

	///  compute track  poins  --------------
	float timeTrk = data->tracks->times[trk.name];
	if (timeTrk < 1.f)
	{	LogO("|] Error: Track has no best time !");  timeTrk = 10.f;	}
	timeTrk *= trk.laps;

	LogO("|] Track: " + trk.name);
	LogO("|] Your time: " + toStr(timeCur));
	LogO("|] Best time: " + toStr(timeTrk));

	float carMul = app->GetCarTimeMul(pSet->game.car[0], pSet->game.sim_mode);
	float points = 0.f;  int pos = 0;

	#if 1  // test score +- sec diff
	for (int i=-2; i <= 2; ++i)
	{
		pos = app->GetRacePos(timeCur + i*2.f, timeTrk, carMul, true, &points);
		LogO("|] var, add time: "+toStr(i*2)+" sec, points: "+fToStr(points,2));
	}
	#endif
	pos = app->GetRacePos(timeCur, timeTrk, carMul, true, &points);

	pt.time = timeCur;  pt.points = points;  pt.pos = pos;

	
	///  Pass Stage  --------------
	bool passed = true, pa;
	if (trk.timeNeeded > 0.f)
	{
		pa = pt.time <= trk.timeNeeded;
		LogO("]] TotTime: " + StrTime(pt.time) + "  Needed: " + StrTime(trk.timeNeeded) + "  Passed: " + (pa ? "yes":"no"));
		passed &= pa;
	}
	if (trk.passPoints > 0.f)
	{
		pa = pt.points >= trk.passPoints;
		LogO("]] Points: " + fToStr(pt.points,1) + "  Needed: " + fToStr(trk.passPoints,1) + "  Passed: " + (pa ? "yes":"no"));
		passed &= pa;
	}
	if (trk.passPos > 0)
	{
		pa = pt.pos <= trk.passPos;
		LogO("]] Pos: " + toStr(pt.pos) + "  Needed: " + toStr(trk.passPos) + "  Passed: " + (pa ? "yes":"no"));
		passed &= pa;
	}
	LogO(String("]] Passed total: ") + (passed ? "yes":"no"));

	
	//  --------------  advance  --------------
	bool last = pc.curTrack+1 == ch.trks.size();
	LogO("|] This was stage " + toStr(pc.curTrack+1) + "/" + toStr(ch.trks.size()));

	pGame->pause = true;
	pGame->timer.waiting = true;
	pGame->timer.end_sim = true;

	//  show stage end [window]
	ChallFillStageInfo(true);  // cur track
	app->mWndChallStage->setVisible(true);
	app->updMouse();

	//  sound  🔉
	if (passed)
		pGame->snd_stage->start();
	else
		pGame->snd_fail->start();


	if (!last || (last && !passed))
	{
		if (pc.curTrack == 0)  // save picked car
			pc.car = pSet->game.car[0];

		if (passed)
			pc.curTrack++;  // next stage

		ProgressSaveChall();
		return;
	}


	//  challenge end
	//-----------------------------------------------------------------------------------------------

	//  compute avg
	float avgPos = 0.f, avgPoints = 0.f, totalTime = 0.f;
	int ntrks = pc.trks.size();
	for (int t=0; t < ntrks; ++t)
	{
		const ProgressTrackL& pt = pc.trks[t];
		totalTime += pt.time;  avgPoints += pt.points;  avgPos += pt.pos;
	}
	avgPoints /= ntrks;  avgPos /= ntrks;

	//  save
	pc.curTrack++;
	pc.totalTime = totalTime;  pc.avgPoints = avgPoints;  pc.avgPos = avgPos;

	LogO("|] Chall finished");
	String s = "\n\n"+
		TR("#C0D0F0#{Challenge}") + ": #C0D0FF" + ch.nameGui +"\n";
	#define sPass(pa)  (pa ? TR("  #00FF00#{Passed}") : TR("  #FF8000#{DidntPass}"))


	///  Pass Challenge  --------------
	String ss;
	passed = true;
	int prize = -1, pp = ch.prizes;

	//  time  1:00  ---------------
	if (ch.totalTime > 0.f)
	{
		pa = pc.totalTime <= ch.totalTime;
		// for p <= pp ..

		LogO("]] TotalTime: "+StrTime(pc.totalTime)+"  Needed: "+StrTime(ch.totalTime)+"  Passed: "+(pa ? "yes":"no"));
		ss += TR("#D8C0FF#{TBTime}")+": "+StrTime(pc.totalTime)+"   /  "+StrTime(ch.totalTime) + sPass(pa) +"\n";
		passed &= pa;
	}

	//  position  1st  ---------------
	if (ch.avgPos > 0.f)
	{
		ss += TR("#D8C0FF#{TBPosition}")+": "+fToStr(pc.avgPos,2,5)+"   /";
		String sp;  bool pa0;
		for (p=0; p <= pp; ++p)
		{
			float pass = ch.avgPos + ciAddPos[p] * ch.factor;
			pa = pc.avgPos <= pass;
			if (pa && p > prize)  prize = p;
			
			LogO("]] AvgPos: "+fToStr(pc.avgPos,1)+"  Needed: "+fToStr(pass,1)+"  Prize: "+toStr(p)+"  Passed: "+(pa ? "yes":"no"));
			sp += "  "+clrPrize[2-pp+ p+1]+ fToStr(pass,1,3);
			if (p == 0)
			{	passed &= pa;  pa0 = pa;  }
		}
		ss += sp +" "+ sPass(pa0) +"\n";
	}

	//  points  10.0  ---------------
	if (ch.avgPoints > 0.f)
	{
		ss += TR("#D8C0FF#{TBPoints}")+": "+fToStr(pc.avgPoints,2,5)+"   /";
		String sp;  bool pa0;
		for (p=0; p <= pp; ++p)
		{
			float pass = ch.avgPoints - cfSubPoints[p] * ch.factor;
			pa = pc.avgPoints >= pass;
			if (pa && p > prize)  prize = p;

			LogO("]] AvgPoints: "+fToStr(pc.avgPoints,1)+"  Needed: "+fToStr(pass,1)+"  Prize: "+toStr(p)+"  Passed: "+(pa ? "yes":"no"));
			sp = "  "+clrPrize[2-pp+ p+1]+ fToStr(pass,1,3) + sp;
			if (p == 0)
			{	passed &= pa;  pa0 = pa;  }  // lost if didn't pass worst prize
		}
		ss += sp +" "+ sPass(pa0) +"\n";
	}else  //if (passed)  // write points always on end
		ss += TR("#C0E0FF#{TBPoints}")+": "+fToStr(pc.avgPoints,2,5) /*+ sPass(pa)*/ +"\n";

	LogO(String("]] Passed total: ") + (passed ? "yes":"no") +"  Prize: "+toStr(prize));
	
	
	//  end
	pc.fin = passed ? prize : -1;
	if (passed)
		s += TR("\n#E0F0F8#{Prize}: ") + StrPrize(pc.fin+1)+"\n";
	s += "\n"+ss;
	
	ProgressSaveChall();

	//  save which sound to play 🔉
	if (!passed)
		iChSnd = -1;
	else
		iChSnd = std::min(2, std::max(0, prize));  // ch.diff ?


	//  upd chall end [window]
	imgChallFail->setVisible(!passed);
	imgChallCup->setVisible( passed);  const int ui[3] = {2,3,4};
	imgChallCup->setImageCoord(IntCoord(ui[std::min(2, std::max(0, pc.fin))]*128,0,128,256));

	txChallEndC->setCaption(passed ? TR("#{ChampEndCongrats}") : "");
	txChallEndF->setCaption(passed ? TR("#{ChallEndFinished}") : TR("#{Finished}"));

	edChallEnd->setCaption(s);
	// app->mWndChallEnd->setVisible(true);  // show after stage end
	
	LogO("|]");
}


//  stage wnd text
//----------------------------------------------------------------------------------------------------------------------
void CGui::ChallFillStageInfo(bool finished)
{
	int chId = pSet->game.chall_num, p = pSet->game.champ_rev ? 1 : 0;
	const ProgressChall& pc = progressL[p].chs[chId];
	const ProgressTrackL& pt = pc.trks[pc.curTrack];
	const Chall& ch = data->chall->all[chId];
	const ChallTrack& trk = ch.trks[pc.curTrack];
	bool last = pc.curTrack+1 == ch.trks.size();

	String s =
		"#80FFE0"+ TR("#{Challenge}") + ":  " + ch.nameGui + "\n\n" +
		"#80FFC0"+ TR("#{Stage}") + ":  " + toStr(pc.curTrack+1) + " / " + toStr(ch.trks.size()) + "\n" +
		"#80FF80"+ TR("#{Track}") + ":  " + trk.name + "\n\n";

	if (!finished)  // track info at start
	{
		int id = data->tracks->trkmap[trk.name];
		if (id > 0)
		{
			const TrackInfo* ti = &data->tracks->trks[id-1];
			s += "#A0D0FF"+ TR("#{Difficulty}:  ") + gcom->clrsDiff[ti->diff] + TR("#{Diff"+toStr(ti->diff)+"}") + "\n";
			if (app->scn->road)
			{	Real len = app->scn->road->st.Length*0.001f * (pSet->show_mph ? 0.621371f : 1.f);
				s += "#A0D0FF"+ TR("#{Distance}:  ") + "#B0E0FF" + 
					fToStr(len, 1,4) + (pSet->show_mph ? TR(" #{UnitMi}") : TR(" #{UnitKm}")) + "\n\n";
		}	}
	}

	if (finished)  // stage
	{
		s += "#80C0FF"+TR("#{Finished}.") + "\n\n";
		
		///  Pass Stage  --------------
		bool passed = true, pa;
		if (trk.timeNeeded > 0.f)
		{
			pa = pt.time <= trk.timeNeeded;
			s += TR("#D8C0FF#{TBTime}: ") + StrTime(pt.time) + "  / " + StrTime(trk.timeNeeded) + sPass(pa) +"\n";
			passed &= pa;
		}
		if (trk.passPoints > 0.f)
		{
			pa = pt.points >= trk.passPoints;
			s += TR("#D8C0FF#{TBPoints}: ") + fToStr(pt.points,1) + "  / " + fToStr(trk.passPoints,1) + sPass(pa) +"\n";
			passed &= pa;
		}
		if (trk.passPos > 0)
		{
			pa = pt.pos <= trk.passPos;
			s += TR("#D8C0FF#{TBPosition}: ") + toStr(pt.pos) + "  / " + toStr(trk.passPos) + sPass(pa) +"\n";
			passed &= pa;
		}
		
		if (passed)	s += "\n\n"+TR(last ? "#00FF00#{Continue}." : "#00FF00#{NextStage}.");
		else		s += "\n\n"+TR("#FF6000#{RepeatStage}.");
	}
	else
	{	///  Pass needed  --------------
		s += "#F0F060"+ TR("#{Needed}") +"\n";
		if (trk.timeNeeded > 0.f)	s += TR("  #D8C0FF#{TBTime}: ") + StrTime(trk.timeNeeded) +"\n";
		if (trk.passPoints > 0.f)	s += TR("  #D8C0FF#{TBPoints}: ") + fToStr(trk.passPoints,1) +"\n";
		if (trk.passPos > 0)		s += TR("  #D8C0FF#{TBPosition}: ") + toStr(trk.passPos) +"\n";
		auto rd = app->scn->road;
		if (rd)
			s += "\n#DDDDBB"+ rd->sTxtAdvice + "\n#ABDDAB"+ rd->sTxtDescr;
	}

	edChallStage->setCaption(s);
	btChallStage->setCaption(finished ? TR("#{Continue}") : TR("#{Start}"));
	
	//  preview image at start
	if (!finished)
	{
		String path = gcom->PathListTrkPrv(0, trk.name);
		app->prvStCh.Load(path+"view.jpg");
	}
}


//  chall details  (gui tab Stages)
//----------------------------------------------------------------------------------------------------------------------
void CGui::UpdChallDetail(int id)
{
	const Chall& ch = data->chall->all[id];
	int ntrks = ch.trks.size();
	int p = pSet->gui.champ_rev ? 1 : 0;
	
	String s1,s2,clr;
	//s1 += "\n";  s2 += "\n";

	//  track  --------
	clr = gcom->getClrDiff(ch.diff);
	s1 += clr+ TR("#{Difficulty}\n");    s2 += clr+ TR("#{Diff"+toStr(ch.diff)+"}")+"\n";

	clr = gcom->getClrDiff(ntrks * 2/3 +1);
	s1 += clr+ TR("#{Tracks}\n");        s2 += clr+ toStr(ntrks)+"\n";

	//s1 += "\n";  s2 += "\n";
	clr = gcom->getClrDiff(ch.time /3.f/60.f);
	s1 += TR("#80F0E0#{Time} [#{TimeMS}.]\n"); s2 += "#C0FFE0"+clr+ StrTime2(ch.time)+"\n";

	//  cars  --------
	s1 += "\n";  s2 += "\n";
	s1 += TR("#F08080#{Vehicles}\n");       s2 += "#FFA0A0" + ch.cars.GetStr(data->cars)+"\n";
	if (ch.carChng)
	{	s1 += TR("#C0B0B0#{CarChange}\n");  s2 += TR("#A0B8A0#{Allowed}")+"\n";  }

	
	//  game  --------
	s1 += "\n";  s2 += "\n";
	s1 += TR("#D090E0#{Game}")+"\n";     s2 += "\n";
	#define cmbs(cmb, i)  (i>=0 ? cmb->getItemNameAt(i) : TR("#{Any}"))
	s1 += TR("#A0B0C0  #{Simulation}\n");  s2 += "#B0C0D0"+ ch.sim_mode +"\n";
	s1 += TR("#A090E0  #{Damage}\n");      s2 += "#B090FF"+ cmbs(cmbDamage, ch.damage_type) +"\n";
	s1 += TR("#B080C0  #{InputMapRewind}\n"); s2 += "#C090D0"+ cmbs(cmbRewind, ch.rewind_type) +"\n";
	//s1 += "\n";  s2 += "\n";
	s1 += TR("#80C0FF  #{Boost}\n");       s2 += "#90D0FF"+ cmbs(cmbBoost, ch.boost_type) +"\n";
	s1 += TR("#6098A0  #{Flip}\n");        s2 += "#7098A0"+ cmbs(cmbFlip, ch.flip_type) +"\n";

	//  hud  --------
	//s1 += "\n";  s2 += "\n";
	//bool minimap, chk_arr, chk_beam, trk_ghost;  // deny using it if false

	txtCh->setCaption(s1);  valCh->setCaption(s2);


	//  pass challenge  --------
	s1 = "";  s2 = "";
	if (ch.totalTime > 0.f || ch.avgPoints > 0.f || ch.avgPos > 0.f)
	{
		s1 += "\n";  s2 += "\n";  int p, pp = ch.prizes;
		s1 += TR("#F0F060#{Needed} - #{Challenge}")+"\n";   s2 += "\n";
		s1 += "#D8C0FF";  s2 += "#F0D8FF";
		if (ch.avgPoints > 0.f){	s1 += TR("  #{TBPoints}\n");
									for (p=0; p <= pp; ++p)  s2 += clrPrize[2-pp+ p+1]+
											fToStr(ch.avgPoints - cfSubPoints[p] * ch.factor ,1,3)+"  ";
									s2 += "\n";  }
		if (ch.avgPos > 0.f)  {		s1 += TR("  #{TBPosition}\n");
									for (p=0; p <= pp; ++p)  s2 += clrPrize[2-pp+ p+1]+
											fToStr(ch.avgPos + ciAddPos[p] * ch.factor ,1,3)+"  ";
									s2 += "\n";  }
		if (ch.totalTime > 0.f){	s1 += TR("  #{TBTime}\n");
									s2 += StrTime(ch.totalTime)+"\n";  }
	}
	txtChP[1]->setCaption(s1);  valChP[1]->setCaption(s2);
	
	//  pass stage  --------
	s1 = "";  s2 = "";
	size_t t = liStages->getIndexSelected();
	if (t != ITEM_NONE)
	{
		const ChallTrack& trk = ch.trks[t];
		if (trk.timeNeeded > 0.f || trk.passPoints > 0.f || trk.passPos > 0)
		{
			s1 += "\n";  s2 += "\n";
			s1 += TR("#FFC060#{Needed} - #{Stage}")+"\n";   s2 += "\n";
			s1 += "#D8C0FF";   s2 += "#F0D8FF";
			if (trk.passPoints > 0.f){	s1 += TR("  #{TBPoints}\n");    s2 += fToStr(trk.passPoints,2,5)+"\n";  }
			if (trk.passPos > 0.f)  {	s1 += TR("  #{TBPosition}\n");  s2 += fToStr(trk.passPos,2,5)+"\n";  }
			if (trk.timeNeeded > 0.f){	s1 += TR("  #{TBTime}\n");      s2 += StrTime(trk.timeNeeded)+"\n";  }
	}	}
	txtChP[0]->setCaption(s1);  valChP[0]->setCaption(s2);


	//  progress  --------
	s1 = "";  s2 = "";
	const ProgressChall& pc = progressL[p].chs[id];
	int cur = pc.curTrack, all = data->chall->all[id].trks.size();
	if (cur > 0)
	{
		s1 += TR("#B0FFFF#{Progress}\n");    s2 += "#D0FFFF"+(cur == all ? TR("#{Finished}").asUTF8() : fToStr(100.f * cur / all,0,3)+" %")+"\n";
		s1 += TR("#F8FCFF#{Prize}\n");       s2 += StrPrize(pc.fin+1)+"\n";
		#define clrP(b)  if (b)  {  s1 += "#C8D0F0";  s2 += "#E0F0FF";  }else{  s1 += "#80A0C0";  s2 += "#90B0D0";  }
		s1 += "\n";  s2 += "\n";  clrP(ch.avgPoints > 0.f);
		s1 += TR("  #{TBPoints}\n");    s2 += fToStr(pc.avgPoints,2,5)+"\n";  clrP(ch.avgPos > 0.f);
		s1 += TR("  #{TBPosition}\n");  s2 += fToStr(pc.avgPos,2,5)+"\n";  clrP(ch.totalTime > 0.f);
		s1 += TR("  #{TBTime}\n");      s2 += StrTime(pc.totalTime)+"\n";
	}
	txtChP[2]->setCaption(s1);  valChP[2]->setCaption(s2);


	//  btn start
	s1 = cur == all ? TR("#{Restart}") : (cur == 0 ? TR("#{Start}") : TR("#{Continue}"));
	btStChall->setCaption(s1);
	btChRestart->setVisible(cur > 0);
}
