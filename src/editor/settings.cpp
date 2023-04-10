#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "tracksurface.h"


void SETTINGS::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
}
void SETTINGS::Save(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);  version = SET_VER;
	Serialize(true, c);  c.Write();
}

//  📄 editor.cfg
void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;
	
	SerializeCommon(w,c);
	
	//  🪧 menu
	Param(c,w, "game.in_main", bMain);		Param(c,w, "game.in_menu", inMenu);

	//  🚗 game common
	Param(c,w, "game.track", gui.track);			Param(c,w, "game.track_user", gui.track_user);
	Param(c,w, "graph_veget.trees", gui.trees);

	//  🚦 pacenotes
	Param(c,w, "pacenotes.show", pace_show);
	Param(c,w, "pacenotes.trk_reverse", trk_reverse);


	//  ✅ hud show
	Param(c,w, "hud_show.trackmap", trackmap);		Param(c,w, "hud_size.minimap", size_minimap);
	Param(c,w, "hud_show.mini_num", num_mini);		Param(c,w, "hud_show.brushpreview", brush_prv);
	
	//  ⚙️ misc
	Param(c,w, "misc.allow_save", allow_save);
	Param(c,w, "misc.inputBar", inputBar);			Param(c,w, "misc.camPos", camPos);
	Param(c,w, "misc.version", version);
	Param(c,w, "misc.check_load", check_load);		Param(c,w, "misc.check_save", check_save);
	
	//  🎥 camera
	Param(c,w, "set_cam.px",cam_x);  Param(c,w, "set_cam.py",cam_y);  Param(c,w, "set_cam.pz",cam_z);
	Param(c,w, "set_cam.dx",cam_dx); Param(c,w, "set_cam.dy",cam_dy); Param(c,w, "set_cam.dz",cam_dz);

	//  ⚙️ settings
	Param(c,w, "set.fog", bFog);					Param(c,w, "set.trees", bTrees);
	Param(c,w, "set.weather", bWeather);			Param(c,w, "set.emitters", bEmitters);
	Param(c,w, "set.cam_speed", cam_speed);			Param(c,w, "set.cam_inert", cam_inert);
	Param(c,w, "set.ter_skip", ter_skip);			Param(c,w, "set.road_sphr", road_sphr);
	Param(c,w, "set.mini_skip", mini_skip);			Param(c,w, "set.road_skip", road_skip);
		
	//  ⛰️ ter generator
	Param(c,w, "ter_gen.scale", gen_scale);
	Param(c,w, "ter_gen.ofsx", gen_ofsx);			Param(c,w, "ter_gen.ofsy", gen_ofsy);
	Param(c,w, "ter_gen.freq", gen_freq);			Param(c,w, "ter_gen.persist", gen_persist);
	Param(c,w, "ter_gen.pow", gen_pow);				Param(c,w, "ter_gen.oct", gen_oct);
	Param(c,w, "ter_gen.mul", gen_mul);				Param(c,w, "ter_gen.ofsh", gen_ofsh);
	Param(c,w, "ter_gen.roadsm", gen_roadsm);
	Param(c,w, "ter_gen.terMinA", gen_terMinA);		Param(c,w, "ter_gen.terMaxA",gen_terMaxA);
	Param(c,w, "ter_gen.terSmA", gen_terSmA);		Param(c,w, "ter_gen.terSmH",gen_terSmH);
	Param(c,w, "ter_gen.terMinH", gen_terMinH);		Param(c,w, "ter_gen.terMaxH",gen_terMaxH);	

	//  ⛰️🛣️ align ter
	Param(c,w, "teralign.w_mul", al_w_mul);			Param(c,w, "teralign.smooth", al_smooth);
	Param(c,w, "teralign.w_add", al_w_add);

	//  🔧 tweak
	Param(c,w, "tweak.mtr", tweak_mtr);

	//  👆 pick
	Param(c,w, "pick.set_par", pick_setpar);		Param(c,w, "pick.objGroup", objGroup);
}
