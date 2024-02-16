#pragma once
#include <string>


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

//  var
	std::string
		path,     // main RoR content or mods path
		dirName,  // subdir name for track
		name;     // track name

	bool hasRoad = 0, hasVeget = 0, hasObjects = 0;

	float hmin = 0.f, half = 0.f;  // min height, half world size
	int water = 0;  float Ywater = 0.f;  // bool, level

	Ogre::String strPos(Ogre::Vector3 pos);  // util convert SR pos to RoR pos
	bool CopyFile(std::string from, std::string to);  // both args are path with filenames

public:
	ExportRoR(App* app1);
	void Default();

	//  main
	void ExportTrack();  // whole / full,  needs paths set in settings

	//  parts
	void SetupPath();
	void ExportTerrain();  //  heightmap  *.raw  and  blendmap:  *.png  and  *-page-0-0.otc
	void ExportObjects();
	void ExportWaterSky();  // water set,  caelum .os
	
	void ExportRoad();     //  *-road.tobj  and  *.as
	void ExportVeget();    //  *-veget.tobj  and  density maps *-veget?.png

	//  other
	void ConvertTerrainTex();
	void ConvertMat();
};
