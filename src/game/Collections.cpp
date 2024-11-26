#include "pch.h"
#include "Def_Str.h"
#include "CData.h"
#include "TracksXml.h"
#include "CollectXml.h"
#include "GuiCom.h"
#include "CScene.h"
#include "paths.h"
#include "game.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "MultiList2.h"
// #include "SoundMgr.h"
#include "settings.h"
#include <MyGUI.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


bool CGui::isCollectGui()
{
	return pSet->iMenu == MN_Collect;
}

void CGui::tabCollectType(Tab wp, size_t id)
{
	pSet->collect_type = id;
	CollectListUpdate();
}

#define MAX_COL_TYP 5  //+1, tabs


///  💎📃 Collections list  fill
//----------------------------------------------------------------------------------------------------------------------
void CGui::CollectListUpdate()
{
	const int all = data->collect->all.size();

	std::vector<int> vIds[MAX_COL_TYP];  // cur diff ids for collect [types]
	for (int id = 0; id < all; ++id)
	{
		const Collection& col = data->collect->all[id];
		if (pSet->ch_all || (
			col.diff >= pSet->difficulty &&
			col.diff <= pSet->difficulty+1))
			vIds[col.type].emplace_back(id);
	}

	int cntCur = vIds[pSet->collect_type].size();

	//  hide empty tabs  ----
	const int tabs = tabCollect->getItemCount();
	for (int t = 0; t < tabs; ++t)
	{
		int cnt = vIds[t].size();

		if (t == tabs-1 && !pSet->dev_keys)  // hide Test
			cnt = 0;

		tabCollect->setButtonWidthAt(t, cnt == 0 ? 1 : -1);

		//  if none visible, set 1st nonempty
		if (cntCur == 0 && cnt > 0)
		{
			pSet->collect_type = t;  cntCur = cnt;
			tabCollect->setIndexSelected(t);
	}	}

	FillCollectList(vIds[pSet->collect_type]);
}


void CGui::FillCollectList(std::vector<int> vIds)
{
	liCollect->removeAllItems();

	size_t sel = ITEM_NONE;
 	for (int i : vIds)
	{
		const Collection& col = data->collect->all[i];
		const ProgressCollect& pc = progressC.col[i];

		int id = data->tracks->trkmap[col.track]-1;  if (id < 0)  continue;
		const TrackInfo& ti = data->tracks->trks[id];
		
		liCollect->addItem("", i+1);  int l = liCollect->getItemCount()-1;
		String c = gcom->GetSceneryColor(col.track, 0);
		liCollect->setSubItemNameAt(1,l, c+ col.nameGui);
		liCollect->setSubItemNameAt(2,l, c+ ti.nshrt.c_str());  //c+ col.track);

		liCollect->setSubItemNameAt(3,l, gcom->clrsDiff[col.diff]+ TR("#{Diff"+toStr(col.diff)+"}"));
		// liCollect->setSubItemNameAt(4,l, col.cars.GetStr(data->cars));

		//  collected  cnt / all
		liCollect->setSubItemNameAt(4,l, toStr(pc.gems.size()) +" #50A0F0/ #60A0D0"+ toStr(col.collects.size()) );
		liCollect->setSubItemNameAt(5,l, pc.bestTime > 10000.f ? "-" :
			gcom->clrsDiff[std::min(8,int(pc.bestTime/3.f/60.f))]+ StrTime2(pc.bestTime));

		liCollect->setSubItemNameAt(6,l, " "+ StrPrize(pc.fin+1));
		if (i == pSet->gui.collect_num)
			sel = l;
	}
	liCollect->setIndexSelected(sel);
}


///  Collections list  sel changed,  fill Stages list
//----------------------------------------------------------------------------------------------------------------------
void CGui::listCollectChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE || liCollect->getItemCount() == 0)  return;
	if (id >= liCollect->getItemCount())
		id = 0;  // liCollect->getItemCount()-1;

	int nc = *liCollect->getItemDataAt<int>(id)-1;
	if (nc < 0 || nc >= data->collect->all.size())  {  LogO("Error collect sel > size.");  return;  }

	CarListUpd();  // filter car list

	//  fill stages
	// liStages->removeAllItems();
	for (int i=0; i < ImgTrk; ++i)
		imgTrk[i]->setVisible(0);

	const Collection& col = data->collect->all[nc];

	edChDesc->setCaption(col.descr);
	txtChName->setCaption(col.nameGui);

	imgTrkBig->setImageTexture(col.track+".jpg");
	imgTrkBig->setVisible(1);

	//  fill track tab
	auto trk = col.track;
	if (txTrkName)  txTrkName->setCaption(TR("#{Track}: ") + trk);
	ReadTrkStatsChamp(trk, 0);
	UpdDrivability(trk, 0);
	gcom->sListTrack = trk;  gcom->bListTrackU = 0;
	// CarListUpd();

	UpdCollectDetail(nc);
	UpdGamesTabVis();
}


//  collect allows car
bool CGui::IsCollectCar(String nameGui)
{
	if (!liCollect || liCollect->getIndexSelected()==ITEM_NONE)  return true;

	int id = *liCollect->getItemDataAt<int>(liCollect->getIndexSelected())-1;

	return data->collect->all[id].cars.Allows(data->cars, nameGui);
}


///  Collect start
//---------------------------------------------------------------------
void CGui::btnCollectStart(WP)
{
	if (liCollect->getIndexSelected()==ITEM_NONE)  return;
	pSet->gui.champ_num = -1;
	pSet->gui.chall_num = -1;
	pSet->gui.collect_num = *liCollect->getItemDataAt<int>(liCollect->getIndexSelected())-1;;

	//  if already finished, restart - will loose progress and scores ..
	/*int chId = pSet->gui.collect_num, p = pSet->game.champ_rev ? 1 : 0;
	LogO("|] Starting Collect: "+toStr(chId)+(p?" rev":""));
	ProgressCollect& pc = progressL[p].chs[chId];
	if (pc.curTrack == pc.trks.size())
	{
		LogO("|] Was at 100%, restarting progress.");
		pc.curTrack = 0;  //pc.score = 0.f;
	}*/
	// change btn caption to start/continue/restart ?..

	btnNewGame(0);
}
/*
//  Collect end
void CGui::btnCollectEndClose(WP)
{
	app->mWndChallEnd->setVisible(false);
	app->isFocGui = true;  // show back gui
	toggleGui(false);
}
*/

///  save progressL and update it on gui
void CGui::ProgressSaveCollect(bool upgGui)
{
	progressC.SaveXml(PATHS::UserConfigDir() + "/progressC.xml");
	if (!upgGui)
		return;
	CollectListUpdate();
	listCollectChng(liCollect, liCollect->getIndexSelected());
	FillGameStats();
}


//  Collect details  text
//----------------------------------------------------------------------------------------------------------------------
void CGui::UpdCollectDetail(int id)
{
	const Collection& col = data->collect->all[id];
	String s1,s2,clr;

	//  track  --------
	// clr = gcom->getClrDiff(col.diff);
	// s1 += clr+ TR("#{Difficulty}\n");    s2 += clr+ TR("#{Diff"+toStr(col.diff)+"}")+"\n";
	// s1 += clr+ TR("#{Track}\n");        s2 += col.track+"\n";

	//s1 += "\n";  s2 += "\n";
	if (col.time > 0)
	{
		s1 += TR("  #80F0E0#{Time}");
		if (col.need)  s1 += TR("#F0FFFF  #{Needed}");
		s1 +="\n";

		int p, pp = col.prizes;
		if (pp > 0)
			for (p=0; p <= pp; ++p)
			{	s2 += StrPrize(3-p) +"  "+ StrTime2(col.time + p * col.next)+"   ";  }
		s2 += "\n";
	}
	//  cars  --------
	// s1 += "\n";  s2 += "\n";
	s1 += TR("#F08080  #{Vehicles}\n");        s2 += "#FFA0A0" + col.cars.GetStr(data->cars)+"\n";
	
	//  game  --------
	// s1 += "\n";  s2 += "\n";
	// s1 += TR("#D090E0#{Game}")+"\n";     s2 += "\n";
	#define cmbs(cmb, i)  (i>=0 ? cmb->getItemNameAt(i) : TR("#{Any}"))
	// s1 += TR("#A0B0C0  #{Simulation}\n");  s2 += "#B0C0D0"+ col.sim_mode +"\n";
	s1 += TR("#A090E0  #{Damage}\n");      s2 += "#B090FF"+ cmbs(cmbDamage, col.damage_type) +"\n";
	s1 += TR("#B080C0  #{InputMapRewind}\n"); s2 += "#C090D0"+ cmbs(cmbRewind, col.rewind_type) +"\n";
	//s1 += "\n";  s2 += "\n";
	s1 += TR("#80C0FF  #{Boost}\n");       s2 += "#90D0FF"+ cmbs(cmbBoost, col.boost_type) +"\n";
	// s1 += TR("#6098A0  #{Flip}\n");        s2 += "#7098A0"+ cmbs(cmbFlip, col.flip_type) +"\n";

	//  hud  --------
	//s1 += "\n";  s2 += "\n";
	//bool minimap, chk_arr, chk_beam, trk_ghost;  // deny using it if false

	txColDetail[0]->setCaption(s1);  txColDetail[1]->setCaption(s2);

/*
	//  progress  --------
	s1 = "";  s2 = "";
	const ProgressCollect& pc = progressL[p].chs[id];
	int cur = pc.curTrack, all = data->collect->all[id].trks.size();
	if (cur > 0)
	{
		s1 += TR("#F8FCFF#{Prize}\n");       s2 += StrPrize(pc.fin+1)+"\n";
*/

	//  btn start
	// s1 = cur == all ? TR("#{Restart}") : (cur == 0 ? TR("#{Start}") : TR("#{Continue}"));
	// btStCollect->setCaption(s1);
	// btnCollectRestart->setVisible(cur > 0);
}
