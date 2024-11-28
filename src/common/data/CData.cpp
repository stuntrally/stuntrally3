#include "pch.h"
#include "CData.h"
#include "paths.h"
#include "Def_Str.h"
using Ogre::String;

#include "FluidsXml.h"
#include "BltObjects.h"
#include "TracksXml.h"
#include "PresetsXml.h"
#ifdef SR_EDITOR
	#include "SceneXml.h"
#else
	#include "ChampsXml.h"
	#include "ChallengesXml.h"
	#include "CollectXml.h"
#endif

#ifndef SR_EDITOR  // game

//  📐 get drivability, vehicle on track fitness
//---------------------------------------------------------------------
float CData::GetDrivability(std::string car, std::string trk, bool track_user)
{
	if (track_user)  return -1.f;  // unknown

	int cid = cars->carmap[car];
	if (cid == 0)  return -1.f;
	const CarInfo& ci = cars->cars[cid-1];

	int tid = tracks->trkmap[trk];
	if (tid == 0)  return -1.f;
	const TrackInfo& ti = tracks->trks[tid-1];

	float undrv = 0.f;  // 0 drivable .. 1 undrivable
	int w = std::max(0, ci.width - 3);
	undrv += 0.8f * w/3.f * ti.narrow /3.f;
	undrv += 0.2f * w/3.f * ti.obstacles /4.f;
	undrv += 0.7f * ci.bumps /3.f * ti.bumps /4.f;  // * tweak params

	undrv += 1.1f * ci.jumps /3.f * ti.jumps /4.f;
	undrv += 1.1f * ci.loops /4.f * ti.loops; // /5.f;
	undrv += 1.4f * ci.pipes /4.f * ti.pipes /4.f;
	undrv += 1.4f * ci.underwater * ti.underwater;

	bool wnt = (ti.scenery == "Winter") || (ti.scenery == "WinterWet");
	if (wnt && ci.wheels >= 2)  // too slippery for fast cars
		undrv += 0.7f * std::max(0.f, ci.speed -7.f) /2.f;  // /10.f;

	return std::min(1.f, undrv);
}

#endif


//  📄📄 Load all xmls
//---------------------------------------------------------------------
void CData::Load(std::map <std::string, int>* surf_map, bool check)
{
	//  common
	fluids->LoadXml(PATHS::Data() + "/materials/fluids.xml", /**/surf_map);
	LogO(String("L*** Loaded Fluids: ") + toStr(fluids->fls.size()));

	objs->LoadXml();  //  collisions.xml
	LogO(String("L*** Loaded Vegetation collisions: ") + toStr(objs->colsMap.size()));
	
	auto snd = PATHS::Sounds();
	reverbs->LoadXml(snd + "/reverbs.xml");
	LogO(String("L*** Loaded Reverbs sets: ") + toStr(reverbs->revs.size()));

	//  cars and tracks
	auto path = PATHS::GameConfigDir();
	tracks->LoadIni(path + "/tracks.ini", check);
	auto pUser = PATHS::UserConfigDir();
	user->LoadXml(pUser + "/tracks.xml", tracks);
	
	pre->LoadXml(path + "/presets.xml");
	LogO(String("L*** Loaded Presets") +
		"  sky: " + toStr(pre->sky.size()) +
		"  ter: " + toStr(pre->ter.size()) +
		"  road: " + toStr(pre->rd.size()) +
		"  grass: " + toStr(pre->gr.size()) +
		"  veget: " + toStr(pre->veg.size()) +
		"  obj: " + toStr(pre->obj.size()) +
		"  col: " + toStr(pre->col.size())
		);
	#ifndef SR_EDITOR
		cars->LoadXml(path + "/cars.xml");
		LoadPaints();

		champs->LoadXml(path + "/championships.xml", tracks, check);
		LogO(String("L*** Loaded Championships: ") + toStr(champs->all.size()));

		chall->LoadXml(path + "/challenges.xml", tracks, check);
		LogO(String("L*** Loaded Challenges: ") + toStr(chall->all.size()));

		collect->LoadXml(path + "/collections.xml", tracks, check);
		LogO(String("L*** Loaded Collections: ") + toStr(collect->all.size()));
	#endif


	#ifndef SR_EDITOR  // game challs drivability
	if (check)
	{
		LogO("))) Checking for low drivability in all challs, tracks, cars");
		for (const Chall& ch : chall->all)
			for (const ChallTrack& t : ch.trks)
			{
				for (const CarInfo& c : cars->cars)
				if (ch.cars.Allows(cars, c.id))
				{
					float drv = GetDrivability(c.id, t.name, false);
					float drvp = (1.f - drv) * 100.f;
					if (drvp < 10.f)  // par  undrivable
						LogO("U  "+ ch.name +"  "+ t.name +"  "+ c.id +"  "+ fToStr(drvp,0,2) +" %");
			}	}
		LogO("");
	}
	#endif
}

#ifndef SR_EDITOR
void CData::LoadPaints(bool forceOrig)
{
	if (!paints)  return;
	auto path = PATHS::GameConfigDir(),
		 user = PATHS::UserConfigDir();
	auto ini = "/paints.ini";

	if (PATHS::FileExists(user + ini) && !forceOrig)
	{
		paints->Load(user + ini);
		LogO(String("L*** Loaded user Paints: ") + toStr(paints->v.size()));
	}else
	{	paints->Load(path + ini);
		LogO(String("L*** Loaded game Paints: ") + toStr(paints->v.size()));
	}
}
#endif


//  🌟 ctor
//---------------------------------------------------------------------
CData::CData()
{
	fluids = new FluidsXml();
	objs = new BltObjects();
	reverbs = new ReverbsXml();

	tracks = new TracksIni();
	  user = new UserXml();
	   pre = new Presets();

	#ifndef SR_EDITOR
		cars = new CarsXml();
		paints = new PaintsIni();
		
		champs = new ChampsXml();
		chall = new ChallXml();
		collect = new CollectXml();
	#endif
}

CData::~CData()
{
	delete fluids;
	delete objs;
	delete reverbs;

	delete tracks;
	delete user;
	delete pre;

	#ifndef SR_EDITOR
		delete cars;
		delete paints;

		delete champs;
		delete chall;
		delete collect;
	#endif
}
