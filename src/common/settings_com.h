#pragma once
#include "configfile.h"

enum EShadowType
{	Sh_None=0, Sh_Depth, Sh_Soft  };

enum ETracksView
{	TV_List=0, TV_ListUser, TV_ListWide, /*TV_GalleryList,*/ TV_GalleryBig, TV_ALL  };

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
	bool ogre_dialog =0, mouse_capture =1;
	//  hlms
	bool debug_shaders =0, debug_properties =0;
	bool cache_hlms =1, cache_shaders =1;

	//  🔧 tweak
	std::string tweak_mtr, find_mtr;

	std::string language;  // "" autodetect


	//  🖥️ Screen  ----
	// int windowx =800, windowy =600;//, fsaa =0;
	// bool fullscreen =0, vsync =0;
	// std::string buffer, rendersystem;
	//  limit
	// bool limit_fps =0;  float limit_fps_val = 60.f;  int limit_sleep = -1;
	
	//  💡 brightness
	float bright = 1.f, contrast = 1.f;  //, gamma = 1.15f;
	float car_light_bright = 1.f;


	//  Hud  ----
	bool vr_mode = 0;  // 👀 forces 1 player
	int fps_bar =0;  // 📈
	float fps_mul = 1.f;  // red vals factor

	//  🪟 font, gui
	float font_gui = 1.f;  // scales
	float font_wnds = 0.f;  // add margin
	
	//  🚦 pacenotes
	float pace_dist = 200.f, pace_size = 1.f, pace_near = 1.f, pace_alpha = 1.f;
	bool trail_show =1;


	//  📊 Graphics  ----------------
	int preset = 4;  // last set, info only
	bool hud_on =1;  // 0 won't create it, in ed hides dialogs
	bool collectRandomClr =0;

	//!  💡 lights  needs Update in presets:  Lights lights
	struct Lights
	{
		bool front =0, front_shdw =0;  // shadows
		bool brake =0, reverse =0, under =0;
		bool boost =0, thrust =0;
		bool collect =0;
		bool track =0;  // int..
		int grid_quality = 0;
	} li;

	struct Veget  // ----
	{
		//  🌳🪨🌿 veget  6x
		float trees = 1.5f, trees_dist = 1.f;  // only for presets [] arrays
		float grass = 1.f, grass_dist = 1.f;  //! trees in gui. set for mplr
		float bushes = 1.f, bushes_dist = 1.f;
		float hor_trees = 0.5f, hor_trees_dist = 0.5f;  // horiz 4x
		float hor_bushes = 0.3f, hor_bushes_dist = 0.3f;
		int limit = 20000;  // max models count
	} veg;

	//!  Any change or order needs Update in:  Detail presets
	struct Detail
	{
		int sim_quality = 3;
		//  🖼️ textures filtering  2x
		int anisotropy = 4;
		int tex_filt = 2;

		//  geom detail  3x
		float view_distance = 60000.f;
		float lod_bias = 1.f, road_dist = 2.f;  // 📦🪨 🛣️

		//  ⛰️ terrain  5x
		int ter_tripl = 1, tripl_horiz = 0;
		int horizons = 3;
		int ter_lod = 2, horiz_lod = 2;

		//  🌒 shadow  5x
		int shadow_size = 2, shadow_count = 3, shadow_type = Sh_Depth;
		float shadow_dist = 600.f;  int shadow_filter = 1;  //3x3

		//  🌊 water  5x
		int water_size = 1;  //, water_skip = 1;  // todo
		bool water_reflect =0, water_refract =0;
		float water_dist = 2000.f, water_lod = 0.3f;  //-

		//  🔮 reflections  6x
		int refl_size = 1, refl_skip = 0, refl_faces = 2;
		float refl_dist = 500.f, refl_lod = 0.4f;  int refl_ibl = 2;
		// int refl_mode = 1;  // todo: 0 static, 1 single, 2 full
		
		//  🪄 effects  4x
		bool ssao =0;  // 🕳️
		bool lens_flare =0;  // 🔆
		bool sunbeams =0;  // 🌄
		bool hdr =0;  // 🌅 bloom
	} g;

	//  🪄 effects params  --------
	float ssao_radius = 1.f, ssao_scale = 1.5f;
	bool gi =0;  // not in presets-

	// bool all_effects =0;  //?
	// bool soft_particles, motion_blur;
	// float bloom_int, bloom_orig, blur_intensity;


	//  🔊 Sounds  ------------
	struct Sound
	{
		//  volume
		float vol_master = 1.f, vol_hud = 1.f,
			vol_ambient = 1.f;
	#ifndef SR_EDITOR  // game only
		float vol_engine = 0.6f, vol_turbo = 1.f,
			vol_tires =1.f, vol_susp =1.f, vol_env =1.f,
			vol_fl_splash =1.f, vol_fl_cont =1.f,
			vol_car_crash =1.f, vol_car_scrap =1.f;
		bool snd_chk = 0, snd_chkwr = 1;  // play hud
	#endif
		//  setup
		bool snd_reverb = 1;  std::string snd_device;
	
		//  8 players * (10? car + 8 wheels * 2 surfaces)
		// int detail;  // less/more sounds todo ..
		int cnt_buffers = 1024, cnt_sources = 244, cnt_dynamics = 12;  // 256
		int cntAll() {  return cnt_sources + cnt_dynamics;  }

		float max_dist = 500.f, ref_dist = 1.f, rolloff = 0.05f;
	} s;


	//  📰🔻 Tracks list  ------------
	//  view, sort column, filter on
	int tracks_view =0, tracks_sort =2;
	bool tracks_sortup =1, tracks_filter =0;

	bool col_vis[TV_ALL][COL_VIS];  // visible columns for track views
	int  col_fil[2][COL_FIL];  // filtering range for columns 0min-1max

	const static bool colVisDef[TV_ALL][COL_VIS];  // view columns
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
