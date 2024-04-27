#pragma once
#include <map>
#include <string>

class FluidsXml;
class BltObjects;
class ReverbsXml;

class TracksIni;
class UserXml;
class CarsXml;
class Presets;

#ifdef SR_EDITOR
#else
	class PaintsIni;
	class ChampsXml;
	class ChallXml;
	class Chall;
	class CollectXml;
#endif


class CData
{
public:
	CData();
	~CData();
	
	void Load(std::map <std::string, int>* surf_map=0, bool check=false);  //
	#ifndef SR_EDITOR
		void LoadPaints(bool forceOrig = false);
	#endif

	FluidsXml* fluids =0;	//  fluids params for car sim
	BltObjects* objs =0;	//  collisions.xml, for vegetation models
	ReverbsXml* reverbs =0;	// reverb presets for sceneries, used in scene.xml
	
	TracksIni* tracks =0;	// tracks.ini info for Gui
	CarsXml* cars =0;		// cars info for Gui
	UserXml* user =0;		// user tracks.xml
	Presets* pre =0;		// data for scene resources, ed and game
	
	#ifndef SR_EDITOR	// game only
		PaintsIni* paints =0;  // car colors.ini
		//  games
		ChampsXml* champs =0;    // ProgressXml progress[2];
		ChallXml* chall =0;      // ProgressLXml progressL[2];
		CollectXml* collect =0;  // ProgressCXml progressC;
	#endif

	//  📐 get drivability, vehicle on track fitness
	#ifndef SR_EDITOR
		float GetDrivability(std::string car, std::string trk, bool track_user);
	#endif
};
