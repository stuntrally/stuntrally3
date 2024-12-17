#pragma once
#include "SceneXml.h"  // need SColor
#include <OgreString.h>
#include <OgreStringVector.h>
#include <string>
#include <vector>


//  Presets xml  for editor Pick lists and game materials
//-----------------------------------------------------------

//  ⛅ Sky  ----
struct PSky
{
	int rate = 0;  // rating  for quality, popular
	Ogre::String mtr = "sky", clr = "#C0E0FF";
	float ldYaw = 0.f, ldPitch = 0.f;  // sun dir
	SColor fogClr,fogClr2;  // fog clr sun-away

	float clouds = 0.2f;    // 0..1 coverage  for RoR sky only
	float daytime = 12.f;   // 0..24  night..day
	float latitude = 22.f;  // 0 equator jungle  40 autumn  90 pole
};

//  ⛰️ Terrain layer ----
struct PTer
{
	int rate = 0;
	Ogre::String texFile, texNorm, sc;
	std::string surfName = "Default";

	float tiling = 8.f;  bool triplanar = false;
	float rough = 0.4f, metal = 0.4f;
	bool reflect = false;

	float dust = 0.f, mud = 0.2f, dustS = 0.f;
	SColor tclr = SColor(0.2f, 0.2f, 0.1f, 0.6f);  // trail

	float angMin = 0.f, angMax = 90.f;
	float dmg = 0.f;
};

//  🛣️ Road  ----
struct PRoad
{
	int rate = 0;
	Ogre::String mtr, sc;
	std::string surfName;
	bool reflect = false;  // adds cube refl
	bool alpha = true;  // on _ter adds smooth border

	float dust = 0.f, mud = 0.2f, dustS = 0.f;
	SColor tclr = SColor(0.2f, 0.2f, 0.1f, 0.6f);  // trail
};

//  🌿 Grass  ----
struct PGrass
{
	int rate = 0;
	//  material, colormap, scenery
	Ogre::String mtr, sc;

	float minSx = 1.2f, minSy = 1.2f;
	float maxSx = 1.6f, maxSy = 1.6f;  // sizes
};

//  🌳🪨 Veget model  ----
struct PVeget
{
	int rate = 0;
	Ogre::String sc;
	std::string name, mtr;
	bool bush =0;

	float minScale = 0.6f, maxScale = 1.2f;
	float windFx = 0.02f, windFy = 0.002f;

	int addRdist = 1;  // road dist
	float maxTerAng = 30.f;  // terrain
	float maxDepth = 0.f;  // in fluid

	bool alpha =0;
	float visDist = 300.f, farDist = 300.f;
	float yOfs = 0.f;
};

//  📦🪨  Materials set  ----
struct PMatSet
{
	std::string name;
	Ogre::StringVector mats;  // possible materials
};

//  📦🪨  Object  or  ----
struct PObject
{
	std::string name;  // .mesh
	std::string sound;  // name from sounds.cfg
	PMatSet* pMatSet =0;
};

//  💎  Collect  ----
struct PCollect
{
	std::string name, mesh, material, beamMtr;
	Ogre::ColourValue clr;
	float scale = 1.f;
};


///  📄 Presets xml  with common params  for ed and game
//-----------------------------------------------------------
class Presets
{
public:
	std::vector<PSky> sky;
	std::map<std::string, int> isky;
	const PSky* GetSky(std::string mtr);
	
	std::vector<PTer> ter;
	std::map<std::string, int> iter;
	const PTer* GetTer(std::string tex);
	
	std::vector<PRoad> rd;
	std::map<std::string, int> ird;
	const PRoad* GetRoad(std::string mtr);


	std::vector<PGrass> gr;
	std::map<std::string, int> igr;
	const PGrass* GetGrass(std::string mtr);
	
	std::vector<PVeget> veg;
	std::map<std::string, int> iveg;
	const PVeget* GetVeget(std::string mesh);

	
	std::vector<PMatSet> matset;
	std::map<std::string, int> imatset;
	PMatSet* GetMatSet(std::string name);

	std::vector<PObject> obj;
	std::map<std::string, int> iobj;
	const PObject* GetObject(std::string mesh);

	std::vector<PCollect> col;
	std::map<std::string, int> icol;
	const PCollect* GetCollect(std::string name);

	bool LoadXml(std::string file);
};
