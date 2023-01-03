#pragma once
#include "configfile.h"

enum eShadowType
{	Sh_None=0, Sh_Depth, Sh_Soft  };

const static int
	COL_VIS = 19, COL_FIL = 14;


class SETcom
{
public:
//------------------------------------------

	//  ⚙️ Startup, other  ----
	bool autostart =0, escquit =1, startInMain =1;
	bool ogre_dialog =0, mouse_capture =1, screen_png =0;

	std::string language;  // "" autodetect

	//  🖥️ Screen  ----
	// int windowx =800, windowy =600;//, fsaa =0;
	// bool fullscreen =0, vsync =0;
	// std::string buffer, rendersystem;
	//  limit
	// bool limit_fps =0;  float limit_fps_val = 60.f;  int limit_sleep = -1;
	//  💡 light
	float bright = 1.f, contrast = 1.f;
	
	//  hud
	bool vr_mode = 0;  // 👀 forces 1 player
	bool show_fps =0;  // 📈
	//  🚦 pacenotes
	float pace_dist = 200.f, pace_size = 1.f, pace_near = 1.f, pace_alpha = 1.f;


	//  📊 Graphics  ----
	int anisotropy = 4, tex_filt = 2;  // textures filtering 🖼️
	//int ter_tripl;  // terrain  // ⛰️
	float view_distance = 20000.f;
	float lod_bias = 1.f, ter_detail = 1.f, road_dist = 2.f;  // detail
	
	//  🌊 water
	// bool water_reflect, water_refract;  int water_rttsize;
	//  🌒 shadow
	int shadow_size = 2, shadow_count = 3, shadow_type = Sh_Depth;
	float shadow_dist = 1000.f;

	//  🌳🪨 🌿
	float grass =1.f, trees_dist =1.f, grass_dist =1.f;  // trees in gui.

	//  graphics other
	int preset = 4;  // last set, info only
	// int horizon;


	//  📰🔻 Tracks list  ----
	//  view, sort column, filter on
	int tracks_view =0, tracks_sort =2;
	bool tracks_sortup =1, tracks_filter =0;


	bool col_vis[2][COL_VIS];  // visible columns for track views
	int  col_fil[2][COL_FIL];  // filtering range for columns 0min-1max

	const static bool colVisDef[2][COL_VIS];
	const static char colFilDef[2][COL_FIL];
	

//------------------------------------------
	SETcom();

	template <typename T>
	bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	void SerializeCommon(bool write, CONFIGFILE & config);
};
