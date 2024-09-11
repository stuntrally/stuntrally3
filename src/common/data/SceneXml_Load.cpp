#include "pch.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "SceneClasses.h"
#include "FluidsXml.h"

#include "game.h"  // for surfaces map
#include "App.h"
#include "CScene.h"
#include "GuiCom.h"
#include "paths.h"

#include <OgreSceneNode.h>
#include <algorithm>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace std;
using namespace Ogre;


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::LoadXml(String file, bool bTer)
{
	XMLDocument doc;
	XMLError er = doc.LoadFile(file.c_str());
	if (er != XML_SUCCESS)
	{	LogO("!Error: Can't load scene.xml: "+file);  return false;  }
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear  --
	Default();

	//td.layers.clear();
	//pgLayers.clear();

	//  read
	XMLElement* e, *u;
	const char* a;


 	///  ed version
 	int ver = 2300;  // old
 	e = root->FirstChildElement("ver");
	if (e)
	{	a = e->Attribute("num");		if (a)  ver = s2i(a);
		a = e->Attribute("baseTrk");	if (a)  baseTrk = string(a);
		a = e->Attribute("secEd");		if (a)  secEdited = s2i(a);
	}
	
 	///  🚗 car setup game
 	e = root->FirstChildElement("car");
	if (e)
	{	a = e->Attribute("tires");		if (a)  asphalt = s2i(a) > 0;
		a = e->Attribute("damage");		if (a)  damageMul = s2r(a);
		a = e->Attribute("road1mtr");	if (a)  road1mtr = s2i(a) > 0;

		a = e->Attribute("denyRev");	if (a)  denyReversed = s2i(a) > 0;
		a = e->Attribute("gravity");	if (a)  gravity = s2r(a);
		a = e->Attribute("noWrongChks"); if (a)  noWrongChks = s2i(a) > 0;
		a = e->Attribute("needLights"); if (a)  needLights = s2i(a) > 0;
	}

	///  🏁 car start
	e = root->FirstChildElement("start");
	if (e)
	{	a = e->Attribute("pos");		if (a)  {  Vector3 v = s2v(a);   startPos[0] = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
		a = e->Attribute("rot");		if (a)  {  Vector4 v = s2v4(a);  startRot[0] = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
		a = e->Attribute("pos2");		if (a)  {  Vector3 v = s2v(a);   startPos[1] = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
		a = e->Attribute("rot2");		if (a)  {  Vector4 v = s2v4(a);  startRot[1] = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
	}

	///  for RoR export only
	auto& ror = rorCfg;
	e = root->FirstChildElement("ror1");
	if (e)
	{	a = e->Attribute("aA");		if (a)  ror.lAmbAdd = s2r(a);
		a = e->Attribute("lA");		if (a)  ror.lAmb = s2r(a);
		a = e->Attribute("lD");		if (a)  ror.lDiff = s2r(a);
		a = e->Attribute("lS");		if (a)  ror.lSpec = s2r(a);
		a = e->Attribute("fg");		if (a)  ror.fogMul = s2r(a);

		a = e->Attribute("wtr");	if (a)  ror.water = s2i(a);
		a = e->Attribute("yW");		if (a)  ror.yWaterOfs = s2r(a);

		a = e->Attribute("tr");		if (a)  ror.treesMul = s2r(a);
		a = e->Attribute("gr");		if (a)  ror.grassMul = s2r(a);
	}
	e = root->FirstChildElement("ror2");
	if (e)
	{	a = e->Attribute("rtx");	if (a)  ror.roadTerTexLayer = s2r(a);
		a = e->Attribute("col");	if (a)  ror.roadCols = s2i(a) > 0;
		
		a = e->Attribute("d");		if (a)  ror.roadStepDist = s2r(a);
		a = e->Attribute("a");		if (a)  ror.roadAngDiv = s2r(a);
		
		a = e->Attribute("h");		if (a)  ror.roadHadd = s2r(a);
		a = e->Attribute("wx");		if (a)  ror.wallX = s2r(a);
		a = e->Attribute("wy");		if (a)  ror.wallY = s2r(a);
		a = e->Attribute("v");		if (a)  ror.roadVegetDist = s2i(a);

		a = e->Attribute("sc");		if (a)  ror.tileMul = s2r(a);
		a = e->Attribute("oy");		if (a)  ror.yObjOfs = s2r(a);
	}

	///  🔉 sound
	e = root->FirstChildElement("sound");
	if (e)
	{	a = e->Attribute("ambient");	if (a)  ambientSnd = string(a);
		a = e->Attribute("vol");		if (a)  ambientVol = s2r(a);
		a = e->Attribute("reverbs");	if (a)  sReverbs = string(a);
		UpdRevSet();
	}

	///  ⛅ sky
	e = root->FirstChildElement("sky");
	if (e)
	{	a = e->Attribute("material");	if (a)  skyMtr = String(a);
		
		for (int i=0; i < NumWeather; ++i)
		{
			string sn, si = i==0 ? "" : toStr(i+1);
			sn = "rain"+si+"Emit";  a = e->Attribute(sn.c_str());	if (a)  rainEmit[i] = s2i(a);
			sn = "rain"+si+"Name";  a = e->Attribute(sn.c_str());	if (a)  rainName[i] = String(a);
		}		
		a = e->Attribute("windAmt");	if (a)  windForce = s2r(a);
		a = e->Attribute("skyYaw");		if (a)  skyYaw = s2r(a);
	}
	///  🌫️ fog
	e = root->FirstChildElement("fog");
	if (e)
	{	a = e->Attribute("linStart");	if (a)  fogStart = s2r(a);
		a = e->Attribute("linEnd");		if (a)  fogEnd = s2r(a);
		a = e->Attribute("color");		if (a)  fogClr.Load(a);
		a = e->Attribute("color2");		if (a)  fogClr2.Load(a);  else  fogClr2 = fogClr;
	}
	e = root->FirstChildElement("fogH");
	if (e)
	{	a = e->Attribute("color");		if (a)  fogClrH.Load(a);
		a = e->Attribute("height");		if (a)  fogHeight = s2r(a);
		a = e->Attribute("dens");		if (a)  fogHDensity = s2r(a);
		a = e->Attribute("linStart");	if (a)  fogHStart = s2r(a);
		a = e->Attribute("linEnd");		if (a)  fogHEnd = s2r(a);
		a = e->Attribute("dmg");		if (a)  fHDamage = s2r(a);
	}

	///  💡 light
	e = root->FirstChildElement("light");
	if (e)
	{	a = e->Attribute("pitch");		if (a)  ldPitch = s2r(a);
		a = e->Attribute("yaw");		if (a)  ldYaw = s2r(a);

		a = e->Attribute("ambient");	if (a)  lAmb.Load(a);
		a = e->Attribute("diffuse");	if (a)  lDiff.Load(a);
		a = e->Attribute("specular");	if (a)  lSpec.Load(a);
	}
	///  🌪️ wind
	e = root->FirstChildElement("wind");
	if (e)
	{	a = e->Attribute("ofs");	if (a)  windOfs = s2r(a);
		a = e->Attribute("amp");	if (a)  windAmpl = s2r(a);
		a = e->Attribute("fq");		if (a)  windFreq = s2r(a);
		a = e->Attribute("yaw");	if (a)  windYaw = s2r(a);
	}
	
	
	///  💧 Fluids
	e = root->FirstChildElement("fluids");
	if (e)
	{	u = e->FirstChildElement("fluid");
		while (u)
		{
			FluidBox fb;
			a = u->Attribute("hq");		if (a)  fb.hq = s2i(a);
			a = u->Attribute("name");	if (a)  fb.name = string(a);

			a = u->Attribute("pos");	if (a)  fb.pos = s2v(a);
			a = u->Attribute("rot");	if (a)  fb.rot = s2v(a);
			a = u->Attribute("size");	if (a)  fb.size = s2v(a);
			a = u->Attribute("tile");	if (a)  fb.tile = s2v2(a);

			fluids.push_back(fb);
			u = u->NextSiblingElement("fluid");
		}
	}
	

	///  ⛰️ Terrain
	//  new 3, many
	e = root->FirstChildElement("terrains");
	if (e)
	{
		u = e->FirstChildElement("terrain");	
		while (u)
		{
			TerData td;
			LoadTerData(file, td, u);
			tds.push_back(td);
			u = u->NextSiblingElement("terrain");
		}
	}

	//  old 2.7, one
	e = root->FirstChildElement("terrain");
	if (e)
	{
		TerData td;
		LoadTerData(file, td, e);
		tds.push_back(td);
	}
	

	///  🌳🪨 Vegetation
 	e = root->FirstChildElement("veget");
	if (!e)
 		e = root->FirstChildElement("paged");  // old
	if (e)
	{	a = e->Attribute("densTrees");		if (a)  // old
		{
			densTrees = s2r(a);
			// LogO("old dens: "+toStr(densTrees)+" tws: "+fToStr(tds[0].fTerWorldSize)+" tws1: "+fToStr(tds[1].fTerWorldSize));
			auto tws = tds[0].fTerWorldSize;
			densTrees = 1000000.f * densTrees / tws / tws;
			// LogO("new dens2: "+fToStr(densTrees,9,11));
		}
		a = e->Attribute("densTrees2");		if (a)  densTrees = s2r(a);
		a = e->Attribute("densGrass");		if (a)  densGrass = s2r(a);
		a = e->Attribute("grDensSmooth");	if (a)  grDensSmooth = s2i(a);
		a = e->Attribute("trRdDist");		if (a)  trRdDist = s2i(a);

		//  🌿 Grass
		int grl = 0;
		u = e->FirstChildElement("grass");
		while (u && grl < ciNumGrLay)
		{
			SGrassLayer g;
			a = u->Attribute("on");			if (a)  g.on = s2i(a);  else  g.on = 1;
			a = u->Attribute("mtr");		if (a)  g.material = String(a);
			a = u->Attribute("cl");			if (a)  g.color.Load(a);  // new
			a = u->Attribute("dens");		if (a)  g.dens = s2r(a);
			a = u->Attribute("chan");		if (a)  g.iChan = s2i(a);
										
			a = u->Attribute("minSx");		if (a)  g.minSx = s2r(a);
			a = u->Attribute("maxSx");		if (a)  g.maxSx = s2r(a);
			a = u->Attribute("minSy");		if (a)  g.minSy = s2r(a);
			a = u->Attribute("maxSy");		if (a)  g.maxSy = s2r(a);
			
			grLayersAll[grl++] = g;
			u = u->NextSiblingElement("grass");
		}

		int c;
		for (c=0; c < 4; c++)
			grChan[c].nFreq += c * 3.f;  // default variation
		c = 0;

		u = e->FirstChildElement("gchan");
		while (u && c < 4)
		{
			SGrassChannel& g = grChan[c++];

			a = u->Attribute("amin");	if (a)  g.angMin = s2r(a);
			a = u->Attribute("amax");	if (a)  g.angMax = s2r(a);
			a = u->Attribute("asm");	if (a)  g.angSm = s2r(a);

			a = u->Attribute("hmin");	if (a)  g.hMin = s2r(a);
			a = u->Attribute("hmax");	if (a)  g.hMax = s2r(a);
			a = u->Attribute("hsm");	if (a)  g.hSm = s2r(a);

			a = u->Attribute("ns");		if (a)  g.noise = s2r(a);
			a = u->Attribute("frq");	if (a)  g.nFreq = s2r(a);
			a = u->Attribute("oct");	if (a)  g.nOct  = s2i(a);
			a = u->Attribute("prs");	if (a)  g.nPers = s2r(a);
			a = u->Attribute("pow");	if (a)  g.nPow  = s2r(a);

			a = u->Attribute("rd");		if (a)  g.rdPow = s2r(a);
			u = u->NextSiblingElement("gchan");
		}
		
		///  🌳🪨 Vegetation
		int pgl = 0;
		u = e->FirstChildElement("layer");
		while (u && pgl < ciNumVegLay)
		{
			VegetLayer l;
			a = u->Attribute("on");			if (a)  l.on = s2i(a);  else  l.on = 1;
			a = u->Attribute("name");		if (a)  l.name = String(a);
			a = u->Attribute("mtr");		if (a)  l.mtr = String(a);
			a = u->Attribute("dens");		if (a)  l.dens = s2r(a);
			a = u->Attribute("minScale");	if (a)  l.minScale = s2r(a);
			a = u->Attribute("maxScale");	if (a)  l.maxScale = s2r(a);

			a = u->Attribute("ofsY");		if (a)  l.ofsY = s2r(a);
			a = u->Attribute("addTrRdDist");if (a)  l.addRdist = s2i(a);
			a = u->Attribute("maxRdist");	if (a)  l.maxRdist = s2i(a);

			a = u->Attribute("maxTerAng");	if (a)  l.maxTerAng = s2r(a);
			a = u->Attribute("minTerH");	if (a)  l.minTerH = s2r(a);
			a = u->Attribute("maxTerH");	if (a)  l.maxTerH = s2r(a);
			a = u->Attribute("maxDepth");	if (a)  l.maxDepth = s2r(a);

			vegLayersAll[pgl++] = l;
			u = u->NextSiblingElement("layer");
		}
		UpdVegLayers();
	}
	
 	///  🎥 camera
 	e = root->FirstChildElement("cam");
	if (e)
	{	a = e->Attribute("pos");		if (a)  camPos = s2v(a);
		a = e->Attribute("dir");		if (a)  camDir = s2v(a);
	}
	
	///  📦 Objects
	e = root->FirstChildElement("objects");
	if (e)
	{	u = e->FirstChildElement("o");
		while (u)
		{
			Object o;
			a = u->Attribute("name");	if (a)  o.name = string(a);
			a = u->Attribute("stat");	if (a)  o.stat = s2i(a) > 0;

			a = u->Attribute("mat");	if (a)  o.material = string(a);

			a = u->Attribute("pos");	if (a)  {  Vector3 v = s2v(a);  o.pos = MATHVECTOR<float,3>(v.x,v.y,v.z);  }
			a = u->Attribute("rot");	if (a)  {  Vector4 v = s2v4(a);  o.rot = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
			a = u->Attribute("sc");		if (a)  o.scale = s2v(a);
			
			a = u->Attribute("sh");		if (a)  o.shadow = s2i(a) > 0;

			objects.push_back(o);
			u = u->NextSiblingElement("o");
	}	}

	///  🔥 Particles
	e = root->FirstChildElement("emitters");
	if (e)
	{	u = e->FirstChildElement("e");
		while (u)
		{
			SEmitter p;
			a = u->Attribute("name");	if (a)  p.name = string(a);

			a = u->Attribute("pos");	if (a)  p.pos = s2v(a);
			a = u->Attribute("sc");		if (a)  p.size = s2v(a);
			a = u->Attribute("yaw");	if (a)  p.yaw = s2r(a);  // dir
			a = u->Attribute("pt");		if (a)  p.pitch = s2r(a);
			
			a = u->Attribute("rate");	if (a)  p.rate = s2r(a);
			a = u->Attribute("st");		if (a)  p.stat = s2i(a);
			a = u->Attribute("psc");	if (a)  p.parScale = s2r(a);

			emitters.push_back(p);
			u = u->NextSiblingElement("e");
	}	}

	//  💎 Collectible gems
	e = root->FirstChildElement("collectible");
	if (e)
	{	u = e->FirstChildElement("c");
		while (u)
		{
			SCollect c;
			a = u->Attribute("n");		if (a)  c.name = string(a);
			a = u->Attribute("gr");		if (a)  c.group = s2i(a);

			a = u->Attribute("pos");	if (a)  c.pos = s2v(a);
			a = u->Attribute("sc");		if (a)  c.scale = s2r(a);

			collects.push_back(c);
			u = u->NextSiblingElement("c");
	}	}

	//  🎆 Fields
	e = root->FirstChildElement("fields");
	if (e)
	{	u = e->FirstChildElement("f");
		while (u)
		{
			SField f;
			a = u->Attribute("t");		if (a)  f.type = (FieldTypes)s2i(a);

			a = u->Attribute("pos");	if (a)  f.pos = s2v(a);
			a = u->Attribute("sz");		if (a)  f.size = s2v(a);
			a = u->Attribute("yaw");	if (a)  f.yaw = s2r(a);
			a = u->Attribute("pt");		if (a)  f.pitch = s2r(a);

			a = u->Attribute("f");		if (a)  f.factor = s2r(a);
			a = u->Attribute("pos2");	if (a)  {  Vector3 v = s2v(a);   f.pos2 = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
			a = u->Attribute("dir2");	if (a)  {  Vector4 v = s2v4(a);  f.dir2 = QUATERNION<float>(v.x,v.y,v.z,v.w);  }

			fields.push_back(f);
			u = u->NextSiblingElement("f");
	}	}


	UpdateFluidsId();

	UpdateSurfId();
	
	return true;
}


//  💎 Collectible gems  only
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
bool Scene::LoadXmlCollects(String file)
{
	XMLDocument doc;
	XMLError er = doc.LoadFile(file.c_str());
	if (er != XML_SUCCESS)
	{	LogO("!Error: Can't load scene.xml: "+file);  return false;  }
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear  --
	collects.clear();

	//  read only
	XMLElement* e, *u;
	const char* a;

	e = root->FirstChildElement("collectible");
	if (e)
	{	u = e->FirstChildElement("c");
		while (u)
		{
			QCollect c;
			// a = u->Attribute("n");		if (a)  c.name = string(a);
			a = u->Attribute("gr");		if (a)  c.group = s2i(a);

			// a = u->Attribute("pos");	if (a)  c.pos = s2v(a);
			// a = u->Attribute("sc");		if (a)  c.scale = s2r(a);

			qcols.push_back(c);
			u = u->NextSiblingElement("c");
	}	}
	return true;
}


//  Ter Data
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
bool Scene::LoadTerData(String file, TerData& td, XMLElement* e)
{
	XMLElement* u;
	const char* a;

	//a = e->Attribute("size");		if (a)  td.iVertsXold = s2i(a);  // got from hmap file size later
	a = e->Attribute("ofsZ");		if (a)  td.ofsZ = s2r(a);
	a = e->Attribute("triangle");	if (a)  td.fTriangleSize = s2r(a);
	a = e->Attribute("posX");		if (a)  td.posX = s2r(a);
	a = e->Attribute("posZ");		if (a)  td.posZ = s2r(a);

	a = e->Attribute("diff");		if (a)  td.clrDiff = s2v(a);
	a = e->Attribute("emissive");	if (a)  td.emissive = s2i(a)>0;
	a = e->Attribute("reflect");	if (a)  td.reflect = s2i(a);
	// a = e->Attribute("specPow");	if (a)  td.specularPow = s2r(a);  // roughness?
	a = e->Attribute("specPowEm");	if (a)  td.specularPowEm = s2r(a);

	a = e->Attribute("horiz");		if (a)  td.iHorizon = s2i(a);
	a = e->Attribute("collis");		if (a)  td.collis = s2i(a)>0;
	a = e->Attribute("bL");		if (a)  td.bL = s2i(a)>0;
	a = e->Attribute("bR");		if (a)  td.bR = s2i(a)>0;
	a = e->Attribute("bF");		if (a)  td.bF = s2i(a)>0;
	a = e->Attribute("bB");		if (a)  td.bB = s2i(a)>0;
	a = e->Attribute("veget");		if (a)  td.fVeget = s2r(a);


	//  get iVerts from hmap file
	int n = tds.size();
	// String fname = gcom->app->scn->getHmap(n, 0);  // tool cant
	String hmap = "heightmap" + (n > 0 ? toStr(n+1) : "") /*+ (bNew ? "-new" : "")*/ + ".f32";
	String hfile = StringUtil::replaceAll(file, "scene.xml", hmap);

	bool exists = PATHS::FileExists(hfile);
	if (!exists)
		LogO("Terrains error! No hmap file: "+hfile);
	else
		td.getFileSize(hfile);
	// td.UpdVals();  //^in

	int il = 0;
	u = e->FirstChildElement("texture");
	while (u)
	{
		int road = -1;
		a = u->Attribute("road");	if (a)  road = s2i(a)-1;
		bool ter = road == -1;
		
		TerLayer lay, *l = ter ? &lay : &layerRoad[road];
		lay.nFreq[0] += (il-0.7f) * 4.f;  // default, can't be same, needs variation
		lay.nFreq[1] += (il-0.5f) * 3.f;

		a = u->Attribute("on");		if (a)  l->on = s2i(a)>0;  else  l->on = true;
		a = u->Attribute("file");	if (a)  l->texFile = String(a);
		a = u->Attribute("fnorm");	if (a)  l->texNorm = String(a);
		a = u->Attribute("scale");	if (a)  l->tiling = s2r(a);
		a = u->Attribute("surf");	if (a)  l->surfName = String(a);

		a = u->Attribute("dust");	if (a)  l->dust = s2r(a);  // ⚫💭
		a = u->Attribute("dustS");	if (a)  l->dustS = s2r(a);
		a = u->Attribute("mud");	if (a)  l->mud = s2r(a);
		a = u->Attribute("smoke");	if (a)  l->smoke = s2r(a);
		a = u->Attribute("tclr");	if (a){ l->tclr.Load(a);  l->tcl = l->tclr.GetRGBA();  }
		a = u->Attribute("dmg");	if (a)  l->fDamage = s2r(a);

		a = u->Attribute("angMin");	if (a)  l->angMin = s2r(a);  // 🏔️
		a = u->Attribute("angMax");	if (a)  l->angMax = s2r(a);
		a = u->Attribute("angSm");	if (a)  l->angSm = s2r(a);
		a = u->Attribute("hMin");	if (a)  l->hMin = s2r(a);
		a = u->Attribute("hMax");	if (a)  l->hMax = s2r(a);
		a = u->Attribute("hSm");	if (a)  l->hSm = s2r(a);

		a = u->Attribute("nOn");		if (a)  l->nOnly = s2i(a)>0;
		a = u->Attribute("triplanar");	if (a)  l->triplanar = true;  else  l->triplanar = false;

		a = u->Attribute("noise");	if (a)  l->noise = s2r(a);  // 🌀
		a = u->Attribute("n_1");	if (a)  l->nprev = s2r(a);
		a = u->Attribute("n2");		if (a)  l->nnext2 = s2r(a);

		XMLElement* eNoi = u->FirstChildElement("noise");
		if (eNoi)
		for (int n=0; n < 2; ++n)
		{	string sn = toStr(n), s;
			s = "frq"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nFreq[n]= s2r(a);
			s = "oct"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nOct[n] = s2i(a);
			s = "prs"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nPers[n]= s2r(a);
			s = "pow"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nPow[n] = s2r(a);
		}
		if (ter && il < td.ciNumLay)
			td.layersAll[il++] = lay;
		u = u->NextSiblingElement("texture");
	}
	td.UpdLayers();

	u = e->FirstChildElement("par");  // ⚫💭
	if (u)
	{	a = u->Attribute("dust");	if (a)  sParDust = String(a);
		a = u->Attribute("mud");	if (a)  sParMud = String(a);
		a = u->Attribute("smoke");	if (a)  sParSmoke = String(a);

		a = u->Attribute("flWater");	if (a)  sFluidWater = String(a);
		a = u->Attribute("flMud");		if (a)  sFluidMud = String(a);
		a = u->Attribute("flMudSoft");	if (a)  sFluidMudSoft = String(a);
	}
	return true;
}
