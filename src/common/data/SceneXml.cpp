#include "pch.h"
#include "Def_Str.h"
#include "par.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "TracksXml.h"
#include "SColor.h"
#include "Axes.h"
#include "game.h"  // for surfaces map

#include <OgreSceneNode.h>
#include <OgreVector3.h>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace std;
using namespace Ogre;


Scene::Scene()
	: pGame(0)
{
	pFluidsXml = 0;  pReverbsXml = 0;
	Default();
}
void Scene::Default()
{
	secEdited = 0;

	asphalt = false;  denyReversed = false;  noWrongChks = false;
	needLights = false;
	
	windAmt = 0.f;  damageMul = 1.f;
	gravity = 9.81f;
	
	ambientSnd = "";  sReverbs = "";
	ambientVol = 1.f;

	skyMtr = "World/NoonSky";  skyYaw = 0.f;
	rainEmit = 0;  rainName = "";
	rain2Emit = 0;  rain2Name = "";

	fogStart = 600;  fogEnd = 1600;
	fogClr = fogClr2 = fogClrH = SColor(0.73f, 0.86f, 1.0f, 1.f);
	fogHeight = -300.f;  fogHDensity = 100.f;  fogHStart = 0;  fogHEnd = 400;
	fHDamage = 0.f;
	road1mtr = true;

	ldPitch = 50.f;  ldYaw = 30.f;
	lAmb  = SColor(0.16f, 0.0f, 0.45f);
	lDiff = SColor(0.16f, 0.0f, 0.7f);	lSpec = SColor(0.16f, 0.05f, 1.f);

	sParDust = "Dust";  sParMud = "Mud";  sParSmoke = "Smoke";
	sFluidWater = "FluidWater";  sFluidMud = "FluidMud";  sFluidMudSoft = "FluidMudSoft";

	tds.clear();
	// td.Default();

	densTrees=0;  densGrass=0;  grDensSmooth=6;
	grPage = 80;  grDist = 80;

	for (int i=0; i < ciNumGrLay; ++i)
	{
		SGrassLayer* gr = &grLayersAll[i];
		gr->on = i == 0;
		gr->material = "grassJungle";  gr->color = SColor(0.f,0.f,1.f);  // white
		gr->minSx = 1.2f;  gr->minSy = 1.2f;  gr->maxSx = 1.6f;  gr->maxSy = 1.6f;
		gr->swayDistr = 4.0f;  gr->swayLen = 0.2f;  gr->swaySpeed = 0.5f;
	}
	trPage = 200;  trDist = 200;  trDistImp = 800;  trRdDist = 3;

	camPos = Vector3(10.f,20.f,10.f);  camDir = Vector3(0.f,-0.3f,1.f);

	fluids.clear();  objects.clear();  emitters.clear();  //

	for (int i=0; i < 4; ++i)  // road surf
	{
		TerLayer& r = layerRoad[i];
		r.dust = 0.f;  r.mud = 0.f;  // r.smoke = 1.f;
		r.tclr = SColor(0.16f,0.5f,0.2f,0.7f);  r.tcl = r.tclr.GetRGBA();
		r.fDamage = 0.f;
	}

	rorCfg.Default();
}


//  RoR defaults  ------
void RoRconfig::Default()
{
	lAmbAdd = Vector3(0.1f,0.1f,0.1f);  // 🌞 sun
	lAmb = 1.6f;  lDiff = 1.6f;  lSpec = 0.9f;
	
	water = 0;  // 🌊  auto
	yWaterOfs = -0.8f;  // easier drive through

	treesMul = 1.f;   // 🌳🪨  veget
	grassMul = 4.f;

	//  road
	roadTerTexLayer = 1;  // 🛣️  meh guess
	roadCols = 0;

	roadStepDist = 10.f;  // hq
	roadHadd = -0.7f;  //+
	
	tileMul = 0.6f;  // 1
	wallX = 0.6f;  wallY = 1.0f;

	yObjOfs = 0.0f;  // 📦🏢
}


///  start
///------------------------------
pair <MATHVECTOR<float,3>, QUATERNION<float> > Scene::GetStart(int index, bool notLoopedReverse)
{
	int st = notLoopedReverse ? 1 : 0;
	pair <MATHVECTOR<float,3>, QUATERNION<float> > sp = make_pair(startPos[st], startRot[st]);
	if (index == 0)
		return sp;

	MATHVECTOR<float,3> backward(-gPar.startNextDist * index,0,0);
	sp.second.RotateVector(backward);
	sp.first = sp.first + backward;
	return sp;
}


void Scene::UpdRevSet()
{
	if (!pReverbsXml)  return;
	string s = sReverbs == "" ? "base" : sReverbs;

	int id = pReverbsXml->revmap[sReverbs]-1;
	if (id == -1)
	{	LogO("!scene.xml reverb set not found in xml: "+sReverbs);
		//..
	}else
	{	const ReverbSet &r = pReverbsXml->revs[id], &b = pReverbsXml->base;
		revSet.descr   = r.descr   != "" ? r.descr   : b.descr;
		revSet.normal  = r.normal  != "" ? r.normal  : b.normal;
		revSet.cave    = r.cave    != "" ? r.cave    : b.cave;
		revSet.cavebig = r.cavebig != "" ? r.cavebig : b.cavebig;
		revSet.pipe    = r.pipe    != "" ? r.pipe    : b.pipe;
		revSet.pipebig = r.pipebig != "" ? r.pipebig : b.pipebig;
		revSet.influid = r.influid != "" ? r.influid : b.influid;
	}
}


void Scene::UpdateFluidsId()
{
	if (!pFluidsXml)  return;
	
	//  set fluids id from name
	for (int i=0; i < fluids.size(); ++i)
	{
		int id = pFluidsXml->flMap[fluids[i].name]-1;
		fluids[i].id = id;
		fluids[i].idParticles = id == -1 ? -1    : pFluidsXml->fls[id].idParticles;
		fluids[i].solid       = id == -1 ? false : pFluidsXml->fls[id].solid;
		fluids[i].deep        = id == -1 ? false : pFluidsXml->fls[id].deep;
		if (id == -1)
			LogO("!Warning: Scene fluid name: " + fluids[i].name + " not found in xml!");
	}
}


void Scene::UpdateSurfId()
{
	if (!pGame)  return;
	//  update surfId from surfName
	int i;
	auto& td = tds[0];  // 1st ter
	//  terrain
	for (i=0; i < td.ciNumLay; ++i)
	{
		const std::string& s = td.layersAll[i].surfName;
		int id = pGame->surf_map[s]-1;
		if (id == -1)
		{	id = 0;  // default if not found
			LogO("! Warning: Surface not found (terrain): "+s);
		}
		td.layersAll[i].surfId = id;  // cached
	}
	//  road
	for (i=0; i < 4; ++i)
	{
		const std::string& s = layerRoad[i].surfName;
		int id = pGame->surf_map[s]-1;
		if (id == -1)
		{	id = 0;
			LogO("! Warning: Surface not found (road): "+s);
		}
		layerRoad[i].surfId = id;
	}
}


void Scene::UpdVegLayers()
{
	vegLayers.clear();
	for (int i=0; i < ciNumVegLay; ++i)
	{
		if (vegLayersAll[i].on)
			vegLayers.push_back(i);
	}
}


//  util
float Scene::GetDepthInFluids(Vector3 pos)
{
	float fa = 0.f;
	for (const FluidBox& fb : fluids)
	{
		//  dont check when above fluid, or below its size
		if (pos.y < fb.pos.y &&
			pos.y > fb.pos.y - fb.size.y)
		{
			const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
			//  check rect 2d - no rot !
			if (pos.x > fb.pos.x - sizex && pos.x < fb.pos.x + sizex &&
				pos.z > fb.pos.z - sizez && pos.z < fb.pos.z + sizez)
			{
				float f = fb.pos.y - pos.y;
				if (f > fa && f < fb.size.y)  fa = f;
	}	}	}
	return fa;
}
