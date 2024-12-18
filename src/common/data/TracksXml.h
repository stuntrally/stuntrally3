#pragma once
#include "settings.h"  // CarColor
#include <vector>
#include <map>
#include <string>
namespace tinyxml2	{	class XMLElement;	}


struct Date
{
	int day,month,year;
	bool operator < (const Date& d) const
	{
		return year < d.year && month < d.month && day < d.day;
	}
	Date() : day(1),month(1),year(10) {  }
};


//  🏞️ Track's additional info
//  shown on gui [Track] tab, in detailed view
//  for sorting by date, difficulty, etc.
//--------------------------------------------------------------------
class TrackInfo
{
public:
	int n = -1;  // unique id for track, from ini
	float crtver = 0.f;  // created in ver
	int ver = 0;  // x10
	std::string name = "none", nshrt, // short name, no prefix
		scenery = "none", author = "none";
	Date created, modified;

	//  track characteristics  (byte)
	int objects =0, obstacles =0;
	int fluids =0, bumps =0,  jumps =0, loops =0, pipes =0;
	int	banked =0, frenzy =0,  underwater =0,  narrow =0, longn =0;
	int diff =0, rating =0,  sum =0;
	//todo fields

	int nn =0;  // number got from name eg. for Des15-.. it is 15
	bool test =0, testC =0;  // starts with Test.. or TestC..
};

//  🏞️ all tracks infos
//.................................
class TracksIni
{
public:
	std::vector<TrackInfo> trks;
	std::map<std::string, int> trkmap;  // 0 if not found
	std::map<std::string, float> times;  // track times
	
	int cntAll =0;
	bool LoadIni(std::string file, bool check=false);
};


//  ⭐ user's Tracks info
//  rating, stats
//-------------------------------------
class UserTrkInfo
{
public:
	std::string name;
	int rating =0, bookm =0;
	Date last;  // driven
	int laps =0;
};

//  ⭐ user xml (tracks)
class UserXml
{
public:
	std::vector<UserTrkInfo> trks;
	std::map<std::string, int> trkmap;  // 0 if not found
	
	bool LoadXml(std::string file, TracksIni* ini), SaveXml(std::string file);
};


#ifndef SR_EDITOR

//  🚗 Car's additional info  (all Car = Vehicle)
//  shown on gui [Car] tab, in detailed view
//  for sorting by speed, type, etc.
//--------------------------------------------------------------------
class CarInfo
{
public:
	std::string id = "AA", name = "Other";  // id = cars/subdir, name = animal nick
	std::string type = "Other", author;

	float speed = 5.f;
	int year = 2015, wheels = 4;
	int width = 3;  // used in CData::GetDrivability, with tracks.ini narrow
	int rating = 5, diff = 3;
	
	//  intolerance of these, for track fitness
	int bumps = 0, jumps = 0, loops = 0, pipes = 0;
	int underwater = 0;  // 1 if cant go,  used with tracks.ini underwater
	
	//  time mul factors, for race postion, in sim modes
	float easy = 0.96f, norm = 1.f;
};


//  🚗 all cars infos
//.................................
class CarsXml
{
public:
	std::vector<CarInfo> cars;
	std::map<std::string, int> carmap;  // 0 if not found
	std::map<std::string, std::string> colormap;  // car type to list color
	float magic = 0.010f;
	
	bool LoadXml(std::string file);
};

#endif


//  🔉 Reverb presets on sceneries  ed combo
//--------------------------------------------------------------------
struct ReverbSet
{
	std::string name, descr,  // info
		normal, cave, cavebig, pipe, pipebig, influid;  // reverb preset names
};

class ReverbsXml
{
public:
	ReverbSet base;  // inherited defaults if not set
	std::vector<ReverbSet> revs;
	std::map<std::string, int> revmap;  // 0 if not found
	
	bool LoadXml(std::string file);
	void GetParams(tinyxml2::XMLElement* e, ReverbSet& r);
};
