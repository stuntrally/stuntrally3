#include "pch.h"
#include "settings.h"
//#include "protocol.hpp"
#include <stdio.h>
#include <OgreString.h>
using namespace Ogre;


void SETTINGS::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);

	String p_file = StringUtil::replaceAll(sfile, "game", "paint");

	CONFIGFILE p;  p.Load(p_file);
	SerPaints(false, p);
}
void SETTINGS::Save(std::string sfile)
{
	if (net_local_plr > 0)  // save only for host for many local games
		return;
	CONFIGFILE c;  //c.Load(sfile);
	version = SET_VER;
	Serialize(true, c);  c.Write(sfile);

	String p_file = StringUtil::replaceAll(sfile, "game", "paint");

	CONFIGFILE p;  //p.Load(p_file);
	SerPaints(true, p);  p.Write(p_file);
}

//  📄 paint.cfg
void SETTINGS::SerPaints(bool wr, CONFIGFILE & cf)
{
	for (int i=0; i < 6; ++i)  // cars
	{
		char ss[64];
		sprintf(ss, "car%d.", i+1);
		const std::string s = ss;

		auto& p = gui.clr[i];
		PaintsIni::SerPaint(wr, cf, s, p);
	}
}


//  📄 game.cfg
void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;

	SerializeCommon(w,c);

	//  🪧 menu
	Param(c,w, "game.menu", iMenu);  Param(c,w, "game.ymain", yMain);  Param(c,w, "game.yrace", yRace);
	Param(c,w, "game.difficulty", difficulty);

	//  game common
	Param(c,w, "game.track", gui.track);				Param(c,w, "game.track_user", gui.track_user);
	Param(c,w, "graph_veget.trees", gui.trees);


	//  🚗 cars
	for (int i=0; i < 6; ++i)
	{
		char ss[64];  sprintf(ss, "car%d.", i+1);   std::string s = ss;
		if (i < 4)
		{	Param(c,w, s+"car", gui.car[i]);		Param(c,w, s+"camera", cam_view[i]);
		}
	}
	// todo: this for all 4 cars-
	Param(c,w, "car1.autotrans", autoshift);
	Param(c,w, "car1.autorear", autorear);		Param(c,w, "car1.autorear_inv", rear_inv);
	for (int i=0; i <= 1; ++i)  // steering
	{	std::string s = i==1 ? "A":"";
		Param(c,w, "car1.abs"+s, abs[i]);		Param(c,w, "car1.tcs"+s, tcs[i]);
		Param(c,w, "car1.sss_effect"+s, sss_effect[i]);
		Param(c,w, "car1.sss_velfactor"+s, sss_velfactor[i]);
		Param(c,w, "car1.steer_range"+s, steer_range[i]);
	}
	Param(c,w, "car1.steer_sim_easy", steer_sim[0]);
	Param(c,w, "car1.steer_sim_normal", steer_sim[1]);

	//  🚗 game
	Param(c,w, "game.pre_time", gui.pre_time);			Param(c,w, "game.chall_num", gui.chall_num);  //rem-
	Param(c,w, "game.champ_num", gui.champ_num);		Param(c,w, "game.champ_rev", gui.champ_rev);
	Param(c,w, "game.ch_all", ch_all);

	Param(c,w, "game.boost_type", gui.boost_type);		Param(c,w, "game.flip_type", gui.flip_type);
	Param(c,w, "game.rewind_type", gui.rewind_type);
	Param(c,w, "game.damage_type", gui.damage_type);	Param(c,w, "game.damage_dec", gui.damage_dec);

	Param(c,w, "game.boost_power", gui.boost_power);
	Param(c,w, "game.boost_min", gui.boost_min);		Param(c,w, "game.boost_max", gui.boost_max);
	Param(c,w, "game.boost_per_km", gui.boost_per_km);	Param(c,w, "game.boost_add_sec", gui.boost_add_sec);

	Param(c,w, "game.collis_cars", gui.collis_cars);	Param(c,w, "game.collis_veget", gui.collis_veget);
	Param(c,w, "game.collis_roadw", gui.collis_roadw);	Param(c,w, "game.dyn_objects", gui.dyn_objects);
	Param(c,w, "game.drive_horiz", gui.drive_horiz);

	Param(c,w, "game.trk_reverse", gui.track_reversed);	Param(c,w, "game.sim_mode", gui.sim_mode);
	Param(c,w, "game.local_players", gui.local_players); Param(c,w, "game.num_laps", gui.num_laps);
	Param(c,w, "game.start_order", gui.start_order);	Param(c,w, "game.split_vertically", split_vertically);

	//  📉 graphs
	Param(c,w, "graphs.tc_r", tc_r);			Param(c,w, "graphs.tc_xr", tc_xr);
	Param(c,w, "graphs.te_yf", te_yf);			Param(c,w, "graphs.te_xf_pow", te_xf_pow);
	Param(c,w, "graphs.te_xfx", te_xfx);		Param(c,w, "graphs.te_xfy", te_xfy);
	Param(c,w, "graphs.te_reference", te_reference);	Param(c,w, "graphs.te_common", te_common);


	//  ✅ hud
	Param(c,w, "hud_show.mph", show_mph);
	Param(c,w, "hud_show.gauges", show_gauges);				Param(c,w, "hud_show.show_digits", show_digits);
	Param(c,w, "hud_show.trackmap", trackmap);				Param(c,w, "hud_show.times", show_times);
	Param(c,w, "hud_show.caminfo", show_cam);				Param(c,w, "hud_show.cam_tilt", cam_tilt);
	Param(c,w, "hud_show.car_dbgtxt", car_dbgtxt);			Param(c,w, "hud_show.show_cardbg", car_dbgbars);
	Param(c,w, "hud_show.car_dbgsurf", car_dbgsurf);		Param(c,w, "hud_show.car_tirevis", car_tirevis);
	Param(c,w, "hud_show.car_dbgtxtclr", car_dbgtxtclr);	Param(c,w, "hud_show.car_dbgtxtcnt", car_dbgtxtcnt);
	Param(c,w, "hud_show.check_arrow", check_arrow);		Param(c,w, "hud_show.check_beam", check_beam);
	Param(c,w, "hud_show.opponents", show_opponents);		Param(c,w, "hud_show.opplist_sort", opplist_sort);
	Param(c,w, "hud_show.graphs", show_graphs);				Param(c,w, "hud_show.graphs_type", (int&)graphs_type);
	//  🎛️ gui
	Param(c,w, "gui.cars_view", cars_view);			Param(c,w, "gui.cars_sort", cars_sort);
	Param(c,w, "gui.car_ed_tab", car_ed_tab);		Param(c,w, "gui.tweak_tab", tweak_tab);
	Param(c,w, "gui.champ_tab", champ_type);
	Param(c,w, "gui.chall_tab", chall_type);		Param(c,w, "gui.champ_info", champ_info);
	Param(c,w, "gui.car_clr", car_clr);
	//  🎚️ hud size
	Param(c,w, "hud_size.gauges", size_gauges);			Param(c,w, "hud_size.arrow", size_arrow);
	Param(c,w, "hud_size.minimap", size_minimap);		Param(c,w, "hud_size.minipos", size_minipos);
	Param(c,w, "hud_size.mini_zoom", zoom_minimap);		Param(c,w, "hud_size.mini_zoomed", mini_zoomed);
	Param(c,w, "hud_size.mini_rotated", mini_rotated);	Param(c,w, "hud_size.mini_terrain", mini_terrain);
	Param(c,w, "hud_size.mini_border", mini_border);
	Param(c,w, "hud_size.gauges_type", gauges_type);	//Param(c,w, "hud_size.gauges_layout", gauges_layout);
	//  🎥 camera
	Param(c,w, "hud_size.cam_loop_chng", cam_loop_chng);	Param(c,w, "hud_size.cam_in_loop", cam_in_loop);
	Param(c,w, "hud_size.fov", fov_min);					Param(c,w, "hud_size.fov_boost", fov_boost);
	Param(c,w, "hud_size.fov_smooth", fov_smooth);
	Param(c,w, "hud_size.cam_bounce", cam_bounce);			Param(c,w, "hud_size.cam_bnc_mul", cam_bnc_mul);
	//  🚦 pacenotes
	Param(c,w, "pacenotes.show", pace_show);	Param(c,w, "pacenotes.trail", trail_show);


	//  📈 graphics
	Param(c,w, "graph_par.particles", particles);			Param(c,w, "graph_par.trails", trails);
	Param(c,w, "graph_par.particles_len", particles_len);	Param(c,w, "graph_par.trail_len", trails_len);

	//  ⚙️ misc
	Param(c,w, "misc.version", version);
	Param(c,w, "misc.bulletDebug", bltDebug);		Param(c,w, "misc.bulletLines", bltLines);
	Param(c,w, "misc.profilerTxt", profilerTxt);	Param(c,w, "misc.bulletProfilerTxt", bltProfilerTxt);
	Param(c,w, "misc.dev_keys", dev_keys);			Param(c,w, "misc.dev_no_prvs", dev_no_prvs);

	Param(c,w, "misc.show_welcome", show_welcome);	Param(c,w, "misc.loadingback", loadingbackground);
	Param(c,w, "misc.paintAdj", paintAdj);

	//  📡 network
	Param(c,w, "network.master_server_address", master_server_address);	Param(c,w, "network.nickname", nickname);
	Param(c,w, "network.master_server_port", master_server_port);		Param(c,w, "network.local_port", local_port);
	Param(c,w, "network.connect_address", connect_address);				Param(c,w, "network.game_name", netGameName);
	Param(c,w, "network.connect_port", connect_port);

	//  📽️ replay
	Param(c,w, "replay.record", rpl_rec);			Param(c,w, "replay.ghost", rpl_ghost);
	Param(c,w, "replay.bestonly", rpl_bestonly);	Param(c,w, "replay.trackghost", rpl_trackghost);
	Param(c,w, "replay.listview", rpl_listview);	Param(c,w, "replay.listghosts", rpl_listghosts);
	Param(c,w, "replay.ghostpar", rpl_ghostpar);	Param(c,w, "replay.ghostother", rpl_ghostother);
	Param(c,w, "replay.num_views", rpl_numViews);	Param(c,w, "replay.ghostrewind", rpl_ghostrewind);
	Param(c,w, "replay.ghoHideDist", ghoHideDist);	Param(c,w, "replay.ghoHideDistTrk", ghoHideDistTrk);
	Param(c,w, "replay.hideHudAids", rpl_hideHudAids);

	//  🎳 sim
	Param(c,w, "sim.game_freq", game_fq);			Param(c,w, "sim.dynamics_iter", dyn_iter);
	Param(c,w, "sim.bullet_freq", blt_fq);			Param(c,w, "sim.bullet_iter", blt_iter);
	Param(c,w, "sim.thread_sleep", thread_sleep);	Param(c,w, "sim.perf_speed", perf_speed);
	//Param(c,w, "sim.gui_sleep", gui_sleep);


	//  🔊 sound
	Param(c,w, "sound.device", snd_device);			Param(c,w, "sound.reverb", snd_reverb);
	Param(c,w, "sound.volume", vol_master);			Param(c,w, "sound.vol_engine", vol_engine);
	Param(c,w, "sound.vol_tires", vol_tires);		Param(c,w, "sound.vol_env", vol_env);
	Param(c,w, "sound.vol_susp", vol_susp);
	Param(c,w, "sound.vol_fl_splash", vol_fl_splash);	Param(c,w, "sound.vol_fl_cont", vol_fl_cont);
	Param(c,w, "sound.vol_car_crash", vol_car_crash);	Param(c,w, "sound.vol_car_scrap", vol_car_scrap);
	Param(c,w, "sound.hud_vol", vol_hud);
	Param(c,w, "sound.hud_chk", snd_chk);			Param(c,w, "sound.hud_chk_wrong", snd_chkwr);

	//  effects
	/*Param(c,w, "video_eff.all_effects", all_effects);
	Param(c,w, "video_eff.bloom", bloom);				Param(c,w, "video_eff.bloomintensity", bloom_int);
	Param(c,w, "video_eff.ssao", ssao);					Param(c,w, "video_eff.softparticles", softparticles);
	Param(c,w, "video_eff.godrays", godrays);
	Param(c,w, "video_eff.boost_fov", boost_fov);
	//  effects hdr
	Param(c,w, "video_eff.hdr", hdr);					Param(c,w, "video_eff.hdr_p1", hdrParam1);*/
}

SETTINGS::SETTINGS()
{
	cam_view.resize(4);
	for (int i=0; i < 4; ++i)
		cam_view[i] = 9;
}


SETTINGS::GameSet::GameSet()
{
	clr.resize(6);
	car.resize(4);

	BoostDefault();

	for (int i=0; i < 4; ++i)
		car[i] = "HI";
}

void SETTINGS::GameSet::BoostDefault()
{
	boost_power = 1.f;
	boost_max = 6.f;  boost_min = 2.f;
	boost_per_km = 1.f;  boost_add_sec = 0.1f;
}
