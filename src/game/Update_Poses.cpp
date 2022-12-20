#include "RenderConst.h"
#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "Axes.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "SceneXml.h"
#include "CScene.h"
#include "FollowCamera.h"
#include "Road.h"
#include "game.h"
#include "quickprof.h"
#include "dbl.h"
// #include "gameclient.hpp"
#include "SoundMgr.h"
// #include "Slider.h"
// #include "SplitScreen.h"
#include "settings.h"
#include "CarModel.h"
#include <Ogre.h>
#include <MyGUI.h>
// #include <OgreCamera.h>
// #include <OgreQuaternion.h>
// #include <OgreSceneNode.h>
// #include <OgreVector3.h>
using namespace Ogre;


//  newPoses - Get new car pos from game
//  caution: called from GAME, 2nd thread, no Ogre stuff here
/// Todo: move arrow update and ChampionshipAdvance to updatePoses ...
//---------------------------------------------------------------------------------------------------------------
void App::newPoses(float time)  // time only for camera update
{
	if (!pGame || bLoading || pGame->cars.empty() /*|| carPoses.empty() || iCurPoses.empty()*/)
		return;
	PROFILER.beginBlock(".newPos ");

	double rplTime = pGame->timer.GetReplayTime(0);  // from start
	double lapTime = pGame->timer.GetPlayerTime(0);
	double rewTime = /*pSet->rpl_ghostrewind*/1 ? pGame->timer.GetRewindTimeGh(0) : lapTime;

	//  iterate through all car models and set new pos info (from vdrift sim or replay)
	CarModel* carM0 = carModels[0];
	for (int c = 0; c < carModels.size(); ++c)
	{
		CarModel* carM = carModels[c];
		bool bGhost = carM->isGhost();
		CAR* pCar = carM->pCar;

		PosInfo pi;  // new, to fill data


		//  car perf test  logic
		//--------------------------------
		// if (bPerfTest && c==0)
		// 	newPerfTest(time);


		//  play  get data from replay / ghost
		///-----------------------------------------------------------------------
		if (bGhost)
		{	///>>  🏞️👻 track's ghost
			if (carM->isGhostTrk())
			{
				TrackFrame tf;       // par: sec after, 1st lap
				float lap1 = pGame->timer.GetCurrentLap(0) > 0 ? 2.f : 0.f;
				bool ok = ghTrk.GetFrame(rewTime + lap1, &tf);
				//  car
				Axes::toOgre(pi.pos, tf.pos);
				pi.rot = Axes::toOgre(tf.rot);
				pi.braking = tf.brake;
				pi.steer = tf.steer / 127.f;
				//pi.fboost = 0.f;  pi.speed = 0.f;  pi.percent = 0.f;
				//pi.fHitTime = 0.f;  pi.fParIntens = 0.f;  pi.fParVel = 0.f;
				//pi.vHitPos = Vector3::ZERO;  pi.vHitNorm = Vector3::UNIT_Y;

				//  wheels
				//dynamics.SetSteering(state.steer, pGame->GetSteerRange());  //peers can have other game settings..
				for (int w=0; w < carM->numWheels; ++w)
				{
					MATHVECTOR<float,3> whP = carM->whPos[w];
					whP[2] += 0.05f;  // up
					tf.rot.RotateVector(whP);
					Axes::toOgre(pi.whPos[w], tf.pos + whP);

					if (w < 2)  // front steer
					{	float a = (pi.steer * carM->maxangle) * -PI_d/180.f;
						QUATERNION<float> q;  q.Rotate(a, 0,0,1);
						pi.whRot[w] = Axes::toOgreW(tf.rot * carM->qFixWh[w%2] * q);
					}else
						pi.whRot[w] = Axes::toOgreW(tf.rot * carM->qFixWh[w%2]);
				}
			}else  ///>>  👻 ghost
			{
				ReplayFrame2 gf;
				float ti = std::min((float)rewTime, ghPlay.GetTimeLength());
				bool ok = ghPlay.GetFrame(ti, &gf, 0);
				if (ok)
					pi.FromRpl2(&gf, 0);

				if (carM->vType == V_Sphere)
				{	//  weird fix, mini rot
					pi.carY = Vector3::UNIT_Y;
					pi.hov_roll = -pi.hov_roll;
				}
			}
		}
		else  ///>>  📽️ replay
		if (bRplPlay)
		{
			#ifdef DEBUG
			assert(c < frm.size());
			#endif
			ReplayFrame2& rf = frm[c];  // frm also used in car.cpp for sounds
			if (c < replay.header.numPlayers)
			{
				bool ok = replay.GetFrame(rplTime, &rf, c);
				if (ok)
				{	pi.FromRpl2(&rf, &pCar->dynamics);
					pCar->SetPosition(rf.pos, rf.rot);  // for objs hit
					carM->trackPercent = rf.percent /255.f*100.f;
				}else
				{	carM->fCam->First();
					pGame->timer.RestartReplay(0);  //at end
			}	}
		}
		else  ///>>  sim, game  -  get data from vdrift
		if (pCar)
		{
			pi.FromCar(pCar);
			pi.percent = carM->trackPercent;
		}
		pi.bNew = true;
		

		//<<  ⏪ rewind
		///-----------------------------------------------------------------------
		if (!bRplPlay && !pGame->pause && !bGhost && pCar)
		if (pCar->bRewind && pSet->game.rewind_type > 0)
		{	//  do rewind (go back)
			double& gtime = pGame->timer.GetRewindTime(c);
			gtime = std::max(0.0, gtime - time * gPar.rewindSpeed);
			double& ghtim = pGame->timer.GetRewindTimeGh(c);
			ghtim = std::max(0.0, ghtim - time * gPar.rewindSpeed);  //rewind ghost time too
			if (pSet->game.rewind_type == 2 || gPar.backTime)
			{	pGame->timer.Back(c, - time * gPar.rewindSpeed);
				ghost.DeleteFrames(0, ghtim);
			}
			RewindFrame rf;
			bool ok = rewind.GetFrame(gtime, &rf, c);

			pCar->SetPosRewind(rf.pos, rf.rot, rf.vel, rf.angvel);
			pCar->dynamics.fDamage = rf.fDamage;  // take damage back
			if (carModels[c]->vType == V_Sphere)
				pCar->dynamics.sphereYaw = rf.hov_roll;
			carModels[c]->First();
		}
		else if (c < 4)  // save data
		{
			const CARDYNAMICS& cd = pCar->dynamics;
			RewindFrame fr;
			fr.time = pGame->timer.GetRewindTime(c);
			
			fr.pos = cd.body.GetPosition();
			fr.rot = cd.body.GetOrientation();
			fr.vel = cd.GetVelocity();
			fr.angvel = cd.GetAngularVelocity();
			fr.fDamage = cd.fDamage;
			if (cd.vtype == V_Sphere)
				fr.hov_roll = cd.sphereYaw;
			else
				fr.hov_roll = cd.hov_roll;  //? fr.hov_throttle = cd.hov_throttle;

			rewind.AddFrame(fr, c);  // rec rewind
		}
		
		//<<  👻 record  save data
		///-----------------------------------------------------------------------
		if (pSet->rpl_rec && !bRplPlay && !pGame->pause && !bGhost && pCar)
		{
			if (iRplSkip++ >= 1)  // 1 half game framerate
			{	iRplSkip = 0;

				ReplayFrame2 fr;
				fr.time = rplTime;
				fr.percent = carM->trackPercent /100.f*255.f;

				fr.FromCar(pCar, replay.GetLastHitTime(c));
				
				replay.AddFrame(fr, c);  // rec replay
				if (c==0)  /// rec ghost lap
				{
					fr.time = lapTime;
					ghost.AddFrame(fr, 0);
				}
				
				//  recorded info ..in update
				{
					int size = replay.GetNumFrames() * 232;  //par approx  sizeof(ReplayFrame);
					std::string s = fToStr( float(size)/1000000.f, 2,5);
					String ss = String( TR("#{RplRecTime}: ")) + StrTime(replay.GetTimeLength()) + TR("   #{RplSize}: ") + s + TR(" #{UnitMB}");
					gui->valRplName2->setCaption(ss);
				}
			}
		}
		if (bRplPlay)  gui->valRplName2->setCaption("");
		///-----------------------------------------------------------------------
		

		//  🏁 checkpoints, lap start
		//-----------------------------------------------------------------------
		if (bGhost && !gui->bLesson)   // dont check for ghost
			carM->bWrongChk = false;
		else
		{
			///  🔝 arrow update  --------------------------------------
			SplineRoad* road = scn->road;
			if (pSet->check_arrow && carM->cType == CarModel::CT_LOCAL
			  && !bRplPlay && hud->arrow.node && road && road->mChks.size()>0)
		  		hud->arrow.UpdateChk(road, carM, pi.pos);
			
			//----------------------------------------------------------------------------
			if (carM->bGetStPos)  // get finish, end box
			{	carM->bGetStPos = false;
				int st = !road || road->isLooped ? 0 : pSet->game.trackreverse ? 0 : 1;
				carM->matStPos.makeInverseTransform(
					Axes::toOgre(scn->sc->startPos[st]), Vector3::UNIT_SCALE,
					Axes::toOgre(scn->sc->startRot[st]));
				carM->ResetChecks();
			}
			if (road && !carM->bGetStPos)
			{
				//  finish box dist
				Vector4 carP(pi.pos.x,pi.pos.y,pi.pos.z,1);
				carM->vStDist = carM0->matStPos * carP;  // start pos from 1st car always
				carM->bInSt = abs(carM->vStDist.x) < road->vStBoxDim.x && 
					abs(carM->vStDist.y) < road->vStBoxDim.y && 
					abs(carM->vStDist.z) < road->vStBoxDim.z;
							
				carM->iInChk = -1;  carM->bWrongChk = false;
				bool locar = carM->cType == CarModel::CT_LOCAL;
				int ncs = road->mChks.size();
				if (ncs > 0)
				{
					//  🏁 Finish  --------------------------------------
					if (locar && !bRplPlay &&
						(carM->bInSt && carM->iNumChks == ncs && carM->iCurChk != -1))
					{
						///  Lap
						bool finished = (pGame->timer.GetCurrentLap(c) >= pSet->game.num_laps)
										&& (/*mClient ||*/ pSet->game.local_players > 1);  // multiplay or split
						bool best = finished ? false :  // dont inc laps when race over (in multiplayer or splitscreen mode)
							pGame->timer.Lap(c, !finished, pSet->game.trackreverse);  //,boost_type?
						double timeCur = pGame->timer.GetPlayerTimeTot(c);

						//  Network notification, send: car id, lap time  ----
						//; if (mClient && c == 0 && !finished)
						// 	mClient->lap(pGame->timer.GetCurrentLap(c), pGame->timer.GetLastLap(c));

						///  new best lap, save ghost
						bool newbest = false;
						if (!pSet->rpl_bestonly || best || pSet->game.rewind_type == 2 || gPar.backTime)
						if (c==0 && pSet->rpl_rec)  // for many, only 1st car
						{
							ghost.SaveFile(gui->GetGhostFile());  //,boost_type?
							ghPlay.CopyFrom(ghost);
							isGhost2nd = false;  // hide 2nd ghost
							newbest = true;
						}

						bool champ = pSet->game.champ_num >= 0, chall = pSet->game.chall_num >= 0;
						bool chs = champ || chall;
						
						if (!chs)
						{	if (newbest)
								pGame->snd_lapbest->start();  //)
							else
								pGame->snd_lap->start();  //)
						}
						ghost.Clear();
						
						carM->ResetChecks();
						
						//  restore boost fuel, each lap  ----
						if (pSet->game.boost_type == 1 && carM->pCar)
							carM->pCar->dynamics.boostFuel = carM->pCar->dynamics.boostFuelStart;

						//  damage decrease, each lap  ---
						if (pSet->game.damage_dec > 0.f)
							carM->pCar->dynamics.fDamage = std::max(0.f,
								carM->pCar->dynamics.fDamage - pSet->game.damage_dec);

						//  upd lap results ----
						carM->updLap = false;
						carM->fLapAlpha = 1.f;

						///  all laps
						finished = pGame->timer.GetCurrentLap(c) >= pSet->game.num_laps;
						if (finished /*&& !mClient*/)
						{
							if (!chs)
							{
								if (carM->iWonPlace == 0)	//  split screen winner places
								{
									if (pSet->game.local_players > 1)
									{
										int n = std::min(2, std::max(0, 3 - carIdWin));
										pGame->snd_win[n]->start();  //)
									}
									carM->iWonPlace = carIdWin++;
								}
							}
							else if (champ)
								gui->ChampionshipAdvance(timeCur);
							else
								gui->ChallengeAdvance(timeCur);
						}
					}
					
					//  🏁 checkpoints  --------------------------------------
					for (int i=0; i < ncs; ++i)
					{
						const CheckSphere& cs = road->mChks[i];
						Real d2 = pi.pos.squaredDistance(cs.pos);
						if (d2 < cs.r2)  // car in checkpoint
						{
							carM->iInChk = i;
							
							//\  ➰🎥 loop camera change
							if (pSet->cam_loop_chng && carM->fCam &&
								cs.loop && (carM->iLoopChk == -1 || carM->iLoopChk != i) &&
								pCar->dynamics.vtype != V_Sphere)
							{
								carM->iLoopChk = i;
								if (carM->iLoopLastCam == -1)
								{
									carM->iLoopLastCam = carM->fCam->miCurrent;
									carM->fCam->setCamera(pSet->cam_in_loop);
								}else
								{	carM->fCam->setCamera(carM->iLoopLastCam);
									carM->iLoopLastCam = -1;
								}
							}
							//  next check  ()
							if (i == carM->iNextChk && carM->iNumChks < ncs)
							{
								carM->iCurChk = i;  carM->iNumChks++;
								carM->timeAtCurChk = pGame->timer.GetPlayerTime(c);
								bool rev = pSet->game.trackreverse;
								int inc = (rev ? -1 : 1) * road->iDir;
								
								carM->iNextChk = (carM->iCurChk + inc + ncs) % ncs;
								carM->UpdNextCheck();
								//  save car pos and rot
								carM->pCar->SavePosAtCheck();
								carM->updTimes = true;

								//  trail  next start ->--
								if (scn->trail && carM->iNumChks < ncs-1)  // skip last
									scn->trail->trailSegId =
										(inc * (i - scn->road->iChkId1) + (rev ? 0 : 1) + ncs) % ncs;

								if (pSet->snd_chk && locar)
									pGame->snd_chk->start();  //)
							}
							else
							if (carM->iInChk != carM->iCurChk && !bRplPlay &&
								!scn->sc->noWrongChks)  // denies
							{
								carM->bWrongChk = true;
								
								if (carM->iInWrChk != carM->iInChk)
								{	carM->iInWrChk = carM->iInChk;
									
									if (pSet->snd_chkwr && locar)
										pGame->snd_chkwr->start();  //)
							}	}
							break;
						}
				}	}
		}	}

		
		///  store new pos info in queue  _________
		//if (!(isFocGui /*|| isTweakTab()) || mClient*/))  // dont if gui, but network always
		{
			int qn = (iCurPoses[c] + 1) % CarPosCnt;  // next index in queue
			carPoses[qn][c] = pi;
			//  🎥 update camera
			if (carM->fCam)
				carM->fCam->update(time, pi, &carPoses[qn][c], &pGame->collision,
					!bRplPlay && pSet->cam_bounce, carM->vType);
			iCurPoses[c] = qn;  // atomic, set new index in queue
			
			///))  🔉🎥 upd sound camera
			if (c == 0 && pGame->snd)
			{
				Vector3 x,y,z;
				carPoses[qn][c].camRot.ToAxes(x,y,z);
				bool camCar = carM->fCam && carM->fCam->TypeCar();  // fix
				pGame->snd->setCamera(carPoses[qn][c].camPos, camCar ? -y : -z, camCar ? -z : y, Vector3::ZERO);
			}
		}
	}
	PROFILER.endBlock(".newPos ");
}


//  updatePoses - Set car pos for Ogre nodes, update particles, trails
//---------------------------------------------------------------------------------------------------------------
void App::updatePoses(float time)
{
	if (carModels.empty())  return;
	PROFILER.beginBlock(".updPos ");
	
	//  Update all carmodels from their carPos
	const CarModel* playerCar = carModels.front();

	int cgh = -1;
	for (int c = 0; c < carModels.size(); ++c)
	{		
		CarModel* carM = carModels[c];
		if (!carM)  {
			PROFILER.endBlock(".updPos ");
			return;  }
		
		///  👻 ghosts visibility  . . .
		//  hide when empty or near car
		bool bGhostCar = carM->cType == (isGhost2nd ? CarModel::CT_GHOST2 : CarModel::CT_GHOST),  // show only actual
			bGhTrkVis = carM->isGhostTrk() && ghTrk.GetTimeLength()>0 && pSet->rpl_trackghost,
			bGhostVis = ghPlay.GetNumFrames()>0 && pSet->rpl_ghost,
			bGhostEnd = pGame->timer.GetPlayerTime(0) > ghPlay.GetTimeLength();
		if (bGhostCar)  cgh = c;

		if (carM->isGhost())  // for all
		{
			bool loading = iLoad1stFrames >= 0;  // show during load ?..
			bool curVisible = carM->mbVisible;
			bool newVisible = bGhostVis && bGhostCar /**/&& !bGhostEnd/**/ || bGhTrkVis;
			
			if (loading)
				carM->setVisible(true);  //!carM->isGhost());
			else
			{	//  hide ghost when close to player
				float d = carM->pMainNode->getPosition().squaredDistance(playerCar->pMainNode->getPosition());
				if (d < pSet->ghoHideDist * pSet->ghoHideDist)
					newVisible = false;

				if (carM->isGhostTrk() && cgh >= 0)  // hide track's ghost when near ghost
				{
					float d = carM->pMainNode->getPosition().squaredDistance(carModels[cgh]->pMainNode->getPosition());
					if (d < pSet->ghoHideDistTrk * pSet->ghoHideDistTrk)
						newVisible = false;
				}
				if (curVisible == newVisible)
					carM->hideTime = 0.f;
				else
				{	carM->hideTime += time;  // change vis after delay
					if (carM->hideTime > gPar.ghostHideTime)
						carM->setVisible(newVisible);
				}
		}	}

		
		//  🚗💫 update car pos  ----
		int q = iCurPoses[c];
		int cc = (c + iRplCarOfs) % carModels.size();  // replay offset, camera from other car
		int qq = iCurPoses[cc];
		PosInfo& pi = carPoses[q][c], &pic = carPoses[qq][cc];
		carM->Update(carPoses[q][c], carPoses[qq][cc], time);

		if (mCubeCamera && c == 0)  // 🔮 reflection
		{	mCubeCamera->setPosition(carM->pMainNode->getPosition());// carPoses[q][c].pos);
			// mCubeCamera->setVisibilityFlags( RV_MaskReflect /*32*/ );
			// mCubeCamera->setOrientation(carM->pMainNode->getOrientation());
		}


		//  nick text pos upd  3d to 2d
		/*if (carM->pNickTxt && carM->pMainNode)
		{
			Camera* cam = playerCar->fCam->mCamera;  //above car 1m
			Vector3 p = hud->projectPoint(cam, carM->pMainNode->getPosition() + Vector3(0,1.f,0));
			p.x = p.x * mSplitMgr->mDims[0].width * 0.5f;  //1st viewport dims
			p.y = p.y * mSplitMgr->mDims[0].height * 0.5f;
			carM->pNickTxt->setPosition(p.x-40, p.y-16);  //center doesnt work
			carM->pNickTxt->setVisible(p.z > 0.f);
		}*/
	}
	
	///  📽️ Replay info
	if (bRplPlay && !pGame->cars.empty())
	{
		double pos = pGame->timer.GetPlayerTime(0);
		float len = replay.GetTimeLength();
		// gui->valRplPerc->setCaption(fToStr(pos/len*100.f, 1,4)+" %");
		// gui->valRplCur->setCaption(StrTime(pos));
		// gui->valRplLen->setCaption(StrTime(len));

		float v = pos/len;  //gui->slRplPos->setValue(v);

		if (pos < 0.05)  // rpl trail end
		for (auto& carM : carModels)
			carM->ResetChecks();


		//  👈 lessons  >> >
		if (gui->bLesson)
		{
			if (v > 0.98f && !isFocGui)  // end, back to gui
			{	//bRplPause = true;
				pSet->iMenu = MN_HowTo;  isFocGui = true;  gui->toggleGui(false);
				//  hud restore ..
				gui->ckTimes.SetValue(1);
			}
			bool vis = false;
			for (auto s : gui->rplSubtitles)
				if (pos > s.beg && pos < s.end)
				{
					vis = true;
					if (!mWndRplTxt->getVisible())
					{
						gui->rplSubText->setCaption(TR(s.txt));
						gui->setHintImg(gui->rplSubImg, s.hintImg);
					}	break;
				}
			mWndRplTxt->setVisible(vis);
		}
	}	
	
	
	///  📦 objects - dynamic (props)  -------------------------------------------------------------
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		Object& o = scn->sc->objects[i];
		if (o.ms)
		{
			btTransform tr, ofs;
			o.ms->getWorldTransform(tr);
			const btVector3& p = tr.getOrigin();
			const btQuaternion& q = tr.getRotation();
			o.pos[0] = p.x();  o.pos[1] = p.y();  o.pos[2] = p.z();
			o.rot[0] = q.x();  o.rot[1] = q.y();  o.rot[2] = q.z();  o.rot[3] = q.w();
			o.SetFromBlt();
		}
	}

	PROFILER.endBlock(".updPos ");
}
