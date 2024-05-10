#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "CData.h"
#include "TracksXml.h"
#include "ChampsXml.h"
#include "ChallengesXml.h"
#include "CollectXml.h"
#include "GuiCom.h"
#include "paths.h"
#include "game.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "MultiList2.h"
#include <MyGUI.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///______________________________________________________________________________________________
///
///  load 🏆 championship or 🥇 challenge track
///______________________________________________________________________________________________
void App::Ch_NewGame()
{
	//  range
	int& iChamp = pSet->game.champ_num;
	int& iChall = pSet->game.chall_num;
	int& iCollect = pSet->game.collect_num;
	if (iChamp   >= (int)data->champs->all.size())   iChamp = -1;
	if (iChall   >= (int)data->chall->all.size())    iChall = -1;
	if (iCollect >= (int)data->collect->all.size())  iCollect = -1;

	pGame->timer.end_sim = false;

	//---------------------------------------------------------
	if (iChall >= 0)
	{
		///  🥇 challenge stage
		int p = pSet->game.champ_rev ? 1 : 0;
		ProgressChall& pc = gui->progressL[p].chs[iChall];
		Chall& chl = data->chall->all[iChall];
		if (pc.curTrack >= chl.trks.size())  pc.curTrack = 0;  // restart
		const ChallTrack& trk = chl.trks[pc.curTrack];
		gui->pChall = &chl;  // set
		
		pSet->game.track = trk.name;  pSet->game.track_user = 0;
		pSet->game.track_reversed = pSet->game.champ_rev ? !trk.reversed : trk.reversed;
		pSet->game.num_laps = trk.laps;

		pSet->game.sim_mode = chl.sim_mode;
		pSet->game.damage_type = chl.damage_type;
		pSet->game.damage_dec = chl.dmg_lap;
		
		pSet->game.boost_type = chl.boost_type;
		pSet->game.flip_type = chl.flip_type;
		pSet->game.rewind_type = chl.rewind_type;
		pSet->game.BoostDefault();  //

		//  car not set, and not allowed in chall
		if (!gui->IsChallCar(pSet->game.car[0]))  // pick last
		{	int cnt = gui->carList->getItemCount();
			if (cnt > 0)
				pSet->game.car[0] = gui->carList->getItemNameAt(std::min(0,cnt-1)).substr(7);
			else
			{	LogO("Error: Challenge cars empty!");  return;  }
		}
		//  set picked car when continuing
		if (pc.curTrack > 0 && !pc.car.empty() && !chl.carChng)
			pSet->game.car[0] = pc.car;

		//TODO: ?challenge icons near denied combos,chkboxes
		
		pSet->game.trees = 1.5f;  //-
		pSet->game.collis_veget = true;
		pSet->game.dyn_objects = true;  //-

		pGame->pause = true;  // wait for stage wnd close
		pGame->timer.waiting = true;
	}
	//---------------------------------------------------------
	else if (iChamp >= 0)
	{
		///  🏆 championship stage
		int p = pSet->game.champ_rev ? 1 : 0;
		ProgressChamp& pc = gui->progress[p].chs[iChamp];
		const Champ& ch = data->champs->all[iChamp];
		if (pc.curTrack >= ch.trks.size())  pc.curTrack = 0;  // restart
		const ChampTrack& trk = ch.trks[pc.curTrack];
		pSet->game.track = trk.name;  pSet->game.track_user = 0;
		pSet->game.track_reversed = pSet->game.champ_rev ? !trk.reversed : trk.reversed;
		pSet->game.num_laps = trk.laps;

		pSet->game.boost_type = 1;  // from trk.?
		pSet->game.flip_type = 2;
		pSet->game.rewind_type = 1; // gui.rewind_type?
		pSet->game.BoostDefault();  //
		//pSet->game.damage_dec = 40.f;  //?
		//pSet->game.trees = 1.f;  // >=1 ?
		//pSet->game.collis_veget = true;

		pGame->pause = true;  // wait for stage wnd close
		pGame->timer.waiting = true;
	}
	//---------------------------------------------------------
	else if (iCollect >= 0)
	{
		///  💎 collection
		// ProgressCollect& pc = gui->progressC.col[iCollect];
		const Collection& col = data->collect->all[iCollect];
		pSet->game.track = col.track;  pSet->game.track_user = 0;
		pSet->game.track_reversed = 0;
		pSet->game.num_laps = 0;

		pSet->game.boost_type = col.boost_type;
		pSet->game.flip_type = col.flip_type;
		pSet->game.rewind_type = col.rewind_type;
		pSet->game.BoostDefault();  //

		//  car not set, or not allowed in collect
		if (!gui->IsCollectCar(pSet->game.car[0]))  // pick last
		{	int cnt = gui->carList->getItemCount();
			if (cnt > 0)
				pSet->game.car[0] = gui->carList->getItemNameAt(std::min(0,cnt-1)).substr(7);
			else
			{	LogO("Error: Collect cars empty!");  return;  }
		}

		pGame->pause = false;  // no wait
		pGame->timer.waiting = false;
		// pSet->game.trees = 1.5f;  // ..?!
		pSet->game.collis_veget = true;
	}
	else
	{	pGame->pause = false;  // single race
		pGame->timer.waiting = false;
		// gui->career.SaveXml(PATHS::UserConfigDir() + "/career.xml");
	}
}

///  ⏱️ car time mul
//-----------------------------------------------------------------------------------------------
float App::GetCarTimeMul(const string& car, const string& sim_mode)
{
	//  car factor (time mul, for less power)
	float carMul = 1.f;
	int id = data->cars->carmap[car];
	if (id > 0)
	{	const CarInfo& ci = data->cars->cars[id-1];
		bool easy = sim_mode == "easy";
		carMul = easy ? ci.easy : ci.norm;
	}
	return carMul;
}

///  🏁 compute race position,  basing on car and track time
//-----------------------------------------------------------------------------------------------

int App::GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints)
{
	//  magic factor: seconds needed for 1 second of track time for 1 race place difference
	//  eg. if track time is 3min = 180 sec, then 180*magic = 2.16 sec
	//  and this is the difference between car race positions (1 and 2, 2 and 3 etc)
	//  0.006 .. 0.0012				// par
	//float time1pl = magic * timeTrk;

	//  if already driving at start, add 1 sec (times are for 1st lap)
	float timeC = timeCur + (coldStart ? 0 : 1);
	float time = timeC * carTimeMul;

	float place = (time - timeTrk)/timeTrk / data->cars->magic;
	// time = (place * magic * timeTrk + timeTrk) / carTimeMul;  //todo: show this in lists and hud..
	if (pPoints)
		*pPoints = std::max(0.f, (20.f - place) * 0.5f);

	int plc = place < 1.f ? 1 : std::min(30, (int)( floor(place +1.f) ));
	return plc;
}


///  🪟 upd Gui vis  tutor champ chall
//-----------------------------------------------------------------------------------------------
void CGui::UpdChsTabVis()
{
	if (!liChamps || !tabChamp || !btStChamp || !btStCollect)  return;
	bool game = pSet->iMenu == MN_Single,   champ = pSet->iMenu == MN_Champ,
		tutor = pSet->iMenu == MN_Tutorial, chall = pSet->iMenu == MN_Chall,
		collect = pSet->iMenu == MN_Collect, career = pSet->iMenu == MN_Career;
	bool chAny = tutor || champ || chall || collect || career;

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

	if (collect)  CollectListUpdate();  else
	if (chall)  ChallsListUpdate();  else
		ChampsListUpdate();

	//if (pSet->inRace == Race_Single)
	//	BackFromChs();
	
	edChInfo->setCaption(
		career ? "" :
		collect ? TR("#{CollectInfo}") :
		chall ? TR("#{ChallInfo2}")+"\n"+TR("#{ChallInfo}") :
		tutor ? TR("#{TutorInfo}")+"\n"+TR("#{ChampInfo}") :
				TR("#{ChampInfo2}")+"\n"+TR("#{ChampInfo}"));
	
	btChRestart->setVisible(false);
}

void CGui::btnChampInfo(WP)
{
	pSet->champ_info = !pSet->champ_info;
	if (edChInfo)  edChInfo->setVisible(pSet->champ_info);
}


///  📃 Stages list  add item
//-----------------------------------------------------------------------------------------------
void CGui::StageListAdd(int n, String name, int laps, String progress)
{
	String clr = gcom->GetSceneryColor(name);
	liStages->addItem(clr+ toStr(n/10)+toStr(n%10), 0);  int l = liStages->getItemCount()-1;

	int id = data->tracks->trkmap[name]-1;  if (id < 0)  return;
	const TrackInfo& ti = data->tracks->trks[id];

	liStages->setSubItemNameAt(1,l, clr+ ti.nshrt.c_str()); //name.c_str());

	float carMul = app->GetCarTimeMul(pSet->game.car[0], pSet->game.sim_mode);
	float time = (data->tracks->times[name] * laps /*laps > 1 -1*/) / carMul;

	liStages->setSubItemNameAt(2,l, clr+ ti.scenery);
	liStages->setSubItemNameAt(3,l, gcom->getClrDiff(ti.diff)+ TR("#{Diff"+toStr(ti.diff)+"}"));
	liStages->setSubItemNameAt(4,l, "#60C0A0  "+toStr(laps));
	liStages->setSubItemNameAt(5,l, "#80C0F0 "+StrTime2(time));
	liStages->setSubItemNameAt(6,l, progress);
}

///  📃 Stages list  sel changed,  update Track info
//-----------------------------------------------------------------------------------------------
void CGui::listStageChng(MyGUI::MultiList2* li, size_t pos)
{
	if (valStageNum)  valStageNum->setVisible(pos!=ITEM_NONE);
	if (pos==ITEM_NONE)  return;
	
	string trk;  bool rev=false;  int all=1;
	/*if (isCollectGui())
	else*/ if (isChallGui())
	{	if (liChalls->getIndexSelected()==ITEM_NONE)  return;
		int nch = *liChalls->getItemDataAt<int>(liChalls->getIndexSelected())-1;
		if (nch >= data->chall->all.size())  {  LogO("Error chall sel > size.");  return;  }

		const Chall& ch = data->chall->all[nch];
		if (pos >= ch.trks.size())  {  LogO("Error stage sel > tracks.");  return;  }
		trk = ch.trks[pos].name;  rev = ch.trks[pos].reversed;  all = ch.trks.size();

		UpdChallDetail(nch);  // stage pass upd txt
	}else
	{	if (liChamps->getIndexSelected()==ITEM_NONE)  return;
		int nch = *liChamps->getItemDataAt<int>(liChamps->getIndexSelected())-1;
		if (nch >= data->champs->all.size())  {  LogO("Error champ sel > size.");  return;  }

		const Champ& ch = data->champs->all[nch];
		if (pos >= ch.trks.size())  {  LogO("Error stage sel > tracks.");  return;  }
		trk = ch.trks[pos].name;  rev = ch.trks[pos].reversed;  all = ch.trks.size();
	}
	if (pSet->gui.champ_rev)  rev = !rev;

	if (txTrkName)  txTrkName->setCaption(TR("#{Track}: ") + trk);
	ReadTrkStatsChamp(trk, rev);
	UpdDrivability(trk, 0);
	gcom->sListTrack = trk;  gcom->bListTrackU = 0;
	CarListUpd();
	if (valStageNum)  valStageNum->setCaption(toStr(pos+1) +" / "+ toStr(all));
}


//  stage loaded
void CGui::Ch_LoadEnd()
{
	if (pSet->game.champ_num >= 0)
	{
		ChampFillStageInfo(false);
		app->mWndChampStage->setVisible(true);
		app->updMouse();
	}
	if (pSet->game.chall_num >= 0)
	{
		ChallFillStageInfo(false);
		app->mWndChallStage->setVisible(true);
		app->updMouse();
	}
}

//  Stages gui tab  <  >
void CGui::btnStageNext(WP)
{
	size_t id = liStages->getIndexSelected(), all = liStages->getItemCount();
	if (all == 0)  return;
	if (id == ITEM_NONE)  id = 0;
	else
		id = (id +1) % all;
	liStages->setIndexSelected(id);
	listStageChng(liStages, id);
}

void CGui::btnStagePrev(WP)
{
	size_t id = liStages->getIndexSelected(), all = liStages->getItemCount();
	if (all == 0)  return;
	if (id == ITEM_NONE)  id = 0;
	id = (id + all -1) % all;
	liStages->setIndexSelected(id);
	listStageChng(liStages, id);
}


//  ❌ restart progress curtrack
void CGui::btnChRestart(WP)
{
	int p = pSet->game.champ_rev ? 1 : 0;
	if (pSet->iMenu == MN_Tutorial || pSet->iMenu == MN_Champ)
	{
		if (liChamps->getIndexSelected()==ITEM_NONE)  return;
		int chId = *liChamps->getItemDataAt<int>(liChamps->getIndexSelected())-1;
		ProgressChamp& pc = progress[p].chs[chId];
		pc.curTrack = 0;  ChampsListUpdate();
	}
	else if (pSet->iMenu == MN_Chall)
	{
		if (liChalls->getIndexSelected()==ITEM_NONE)  return;
		int chId = *liChalls->getItemDataAt<int>(liChalls->getIndexSelected())-1;
		ProgressChall& pc = progressL[p].chs[chId];
		pc.curTrack = 0;  ChallsListUpdate();
	}
}
