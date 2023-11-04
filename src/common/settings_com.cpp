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
	Param(c,w, "misc.debug_shaders", debug_shaders);
	Param(c,w, "misc.debug_properties", debug_properties);
	Param(c,w, "misc.cache_hlms", cache_hlms);
	Param(c,w, "misc.cache_shaders", cache_shaders);

	//  🖥️ video  // todo: ..
	// Param(c,w, "video.windowx", windowx);			Param(c,w, "video.windowy", windowy);
	// Param(c,w, "video.fullscreen", fullscreen);		Param(c,w, "video.vsync", vsync);

	// Param(c,w, "video.fsaa", fsaa);
	// Param(c,w, "video.buffer", buffer);				Param(c,w, "video.rendersystem", rendersystem);

	// Param(c,w, "video.limit_fps", limit_fps);
	// Param(c,w, "video.limit_fps_val", limit_fps_val);	Param(c,w, "video.limit_sleep", limit_sleep);

	//  💡
	Param(c,w, "video.bright", bright);			Param(c,w, "video.contrast", contrast);
	Param(c,w, "graph_lights.bright", car_light_bright);



	//  📊 graphics  --------
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
	Param(c,w, "graph_veget.grass", g.grass);				// graph_veget.trees later in settings
	Param(c,w, "graph_veget.trees_dist", g.trees_dist);		Param(c,w, "graph_veget.grass_dist", g.grass_dist);

	//  🌒 shadow
	Param(c,w, "graph_shadow.size", g.shadow_size);
	Param(c,w, "graph_shadow.count", g.shadow_count);		Param(c,w, "graph_shadow.type", g.shadow_type);
	Param(c,w, "graph_shadow.dist", g.shadow_dist);			Param(c,w, "graph_shadow.filter", g.shadow_filter);
	
	//  💡 lights
	Param(c,w, "graph_lights.car", g.car_lights);			Param(c,w, "graph_lights.car_shadows", g.car_light_shadows);

	//  🌊 water
	Param(c,w, "graph_water.map_size", g.water_size);		//Param(c,w, "graph_water.skip_frames", g.water_skip);  // todo
	Param(c,w, "graph_water.reflect", g.water_reflect);		Param(c,w, "graph_water.refract", g.water_refract);
	Param(c,w, "graph_water.water_dist", g.water_dist);		Param(c,w, "graph_water.lod", g.water_lod);

	//  🔮 reflections
	Param(c,w, "graph_reflect.map_size", g.refl_size);		
	Param(c,w, "graph_reflect.skip_frames", g.refl_skip);	Param(c,w, "graph_reflect.faces_once", g.refl_faces);
	Param(c,w, "graph_reflect.dist", g.refl_dist);			Param(c,w, "graph_reflect.lod", g.refl_lod);
	Param(c,w, "graph_reflect.ibl", g.refl_ibl);			// Param(c,w, "graph_reflect.mode", g.refl_mode);


	//  📈 fps  ----
	Param(c,w, "tweak.fps", fps_bar);				Param(c,w, "tweak.fps_mul", fps_mul);
	//  🚦 pacenotes
	Param(c,w, "pacenotes.dist", pace_dist);		Param(c,w, "pacenotes.alpha", pace_alpha);
	Param(c,w, "pacenotes.size", pace_size);		Param(c,w, "pacenotes.near", pace_near);


	//  🏞️📃 gui tracks  ---
	Param(c,w, "gui_tracks.view", tracks_view);		Param(c,w, "gui_tracks.filter", tracks_filter);
	Param(c,w, "gui_tracks.sort", tracks_sort);		Param(c,w, "gui_tracks.sortup", tracks_sortup);

	//  🏛️🔻 columns, filters
	std::string s;
	int i,v,ii,a;
	
	if (w)	//  write  ----
	{	for (v=0; v < TrkViews; ++v)
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
		for (v=0; v < TrkViews; ++v)
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
	for (v=0; v < TrkViews; ++v)
		for (i=0; i < COL_VIS; ++i)  col_vis[v][i] = colVisDef[v][i];
	for (v=0; v < 2; ++v)
		for (i=0; i < COL_FIL; ++i)  col_fil[v][i] = colFilDef[v][i];
}

//  🏞️📃🏛️ tracks list views columns  --
const bool SETcom::colVisDef[TrkViews][COL_VIS] =
	{{0,0,1, 0,0,0, 1,1, 0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {1,0,1, 1,1,1, 1,1, 1,1,1,1,1,1,1,1,1,1,1,0,0},
	 {0,0,1, 0,0,0, 1,0, 0,0,0,0,0,0,0,0,0,0,0,1,1}};
	
const char SETcom::colFilDef[2][COL_FIL] =
// ver diff rating  objects obstacles  fluids bumps  jumps loops pipes  banked frenzy  sum longn
	// v  ! *   o c  w ~  J L P  b s  E l    * +
	{{01, 0,0,  0,0, 0,0, 0,0,0, 0,0, 0,0,   0,0},  // min
	 {30, 7,6,  4,4, 5,5, 4,6,4, 5,5, 30,25, 6,1}}; // max
	///^ up in next ver, also in *default.cfg
