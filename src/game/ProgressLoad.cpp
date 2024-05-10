#include "pch.h"
#include "Def_Str.h"
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


//-----------------------------------------------------------------------------------------------
///  📄 Load  progress*.xml  (once) for all games: champs, challs, collect
//-----------------------------------------------------------------------------------------------
void CGui::Ch_XmlLoad()
{
	/*  stats  */
	float time = 0.f;  int trks = 0;
	for (auto it = data->tracks->times.begin();
		it != data->tracks->times.end(); ++it)
	{
		const string& trk = (*it).first;
		if (trk.substr(0,4) != "Test")
		{
			time += (*it).second;
			++trks;
	}	}
	LogO("Total tracks: "+ toStr(trks) + ", total time: "+ StrTime2(time/60.f)+" h:m");

	
	#if 0  /* test race pos,points */
	float trk = 100.f;
	LogO("Test Time "+StrTime(trk)+"   Pos   Points");
	for (int i=-5; i <= 20; ++i)
	{
		float points = 0.f, t = trk + i*1.f;
		int pos = GetRacePos(t, trk, 1.0f, true, &points);
		LogO("  "+StrTime(t)+"  "+iToStr(pos,2)+"  "+fToStr(points,2,5));
	}
	#endif
	
	
	ProgressLoadChamp();
	ProgressLoadChall();
	ProgressLoadCollect();
}


/// 🏆 Champs
//-----------------------------------------------------------------------------------------------
void CGui::ProgressLoadChamp()
{
	ProgressXml oldprog[2];
	oldprog[0].LoadXml(PATHS::UserConfigDir() + "/progress.xml");
	oldprog[1].LoadXml(PATHS::UserConfigDir() + "/progress_rev.xml");

	int chs = data->champs->all.size();
	
	///  this is for old progress ver loading, from game with newer champs
	///  it resets progress only for champs which ver has changed (or track count)
	//  fill progress
	for (int pr=0; pr < 2; ++pr)
	{
		progress[pr].chs.clear();
		for (int c=0; c < chs; ++c)
		{
			const Champ& ch = data->champs->all[c];
			
			//  find this champ in loaded progress
			bool found = false;  int p = 0;
			ProgressChamp* opc = 0;
			while (!found && p < oldprog[pr].chs.size())
			{
				opc = &oldprog[pr].chs[p];
				//  same name, ver and trks count
				if (opc->name == ch.name && opc->ver == ch.ver &&
					opc->trks.size() == ch.trks.size())
					found = true;
				++p;
			}
			if (!found)
				LogO("|| reset progress for champ: " + ch.name);
			
			ProgressChamp pc;
			pc.name = ch.name;  pc.ver = ch.ver;

			if (found)  //  found progress, points
			{	pc.curTrack = opc->curTrack;
				pc.points = opc->points;
			}

			//  fill tracks
			for (int t=0; t < ch.trks.size(); ++t)
			{
				ProgressTrack pt;
				if (found)  // found track points
					pt.points = opc->trks[t].points;
				pc.trks.push_back(pt);
			}

			progress[pr].chs.push_back(pc);
	}	}
	ProgressSaveChamp(false);  //will be later in guiInit
	
	if (progress[0].chs.size() != data->champs->all.size() ||
		progress[1].chs.size() != data->champs->all.size())
		LogO("|| ERROR: champs and progress sizes differ !");
}


/// 🥇 Challenges
//-----------------------------------------------------------------------------------------------
void CGui::ProgressLoadChall()
{
	ProgressLXml oldpr[2];
	oldpr[0].LoadXml(PATHS::UserConfigDir() + "/progressL.xml");
	oldpr[1].LoadXml(PATHS::UserConfigDir() + "/progressL_rev.xml");

	int chs = data->chall->all.size();
	
	///  this is for old progress ver loading, from game with newer challs
	///  it resets progress only for challs which ver has changed (or track count)
	//  fill progress
	for (int pr=0; pr < 2; ++pr)
	{
		progressL[pr].chs.clear();
		for (int c=0; c < chs; ++c)
		{
			const Chall& ch = data->chall->all[c];
			
			//  find this chall in loaded progress
			bool found = false;  int p = 0;
			ProgressChall* opc = 0;
			while (!found && p < oldpr[pr].chs.size())
			{
				opc = &oldpr[pr].chs[p];
				//  same name, ver and trks count
				if (opc->name == ch.name && opc->ver == ch.ver &&
					opc->trks.size() == ch.trks.size())
					found = true;
				++p;
			}
			if (!found)
				LogO("|| reset progressL for chall: " + ch.name);
			
			ProgressChall pc;
			pc.name = ch.name;  pc.ver = ch.ver;

			if (found)
			{	//  found progress, copy
				pc.curTrack = opc->curTrack;  pc.car = opc->car;
				pc.avgPoints = opc->avgPoints;  pc.totalTime = opc->totalTime;
				pc.avgPos = opc->avgPos;  pc.fin = opc->fin;
			}

			//  fill tracks
			for (int t=0; t < ch.trks.size(); ++t)
			{
				ProgressTrackL pt;
				if (found)  
				{	//  found track points
					const ProgressTrackL& opt = opc->trks[t];  //pt = opt;
					pt.points = opt.points;  pt.time = opt.time;  pt.pos = opt.pos;
				}
				pc.trks.push_back(pt);
			}

			progressL[pr].chs.push_back(pc);
	}	}
	ProgressSaveChall(false);  //will be later in guiInit
	
	if (progressL[0].chs.size() != data->chall->all.size() ||
		progressL[1].chs.size() != data->chall->all.size())
		LogO("|] ERROR: challs and progressL sizes differ !");
}


//  💎 Collection
//-----------------------------------------------------------------------------------------------
void CGui::ProgressLoadCollect()
{
	ProgressCXml oldpr;
	oldpr.LoadXml(PATHS::UserConfigDir() + "/progressC.xml");

	int cols = data->collect->all.size();
	
	///  this is for old progress ver loading, from game with newer collects
	///  it resets progress only for collects which ver has changed (or track count)
	//  fill progress
	{
		progressC.col.clear();
		progressC.icol.clear();

		for (int c=0; c < cols; ++c)
		{
			const Collection& col = data->collect->all[c];
			
			//  find this collect in loaded progress
			bool found = false;  int p = 0;
			ProgressCollect* opc = 0;
			while (!found && p < oldpr.col.size())
			{
				opc = &oldpr.col[p];
				//  same name, ver and trks count
				if (opc->name == col.name && opc->track == col.track && opc->ver == col.ver)
					// && opc->gems.size() == col.gems.size())
					found = true;
				++p;
			}
			if (!found)
				LogO("|| reset progressC for collection: " + col.name);
			
			ProgressCollect pc;
			pc.name = col.name;  pc.track = col.track;  pc.ver = col.ver;
			// pc.gems = col.gems;  //..

			if (found)
			{	//  found progress, copy
				pc.bestTime = opc->bestTime;  pc.fin = opc->fin;
				for (auto& g : opc->gems)
					pc.gems[g.first] = g.second;
			}

			//  fill tracks
			/*for (int t=0; t < col.gems.size(); ++t)
			{
				ProgressTrackL pt;
				if (found)  
				{	//  found track points
					const auto& opt = opc->gems[t];  //pt = opt;
					// pt.points = opt.points;  pt.time = opt.time;  pt.pos = opt.pos;
				}
				pc.gems.push_back(pt);
			}*/

			progressC.col.push_back(pc);
			progressC.icol[pc.name] = progressC.col.size();
	}	}
	ProgressSaveCollect(false);  //will be later in guiInit
	
	if (progressC.col.size() != data->collect->all.size())
		LogO("|] ERROR: collect and progressC sizes differ !");
}
