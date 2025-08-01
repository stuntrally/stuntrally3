#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CarModel.h"
#include "mathvector.h"
#include "settings.h"
#include "game.h"
#include "SceneXml.h"
#include "CScene.h"
#include "CGame.h"
#include "FollowCamera.h"
#include "Road.h"

#include <OgreRoot.h>
#include <OgreMath.h>
#include <OgreVector4.h>
#include <OgreEntity.h>
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleAffector.h>
#include <OgreRibbonTrail.h>
#include <OgreBillboardSet.h>
#include <OgreSceneNode.h>

#include <OgreHlmsPbsDatablock.h>
#include "Terra.h"
#include "OgreHlmsPbsTerraShadows.h"
#include "HlmsPbs2.h"
// #include <MyGUI_TextBox.h>
using namespace Ogre;


void CarModel::setVisible(bool vis)
{
	bVisible = vis;
	hideTime = 0.f;

	ndMain->setVisible(vis);
	if (bsBrakes)  bsBrakes->setVisible(vis && bBraking);
	if (bsFlares)  bsFlares->setVisible(vis && pSet->li.front);
	// todo: fade from behind..

	for (int w=0; w < numWheels; ++w)
	{	ndWh[w]->setVisible(vis);
		if (ndBrake[w])
			ndBrake[w]->setVisible(vis);
	}
	UpdParsTrails(vis);
}

void CarModel::UpdNextCheck()  // 📍
{
	updTimes = true;
	if (cType != CarModel::CT_LOCAL)  return;
	if (!ndNextChk || !pApp || !pApp->scn->road)  return;
	if (pApp->scn->road->mChks.empty())  return;

	Vector3 p{0,0,0};
	if (iNumChks == pApp->scn->road->mChks.size() && iCurChk != -1)
	{
		int lap = pGame->timer.GetCurrentLap(iIndex) + 1, laps = pSet->game.num_laps;
		String smtr = "checkpoint_lap";
		if (pSet->game.hasLaps())
			if (lap == laps - 1)   smtr = "checkpoint_lastlap";
			else if (lap == laps)  smtr = "checkpoint_finish";
		p = vStartPos;  // finish
		sChkMtr = smtr;
		bChkUpd = true;
	}else
	{	p = pApp->scn->road->mChks[iNextChk].pos;
		sChkMtr = "checkpoint_normal";
		bChkUpd = true;
	}
		
	p.y += gPar.chkBeamYofs;  // lower
	ndNextChk->setPosition(p);
	ndNextChk->setScale(gPar.chkBeamSx, gPar.chkBeamSy, gPar.chkBeamSx);
	ndNextChk->setVisible(pSet->check_beam && !pApp->bHideHudBeam);
	ndNextChk->_getFullTransformUpdated();
}
void CarModel::ShowNextChk(bool visible)
{
	if (ndNextChk)
		ndNextChk->setVisible(visible && !pApp->bHideHudBeam);
}


void CarModel::ResetChecks(bool bDist)  // needs to be done after road load!
{
	updTimes = true;
	iCurChk = -1;  iNumChks = 0;  // reset lap, chk vars
	iLoopChk = -1;  iLoopLastCam = -1;
	trackPercent = 0.f;
	if (!pApp || !pApp->scn->road)  return;
	
	const SplineRoad* road = pApp->scn->road;
	iNextChk = pSet->game.track_reversed ? road->iChkId1Rev : road->iChkId1;
	UpdNextCheck();

	//LogO("ResetChecks");
	if (iIndex < MAX_Players)
	{	SplineRoad* trail = pApp->scn->trail[iIndex];
		if (trail)  // +
			trail->trailSegId = 0;
	}
	//  percent const  ------
	if (bDist && !road->mChks.empty())
	{
		const Vector3& firstC = road->mChks[road->iChkId1].pos, lastC = road->mChks[road->iChkId1Rev].pos;

		Vector3 vFirst = vStartPos - firstC;  distFirst = vFirst.length();
		Vector3 vLast  = lastC - vStartPos;   distLast = vLast.length();
		distTotal = distFirst + distLast + road->chksRoadLen;
		//LogO("Chk first: "+toStr(distFirst)+" last: "+toStr(distLast)+" total: "+toStr(distTotal));
	}
}

//  get track driven dist part in %
//--------------------------------------------------------------------------------------------------------
void CarModel::UpdTrackPercent()
{
	if (!pApp || !pApp->scn->road)  return;
	const SplineRoad* road = pApp->scn->road;
	
	float perc = 0.f;
	if (road && !road->mChks.empty() && !isGhost() && ndMain)
	{
		const Vector3& car = ndMain->getPosition(), next = road->mChks[iNextChk].pos,
			start = vStartPos, curr = road->mChks[std::max(0,iCurChk)].pos;
		bool bRev = pSet->game.track_reversed;
		Real firstD = bRev ? distLast : distFirst;
		Real nextR = road->mChks[iNextChk].r;  // chk .r radius to tweak when entering chk..

		Real dist = 0.f;
		if (iNumChks > 0)  dist = firstD;  // already after 1st chk
		if (iNumChks > 1)  dist +=  // after 1st to 2nd chk or more
			road->mChks[iNumChks-2].dist[bRev ? 1 : 0];
							
		
		float dist01 = 0.f;  // smooth dist part
		//  start to 1st chk
		if (iNumChks == 0)
		{
			Vector3 curDist  = car - start;
			Vector3 chksDist = next - start;  // first
			dist01 = (curDist.length() /*- nextR*/) / (chksDist.length() - nextR);

			float percD = std::min(1.f, std::max(0.f, dist01 ));  // clamp to 0..1
			dist += percD * firstD;
		}
		//  last chk to finish
		else if (iNumChks == road->mChks.size())
		{
			Vector3 curDist  = start - car;
			Vector3 chksDist = curr - start;  // last
			dist01 = 1.f - (curDist.length() /*- nextR*/) / (chksDist.length() - nextR);

			float percD = std::min(1.f, std::max(0.f, dist01 ));  // clamp to 0..1
			dist += percD * (bRev ? distFirst : distLast);  //lastD;
		}
		else  // between 2 checks
		{
			Vector3 curDist  = car  - next;   // car dist to next checkpoint
			Vector3 chksDist = curr - next;  // divide by (cur to next) checks distance
			Real ckD = chksDist.length();

			dist01 = 1.f - (curDist.length() - nextR) / (ckD - road->mChks[iCurChk].r);

			float percD = std::min(1.f, std::max(0.f, dist01 ));  // clamp to 0..1
			dist += percD * (ckD + road->mChks[iCurChk].r*0.8f);  //road->mChks[iNumChks-1].dist;
		}
		perc = 100.f * dist / distTotal;

		if (perc > trackPercent)
			trackPercent = perc;
	}
}


//-------------------------------------------------------------------------------------------------------
//  💫 Update
//-------------------------------------------------------------------------------------------------------
void CarModel::Update(PosInfo& posInfo, PosInfo& posInfoCam, float time)
{	
	int w,i;
	
	//  🔵 upd chk mtr
	if (bChkUpd && itNextChk)
		itNextChk->setDatablockOrMaterialName(sChkMtr);


	//  ✨ stop/resume par sys
	float fa = pGame->pause ? 0.f : 1.f;
	for (w=0; w < numWheels; ++w)
	{
		for (i=0; i < PAR_ALL; ++i)
			if (par[i][w])  par[i][w]->setSpeedFactor(fa);
		if (w < PAR_BOOST && parBoost[w])  parBoost[w]->setSpeedFactor(fa);
		if (parHit)  parHit->setSpeedFactor(fa);
	}
	for (w=0; w < PAR_THRUST*2; ++w)
		if (parThrust[w])  parThrust[w]->setSpeedFactor(fa);


	if (!posInfo.bNew)  return;  // new only ?
	posInfo.bNew = false;
	///  Don't get anything from pCar or car.dynamics here
	///  All must be read from posInfo (it is filled from vdrift car or from replay)
	
	if (!ndMain)  return;


	//  🚗 set car pos and rot  ------------------------------------
	ndMain->setPosition(posInfo.pos);
	if (vType == V_Sphere)
		ndMain->setOrientation(Quaternion(Quaternion(Degree(-posInfo.hov_roll),Vector3::UNIT_Y)));
	else
	if (vType == V_Spaceship)  // roll  vis only
		ndMain->setOrientation(posInfo.rot * Quaternion(Degree(posInfo.hov_roll),Vector3::UNIT_X));
	else
		ndMain->setOrientation(posInfo.rot);
	ndMain->_getFullTransformUpdated();  //?
	

	//  🟢🌿 grass sphere pos
	Vector3 vx(1,0,0);  // car x dir
	vx = posInfo.rot * vx * 1.1;  //par
	posSph[0] = posInfo.pos + vx;  posSph[0].y += 0.5f;
	posSph[1] = posInfo.pos - vx;  posSph[1].y += 0.5f;
	if (ndSph)  // sph test
	{	ndSph->setPosition(posSph[0]);
		ndSph->setScale(Vector3::UNIT_SCALE * 1.7 *2/0.6f);  //par
		ndSph->_getFullTransformUpdated();
	}

	//  🎥 set camera view  ------------------------------------
	if (fCam && cType != CT_REMOTE)
	{	fCam->Apply(posInfoCam);

		if (pCar && cam && pSet->boost_fov)  // not here?-
		 	cam->cam->setFOVy(Degree( 0.5f * (
				pSet->fov_min + pSet->fov_boost * pCar->dynamics.fBoostFov)));
		

		///~~  💧🎥 camera in fluid fog, detect and compute
		//------------------------------------------------------------------------
		iCamFluid = -1;  fCamFl = 0.f;  // none
		const size_t sf = sc->fluids.size();
		if (sf > 0 && pSet->game.local_players == 1)
		{
			const Vector3& p = posInfo.camPos;
			const float r = 0.2f;  //par, near cam?
			
			//  check if any fluid box overlaps camera pos sphere
			bool srch = true;  size_t f = 0;
			while (srch && f < sf)
			{
				const FluidBox& fb = sc->fluids[f];
				const Vector3& fp = fb.pos;
				Vector3 fs = fb.size;  fs.x *= 0.5f;  fs.z *= 0.5f;
				
				bool inFl =  //  p +r   -fs fp +fs  -r p
					p.y +r > fp.y - fs.y && p.y -r < fp.y &&
					p.x +r > fp.x - fs.x && p.x -r < fp.x + fs.x &&
					p.z +r > fp.z - fs.z && p.z -r < fp.z + fs.z;
				
				if (inFl)  // 1st only
				{	iCamFluid = f;  fCamFl = std::min(1.f, std::max(0.f, fp.y - p.y)) * 3.f;
					srch = false;  }
				++f;
			}
	}	}

	//  🌍 upd rotY for minimap
	if (vType == V_Sphere)
		angCarY = posInfo.hov_roll * 180.f / PI_d + 180.f;
	else
	{	Quaternion q = posInfo.rot * Quaternion(Degree(90),Vector3(0,1,0));
		angCarY = q.getYaw().valueDegrees() + 90.f;
	}
	
	
	//  🔴 brake state  flares
	//------------------------------------------------------------------------
	bool braking = posInfo.braking > 0;
	if (bBraking != braking  && gPar.carPrv == 0)
	{
		bBraking = braking;
		bool vis = bBraking && bVisible;
		if (bsBrakes)
			bsBrakes->setVisible(vis);
	}
	//  💡 reverse
	bool reverse = posInfo.reverse > 0;
	if (bReverse != reverse  && gPar.carPrv == 0)
	{
		bReverse = reverse;
		bool vis = bReverse && bVisible;
		if (bsReverse)
			bsReverse->setVisible(vis);
	}
	bReverse = reverse;
	if (bReverse != bReverseOld)
	{
		for (auto& l : lights)
		if (l.type == LI_Revese)
			l.li->setVisible(bReverse);
		if (bsReverse)
			bsReverse->setVisible(bReverse && pSet->li.front);
	}
	bReverseOld = bReverse;

	//  💡 Lights
	//------------------------------------------------------------------------
	if (pCar)
	{	// not from posInfo, always can toggle
		bLightsOld = bLights;
		bLights = pCar->bLightsOn;
		if (bLights != bLightsOld)
		{
			for (auto& l : lights)
			if (l.type == LI_Front)
				l.li->setVisible(bLights);
			if (bsFlares)
				bsFlares->setVisible(bLights && pSet->li.front);
	}	}

	const float br = bBraking ? 1.f : 0.f,
		bo = posInfo.fboost, th = posInfo.hov_throttle;
	const float bri = pSet->car_light_bright;

	//  var each frame-
	for (auto& l : lights)
	{
		if (l.type == LI_Thrust)
		{	l.li->setPowerScale( th * l.power * bri);
			l.li->setVisible( th > 0.1f);
		}else
		if (l.type == LI_Boost)
		{	l.li->setPowerScale( bo * l.power * bri);
			l.li->setVisible( bo > 0.1f);
		}else
		if (l.type == LI_Brake)
		{	l.li->setPowerScale( br * l.power * bri);  //par
			l.li->setVisible( br > 0.1f  && gPar.carPrv == 0);
	}	}


	//  ✨ update particle emitters  ------------------------------------
	if (pSet->particles && pCar)
	{
		//  💨 boost
		for (i=0; i < PAR_BOOST; ++i)  if (parBoost[i])
		{
			/// <><> damage reduce
			float dmg = pCar->dynamics.fDamage >= 80.f ? 0.f : std::max(0.f, 1.4f - pCar->dynamics.fDamage*0.01f);
			float emitB = posInfo.fboost * 40.f * dmg;  // par
			ParticleEmitter* pe = parBoost[i]->getEmitter(0);
			pe->setEmissionRate(emitB);
		}
		//  🔥 spaceship thrusters
		for (i=0; i < PAR_THRUST*2; ++i)  if (parThrust[i])
		{
			float dmg = 1.f - 0.5f * pCar->dynamics.fDamage*0.01f;
			float emitT = posInfo.hov_throttle * 60.f * dmg;  // par
			ParticleEmitter* pe = parThrust[i]->getEmitter(0);
			pe->setEmissionRate(emitT);
		}
	}

	//  ✨ world hit  ------------------------------------
	if (parHit)
	{	ParticleEmitter* pe = parHit->getEmitter(0);
		if (posInfo.fHitTime > 0.f && pSet->particles)
		{
			pe->setPosition(posInfo.vHitPos);
			pe->setDirection(posInfo.vHitNorm);

			pe->setEmissionRate(pSet->particles_len * std::min(160.f, posInfo.fParIntens) * posInfo.fHitTime);
			pe->setParticleVelocity(posInfo.fParVel);
		}else
			pe->setEmissionRate(0.f);
	}

	//  ⚫💭 wheels  ------------------------------------------------------------------------
	for (w=0; w < numWheels; ++w)
	{
		float wR = whRadius[w];
		if (ndWh[w])
		{
			#ifdef CAM_TILT_DBG  // cam debug test only
				if (fCam)
					ndWh[w]->setPosition(fCam->posHit[w]);
				ndWh[w]->setScale(0.5f*Vector3::UNIT_SCALE);
			#else
			ndWh[w]->setPosition(posInfo.whPos[w]);
			#endif
			ndWh[w]->setOrientation(posInfo.whRot[w]);
			ndWh[w]->_getFullTransformUpdated();  //?
		}

		///  Update particles and trails
		if (isGhostTrk())
			continue;  // doesnt have any
		
		int whMtr = posInfo.whTerMtr[w];
		int whRd = posInfo.whRoadMtr[w];
		
		bool pipe = whRd >= 30 && whRd < 60;  //old: whRd == 2;
		//todo: road,pipe 4mtr [whRd] layer params..
		float whVel = posInfo.whVel[w] * 3.6f;  //kmh
		float slide = posInfo.whSlide[w], squeal = posInfo.whSqueal[w];
			//LogO(" slide:"+fToStr(slide,3,5)+" squeal:"+fToStr(squeal,3,5));
		float onGr = slide < 0.f ? 0.f : 1.f;  // ground

		//  ⚫🌡️ wheel temp
		whTemp[w] += std::min(12.f, std::max(0.f, squeal*8 - slide*2 + squeal*slide*2)*time);
		whTemp[w] = std::min(1.5f, whTemp[w]);  ///*
		whTemp[w] -= time*7.f;  if (whTemp[w] < 0.f)  whTemp[w] = 0.f;
			//LogO(toStr(w)+" wht "+fToStr(wht[w],3,5));

		///  ✨ emit rates +
		Real sq = squeal* std::min(1.f, whTemp[w]), l = pSet->particles_len * onGr;
		Real emitS = sq * (whVel * 30) * l * 0.45f;  ///*
		Real emitM = slide < 1.4f ? 0.f :  (8.f * sq * std::min(5.f, slide) * l);
		Real emitD = (std::min(140.f, whVel) / 3.5f + slide * 1.f ) * l;  
		Real sizeD = (0.8f + 0.6f * std::min(140.f, whVel) / 140.f) * (w < 2 ? 0.7f : 1.1f);

		//  ter mtr factors
		const auto& td = sc->tds[0];
		int mtr = std::max(0, std::min(whMtr-1, (int)(td.layers.size()-1)));  // 1st ter!-
		int rd  = sc->road1mtr ? 0 : std::max(0, std::min(3, whRd));

		const TerLayer& lay = whMtr==0 ? sc->layerRoad[rd] : td.layersAll[td.layers[mtr]];  // 1st ter!-
		emitD *= lay.dust;  emitM *= lay.mud;  sizeD *= lay.dustS;  emitS *= lay.smoke;

		if (pipe)  emitD = 0;  // no dust in pipes
		if (posInfo.whH[w] > 0.1f)  emitD = 0;  // no dust in fluids

		bool ghost = isGhost();  // opt dis for ghost
		bool ghPar = !(ghost && !pSet->rpl_ghostpar);
		if (!ghPar)
		{	emitD = 0.f;  emitM = 0.f;  emitS = 0.f;  }


		///  💭 emit particles
		Vector3 vpos = posInfo.whPos[w];
		if (pSet->particles)
		{
			ParticleSystem* ps = par[PAR_Smoke][w];
			if (ps)  //  smoke
			{	ParticleEmitter* pe = ps->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.7f);  ///*
				ps->getAffector(0)->setParameter("alpha", toStr(-0.2f - 0.023f * whVel));  // fade out speed
				pe->setTimeToLive( std::max(0.12f, 2.f - whVel * 0.06f) );  // live time
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitS);
			}
			ps = par[PAR_Mud][w];
			if (ps)	 //  mud
			{	ParticleEmitter* pe = ps->getEmitter(0);
				//pe->setDimensions(sizeM,sizeM);
				pe->setPosition(vpos + posInfo.carY * wR*0.7f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM);
			}
			ps = par[PAR_Dust][w];
			if (ps)	 //  dust
			{	ps->setDefaultDimensions(sizeD,sizeD);
				ParticleEmitter* pe = ps->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.31f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitD);
			}

			//  💧 fluids .::.
			ps = par[PAR_Water][w];
			int idPar = posInfo.whP[w];
			if (ps)  //  Water ~
			{
				float vel = posInfo.speed;  // depth.. only on surface?
				float whH = posInfo.whH[w];  //V*  less on water
				float mul = vType == V_Hovercraft ? (0.1f + 0.2f*whH) : (0.1f + 0.9f*whH);
				bool e = idPar == 0 && ghPar &&  vel > 10.f && whH < 1.f;
				float emitW = e ?  std::min(80.f, 5.0f * vel) * mul  : 0.f;

				ParticleEmitter* pe = ps->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setMinParticleVelocity(0.07* vel);
				pe->setMaxParticleVelocity(0.20* vel);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitW * pSet->particles_len);
			}
			ps = par[PAR_MudHard][w];
			if (ps)  //  Mud ^
			{
				float vel = Math::Abs(posInfo.whAngVel[w]);
				bool e = idPar == 2 && ghPar &&  vel > 30.f;
				float emitM = e ?  posInfo.whH[w] * std::min(80.f, 1.5f * vel)  : 0.f;

				ParticleEmitter* pe = ps->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM * pSet->particles_len);
			}
			ps = par[PAR_MudSoft][w];
			if (ps)  //  Mud soft ^
			{
				float vel = Math::Abs(posInfo.whAngVel[w]);
				bool e = idPar == 1 && ghPar &&  vel > 30.f;
				float emitM = e ?  posInfo.whH[w] * std::min(160.f, 3.f * vel)  : 0.f;

				ParticleEmitter* pe = ps->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM * pSet->particles_len);
			}
		}

		//  🎗️ update trails h+
		if (pSet->trails)
		{	if (ndWhE[w])
			{
				//  upd pos
				const float trlH = 0.90f;
				Vector3 vp = vpos + posInfo.carY * wR*trlH;
				// vp.y += 0.5f;  //test
				if (!sc->tds.empty() && whMtr > 0)
					vp.y = pApp->scn->getTerH(vp.x, vp.z) + 0.02f;  // 0.05f
					//if (/*whOnRoad[w]*/whMtr > 0 && road)  // on road, add ofs
					//	vp.y += road->fHeight;	}/**/
				ndWhE[w]->setPosition(vp);
				//ndWhE[w]->setOrientation(posInfo.rot);  // fixme? upside down
				ndWhE[w]->_getFullTransformUpdated();
			}
			//  const trail alpha
			float ac = pipe ? 0.f : /*own par..*/lay.smoke < 0.5f ? 0.14f : 0.f;
			float al = (ac + 0.6f * std::min(1.f, 0.7f * whTemp[w]) ) * onGr;  // par+
			// al = 1.f;  // test
			
			if (whTrail[w])
			{	whTrail[w]->setInitialColour(0,
				lay.tcl.x, lay.tcl.y, lay.tcl.z, lay.tcl.w * al/**/);
				if (iFirst > 10)  //par
					whTrail[w]->setInitialWidth(0, whWidth[w]);
			}
		}
	}
	
	//  ⚫📉  surf info
	UpdWhTerMtr();
	
	
	//  ⚫🔴  update brake disc meshes orientation
	for (w=0; w < numWheels; ++w)
	if (ndBrake[w])
	{
		ndBrake[w]->setPosition(ndWh[w]->getPosition());
		ndBrake[w]->setOrientation( ndMain->getOrientation() );
		
		// this transformation code is just so the brake mesh can have the same alignment as the wheel mesh
		ndBrake[w]->yaw(Degree(-90), Node::TS_LOCAL);
		if (w%2 == 1)
			ndBrake[w]->setScale(-1, 1, 1);
			
		ndBrake[w]->pitch(Degree(180), Node::TS_LOCAL);
		if (w < 2)  // turn only front wheels
			ndBrake[w]->yaw(-Degree(posInfo.whSteerAng[w]));

		ndBrake[w]->_getFullTransformUpdated();
	}
	
	if (iFirst <= 10)  ++iFirst;  //par
	
	UpdKeysCam();  // 🎥
}


//  🎥 Camera
//-------------------------------------------------------------------------------------------------------
void CarModel::First()
{
	if (fCam)  fCam->First();
	iFirst = 0;

	for (int w=0; w < numWheels; ++w)  // hide trails
	if (whTrail[w])
		whTrail[w]->setInitialWidth(0, 0.f);
}

void CarModel::UpdKeysCam()
{
	if (!pCar)  return;

	///  🔵 goto last checkp - reset cam 🎥
	if (pCar->bLastChk && !bLastChkOld)
		First();
		
	bLastChkOld = pCar->bLastChk;

	///  🎥 change Cameras  ---------------------------------
	int iC = pCar->iCamNext;  // iRplCarOfs..
	if (iC != 0 && iCamNextOld == 0 && !pApp->isFocGui)
	{
		//  with ctrl - change current camera car index  (mouse move camera for many players)
		auto plr = pSet->game.local_players;
		if (pApp->ctrl && iIndex == 0)
			pApp->iCurCam = (pApp->iCurCam + iC + plr) % plr;
		else
		{
			// int visMask = 255;
			// pApp->roadUpdTm = 1.f;

			if (fCam)
			{	fCam->Next(iC < 0, pApp->shift);
				pApp->carsCamNum[iIndex] = fCam->miCurrent +1;  // save for pSet
				
				// uint32 visMask = fCam->ca->mHideGlass ? RV_MaskAll-RV_CarGlass : RV_MaskAll;
				
				// todo: hide glass in car ..
		        // void addListener( CompositorWorkspaceListener *listener );  //? cd ..
				// pApp->mGraphicsSystem->mWorkspace->setVisibilityMask(visMask);

				// for (auto vp : pApp->mSplitMgr->mViewports)
				// 	vp->setVisibilityMask(visMask);
		}	}
	}
	iCamNextOld = iC;
}



//  utility
//-------------------------------------------------------------------------------------------------------
void CarModel::updLightsBright()
{
	for (auto& l : lights)
	if (l.type == LI_Front)  // rest upd each frame
		l.li->setPowerScale( l.power * pSet->car_light_bright);
}

//  ✨ upd particles
void CarModel::UpdParsTrails(bool visible)
{
	bool vis = visible && pSet->particles;
	for (int w=0; w < numWheels; ++w)
	{
		uint8 grp = RQG_CarTrails;  //9=road  after glass
		if (w < PAR_BOOST && parBoost[w]) {  parBoost[w]->setVisible(vis);  /*parBoost[w]->setRenderQueueGroup(grp);  */}
		
		if (whTrail[w]){  whTrail[w]->setVisible(visible && pSet->trails);  whTrail[w]->setRenderQueueGroup(grp);  }
		
		grp = RQG_CarParticles;
		for (int p=0; p < PAR_ALL; ++p)
			if (par[p][w]){  par[p][w]->setVisible(vis);  par[p][w]->setRenderQueueGroup(grp);  }
		
		if (parHit && w==0)	{  parHit->setVisible(vis);  parHit->setRenderQueueGroup(grp);  }
	}
	for (int w=0; w < PAR_THRUST*2; ++w)
		if (parThrust[w]) {  parThrust[w]->setVisible(vis);  /*parThrust[w]->setRenderQueueGroup(RQG_CarTrails);*/  }
}


///  ⚫📉  to display info on wheel surfaces
//-------------------------------------------------------------------------------------------------------
void CarModel::UpdWhTerMtr()
{
	if (!pCar || !ndWh[0])  return;
	//int t = blendMapSize;
	//Real tws = sc->td.fTerWorldSize;

	txtDbgSurf = "";
	for (int i=0; i < pCar->numWheels; ++i)
	{
		//Vector3 w = ndWh[i]->getPosition();
		//int mx = (w.x + 0.5*tws)/tws*t, my = (-w.z + 0.5*tws)/tws*t;
		//mx = std::max(0,std::min(t-1, mx)), my = std::max(0,std::min(t-1, my));

		//int mtr = pCar->dynamics.bWhOnRoad[i] ? 0 : blendMtr[my*t + mx];
		//whTerMtr[i] = mtr;
		//whRoadMtr[i] = pCar->dynamics.bWhOnRoad[i];

		const CARDYNAMICS& cd = pCar->dynamics;  int iRd = cd.iWhOnRoad[i];
		float d = 0.5f * cd.wheel_contact[i].GetDepth() / cd.wheel[i].GetRadius();

		const TRACKSURFACE* tsu = cd.GetWheelContact(WHEEL_POSITION(i)).GetSurfacePtr();
		//pCar->dynamics.bTerrain = true;

		if (pSet->car_dbgsurf)  // dbg info surf  -------
		{
			// const auto& td = sc->tds[0];  // 1st ter only
			// TerLayer& lay = /*mtr == 0 ? td.layerRoad :*/ td.layersAll[ td.layers[ std::min((int)td.layers.size()-1, mtr-1) ] ];
			txtDbgSurf += //"mx " + toStr(mx) + " my " + toStr(my) +
				( iRd == 0	? ( "T" + toStr(cd.whTerMtr[i]) )  // Terrain/Pipe/Road
							: ( (iRd==2 ? "P" : "R") + toStr(cd.whRoadMtr[i]) )  ) +
				(!tsu ? "  --" : (
					"  " + tsu->name + " " + csTRKsurf[tsu->type] + //" [" + lay.texFile + "] " +
					"\n      "+ tsu->tireName + "\n     "+
					" d= " + fToStr(d, 2,5) + "  dr " + fToStr(tsu->rollingDrag, 0,3) + " rr " + fToStr(tsu->rollingResist, 1,3) +
					"  fr " + fToStr(tsu->friction, 2,4) +
					"  ba " + fToStr(tsu->bumpAmplitude, 2,4) + " bw " + fToStr(tsu->bumpWaveLength, 2,4) +
					"  b0 " + fToStr(tsu->tire->longitudinal[0], 3,5)
					//,lay.dust, lay.mud, lay.dustS	//,lay.tclr.r, lay.tclr.g, lay.tclr.b, lay.tclr.a
				)) + "\n";
		}
	}
}


//  🎨 paint
//-------------------------------------------------------------------------------------------------------
void CarModel::SetPaint()
{
	int i = iColor;
	auto gc = pSet->gui.clr[i];
	
	ColourValue diff, spec;  //, fresn1, fresn2;
	auto c = gc.clr[0];
	diff.setHSB(1.f - c.hue, c.sat, c.val);

	bool one = gc.type == CP_OneClr;
	if (one)
		spec = diff;
	else
	{	c = gc.clr[1];
		spec.setHSB(1.f - c.hue, c.sat, c.val);
	}

	color = diff;  // for mini pos clr
	if (!db)  return;

	#define toV3(cv)  Vector3(cv.r, cv.g, cv.b)
	db->setSpecular( toV3(spec) * (!one ? 1.f : gc.gloss) );  // ok~
	db->setDiffuse(  toV3(diff) * (!one ? 1.f : (1.f - gc.gloss)) );
	db->setWorkflow(  // once?
		HlmsPbsDatablock::SpecularWorkflow  // par?
		// HlmsPbsDatablock::SpecularAsFresnelWorkflow
		// HlmsPbsDatablock::MetallicWorkflow
	);
	// db->setMetalness( gc.metal );  // only in metallic
	db->setRoughness( std::max(0.001f, gc.rough) );

	db->setClearCoat( gc.clear_coat );
	db->setClearCoatRoughness( std::max(0.001f, gc.clear_rough) );
	
	// db->setIndexOfRefraction( Vector3::UNIT_SCALE * (3.f-gc.fresnel*3.f), false );


	//  color changing 3 paints
	bool clr3 = gc.type == CP_3Clrs;
	if (!clr3)
		db->paintMul = Vector4::ZERO;
	else
	{	db->paintMul = Vector4(gc.paintMulAll, gc.paintMul2nd, gc.paintPow3rd, 1.f);

		for (int i=0; i < 3; ++i)
		{
			const auto& c = gc.paints[i];
			ColourValue p;
			p.setHSB(1.f - c.hue, c.sat, c.val);
			db->paintClr[i] = Vector4(p.r, p.g, p.b, 1.f);
			if (i == 1)
				color = p;  // for netw, nick
		}
	}

	db->setFresnel( Vector3::UNIT_SCALE * gc.fresnel, false );
	// or db->scheduleConstBufferUpdate();

	if (pNickTxt)
		pNickTxt->setTextColour(MyGUI::Colour(color.r, color.g, color.b));
	
	// opp list text and mini pos colors - auto in hud update


	//  under glow light  --------
	ColourValue cg;
	c = gc.glow;
	cg.setHSB(1.f - c.hue, c.sat, c.val);
	
	for (auto& l : lights)
	if (l.type == LI_Under)
		if (gPar.carPrv == 0)
		{
			float bright = 1.17f * pSet->bright, contrast = pSet->contrast;
			l.li->setDiffuseColour(  cg * bright * contrast );
			l.li->setSpecularColour( cg * bright * contrast );
		}
	else
		l.li->setVisible(0);
}
