#pragma once
#include "configfile.h"
#include "settings_com.h"


#define SET_VER  3016  // 3.0 beta


class SETTINGS : public SETcom
{
public:
///  params
//------------------------------------------
	int version = 100;  // file version =
	
	//  🪧 menu
	bool bMain =1;  int inMenu = 0;

	//  ✅ hud show
	bool trackmap =1, brush_prv =1;  int num_mini =0;
	float size_minimap = 0.5;

	class GameSet
	{
	public:
		std::string track{"Isl6-Flooded"};
		bool track_user =0;
		float trees =1.5f;  // common
	} gui;
	
	//  ⚙️ misc
	bool allow_save =0, inputBar =0, camPos =0;
	bool check_load =0, check_save =1;

	//  ⚙️ settings
	bool bFog =0, bTrees =0, bWeather =0, bEmitters =0;  // toggle
	int ter_skip = 4, mini_skip = 4;
	int road_skip = 12;  float road_sphr = 2.5f;
	float prv_rot_speed = 1.f;  // rad/s
	
	//  🎥 camera
	float cam_speed = 0.9f, cam_inert = 0.4f;
	float cam_x = 0.f, cam_y = 50.f, cam_z = -120.f;
	float cam_dx = 0.f, cam_dy = 0.f, cam_dz = 1.f;
	
	//  ⛰️ ter generator
	float gen_scale = 20.f, gen_ofsx = 0.f, gen_ofsy = 0.f;
	float gen_freq = 0.73f, gen_persist = 0.4f, gen_pow = 1.f;  int gen_oct = 4;
	float gen_mul = 1.f, gen_ofsh = 0.f, gen_roadsm = 0.f;
	float gen_terMinA = 0.f, gen_terMaxA = 90.f, gen_terSmA = 10.f;
	float gen_terMinH = -300.f, gen_terMaxH = 300.f, gen_terSmH = 10.f;

	//  ⛰️🛣️ align ter
	float /*al_w_mul = 1.f,*/ al_w_add = 8.f, al_smooth = 2.f;
	int ah_base_ter = 0;  float ah_below = 10.f/*, ah_border = 10.f*/;  // not saved..
	bool br_adjust =0;
	
	//  🚦 pacenotes
	int pace_show = 3;
	bool trk_reverse =0, show_mph =0;

	//  🔧 tweak
	std::string tweak_mtr, objGroup{"rock"};
	//  👆 pick
	bool pick_setpar =1;

	
//------------------------------------------
	template <typename T>
	bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	void Serialize(bool write, CONFIGFILE & config);
	void Load(std::string sfile), Save(std::string sfile);
};
