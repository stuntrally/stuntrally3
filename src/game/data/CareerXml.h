#pragma once
#include <string>
#include <vector>
#include <map>
#include "CAllowed.h"
#include "paints.h"


///  one Career race event, generated
class CareerRace
{
public:
	//  clean name,  description text,  nameGui with colors for Gui only
	// std::string name = "none", descr = "none", nameGui = "none";
	// int diff = 1;  // difficulty
	// int ver = 1;   // version, if changed resets progress

	// float length = 0.f; // stats for display
	// int type = 0;       // infinite, timed etc, for gui tab
	// float time = 0.f;   // total computed (sum all tracks)
	// int prizes = 2;     // 0 only gold  1 gold,silver  2 gold,silver,bronze
	// float factor = 1.f; // multiplier for silver,bronze points/pos prizes
	
	//  Allowed type(s) or specific vehicle(s), 1 or more
	CAllowed cars;

	//  Game setup
	std::string track = "Jng1-Curly";
	// std::vector<ChallTrack> trks;
	// bool onTime =0;  // 0 infinite  1 timed
	// float timeNeeded = -1;
	// int group =0;  // collectibles group, if more
	int laps =1;

	//  if empty or -1 then allows any
	int game_type = 0;
	std::string sim_mode = "normal";
	// int damage_type = 1;  // reduced..  // todo: easy career game modes
	// int boost_type = 1, flip_type = 2;  // todo: owned vehicle perks not race
	// rewind_type = 1;  // todo: go back device, back time+fuel
	// todo: max garage time for fixing vehicle?

	//  Hud deny if 0
	bool minimap = 1, chk_arr = 0, chk_beam = 0,
		trk_ghost = 0, pacenotes = 1, trail = 1;
	
	//  money
	int race_cost =0;  // entry fee,  from track long,sigma?
	int prize1 =0, prize2 =0, prize3 =0, prize4 =0;
	//  Pass  -1 means not needed, you can use one or more conditions
	// float totalTime = -1.f, avgPoints = -1.f, avgPos = -1.f;
};


//  progress, status of one vehicle
class ProgressCar
{
public:
	std::string name = "HI";  // id from cars.xml
	float damage = 0.f;  // todo: repair cost in cars.xml
	int cur_paint =0;
	std::vector<CarPaint> paints;  // owned paints
};

//  💷 Career game  progress, status
class ProgressCareer
{
public:
	int mode = 0;  // easy, medium, hard
	float money = 0.f;  // credits amount
	int diff_rank;      // difficulty rank
	std::vector<ProgressCar> cars;  // owned vehicles

	struct SFuels
	{
		float car   =0.f;  // 
		float boost =0.f;  // 
		float ele   =0.f;  // U8, UV, BV..  todo: solar charging
		float grav  =0.f;  // R1, Q1..
		float rewind =0.f;  // for game
		float time   =0.f;  // rewind go back
	} fuels;
	// todo: for all vehicle types, each fuel use in cars.xml
	// todo: each planet different price to buy fuels, sell too
	// and higher fee for using car gas in space?
	// scenery travel fee?  starting planet, map

	//  stats
	float totalTime = 0.f;	// time played, added from tracks
	int raced = 0.f;	    // tracks raced
	//  tracks race places counts
	//  i.e. how many times won 1st place on any track
	int pos1 =0, pos2 =0, pos3 =0, pos4, pos5up =0;

	bool LoadXml(std::string file), SaveXml(std::string file);
};


///-----  progress R=career  todo: game slots-
//
/*
class ProgressRXml
{
public:
	std::vector<ProgressCollect> chs;
	bool LoadXml(std::string file), SaveXml(std::string file);
};
*/