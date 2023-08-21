#pragma once
#include "configfile.h"

enum eShadowType
{	Sh_None=0, Sh_Depth, Sh_Soft  };

const static int
	COL_VIS = 21, COL_FIL = 16;

const static int NumTexSizes = 6;
const static int cTexSizes[NumTexSizes] = {
	256, 512, 1024, 2048, 4096, 8192 };


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
	//  💡 brightness
	float bright = 1.f, contrast = 1.f; //, gamma = 1.15f;
	
	//  hud
	bool vr_mode = 0;  // 👀 forces 1 player
	bool show_fps =0;  // 📈
	
	//  🚦 pacenotes
	float pace_dist = 200.f, pace_size = 1.f, pace_near = 1.f, pace_alpha = 1.f;


	//  📊 Graphics  ----
	int preset = 4;  // last set, info only

	//  🖼️ textures filtering
	int anisotropy = 4, tex_filt = 2;
	int ter_tripl = 1, tripl_horiz = 0;  // terrain  // ⛰️

	//  geom detail
	float view_distance = 60000.f;
	float lod_bias = 1.f, road_dist = 2.f;
	int ter_detail = 2;
	int horizons = 3;

	//  🌳🪨🌿 veget
	float grass =1.f, trees_dist =1.f, grass_dist =1.f;  // trees in gui.

	//  🌒 shadow
	int shadow_size = 2, shadow_count = 3, shadow_type = Sh_Depth;
	float shadow_dist = 1000.f;  int shadow_filter = 1;
	//  💡 lights
	bool car_lights =0, car_light_shadows =0;

	//  🌊 water
	bool water_reflect =0, water_refract =0;
	int water_size = 1, water_skip = 1;
	float water_lod = 0.3f, water_dist = 60000.f;  //-

	//  🔮 reflections
	int refl_skip = 2, refl_faces = 2, refl_size = 0, refl_ibl = 32;
	float refl_dist = 60000.f, refl_lod = 1.f;
	int refl_mode;  // 0 static, 1 single, 2 full


	//  📰🔻 Tracks list  ----
	//  view, sort column, filter on
	const static int TrkViews = 3;
	int tracks_view =0, tracks_sort =2;
	bool tracks_sortup =1, tracks_filter =0;


	bool col_vis[TrkViews][COL_VIS];  // visible columns for track views
	int  col_fil[2][COL_FIL];  // filtering range for columns 0min-1max

	const static bool colVisDef[TrkViews][COL_VIS];  // view columns
	const static char colFilDef[2][COL_FIL];  // min,max
	

//------------------------------------------
	SETcom();
	int GetTexSize(int n);

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
