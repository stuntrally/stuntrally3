#pragma once
#include "CAllowed.h"
#include "SceneXml.h"
#include <string>
#include <vector>
#include <map>
#include <OgrePrerequisites.h>  // uint


///  🥇 one Collection track setup
class Collection
{
public:
	//  clean name,  description text,  nameGui with colors for Gui only
	std::string name = "none", descr = "none", nameGui = "none";
	int diff = 1;  // difficulty
	int ver = 1;   // version, if changed resets progress

	// float length = 0.f; // stats for display
	int type = 0;       // gui tab
	bool need =0;       // 0 infinite  1 timed
	float time = 0.f;   // time for gold prize (less needed)
	int prizes = 2;     // 0 only gold  1 gold,silver  2 gold,silver,bronze
	float next = 1.f;   // added time for silver, 2x for bronze
	
	//  Allowed type(s) or specific vehicle(s), 1 or more
	CAllowed cars;

	std::string track = "Jng1-Curly";
	int groups = 15;  // collectibles group mask, if more
	// e.g.  1 easy  2 medium  3=1+2  4 hard  6=4+2  etc
	
	//collectibles from track's scene.xml
	std::vector<QCollect> collects;

	//  Game setup
	std::string sim_mode = "normal";  //  if empty or -1 then allows any
	int damage_type = 2, boost_type = 2, flip_type = 2, rewind_type = 1;

	//  Hud off
	bool minimap = 1, chk_arr = 0, chk_beam = 0,
		trk_ghost = 0, pacenotes = 0, trail = 0;  // deny using it if 0 false
};


///-----  📃 all collection games
//
class CollectXml
{
public:
	std::vector<Collection> all;
	
	bool LoadXml(std::string file, class TracksIni* times, bool check);
};


//  progress on collection game
class ProgressCollect
{
public:
	std::string name, track;
	int ver = 0;  // resets if higher

	int fin = -1;      // final prize  -1 none, 0 bronze, 1 silver, 2 gold
	float bestTime = 1e6;   // not per vehicle

	std::map<int, bool> gems;  // collected gem ids
};


///-----  progress C=collection
//
class ProgressCXml
{
public:
	std::vector<ProgressCollect> col;
	std::map<std::string, int> icol;
	
	ProgressCollect* Get(std::string name);

	bool LoadXml(std::string file), SaveXml(std::string file);
};
