#pragma once
#include "dbl.h"
#include "car.h"
#include "collision_world.h"
// #include "collision_contact.h"
#include "carcontrolmap_local.h"
#include "cartire.h"
#include "tracksurface.h"
#include "timer.h"
// #include "forcefeedback.h"
//#include "SoundMgr.h"
#include <OgreTimer.h>
#include <thread>

class SoundMgr;  class Sound;  class App;  class SETTINGS;


class GAME
{
public:
	App* app =0;
	SETTINGS* pSet =0;

	GAME(SETTINGS* pSettings);

	void Start();

	double TickPeriod() const {  return framerate;  }
	bool OneLoop(double dt);

	void ReloadSimData();
	bool reloadSimNeed = 0, reloadSimDone = 0;  //for tweak tire save
	
	bool InitializeSound();  void LoadHudSounds();
	void End();

	void Test();
	void Tick(double dt);

	void AdvanceGameLogic(double dt);
	void UpdateCar(CAR& car, double dt);
	void UpdateCarInputs(CAR& car);
	void UpdateTimer();
	
	/// ---  🆕 new game  ========
	void LeaveGame(bool dstTrk);  // call in this order
	bool NewGameDoLoadTrack();  // track
	bool NewGameDoLoadMisc(float pre_time,  // timer,etc
		std::string ambient_snd, float ambient_vol);
	
	
	CAR* LoadCar(const std::string& pathCar, const std::string& carname,
		const MATHVECTOR<float,3>& start_pos, const QUATERNION<float>& start_rot,
		bool islocal, bool isRemote/*=false*/, int idCar);

	void ProcessNewSettings();
	void UpdateForceFeedback(float dt);
	float GetSteerRange() const;

//  vars

	unsigned int frame = 0, displayframe = 0;  // physics, display frame counters
	double clocktime = 0.0, target_time = 0.0;  // elapsed wall clock time
	/*const*/ double framerate = 1.0 / 160.0;
	float fps_min = 0.f, fps_max = 0.f;

	bool benchmode = 0, profilingmode = 0, pause = 0;


	//  🚗 cars  ----
	std::vector<CAR*> cars;
	std::pair <CAR*, CARCONTROLMAP_LOCAL> controls;

	COLLISION_WORLD collision;
	bool bResetObj = 0;
	
	TIMER timer;  // ⏱️


	//  🔉 Sound  ----
	SoundMgr* snd =0;
	Sound* snd_chk =0, *snd_chkwr =0,
		*snd_lap =0, *snd_lapbest =0,
		*snd_stage =0, *snd_win[3]={0,0,0}, *snd_fail =0;
	void UpdHudSndVol(), DeleteHudSounds();


	///  New  carsim  -------------
	std::vector <CARTIRE> tires;  /// all tires
	std::map <std::string, int> tires_map;  // name to tires id
	bool LoadTire(CARTIRE& ct, std::string path, std::string& file);
	bool LoadTires();

	//  ref graphs, tire edit
	std::string tire_ref;  int tire_ref_id = 0;
	void PickTireRef(std::string name);


	std::vector <TRACKSURFACE> surfaces;  /// all surfaces
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();
	
	std::vector <std::vector <std::pair<double, double> > > suspS,suspD;  /// all suspension factors files (spring, damper)
	std::map <std::string, int> suspS_map,suspD_map;  // name to susp id
	bool LoadSusp();
	
#ifdef ENABLE_FORCE_FEEDBACK
	std::unique_ptr <FORCEFEEDBACK> forcefeedback;
	double ff_update_time;
#endif
};
