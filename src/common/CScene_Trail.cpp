#include "pch.h"
#include "settings.h"
#include "CScene.h"
#include "SceneXml.h"
#include "paths.h"
#include "Axes.h"
#include "Road.h"
#include "App.h"
#include "TrackGhost.h"

#include <OgreHlmsPbsPrerequisites.h>
using namespace Ogre;
using namespace std;


///  Trail ghost track  ~~--~-~--
//---------------------------------------------------------------------------------------------------------------

void CScene::CreateTrail(Cam* cam, int id, bool bHideHudTrail)
{
#ifndef SR_EDITOR
	if (!app->pSet->hud_on)  return;
#endif	
	// if (!pSet->trail_show)
		// return;  // fixme crash in replay--
	if (trail[id])
		DestroyTrail(id);

	//  load
	TrackGhost gho;
	int frames = 0;

#ifdef SR_EDITOR
	if (app->pSet->gui.track_user)  return;
	bool rev = 0; //-
	string sTrk = app->pSet->gui.track;
#else
	bool rev = app->pSet->game.track_reversed;
	string sTrk = app->pSet->game.track;
#endif	

	string sRev = rev ? "_r" : "";
	string file = PATHS::TrkGhosts()+"/"+ sTrk + sRev + ".gho";
	if (!PATHS::FileExists(file))
		LogO("Trail trk gho not found: "+file);
	else
	{	gho.LoadFile(file, 0);
		frames = gho.getNumFrames();
	}
	if (frames == 0 || !ter)  return;
	LogO("C--~ CreateTrail");


	//  setup trail road
#ifdef SR_EDITOR
	SplineRoad* tr = new SplineRoad(app);
#else
	SplineRoad* tr = new SplineRoad(app->pGame);
#endif	
	tr->player = id;
	tr->Setup("", 0.7, this, app->mSceneMgr, cam, 100);
	tr->sMtrRoad[0] = "trailRoad";  tr->bMtrRoadTer[0] = false;
	tr->type = RD_Trail;  tr->isLooped = false;

	//tr->g_LenDim0 = 2.f;  tr->g_iWidthDiv0 = 1;  //  high quality
	tr->g_LenDim0 = 3.f;  tr->g_iWidthDiv0 = 1;  //  ok
	//tr->g_LenDim0 = 4.f;  tr->g_iWidthDiv0 = 1;  //  low
	//  mergeLen affects Fps most
	tr->g_MergeLen = 200.f;  tr->g_LodPntLen = 30.f;  tr->bMerge = true;  // ok
	//tr->g_MergeLen = 100.f;  tr->g_LodPntLen = 20.f;  tr->bMerge = true;  // low
	tr->newP.onTer = false;  tr->newP.aType = AT_Both;

	//  params  add p
	const float dd = 5*5, al = 0.6f, down = 0.5f,  // high quality
	// const float dd = 10*10, al = 0.6f, down = 0.5f,  // quality  p each 10 m
	// const float dd = 20*20, al = 0.6f, down = 0.45f, // low, cuts loops etcs
		acc_sens = 30.f, acc_sensw = 45.f;  // color sensitivity


	//  chk vars
	int iNextChk = rev ? road->iChkId1Rev : road->iChkId1;
	int iCk = 1;
	const int inc = (rev ? -1 : 1) * road->iDir;
	const int ncs = road->mChks.size();


	//  add points  - > - -
	Vector3 pos{0,0,0}, old{0,0,0};
	float tOld = 0.f, vOld = 0.f;

	for (int i=0; i < frames; ++i)
	{
		const TrackFrame& fr = gho.getFrame0(i);
		Axes::toOgre(pos, fr.pos);  pos.y -= down;
		if (i==0)  old = pos;

		//  if ghost car in next check, inc trail seg id
		const int np = tr->getNumPoints();

		if (ncs > 0)
		{	const auto& ck = road->mChks[iNextChk];
			if (pos.squaredDistance(ck.pos) < ck.r2)
			{
				//LogO(toStr(np) + " in ck "+toStr(iCk)+" r "+toStr(ck.r)+" nCk "+toStr(tr->newP.nCk));
				++iCk;
				iNextChk = (iNextChk + inc + ncs) % ncs;
		}	}

		float d = old.squaredDistance(pos);
		if (d > dd)
		{	d = sqrt(d);  // todo: on angle changes..

			float fa = sc->GetDepthInFluids(pos);
			if (fa > 0.f)  pos.y += fa + 0.2f;  // above fluids

			const float t = fr.time, dt = t - tOld;
			const float vel = d / dt * 3.6f;  // kmh
			const float acc = (vel - vOld) / dt;

			//LogO(toStr(np) + " v " + fToStr(vel) + " a " + fToStr(acc)+" ic "+toStr(iCk));

			float a = max(-1.f, min(1.f, acc / acc_sens));
			float aw = max(-1.f, min(1.f, acc / acc_sensw));
			//tr->newP.width = 0.5f - a * 0.3f;  // par big
			tr->newP.width = 0.3f - aw * 0.15f;  // par sm
			tr->newP.pos = pos;
			tr->newP.nCk = iCk;

			tr->newP.mRoll = fr.steer/127.f * 10.f;
			// todo: get roll from car in road seg space?..
			//auto q = Axes::toOgre(fr.rot);
			//tr->newP.mRoll = q.getRoll().valueDegrees();  //-

			tr->newP.clr = a > 0.f ?
				Vector4(1.f - a, 1.f, 0.f, al) :  // accel clr
				Vector4(1.f + aw*aw*0.5f, 1.f + a, 0.f, al);  // brake

			if (vel < 600.f)  // fix end jmp err
				tr->Insert(INS_End);
			
			old = pos;  tOld = t;  vOld = vel;
	}	}

	tr->Rebuild(true);
	tr->RebuildRoadInt();
	trail[id] = tr;
	
	bool vis = app->pSet->trail_show && !bHideHudTrail;
	tr->SetVisTrail(vis);
}
