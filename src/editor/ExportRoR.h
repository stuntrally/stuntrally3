#pragma once
#include <string>
#include <map>


//  Export current track for Rigs Of Rods
//------------------------------------------------------------------------------------------------------------------------
class ExportRoR
{
protected:
	class App* app =0;  // main editor app
	class CGui* gui =0;  // rest auto set from app
	class SETTINGS* pSet =0;

	class CScene* scn =0;
	class Scene* sc =0;
	struct RoRconfig* cfg =0;

	class CData* data =0;
	class Presets* pre =0;

	bool copyTerTex, copyVeget, copyGrass, copyObjs;

	//  vars
	std::string
		path,     // main RoR content or mods path
		dirName,  // subdir name for track
		name;     // track name
	int version = 1;

	bool hasRoad = 0, hasRoadChks = 0, hasVeget = 0, hasObjects = 0;

	float hmin = 0.f, half = 0.f;  // min height, half world size
	int water = 0;  float Ywater = 0.f;  // bool, level

	//  utils
	Ogre::String strPos(const Ogre::Vector3& pos);  // util convert SR pos to RoR pos
	bool CopyFile(const std::string& from, const std::string& to);  // both args are path with filenames

	//  packs
	void ListPacks();
	bool first =1;
	std::map<std::string, std::string> mesh2pack, ter2pack;

	bool AddPackFor(std::string mesh);  // check if in which mesh2pack, and to packs
	bool AddPackForTer(std::string tex);  // check if in which mesh2pack, and to packs
	std::set<std::string> packs;

public:
	//  🌟 ctor
	ExportRoR(App* app1);
	void Default();

	//  🏗️ main
	void ExportTrack();  // whole / full,  needs paths set in settings

	//  parts
	void SetupPath();
	void ExportTerrain();  //  heightmap  *.raw  and  blendmap:  *.png  and  *-page-0-0.otc
	void ExportObjects();
	void ExportWaterSky();  // water set,  caelum .os
	
	void ExportRoad();     //  *-road.tobj  and  *.as
	void ExportVeget();    //  *-veget.tobj  and  density maps *-veget?.png

	//  🛠️ tools other
	void CreateOdef();  // make .odef for all .mesh files  in main path subdirs
	void ConvertTerrainTex();  // combines _d + _s to _ds and _nh as .png
	void ConvertMat();  // reads old SR .mat and saves as .material
};
