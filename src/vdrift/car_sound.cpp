#ifndef SR_EDITOR
#include "pch.h"
#include "par.h"
#include "car.h"
#include "cardefs.h"
#include "collision_world.h"
#include "tracksurface.h"
#include "settings.h"
#include "CGame.h"  // replay
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
 #include "Axes.h"
#include "game.h"
#include "SoundMgr.h"
#include <OgreCamera.h>
using namespace std;
using namespace Ogre;


//  🆕 Load
//--------------------------------------------------------------------------------------------------------------------------
bool CAR::LoadSounds(const std::string & carpath)
{
	Ogre::Timer ti;
	bool ss = pApp->pSet->game.local_players > 1;  // 👥 split screen same volume
	CARsounds& s = sounds;
	
	SoundMgr* snd = pGame->snd;
	
	//  📈 engine
	const string& eng = dynamics.engine.sound_name;
	s.engine = snd->createInstance(eng);  s.engine->set2D(ss);
	s.engine->setEngine(true);  s.engine->start();  // 🔉
	
	//  turbo
	const string& turbo = dynamics.turbo.sound_name;
	if (!turbo.empty())
	{	s.turbo = snd->createInstance(turbo);  s.turbo->set2D(ss);
		s.turbo->setEngine(true);  s.turbo->start();  // 🔉
	}
	//  gear shift
	s.gearChg = snd->createInstance(dynamics.gear_sound);  s.gearChg->set2D(ss);

	//  ⚫ wheels, tires
	int i;  float fw = numWheels;
	for (i = 0; i < numWheels; ++i)
	{
		s.asphalt[i] = snd->createInstance("asphalt");	s.asphalt[i]->set2D(ss);
		s.grass[i]   = snd->createInstance("grass");
		s.grass[i]->seek(float(i)/fw);  s.grass[i]->set2D(ss);
		s.gravel[i]  = snd->createInstance("gravel");
		s.gravel[i]->seek(float(i)/fw);  s.gravel[i]->set2D(ss);
	}
	//  susp bump
	for (i = 0; i < numWheels; ++i)
	{
		s.bump[i] = snd->createInstance("bump"+toStr(i%4));  s.bump[i]->set2D(ss);
		s.bump[i]->seek(float(i)/fw);
	}

	//  🔨 crashes, hit
	for (i = 0; i < Ncrashsounds; ++i)
	{	string cn = "crash/";  int n=i+1;  cn += toStr(n/10)+toStr(n%10);
		s.crash[i] = snd->createInstance(cn);  s.crash[i]->set2D(ss);
	}
	s.scrap   = snd->createInstance("crash/scrap");    s.scrap->set2D(ss);
	s.screech = snd->createInstance("crash/screech");  s.screech->set2D(ss);

	//  todo: allow variations per track, from scene.xml
	//  🌪️ environment
	s.wind  = snd->createInstance("wind");   s.wind->set2D(ss);
	s.boost = snd->createInstance("boost");  s.boost->set2D(ss);

	//  💧 fluids
	for (i = 0; i < Nwatersounds; ++i)
	{	s.water[i] = snd->createInstance("water"+toStr(i+1));  s.water[i]->set2D(ss);  }

	s.mud        = snd->createInstance("mud1");        s.mud->set2D(ss);
	s.mud_cont   = snd->createInstance("mud_cont");    s.mud_cont->set2D(ss);
	s.water_cont = snd->createInstance("water_cont");  s.water_cont->set2D(ss);

	LogO(":::* Time car Sounds: "/*+carpath+" "*/+ fToStr(ti.getMilliseconds(),0,3) +" ms");
	return true;
}


//  🌟 ctor
CAR::CARsounds::CARsounds()
{
	int i;
	crashtime.resize(Ncrashsounds);
	for (int i=0; i < Ncrashsounds; ++i)
		crashtime[i] = 0.f;

	SetNumWheels(4);
	for (i = 0; i < 4; ++i)  // tires
	{	asphalt[i] = 0;  grass[i] = 0;  gravel[i] = 0;  bump[i] = 0;  }

	crash.resize(Ncrashsounds);
	for (i = 0; i < Ncrashsounds; ++i)  // crashes
		crash[i] = 0;

	water.resize(Nwatersounds);
	for (i = 0; i < Nwatersounds; ++i)  // fluids
		water[i] = 0;
}

void CAR::CARsounds::SetNumWheels(int n)
{
	asphalt.resize(n);  grass.resize(n);  gravel.resize(n);  bump.resize(n);
	bumptime.resize(n);  bumpvol.resize(n);
	for (int i=0; i < n; ++i)
	{	bumpvol[i]=0.f;  bumptime[i] = 5.f;  }
}

//  💥 destroy
void CAR::CARsounds::Destroy()
{
	delete engine;
	delete turbo;
	int i;
	for (i = 0; i < gravel.size(); ++i)  // tires
	{
		delete asphalt[i];
		delete grass[i];
		delete gravel[i];
		delete bump[i];
	}

	for (i = 0; i < Ncrashsounds; ++i)  // crashes
		delete crash[i];

	delete scrap;  delete screech;

	delete wind;  delete boost;

	for (i = 0; i < Nwatersounds; ++i)  // fluids
		delete water[i];

	delete mud;  delete mud_cont;  delete water_cont;
}


//  🔊💫 Update Sounds
//--------------------------------------------------------------------------------------------------------------------------
void CAR::UpdateSounds(float dt)
{
	float totalgain = 0.0, fullgain = 0.0;
	Sound *carsound = nullptr,
			*turbosound = nullptr,
			*gearsound = nullptr,
			*tyresounds[numWheels],
			*suspbumpsounds[numWheels],
			*windsound = nullptr;
	Sound* fhitsound = nullptr, 
	*watercontsound = nullptr, 
	*mudcontsound = nullptr, *boostsound = nullptr;
	float fhitgain = 0.0, mudcontgain = 0.0, watercontgain = 0.0, boostgain = 0.0;
	Sound *crashsound = nullptr, 
	*scrapsound = nullptr, 
	*screechsound = nullptr;
	float crashgain = 0.0, scrapgain = 0.0, screechgain = 0.0;
	
	
	float cargain = 0.0,
			turbogain = 0.0,
			geargain = 0.0,
			tyregains[numWheels],
			suspbumpgains[numWheels],
			windgain = 0.0;
	for (int i = 0; i < numWheels; i++) {
		tyresounds[i] = nullptr;
		suspbumpsounds[i] = nullptr;
		tyregains[i] = 0.0;
		suspbumpgains[i] = 0.0;
	}

	//  get data  //
	//  note: Damage is updated here
	bool bSound = !pGame->snd->isDisabled();
	CARsounds& s = sounds;
	
	//  prepare vars  ------------------------------------------
	float rpm, throttle, brake, speed, dynVel;  bool hitp = false; float maxWheelVel;
	MATHVECTOR<float,3> pos, engPos, whPos[MAX_WHEELS], hitPos;  // car, engine, wheels pos
	QUATERNION<float> rot;
	TRACKSURFACE::TYPE surfType[MAX_WHEELS];
	float squeal[MAX_WHEELS],whVel[MAX_WHEELS], suspVel[MAX_WHEELS],suspDisp[MAX_WHEELS];
	float whH_all = 0.f;  bool mud = false;
	float fHitForce = 0.f, boostVal = 0.f, fCarScrap = 0.f, fCarScreech = 0.f;

	bool dmg = pSet->game.damage_type > 0, reduced = pSet->game.damage_type==1;
	bool terminal = dynamics.fDamage >= 100.f;
	float fDmg = pApp->scn->sc->damageMul;
	bool contacts[MAX_WHEELS];
	bool is_shifting = false;
	//  📽️ replay play  ------------------------------------------
	if (pApp->bRplPlay)
	{	dmg = false;

		#ifdef DEBUG
		assert(id < pApp->frm.size());
		#endif
		const ReplayFrame2& fr = pApp->frm[id];
		pos = fr.pos;  rot = fr.rot;   rpm = fr.rpm;
		
		is_shifting = fr.is_shifting;

		throttle = fr.throttle /255.f;  boostVal = fr.fboost /255.f;
		dynamics.fDamage = fr.damage /255.f*100.f;  //dmg read
		brake = fr.get(b_braking) ? 1.f : 0.f;

		MATHVECTOR<float,3> offset = dynamics.engine.GetPosition();
		rot.RotateVector(offset);
		engPos = offset + pos;

		speed = fr.speed;  dynVel = fr.dynVel;
		s.whMudSpin = fr.get(b_fluid) ? fr.whMudSpin : 0.f;

		if (fr.get(b_scrap))
		{
			const RScrap& sc = fr.scrap[0];
			fCarScrap = sc.fScrap;  fCarScreech = sc.fScreech;
		}

		hitp = fr.get(b_hit);
		if (hitp)
		{
			const RHit& h = fr.hit[0];
			fHitForce = h.fHitForce;
			hitPos[0] = h.vHitPos.x;  hitPos[1] = -h.vHitPos.z;  hitPos[2] = h.vHitPos.y;
		}

		int w, ww = fr.wheels.size();
		for (w=0; w < ww; ++w)
		{
			const RWheel& wh = fr.wheels[w];
			contacts[w] = wh.contact;
			whPos[w] = wh.pos;
			surfType[w] = (TRACKSURFACE::TYPE)wh.surfType;
			squeal[w] = wh.squeal;  whVel[w] = wh.whVel;
		
			suspVel[w] = wh.suspVel;  suspDisp[w] = wh.suspDisp;
			
			if (wh.whAngVel > maxWheelVel)
				maxWheelVel = (float)wh.whAngVel;
			//  fluids
			if (wh.whP >= 0)  // solid no snd
				whH_all += wh.whH;
			if (wh.whP >= 1)  mud = true;
		}
	}
	else  /// game  ------------------------------------------
	{
		
		
		is_shifting = dynamics.rem_shift_time > 0.0001;
		pos = dynamics.GetPosition();  rot = dynamics.GetOrientation();
		rpm = GetEngineRPM();
		throttle = dynamics.GetThrottle();
		brake = dynamics.IsBraking() ? 1.f : 0.f;

		engPos = dynamics.GetEnginePosition();
		speed = GetSpeed();
		dynVel = dynamics.GetVelocity().Magnitude();
		maxWheelVel = 0.0;

		fHitForce = dynamics.fHitForce;  hitp = true;
		hitPos[0] = dynamics.vHitPos.x;  hitPos[1] = -dynamics.vHitPos.z;  hitPos[2] = dynamics.vHitPos.y;
		boostVal = dynamics.boostVal;
		
		for (int w=0; w < numWheels; ++w)
		{
			WHEEL_POSITION wp = WHEEL_POSITION(w);
			whPos[w] = dynamics.GetWheelPosition(wp);

			const TRACKSURFACE* surface = dynamics.GetWheelContact(wp).GetSurfacePtr();

			surfType[w] = !surface ? TRACKSURFACE::NONE : surface->type;
			//  squeal
			squeal[w] = GetTireSquealAmount(wp);
			whVel[w] = dynamics.GetWheelVelocity(wp).Magnitude();
			contacts[w] = (dynamics.GetWheelContact(wp).GetDepth() - 2*GetTireRadius(wp)) > 0.0;
			
			//  susp
			suspVel[w] = dynamics.GetSuspension(wp).GetVelocity();
			suspDisp[w] = dynamics.GetSuspension(wp).GetDisplacementPercent();
			//  fluids
			if (dynamics.whP[w] >= 0)  // solid no snd
				whH_all += dynamics.whH[w];
			if (dynamics.whP[w] >= 1)  mud = true;
		}

		//  ⚫ wheels in mud, spinning intensity
		float mudSpin = 0.f;
		for (int w=0; w < numWheels; ++w)
		{
			float vel = std::abs(dynamics.wheel[w].GetAngularVelocity());
			if (vel > maxWheelVel)
				maxWheelVel = vel;
			if (vel <= 30.f)  continue;
			if (dynamics.whP[w] == 2)
				mudSpin += dynamics.whH[w] * std::min(80.f, 1.5f * vel) / 80.f;
			else if (dynamics.whP[w] == 1)
				mudSpin += dynamics.whH[w] * std::min(160.f, 3.f * vel) / 80.f;
		}
		s.whMudSpin = mudSpin * 0.5f;

		//  car scrap, screech
		float gain = std::min(1.f, dynamics.fCarScrap);
		if (dynamics.fCarScrap > 0.f)
		{	dynamics.fCarScrap -= (-gain * 0.8f + 1.2f)* dt;
			if (dynamics.fCarScrap < 0.f)  dynamics.fCarScrap = 0.f;
		}
		fCarScrap = gain;

		/// <><> 🔨 Damage <><>
		if (dmg && !terminal)
			if (reduced)
				dynamics.fDamage += fDmg * fCarScrap * dt * dynamics.fHitDmgA * gPar.dmgFromScrap;
			else  // normal
				dynamics.fDamage += fDmg * fCarScrap * dt * dynamics.fHitDmgA * gPar.dmgFromScrap2;

		gain = std::min(1.f, dynamics.fCarScreech);
		if (dynamics.fCarScreech > 0.f)
		{	dynamics.fCarScreech -= 3.f * dt;
			if (dynamics.fCarScreech < 0.f)  dynamics.fCarScreech = 0.f;
		}
		fCarScreech = gain;
	}
	
	//  engine pos  //todo: vel..
	Vector3 ep, ev = Vector3::ZERO;
	ep = Axes::toOgre(engPos);

#define nmax(a, b) ((a) > (b))? (a):(b)
#define nmin(a, b) (a > b)? b:a
#define n0(n) nmax(0.001, n)

	float gain = 1.f;
	float F = 0.0f;
	float proxyrpm = rpm;
	carsound = s.engine;
	bool ssnd = false;
	float vfac = dynamics.shift_time / 0.5;
	float damping;
//))  💫 update sounds 🔊  ---------------------------------------------------------------
if (bSound)
{
	///  📈 engine  ====
	
	switch (int(dynamics.vtype))
	{
	case V_Car:  //  🚗 car
		gain = throttle * 0.95 + 0.05;  // par

		// last_c_pitch = c_pitch;
		ssnd = is_shifting;
		proxyrpm = rpm * (!ssnd) + (0.5 + vfac) * rpm * (ssnd);
		damping = (50 + 30 * dynamics.transmission.GetGear()) * vfac;
		F = - 300.0 / vfac * (c_pitch - proxyrpm) - (nmax(30.0, damping)) * c_vel;
		c_vel += F * dt;
		c_pitch += c_vel * dt;
		s.engine->setPitch(c_pitch);
		break;
	
	case V_Hovercraft:	//  hovercraft propeller
		gain = max(brake, throttle) * 0.6 + 0.4;  // par..
		rpm = min(0.7f, dynVel * 0.01f);
		s.engine->setPitch(0.5f + throttle * 0.2f + rpm);
		break;
	
	case V_Hovercar:   //  sf hover 🚤
		gain = max(brake, throttle) * 0.4 + 0.6;  // par..
		rpm = min(0.7f, dynVel * 0.002f);
		s.engine->setPitch(0.4f + throttle * 0.1f + rpm);
		break;
	case V_Drone:      //  sf drone
		gain = max(brake, throttle) * 0.7 + 0.3;  // par..
		rpm = min(0.7f, dynVel * 0.003f);
		s.engine->setPitch(0.4f + throttle * 0.2f + rpm);
		break;

	case V_Sphere:     //  sph 🔘
	case V_Spaceship:  //  spc 🚀
		s.engine->setPitch(1.f);
		gain = throttle;
		break;
	}
	s.engine->setPosition(ep, ev);
	float low0 = 1.0;
	float peak = 8.0;
	float low1 = 4.0;
	float peakx = 0.75;

	float maxrpm = dynamics.GetEngine().GetRpmMax();;

	float px = peakx * maxrpm;

	float sndgainp = 0.0;
	if (rpm < px) {
		float t = rpm / px;
		sndgainp = low0 + t * (peak - low0);
		// sndgainp = 0.0;
	}
	else {
		float t = (rpm - px) / (maxrpm - px);
		sndgainp = peak + t * (low1 - peak);
		// sndgainp = 10.0;
	}

	// s.engine->setGain((n0(sndgainp) + gain) * dynamics.engine_vol_mul * pSet->s.vol_engine);
	cargain = (n0(sndgainp) * gain) * dynamics.engine_vol_mul * pSet->s.vol_engine;
	totalgain += cargain;
	fullgain += (1.0 + peak);


	//  turbo  ----
	if (s.turbo)
	{	
		

		// s.bov->setGain(turbopressure * wastegate);

		
		s.turbo->setPosition(ep, ev);
		auto& tb = dynamics.turbo;
		
		s.turbo->setPitch(maxWheelVel / 100.0);
		gain = (c_pitch - tb.rpm_min) / (tb.rpm_max - tb.rpm_min);
		gain = 20.0 * min(1.f, max(0.001f, gain));
		
		float thr = throttle * tb.vol_max + tb.vol_idle;
		// s.turbo->setGain(thr * gain * pSet->s.vol_turbo * pSet->s.vol_engine);

		turbosound = s.turbo;
		turbogain = thr * gain * pSet->s.vol_turbo * pSet->s.vol_engine;
		totalgain += turbogain;
		
		fullgain += 3;
		
	}

	gearsound = s.gearChg;
	float r1000 = rpm / maxrpm;
	bool overrun = r1000 > 0.9;
	bool gchange = is_shifting; // shift in process
	float overrungain = 0.0, gchangegain = 0.0;
	if (overrun) {
		overrungain = 4.0 * pSet->s.vol_gear * n0((rpm - 0.9* maxrpm)) / (0.1 * maxrpm);
		gchangegain = 0.0;
		
	}
	if (gchange) {
		gchangegain = 4.0 * (r1000 * pSet->s.vol_gear);
		overrungain = 0.0;
		
	}
	if (overrun || gchange) {
		fullgain += 1.0;
	}

	float ggain = gchangegain + overrungain;
	// s.gearChg->setGain(ggain + (0.001));
	geargain = ggain + 0.001;
	totalgain += geargain;
	// fullgain += 1.0;//(overrun + gchange + 2.0);
	s.gearChg->setPosition(ep, ev);
	s.gearChg->runOnce();
	
	for (int i = 0; i < numWheels; ++i)
	{
		tyregains[i] = 0.0;
		suspbumpgains[i] = 0.0;

		Vector3 wh = Axes::toOgre(whPos[i]);

		float maxgain = 0.6f, pitchvar = 0.4f, pmul = 1.f;

		Sound* snd = s.gravel[i];
		switch (surfType[i])
		{
		case TRACKSURFACE::ASPHALT:		snd = s.asphalt[i];  maxgain = 0.6f;  pitchvar = 0.40f;  pmul = 0.8f;  break;
		case TRACKSURFACE::GRASS:		snd = s.grass[i];    maxgain = 1.5f;  pitchvar = 0.25f;  break;
		case TRACKSURFACE::GRAVEL:		snd = s.gravel[i];   maxgain = 1.0f;  break;
		case TRACKSURFACE::CONCRETE:	snd = s.asphalt[i];  maxgain = 1.4f;  pitchvar = 0.25f;  pmul = 0.7f;  break;
		case TRACKSURFACE::SAND:		snd = s.gravel[i];    maxgain = 1.0f;  pitchvar = 0.25f;  break;
		case TRACKSURFACE::NONE:
						default:		snd = s.asphalt[i];  maxgain = 0.6f;  break;
		}
		/// todo: more,sounds.. sand,snow,grass-new,mud..
		// todo: sum slip, spin, stop tire sounds

		float pitch = 0.0;
		// pitch = std::min(1.f, std::max(0.f, (whVel[i]-5.0f)*0.1f ));
		// pitch = (1.f - pitch) * pitchvar;
		// pitch = (pitch + (1.f - pitchvar));
		pitch = std::min(1.f, std::max(0.28f, squeal[i]));

		snd->setPosition(wh, ev);
		// snd->setGain(5 * squeal[i]*maxgain * pSet->s.vol_tires);

		tyresounds[i] = snd;
		float fac = nmin(nmax(0.01, dynVel), 75.0) / 75.0;
		
		tyregains[i] = !contacts[i] * (0.5 * fac + 0.3 * squeal[i]) * maxgain * pSet->s.vol_tires;
		totalgain += tyregains[i];
		fullgain += 1.0;

		snd->setPitch(pitch * pmul);
		//  mute others
		if (snd != s.asphalt[i])  s.asphalt[i]->setGain(0.f);
		if (snd != s.grass[i])    s.grass[i]->setGain(0.f);
		if (snd != s.gravel[i])   s.gravel[i]->setGain(0.f);


		//  🪜 susp bump  ~~~
		auto vt = dynamics.vtype;
		if (vt == V_Car || vt == V_Hovercraft)
		{
			float mul = vt == V_Hovercraft ? 2.f : 1.f;  //par
			suspbump[i].Update(suspVel[i] * mul, suspDisp[i] * mul, dt);
			if (suspbump[i].JustSettled())
			{
				float bumpsize = suspbump[i].GetTotalBumpSize();
				float gain = mul * bumpsize * speed * 0.2f;  //par
				gain = std::max(0.f, std::min(1.2f, gain));

				if (gain > 0.1f &&  //par  //!tirebump[i]->isAudible() &&
					(gain > s.bumpvol[i] || s.bumptime[i] > 0.22f))
				{
					s.bumpvol[i] = gain;
					s.bumptime[i] = 0.f;
					//tirebump[i]->start();  // 🔉
					//LogO("bump "+toStr(i)+" "+fToStr(gain));
				}
			}
			s.bump[i]->setPosition(wh, ev);  //par gain, time fade
			float gain = 0.5f + 0.7f*s.bumpvol[i] - s.bumptime[i]*(2.f+2.f*s.bumpvol[i]);
			gain = std::max(0.f, std::min(1.0f, gain));

			// s.bump[i]->setGain(gain * pSet->s.vol_susp);
			
			suspbumpsounds[i] = s.bump[i];
			suspbumpgains[i] = gain * pSet->s.vol_susp;
			totalgain += suspbumpgains[i];
			fullgain += 1.0;
			
			if (s.bumptime[i] < 5.f)
				s.bumptime[i] += dt;
		}
	}
	

	//  🌪️ wind  ----
	gain = dynVel;
	if (dynamics.vtype == V_Spaceship)   gain *= 0.7f;
	//if (dynamics.sphere)  gain *= 0.9f;
	if (gain < 0.f)	gain = -gain;
	gain *= 0.02f;	gain *= gain;
	if (gain > 1.f)	gain = 1.f;
	
	// s.wind->setGain(gain * pSet->s.vol_env);
	
	windsound = s.wind;
	
	windgain = gain * pSet->s.vol_env;
	totalgain += windgain;
	fullgain += 1.0;
	
	s.wind->setPosition(ep, ev);

	//  💨 boost  ----
	boostsound = s.boost;
	boostgain = boostVal * 0.55f * pSet->s.vol_engine; 
	totalgain += boostgain;
	// s.boost->setGain(boostVal * 0.55f * pSet->s.vol_engine);

	s.boost->setPosition(ep, ev);  //back?-


	//  💧 fluids - hit  ~~~~
	bool fluidHit = whH_all > 1.f;
	//LogO(toStr(whH_all) + "  v "+ toStr(dynVel));
	
	if (fluidHit && !s.fluidHitOld)
	//if (dynVel > 10.f && whH_all > 1.f && )
	{
		int i = std::min(Nwatersounds-1, (int)(dynVel / 15.f));
		float gain = std::min(3.0f, 0.3f + dynVel / 30.f);
		Sound* snd = /*mud ? s.mud : */s.water[i];
		float fhitgain = 0.0;
		//LogO("fluid hit i"+toStr(i)+" g"+toStr(gain)+" "+(mud?"mud":"wtr"));
		if (!snd->isAudible())
		{
			// snd->setGain(gain * pSet->s.vol_fl_splash * (mud ? 0.6f : 1.f));
			fhitgain = 8 * gain * pSet->s.vol_fl_splash * (mud ? 0.6f : 1.f);
			snd->setPosition(ep, ev);
			snd->start();  // 🔉
		}

		if (s.mud)  {  snd = s.mud;
		if (!snd->isAudible())
		{
			// snd->setGain(gain * pSet->s.vol_fl_splash);
			fhitgain = 8 * gain * pSet->s.vol_fl_splash;
			snd->setPosition(ep, ev);
			snd->start();  // 🔉
		}	}
		fhitsound = snd;
	}
	s.fluidHitOld = fluidHit;

	//  🌊 fluids - continuous
	float velM = mud && whH_all > 0.1f ?
		s.whMudSpin * 2.5f : 0.f;
	mudcontsound = s.mud_cont;
	mudcontgain = std::min(4.f, 6 * velM) * pSet->s.vol_fl_cont * 0.85f;
	// s.mud_cont->setGain(std::min(1.f, velM) * pSet->s.vol_fl_cont * 0.85f);
	s.mud_cont->setPitch(std::max(0.7f, std::min(/*3.f*/2.f, velM * 0.35f)));
	s.mud_cont->setPosition(ep, ev);

	float velW = !mud && whH_all > 0.1f && whH_all < 3.9f ?
		dynVel / 30.f : 0.f;
	watercontsound = s.water_cont;
	watercontgain = std::min(4.f, velW * 5.5f) * pSet->s.vol_fl_cont;
	// s.water_cont->setGain(std::min(1.f, velW * 1.5f) * pSet->s.vol_fl_cont);
	s.water_cont->setPitch(std::max(0.7f, std::min(1.3f, velW)));
	s.water_cont->setPosition(ep, ev);
	totalgain += watercontgain + mudcontgain + fhitgain;
}
//))  sounds 🔊  ---------------------------------------------------------------
	
	
	//  🔨 crash  ----
	Vector3 hp = Axes::toOgre(hitPos);
	{
		crashdetection2.Update(-fHitForce, dt);
		crashdetection2.deceltrigger = 1.f;
		float crashdecel2 = crashdetection2.GetMaxDecel();
		dynamics.fHitForce3 = crashdecel2 / 30.f;

		if (crashdecel2 > 0)
		{
			float gain = 0.9f;
			
			int f = crashdecel2 / 30.f * Ncrashsounds;
			int i = std::max(1, std::min(Ncrashsounds-1, f));
			//LogO("crash: "+toStr(i));

			if (s.crashtime[i] > /*ti*/0.4f)  //!crashsound.isAudible())
			{
				
				if (bSound)
				{
					crashsound = s.crash[i];
					crashgain = 15 * gain * pSet->s.vol_car_crash;
					totalgain += crashgain;
					// s.crash[i]->setGain(gain * pSet->s.vol_car_crash);
					if (hitp)
					s.crash[i]->setPosition(hp, ev);
					s.crash[i]->start();  // 🔉
				}
				s.crashtime[i] = 0.f;
				
				/// <><> Damage <><> 
				if (dmg && !terminal)
					if (reduced)
						dynamics.fDamage += fDmg * crashdecel2 * dynamics.fHitDmgA * gPar.dmgFromHit;
					else  // normal
					{	float f = std::min(1.f, crashdecel2 / 30.f);
						f = powf(f, gPar.dmgPow2);
						dynamics.fDamage += fDmg * crashdecel2 * dynamics.fHitDmgA * gPar.dmgFromHit2 * f;
					}
			}
			//LogO("Car Snd: " + toStr(crashdecel));// + " force " + toStr(hit.force)
		}
	}
	//  time played
	for (int i=0; i < Ncrashsounds; ++i)
		if (s.crashtime[i] < 5.f)
			s.crashtime[i] += dt;
	

	//  🔨 crash scrap and screech
	if (bSound)
	{
		scrapsound = s.scrap;
		screechsound = s.screech;
		scrapgain = 3 * fCarScrap * pSet->s.vol_car_scrap;
		screechgain = 3 * fCarScreech * pSet->s.vol_car_scrap * 0.6f;
		totalgain += scrapgain + screechgain;
		// s.scrap->setGain(fCarScrap * pSet->s.vol_car_scrap);
		if (hitp)
		s.scrap->setPosition(hp, ev);

		// s.screech->setGain(fCarScreech * pSet->s.vol_car_scrap * 0.6f);
		if (hitp)
		s.screech->setPosition(hp, ev);
	}


	/// <><> 🔨 Damage <><> 
	if (dmg)
		if (dynamics.fDamage > 100.f)  dynamics.fDamage = 100.f;

	// float fgain = fullgain + 0.001;
	// float x = totalgain / (fgain);
	float threshold = 1.0;
	float cmpRatio = 0.25;
	float tGain = totalgain;
	if (totalgain > threshold)
		tGain = threshold * Math::Pow(totalgain / threshold, cmpRatio);
		// tGain = totalgain * cmpRatio;
	float tot_gd = totalgain - tGain;
	// tgd *= 1.0 / 12.0;
	// tot_gd = nmax(0.0, tot_gd);
	tgd = n0(tgd + (tot_gd - tgd) * 0.6); 

	if (carsound) carsound->setGain(n0(cargain - tgd * cargain / totalgain));
	// if (carsound) carsound->setGain(n0(0));
	if (fhitsound) fhitsound->setGain(n0(fhitgain - tgd * fhitgain / totalgain));
	if (watercontsound) watercontsound->setGain(n0(watercontgain - tgd * watercontgain/totalgain));
	if (mudcontgain) mudcontsound->setGain(n0(mudcontgain - tgd * mudcontgain/totalgain));
	if (crashsound) crashsound->setGain(n0(crashgain - tgd * crashgain / totalgain));
	if (scrapsound) scrapsound->setGain(n0(scrapgain - tgd *scrapgain / totalgain));
	if (screechsound) screechsound->setGain(n0(screechgain - tgd * screechgain / totalgain));
	if (boostsound) boostsound->setGain(n0(boostgain - tgd * boostgain / totalgain));
	if (turbosound) turbosound->setGain(n0(turbogain  - tgd * turbogain / totalgain));
	if (gearsound) gearsound->setGain(n0(geargain - tgd * geargain / totalgain));
	
	for (int i = 0; i < numWheels; i++) {
		if (tyresounds[i]) tyresounds[i]->setGain(n0(tyregains[i] - 0.5 * tyregains[i] / totalgain * tgd));
		if (suspbumpsounds[i]) suspbumpsounds[i]->setGain(n0(suspbumpgains[i] - suspbumpgains[i] / totalgain * tgd));
	}
	if (windsound) windsound->setGain(n0(windgain - windgain / totalgain * tgd));

	// if (carsound) carsound->setGain(n0(totalGainDifference * cargain / totalgain));
	// // if (carsound) carsound->setGain(n0(0));

	// if (turbosound) turbosound->setGain(n0(totalGainDifference * turbogain / totalgain));
	// if (gearsound) gearsound->setGain(n0(totalGainDifference * geargain / totalgain));
	// for (int i = 0; i < numWheels; i++) {
	// 	if (tyresounds[i]) tyresounds[i]->setGain(n0(tyregains[i] / totalgain * totalGainDifference));
	// 	if (suspbumpsounds[i]) suspbumpsounds[i]->setGain(n0(suspbumpgains[i] / totalgain * totalGainDifference));
	// }
	// if (windsound) windsound->setGain(n0(windgain / totalgain * totalGainDifference));

	// if (carsound) carsound->setGain(n0(cargain - totalGainDifference));
	// if (turbosound) turbosound->setGain(n0(turbogain  - totalGainDifference));
	// if (gearsound) gearsound->setGain(n0(geargain - totalGainDifference));
	// for (int i = 0; i < numWheels; i++) {
	// 	if (tyresounds[i]) tyresounds[i]->setGain(n0(tyregains[i] - totalGainDifference));
	// 	if (suspbumpsounds[i]) suspbumpsounds[i]->setGain(n0(suspbumpgains[i] - totalGainDifference));
	// }
	// if (windsound) windsound->setGain(n0(windgain - totalGainDifference));

	// if (carsound) carsound->setGain(totalGainDifference);
	// if (turbosound) turbosound->setGain(totalGainDifference);
	// if (gearsound) gearsound->setGain(totalGainDifference);

	// for (int i = 0; i < numWheels; i++) {
	// 	if (tyresounds[i]) tyresounds[i]->setGain(totalGainDifference);
	// 	if (suspbumpsounds[i]) suspbumpsounds[i]->setGain(0.001);
	// }

	// if (windsound) windsound->setGain(totalGainDifference);
}
#endif
