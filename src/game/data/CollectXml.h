#pragma once
#include <string>
#include <vector>
#include <map>
#include "CAllowed.h"


///  🥇 one Collection track setup
class Collection
{
public:
	//  clean name,  description text,  nameGui with colors for Gui only
	std::string name = "none", descr = "none", nameGui = "none";
	int diff = 1;  // difficulty
	int ver = 1;   // version, if changed resets progress

	// float length = 0.f; // stats for display
	int type = 0;       // infinite, timed etc, for gui tab
	// float time = 0.f;   // total computed (sum all tracks)
	// int prizes = 2;     // 0 only gold  1 gold,silver  2 gold,silver,bronze
	float factor = 1.f; // multiplier for silver,bronze points/pos prizes
	
	//  Allowed type(s) or specific vehicle(s), 1 or more
	CAllowed cars;

	std::string track = "Jng1-Curly";
	bool onTime =0;  // 0 infinite  1 timed
	float timeNeeded = 0.f;  // for gold?
	//collectibles in track's scene.xml
	uint groups = 15;  // collectibles group mask, if more
		// e.g.  1 easy  2 medium  3=1+2  4 hard  6=4+2 etc

	//  Game setup
	//  if empty or -1 then allows any
	std::string sim_mode = "normal";
	int damage_type = 2, boost_type = 2, flip_type = 2, rewind_type = 1;

	//  Hud off
	bool minimap = 1, chk_arr = 0, chk_beam = 0,
		trk_ghost = 0, pacenotes = 0, trail = 0;  // deny using it if false
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
	std::string name;
	int ver = 0;  // resets if higher..

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
