#include "pch.h"
#include "settings_com.h"
#include "Def_Str.h"
#include <stdio.h>


void SETcom::SerializeCommon(bool w, CONFIGFILE & c)
{
	//  🪧 menu
	Param(c,w, "game.start_in_main", startInMain);
	
	//  ⚙️ misc
	Param(c,w, "misc.autostart", autostart);		Param(c,w, "misc.ogredialog", ogre_dialog);
	Param(c,w, "misc.escquit", escquit);
	
	Param(c,w, "misc.language", language);
	Param(c,w, "misc.mouse_capture", mouse_capture);
	Param(c,w, "misc.screenshot_png", screen_png);

	//  🖥️ video  // todo..
	// Param(c,w, "video.windowx", windowx);			Param(c,w, "video.windowy", windowy);
	// Param(c,w, "video.fullscreen", fullscreen);		Param(c,w, "video.vsync", vsync);

	// Param(c,w, "video.fsaa", fsaa);
	// Param(c,w, "video.buffer", buffer);				Param(c,w, "video.rendersystem", rendersystem);

	// Param(c,w, "video.limit_fps", limit_fps);
	// Param(c,w, "video.limit_fps_val", limit_fps_val);	Param(c,w, "video.limit_sleep", limit_sleep);


	//  📊 graphics  ----
	Param(c,w, "graph_detail.preset", preset);
	
	//  textures
	Param(c,w, "graph_detail.anisotropy", anisotropy);		Param(c,w, "graph_detail.tex_filter", tex_filt);
	// Param(c,w, "graph_detail.ter_mtr", ter_mtr);			Param(c,w, "graph_detail.ter_tripl", ter_tripl);
	
	//  detail
	Param(c,w, "graph_detail.view_dist", view_distance);
	Param(c,w, "graph_detail.lod_bias", lod_bias);
	Param(c,w, "graph_detail.ter_detail", ter_detail);		Param(c,w, "graph_detail.road_dist", road_dist);
	
	//  🌒 shadow
	Param(c,w, "graph_shadow.dist", shadow_dist);			Param(c,w, "graph_shadow.size", shadow_size);
	Param(c,w, "graph_shadow.count", shadow_count);			Param(c,w, "graph_shadow.type", shadow_type);
	Param(c,w, "graph_shadow.filter", shadow_filter);
	
	//  💡 lights
	Param(c,w, "graph_lights.car", car_lights);		Param(c,w, "graph_lights.car_shadows", car_light_shadows);

	//  🌳🪨🌿 veget
	Param(c,w, "graph_veget.grass", grass);
	Param(c,w, "graph_veget.trees_dist", trees_dist);		Param(c,w, "graph_veget.grass_dist", grass_dist);

	//  🔮 reflections
	Param(c,w, "graph_reflect.skip_frames", refl_skip);		Param(c,w, "graph_reflect.faces_once", refl_faces);
	Param(c,w, "graph_reflect.map_size", refl_size);		Param(c,w, "graph_reflect.dist", refl_dist);
	Param(c,w, "graph_reflect.mode", refl_mode);

	//  🌊 water
	// Param(c,w, "graph_reflect.water_reflect", water_reflect); Param(c,w, "graph_reflect.water_refract", water_refract);
	// Param(c,w, "graph_reflect.water_rttsize", water_rttsize);


	//  📈 hud
	Param(c,w, "hud_show.fps", show_fps);
	//  🚦 pacenotes
	Param(c,w, "pacenotes.dist", pace_dist);		Param(c,w, "pacenotes.alpha", pace_alpha);
	Param(c,w, "pacenotes.size", pace_size);		Param(c,w, "pacenotes.near", pace_near);


	//  🏞️📃 gui tracks  ---
	Param(c,w, "gui_tracks.view", tracks_view);		Param(c,w, "gui_tracks.filter", tracks_filter);
	Param(c,w, "gui_tracks.sort", tracks_sort);		Param(c,w, "gui_tracks.sortup", tracks_sortup);

	//  🏛️🔻 columns, filters
	std::string s;
	int i,v,ii,a;
	
	if (w)	//  write
		for (v=0; v < 2; ++v)
		{
			s = "";  ii = COL_VIS;
			for (i=0; i < ii; ++i)
			{
				s += iToStr(col_vis[v][i]);
				if (i < ii-1)  s += " ";
			}
			Param(c,w, "gui_tracks.columns"+iToStr(v), s);

			s = "";  ii = COL_FIL;
			for (i=0; i < ii; ++i)
			{
				s += iToStr(col_fil[v][i]);
				if (i < ii-1)  s += " ";
			}
			Param(c,w, "gui_tracks.filters"+iToStr(v), s);
		}
	else	//  read
		for (v=0; v < 2; ++v)
		{
			if (Param(c,w, "gui_tracks.columns"+iToStr(v), s))
			{	std::stringstream sc(s);
				for (i=0; i < COL_VIS; ++i)
				{	sc >> a;  col_vis[v][i] = a > 0;  }
			}

			if (Param(c,w, "gui_tracks.filters"+iToStr(v), s))
			{	std::stringstream sf(s);
				for (i=0; i < COL_FIL; ++i)
				{	sf >> a;  col_fil[v][i] = a;  }
		}	}
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
	for (v=0; v < 2; ++v)
	{	for (i=0; i < COL_FIL; ++i)  col_fil[v][i] = colFilDef[v][i];
		for (i=0; i < COL_VIS; ++i)  col_vis[v][i] = colVisDef[v][i];
	}
}

//  🏞️📃🏛️ tracks list views columns  --
const bool SETcom::colVisDef[2][COL_VIS] =
	{{0,0,1, 0,0,0, 1,1, 0,0,0,0,0,0,0,0,0,0,0},
	 {1,0,1, 1,1,1, 1,1, 1,1,1,1,1,1,1,1,1,1,1}};
	
const char SETcom::colFilDef[2][COL_FIL] =
// ver diff rating  objects obstacles  fluids bumps  jumps loops pipes  banked frenzy  sum longn
	// v  ! *   o c  w ~  J L P  b s  E l
	{{01, 0,0,  0,0, 0,0, 0,0,0, 0,0, 0,0},
	 {27, 6,6,  4,4, 5,5, 4,5,4, 5,5, 25,24}};
	///^ up in next ver, also in *default.cfg
