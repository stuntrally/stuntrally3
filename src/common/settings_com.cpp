#include "pch.h"
#include "settings_com.h"
#include "Def_Str.h"
#include <stdio.h>


void SETcom::SerializeCommon(bool w, CONFIGFILE & c)
{
	//  🪧 menu
	Param(c,w, "game.start_in_main", startInMain);
	
	//  ⚙️ misc
	Param(c,w, "misc.autostart", autostart);
	Param(c,w, "misc.escquit", escquit);
	
	Param(c,w, "misc.language", language);
	Param(c,w, "misc.ogredialog", ogre_dialog);
	Param(c,w, "misc.mouse_capture", mouse_capture);
	//  hlms
	Param(c,w, "misc.debug_shaders", debug_shaders);	Param(c,w, "misc.debug_properties", debug_properties);
	Param(c,w, "misc.cache_hlms", cache_hlms);			Param(c,w, "misc.cache_shaders", cache_shaders);
	Param(c,w, "misc.hud_on", hud_on);

	//  🔧 tweak
	Param(c,w, "tweak.mtr", tweak_mtr);  		Param(c,w, "tweak.find", find_mtr);

	//  🖥️ video  // todo: ..
	// Param(c,w, "video.windowx", windowx);			Param(c,w, "video.windowy", windowy);
	// Param(c,w, "video.fullscreen", fullscreen);		Param(c,w, "video.vsync", vsync);

	// Param(c,w, "video.fsaa", fsaa);
	// Param(c,w, "video.buffer", buffer);				Param(c,w, "video.rendersystem", rendersystem);

	// Param(c,w, "video.limit_fps", limit_fps);
	// Param(c,w, "video.limit_fps_val", limit_fps_val);	Param(c,w, "video.limit_sleep", limit_sleep);

	//  💡 brightness
	Param(c,w, "video.bright", bright);			Param(c,w, "video.contrast", contrast);
	Param(c,w, "graph_lights.bright", car_light_bright);

	//  🪄 Effects  --------
	Param(c,w, "video_eff.ssao", g.ssao);
	Param(c,w, "video_eff.ssao_radius", ssao_radius);	Param(c,w, "video_eff.ssao_scale", ssao_scale);

	Param(c,w, "video_eff.lens_flare", g.lens_flare);
	Param(c,w, "video_eff.sunbeams", g.sunbeams);

	Param(c,w, "video_eff.hdr", g.hdr);
	Param(c,w, "video_eff.gi", gi);

	// Param(c,w, "video_eff.all_effects", all_effects);
	// Param(c,w, "video_eff.softparticles", softparticles);
	// Param(c,w, "video_eff.bloom", bloom);
	// Param(c,w, "video_eff.boost_fov", boost_fov);


	//  📊 Graphics  ----------------
	Param(c,w, "graph_detail.preset", preset);
	
	//  🖼️ textures
	Param(c,w, "graph_detail.anisotropy", g.anisotropy);	Param(c,w, "graph_detail.tex_filter", g.tex_filt);
	
	//  geom detail
	Param(c,w, "graph_detail.view_dist", g.view_distance);
	Param(c,w, "graph_detail.lod_bias", g.lod_bias);		Param(c,w, "graph_detail.road_dist", g.road_dist);

	//  ⛰️ terrain
	Param(c,w, "graph_detail.ter_tripl", g.ter_tripl);		Param(c,w, "graph_detail.tripl_horiz", g.tripl_horiz);
	Param(c,w, "graph_detail.horizons", g.horizons);
	Param(c,w, "graph_detail.ter_lod", g.ter_lod);			Param(c,w, "graph_detail.horiz_lod", g.horiz_lod);		
	
	//  🌳🪨🌿 veget
	Param(c,w, "graph_veget.trees_dist", veg.trees_dist);	// graph_veget.trees and bushes later in settings
	Param(c,w, "graph_veget.bushes_dist", veg.bushes_dist);
	Param(c,w, "graph_veget.grass", veg.grass);				Param(c,w, "graph_veget.grass_dist", veg.grass_dist);
	Param(c,w, "graph_veget.hor_trees", veg.hor_trees);		Param(c,w, "graph_veget.hor_trees_dist", veg.hor_trees_dist);
	Param(c,w, "graph_veget.hor_bushes", veg.hor_bushes);	Param(c,w, "graph_veget.hor_bushes_dist", veg.hor_bushes_dist);
	Param(c,w, "graph_veget.limit", veg.limit);

	//  🌒 shadow
	Param(c,w, "graph_shadow.size", g.shadow_size);
	Param(c,w, "graph_shadow.count", g.shadow_count);		Param(c,w, "graph_shadow.type", g.shadow_type);
	Param(c,w, "graph_shadow.dist", g.shadow_dist);			Param(c,w, "graph_shadow.filter", g.shadow_filter);
	
	//  💡 lights
	Param(c,w, "graph_lights.car", li.front);				Param(c,w, "graph_lights.car_shadows", li.front_shdw);
	Param(c,w, "graph_lights.brake", li.brake);				Param(c,w, "graph_lights.reverse", li.reverse);
	Param(c,w, "graph_lights.underglow", li.under);
	Param(c,w, "graph_lights.boost", li.boost);				Param(c,w, "graph_lights.thrust", li.thrust);
	Param(c,w, "graph_lights.collection", li.collect);		Param(c,w, "graph_lights.track", li.track);
	Param(c,w, "graph_lights.collect_random", collectRandomClr);
	Param(c,w, "graph_lights.grid_quality", li.grid_quality);

	//  🌊 water
	Param(c,w, "graph_water.map_size", g.water_size);		//Param(c,w, "graph_water.skip_frames", g.water_skip);  // todo
	Param(c,w, "graph_water.reflect", g.water_reflect);		Param(c,w, "graph_water.refract", g.water_refract);
	Param(c,w, "graph_water.water_dist", g.water_dist);		Param(c,w, "graph_water.lod", g.water_lod);

	//  🔮 reflections
	Param(c,w, "graph_reflect.map_size", g.refl_size);		
	Param(c,w, "graph_reflect.skip_frames", g.refl_skip);	Param(c,w, "graph_reflect.faces_once", g.refl_faces);
	Param(c,w, "graph_reflect.dist", g.refl_dist);			Param(c,w, "graph_reflect.lod", g.refl_lod);
	Param(c,w, "graph_reflect.ibl", g.refl_ibl);			// Param(c,w, "graph_reflect.mode", g.refl_mode);


	//  🔊 Sounds  --------
	Param(c,w, "sound.volume", s.vol_master);				Param(c,w, "sound.ambient", s.vol_ambient);
#ifndef SR_EDITOR  // game only
	Param(c,w, "sound.vol_engine", s.vol_engine);			Param(c,w, "sound.vol_turbo", s.vol_turbo);
	Param(c,w, "sound.vol_tires", s.vol_tires);				Param(c,w, "sound.vol_env", s.vol_env);
	Param(c,w, "sound.vol_susp", s.vol_susp);
	Param(c,w, "sound.vol_fl_splash", s.vol_fl_splash);		Param(c,w, "sound.vol_fl_cont", s.vol_fl_cont);
	Param(c,w, "sound.vol_car_crash", s.vol_car_crash);		Param(c,w, "sound.vol_car_scrap", s.vol_car_scrap);
	Param(c,w, "sound.hud_vol", s.vol_hud);
	Param(c,w, "sound.hud_chk", s.snd_chk);					Param(c,w, "sound.hud_chk_wrong", s.snd_chkwr);
#endif  // snd setup
	Param(c,w, "sound_config.device", s.snd_device);		Param(c,w, "sound_config.reverb", s.snd_reverb);
	Param(c,w, "sound_config.cnt_sources", s.cnt_sources);	Param(c,w, "sound_config.cnt_dynamics", s.cnt_dynamics);
	Param(c,w, "sound_config.cnt_buffers", s.cnt_buffers);
	Param(c,w, "sound_config.ref_dist", s.ref_dist);		Param(c,w, "sound_config.rolloff", s.rolloff);
	Param(c,w, "sound_config.max_dist", s.max_dist);

	//  🪟 font  ----
	Param(c,w, "font.gui", font_gui);
	Param(c,w, "font.wnds", font_wnds);

	//  📈 fps  ----
	Param(c,w, "tweak.fps", fps_bar);				Param(c,w, "tweak.fps_mul", fps_mul);
	//  🚦 pacenotes
	Param(c,w, "pacenotes.dist", pace_dist);		Param(c,w, "pacenotes.alpha", pace_alpha);
	Param(c,w, "pacenotes.size", pace_size);		Param(c,w, "pacenotes.near", pace_near);
	Param(c,w, "pacenotes.trail", trail_show);
	

	//  🏞️📃 gui tracks  ---
	Param(c,w, "gui_tracks.view", tracks_view);		Param(c,w, "gui_tracks.filter", tracks_filter);
	Param(c,w, "gui_tracks.sort", tracks_sort);		Param(c,w, "gui_tracks.sortup", tracks_sortup);

	//  🏛️🔻 columns, filters
	std::string s;
	int i,v,ii,a;
	
	if (w)	//  write  ----
	{	for (v=0; v < TV_ALL; ++v)
		{
			s = "";  ii = COL_VIS;
			for (i=0; i < ii; ++i)
			{
				s += iToStr(col_vis[v][i]);
				if (i < ii-1)  s += " ";
			}
			Param(c,w, "gui_tracks.columns"+iToStr(v), s);
		}
		for (v=0; v < 2; ++v)
		{
			s = "";  ii = COL_FIL;
			for (i=0; i < ii; ++i)
			{
				s += iToStr(col_fil[v][i]);
				if (i < ii-1)  s += " ";
			}
			Param(c,w, "gui_tracks.filters"+iToStr(v), s);
		}
	}else	//  read  ----
	{
		for (v=0; v < TV_ALL; ++v)
		{
			if (Param(c,w, "gui_tracks.columns"+iToStr(v), s))
			{	std::stringstream sc(s);
				for (i=0; i < COL_VIS; ++i)
				{	sc >> a;  col_vis[v][i] = a > 0;  }
		}	}
		for (v=0; v < 2; ++v)
		{
			if (Param(c,w, "gui_tracks.filters"+iToStr(v), s))
			{	std::stringstream sf(s);
				for (i=0; i < COL_FIL; ++i)
				{	sf >> a;  col_fil[v][i] = a;  }
		}	}
	}
}

//  util
int SETcom::GetTexSize(int n)
{
	const int i = std::min(NumTexSizes-1, std::max(0, n));
	return cTexSizes[i];
}

//  ctor
SETcom::SETcom()   ///  Defaults
{
	int i,v;
	for (v=0; v < TV_ALL; ++v)
		for (i=0; i < COL_VIS; ++i)  col_vis[v][i] = colVisDef[v][i];
	for (v=0; v < 2; ++v)
		for (i=0; i < COL_FIL; ++i)  col_fil[v][i] = colFilDef[v][i];
}

//  🏞️📃🏛️ tracks list views columns  --
const bool SETcom::colVisDef[TV_ALL][COL_VIS] =
	{{0,0,1, 0,0,0, 1,1, 0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,1, 0,0,0, 1,0, 0,0,0,0,0,0,0,0,0,0,0,1,1},
	 {1,0,1, 1,1,1, 1,1, 1,1,1,1,1,1,1,1,1,1,1,0,0},
	 {0,0,1, 0,0,0, 0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0}};  // unused
	
const char SETcom::colFilDef[2][COL_FIL] =
// ver diff rating  objects obstacles  fluids bumps  jumps loops pipes  banked frenzy  sum longn
	// v  ! *   o c  w ~  J L P  b s  E l    * +
	{{01, 0,0,  0,0, 0,0, 0,0,0, 0,0, 0,0,   0,0},  // min
	 {33, 7,6,  4,4, 5,5, 4,6,4, 5,5, 35,35, 6,1}}; // max
	///^ up in next ver, also in *default.cfg
