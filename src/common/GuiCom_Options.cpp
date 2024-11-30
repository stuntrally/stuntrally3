#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "App.h"
#include "Cam.h"
// #include "Road.h"
#include "Grass.h"
#ifndef SR_EDITOR
	#include "game.h"
#endif
#include "CGui.h"
// #include "Slider.h"
// #include "Terra.h"
#include <OgreCamera.h>
#include <Vct/OgreVctLighting.h>
#include <Vct/OgreVctVoxelizer.h>
#include <IrradianceField/OgreIrradianceField.h>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;



///  🎛️ Gui Init  📊 Graphics
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitGraphics()  // ? not yet: called on preset change with bGI true
{
	Btn btn;  Cmb cmb;
	SV* sv;  Ck* ck;  int i;

	BtnC("Quit", btnQuit);  bnQuit = btn;
	BtnC("MatEditor", btnMatEditor);

	//  📈 Fps
	sv= &svFps;			sv->Init("Fps",			&pSet->fps_bar, 0, FPS_Modes);  sv->DefaultI(1);  SevC(Fps);
	sv= &svFpsMul;		sv->Init("FpsMul",		&pSet->fps_mul, 0.1f,6.f, 1.5f);  sv->DefaultF(1.f);
	txFpsInfo = fTxt("FpsInfo");   slFps(0);


	//  🧊 Detail, far geometry
	//------------------------------------------------------------
	const Real maxDist = 60000.f;
	sv= &svLodBias;		sv->Init("LodBias",		&pSet->g.lod_bias,	0.01f,4.f, 1.5f);  SevC(LodBias);  sv->DefaultF(1.f);
	sv= &svRoadDist;	sv->Init("RoadDist",	&pSet->g.road_dist,	0.f,5.f, 2.f, 2,5);  sv->DefaultF(1.6f);
	sv= &svViewDist;	sv->Init("ViewDist",	&pSet->g.view_distance, 50.f,maxDist, 2.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																			SevC(ViewDist);  sv->DefaultF(maxDist);

	//  🖼️ Textures filtering
	//------------------------------------------------------------
	CmbC(cmb, "TexFiltering", cmbTexFilter);  cbTexFilter = cmb;
	cmb->removeAllItems();
	cmb->addItem(TR("#{Bilinear}"));     cmb->addItem(TR("#{Trilinear}"));
	cmb->addItem(TR("#{Anisotropic}"));  cmb->addItem(TR("#{Anisotropic} #{RplAll}"));
	cmb->setIndexSelected(pSet->g.tex_filt);  //cmbTexFilter(cmb, pSet->g.tex_filt); //-

	sv= &svAnisotropy;	sv->Init("Anisotropy",	&pSet->g.anisotropy,  1,16);  SevC(Anisotropy);  sv->DefaultI(4);
	// BtnC("ApplyShaders", btnShaders);

	//  ⛰️ Terrain
	//------------------------------------------------------------
	//sv->strMap[1] = TR("#{max} 2");  sv->strMap[2] = TR("#{Any}");  // todo: shader tripl layers only..
	auto triplMap = [&](){
		sv->strMap[0] = TR("0 #{None}");		sv->strMap[1] = TR("1 #{Diffuse}");
		sv->strMap[2] = TR("2 #{Specular}");	sv->strMap[3] = TR("3 = #{Normal}");  };
	sv= &svTerTripl;	triplMap();
						sv->Init("TerTripl",	&pSet->g.ter_tripl,   0,2);  sv->DefaultI(1);  // 3 todo
	sv= &svTriplHoriz;	triplMap();
						sv->Init("TriplHoriz",	&pSet->g.tripl_horiz, 0,2);  sv->DefaultI(0);

	sv= &svTerLod;		sv->Init("TerLod",		&pSet->g.ter_lod,   0,5);  SevC(TerLod);  sv->DefaultI(2);
	sv= &svHorizLod;	sv->Init("HorizLod",	&pSet->g.horiz_lod, 0,5);  SevC(TerLod);  sv->DefaultI(2);
	sv= &svHorizons;	sv->Init("Horizons",	&pSet->g.horizons,  0,3);  sv->DefaultI(2);


	//  🌳🪨 Veget  🌿 grass
	//------------------------------------------------------------
	sv= &svTrees;		sv->Init("Trees",		&pSet->gui.trees,      0.0f,9.f, 2.f);  sv->DefaultF(1.5f);
	sv= &svTreesDist;	sv->Init("TreesDist",	&pSet->veg.trees_dist, 0.1f,9.f, 2.f);  sv->DefaultF(1.f);
	sv= &svBush;		sv->Init("Bush",		&pSet->gui.bushes,     0.0f,9.f, 2.f);  sv->DefaultF(1.f);
	sv= &svBushDist;	sv->Init("BushDist",	&pSet->veg.bushes_dist,0.1f,9.f, 2.f);  sv->DefaultF(1.f);
	sv= &svGrass;		sv->Init("Grass",		&pSet->veg.grass,	   0.0f,9.f, 2.f);  sv->DefaultF(1.f);
	sv= &svGrassDist;	sv->Init("GrassDist",	&pSet->veg.grass_dist, 0.1f,9.f, 2.f);  sv->DefaultF(2.f);

	sv= &svHorTrees;	sv->Init("HorTrees",	&pSet->veg.hor_trees,      0.0f,1.f);  sv->DefaultF(0.5f);
	sv= &svHorTreesDist;sv->Init("HorTreesDist",&pSet->veg.hor_trees_dist, 0.0f,1.f);  sv->DefaultF(0.5f);
	sv= &svHorBush;		sv->Init("HorBush",		&pSet->veg.hor_bushes,     0.0f,1.f);  sv->DefaultF(0.f);
	sv= &svHorBushDist;	sv->Init("HorBushDist",	&pSet->veg.hor_bushes_dist,0.0f,1.f);  sv->DefaultF(0.f);
	sv= &svVegetLimit;	sv->Init("VegetLimit",	&pSet->veg.limit,         0, 100000);  sv->DefaultI(20000);
	BtnC("ApplyVeget",    btnVegetApply);   BtnC("VegetReset",  btnVegetReset);
	BtnC("HorApplyVeget", btnVegetApply);


	//  🌒 Shadows
	//------------------------------------------------------------
	sv= &svShadowType;
		sv->strMap[0] = TR("#{None}");  sv->strMap[1] = TR("#{FogNormal}");  sv->strMap[2] = "= Soft";
						sv->Init("ShadowType",	&pSet->g.shadow_type,  0,2);  sv->DefaultI(1);
	sv= &svShadowCount;	sv->Init("ShadowCount",	&pSet->g.shadow_count, 1,5);  sv->DefaultI(3);
	sv= &svShadowSize;
		for (i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);
						sv->Init("ShadowSize",	&pSet->g.shadow_size, 0,NumTexSizes-1);   sv->DefaultI(3);
	sv= &svShadowDist;	sv->Init("ShadowDist",	&pSet->g.shadow_dist, 20.f,5000.f, 3.f, 0,3, 1.f," m");  sv->DefaultF(600.f);  // todo: splits?
	sv= &svShadowFilter;sv->Init("ShadowFilter",&pSet->g.shadow_filter, 0,4, 1);  sv->DefaultI(2);  // PCF_2x2..6x6
	BtnC("ApplyShadows", btnShadowsApply);

	//  💡 Lights
	//------------------------------------------------------------
#ifndef SR_EDITOR  // game vehicles
	ck= &ckLiFront;			ck->Init("LightsFront",        &pSet->li.front);  //CevC(Lights);
	ck= &ckLiFrontShadows;	ck->Init("LightsFrontShadows", &pSet->li.front_shdw);
	sv= &svCarLightBright;	sv->Init("CarLightBright",   &pSet->car_light_bright, 0.1f,3.f, 1.f);
																sv->DefaultF(1.f);  SevC(CarLightBright);
	sv= &svLightsGrid;		sv->Init("LightsGrid",   &pSet->li.grid_quality, 0,3);  sv->DefaultI(0);

	ck= &ckLiBrake;		ck->Init("LightsBrake",  &pSet->li.brake);
	ck= &ckLiRevese;	ck->Init("LightsRevese", &pSet->li.reverse);
	ck= &ckLiUnder;		ck->Init("LightsUnder", &pSet->li.under);

	ck= &ckLiBoost;		ck->Init("LightsBoost", &pSet->li.boost);
	ck= &ckLiThrust;	ck->Init("LightsThrust", &pSet->li.thrust);
#endif
	ck= &ckLiCollect;	ck->Init("CollectLights", &pSet->li.collect);
	ck= &ckLiColRndClr;	ck->Init("CollectLiRandom", &pSet->collectRandomClr);
	ck= &ckLiTrack;		ck->Init("TrackLights", &pSet->li.track);


	//  🔮 Reflection
	//------------------------------------------------------------
	sv= &svReflSkip;	sv->Init("ReflSkip",	&pSet->g.refl_skip,    0,1000, 2.f);  sv->DefaultI(0);
	sv= &svReflFaces;	sv->Init("ReflFaces",	&pSet->g.refl_faces,   1,6);  sv->DefaultI(1);
	sv= &svReflSize;
		for (i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);  //v way too big
						sv->Init("ReflSize",	&pSet->g.refl_size,    0,NumTexSizes-1 -1, 2.f);  sv->DefaultI(0);

	sv= &svReflDist;	sv->Init("ReflDist",	&pSet->g.refl_dist,	20.f,maxDist, 2.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																	SevC(ReflDist);  sv->DefaultF(1000.f);
	sv= &svReflIbl;		sv->Init("ReflIbl",		&pSet->g.refl_ibl,	0,8, 2.f);  SevC(ReflIbl);  sv->DefaultI(5);
	sv= &svReflLod;		sv->Init("ReflLod",		&pSet->g.refl_lod,	0.01f,4.f, 1.5f);  SevC(ReflLod);  sv->DefaultF(0.4f);
	BtnC("ApplyRefl", btnReflApply);
	/*sv= &svReflMode;  // todo: 1 cube refl for each car..
	sv->strMap[0] = TR("#{ReflMode_static}");  sv->strMap[1] = TR("#{ReflMode_single}");
	sv->strMap[2] = TR("#{ReflMode_full}");
	sv->Init("ReflMode",	&pSet->refl_mode,   0,2);  SevC(ReflMode);  sv->DefaultI(1);*/

	//  🌊 Water  // todo:
	//------------------------------------------------------------
	ck= &ckWaterReflect; ck->Init("WaterReflection", &pSet->g.water_reflect);  CevC(Water);
	ck= &ckWaterRefract; ck->Init("WaterRefraction", &pSet->g.water_refract);  CevC(Water);
	sv= &svWaterSize;
		for (int i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);
						sv->Init("WaterSize",	&pSet->g.water_size, 0,NumTexSizes-1);  sv->DefaultI(0);  SevC(WaterSize);

// sv= &svWaterSkip;	sv->Init("WaterSkip",	&pSet->g.water_skip,	0,20, 2.f);  sv->DefaultI(0);
	sv= &svWaterDist;	sv->Init("WaterDist",	&pSet->g.water_dist,	20.f,maxDist, 4.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																	SevC(WaterDist);  sv->DefaultF(1000.f);
	sv= &svWaterLod;	sv->Init("WaterLod",	&pSet->g.water_lod,	0.01f,2.f, 2.f);  sv->DefaultF(0.4f);
	// BtnC("ApplyShadersWater", btnWaterApply);


	//  📊 Presets
	//------------------------------------------------------------
	CmbC(cmb, "CmbGraphicsPreset", cmbGraphicsPreset);
	cmb->removeAllItems();
	cmb->addItem(TR("#{GraphicsAll_Lowest}"));  cmb->addItem(TR("#{GraphicsAll_Low}"));
	cmb->addItem(TR("#{GraphicsAll_Medium}"));  cmb->addItem(TR("#{GraphicsAll_High}"));
	cmb->addItem(TR("#{GraphicsAll_Higher}"));  cmb->addItem(TR("#{GraphicsAll_VeryHigh}"));
	cmb->addItem(TR("#{GraphicsAll_Highest}")); cmb->addItem(TR("#{GraphicsAll_Ultra}"));
	cmb->setIndexSelected(pSet->preset);
	
	//  Video  // todo?
	// ck= &ckLimitFps;   ck->Init("LimitFpsOn", &pSet->limit_fps);
	// sv= &svLimitFps;   sv->Init("LimitFps",   &pSet->limit_fps_val,	20.f,144.f);  sv->DefaultF(60.f);
	// sv= &svLimitSleep; sv->Init("LimitSleep", &pSet->limit_sleep,  -2,20, 1.5f);  sv->DefaultI(-1);
	
	//  🖥️ Screen
	//------------------------------------------------------------
	// ck= &ckVRmode;  ck->Init("VRmode", &pSet->vr_mode);  // todo: meh

	//  💡 brightness
	float bright = 1.f, contrast = 1.f;
	sv= &svBright;		sv->Init("Bright",		&pSet->bright,   0.2,3.f);  sv->DefaultF(1.f);  SevC(CarLightBright);
	sv= &svContrast;	sv->Init("Contrast",	&pSet->contrast, 0.2,3.f);  sv->DefaultF(1.f);  SevC(CarLightBright);

	/*CmbC(cmb, "CmbAntiAliasing", cmbAntiAliasing);
	int si=0;
	if (cmb)
	{	cmb->removeAllItems();

		int a[6] = {0,1,2,4,8,16};
		for (int i=0; i < 6; ++i)
		{	int v = a[i];

			cmb->addItem(toStr(v));
			if (pSet->fsaa >= v)
				si = i;
		}
		cmb->setIndexSelected(si);
	}*/

	//  Render systems ..
	//------------------------------------------------------------
	#if 0
	CmbC(cmb, "CmbRendSys", comboRenderSystem);
	if (cmb)
	{	cmb->removeAllItems();

		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		const int nRS = 3;  //4
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem",
			"Direct3D9 Rendering Subsystem"/*, "Direct3D11 Rendering Subsystem"*/};
		#else
		const int nRS = 2;
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem"};
		#endif
			
		for (int i=0; i < nRS; ++i)
		{
			cmb->addItem(rs[i]);
			if (pSet->rendersystem == rs[i])
				cmb->setIndexSelected(cmb->findItemIndexWith(rs[i]));
		}
		//const RenderSystemList& rsl = Root::getSingleton().getAvailableRenderers();
		//for (int i=0; i < rsl.size(); ++i)
		//	combo->addItem(rsl[i]->getName());
	}
	#endif

	//  🪄 Effects
	//------------------------------------------------------------
	ck= &ckSSAO;			ck->Init("SSAO",		&pSet->g.ssao);  // 🕳️ SSAO
	sv= &svSsaoRadius;		sv->Init("SsaoRadius",	&pSet->ssao_radius, 0.1,6.f, 1.5f);  sv->DefaultF(1.8f);
	sv= &svSsaoScale;		sv->Init("SsaoScale",	&pSet->ssao_scale,  0.1,6.f, 1.5f);  sv->DefaultF(1.8f);

	ck= &ckHDR;				ck->Init("HDR",			&pSet->g.hdr);  // 🌅 HDR

	ck= &ckGI;				ck->Init("GI",			&pSet->gi);  // 🌄 GI
	txGIinfo = fTxt("GItext");
	BtnC("GInext", btnGInext);  BtnC("GIhq", btnGIhq);  BtnC("GIiso", btnGIiso);
	BtnC("GIvis", btnGIvis);  BtnC("GIvis2", btnGIvis2);
	BtnC("GIbncInc", btnGIbncInc);  BtnC("GIbncDec", btnGIbncDec);

	// ck= &ckAllEffects;	ck->Init("AllEffects",	&pSet->all_effects);  Cev(AllEffects);

	// ck= &ckBloom;		ck->Init("Bloom",		&pSet->bloom);  Cev(EffUpd);
	// sv= &svBloomInt;		sv->Init("BloomInt",	&pSet->bloom_int);   sv->DefaultF(0.13f);  Sev(EffUpd);
	// ck= &ckSoftPar;		ck->Init("SoftParticles",&pSet->softparticles);  Cev(EffUpdShd);


	//  🔉 Sound
	//------------------------------------------------------------
	sv= &svVolMaster;	sv->Init("VolMaster",	&pSet->s.vol_master, 0.f, 2.0f);  sv->DefaultF(1.55f);  SevC(VolMaster);
	sv= &svVolAmbient;	sv->Init("VolAmbient",	&pSet->s.vol_ambient, 0.f, 2.0f);  sv->DefaultF(1.f);

#ifndef SR_EDITOR  // game
	sv= &svVolEngine;	sv->Init("VolEngine",	&pSet->s.vol_engine, 0.f, 1.4f);  sv->DefaultF(0.58f);
	sv= &svVolTurbo;	sv->Init("VolTurbo",	&pSet->s.vol_turbo, 0.f, 1.6f);  sv->DefaultF(1.f);

	sv= &svVolTires;	sv->Init("VolTires",	&pSet->s.vol_tires,  0.f, 1.4f);  sv->DefaultF(0.856f);
	sv= &svVolSusp;		sv->Init("VolSusp",		&pSet->s.vol_susp,   0.f, 1.4f);  sv->DefaultF(0.474f);
	sv= &svVolEnv;		sv->Init("VolEnv",		&pSet->s.vol_env,    0.f, 1.4f);  sv->DefaultF(0.748f);

	sv= &svVolFlSplash;	sv->Init("VolFlSplash",	&pSet->s.vol_fl_splash, 0.f, 1.4f);  sv->DefaultF(0.636f);
	sv= &svVolFlCont;	sv->Init("VolFlCont",	&pSet->s.vol_fl_cont,   0.f, 1.4f);  sv->DefaultF(0.878f);
	sv= &svVolCarCrash;	sv->Init("VolCarCrash",	&pSet->s.vol_car_crash, 0.f, 1.4f);  sv->DefaultF(0.608f);
	sv= &svVolCarScrap;	sv->Init("VolCarScrap",	&pSet->s.vol_car_scrap, 0.f, 1.4f);  sv->DefaultF(0.915f);

	ck= &ckSndChk;		ck->Init("SndChk",		&pSet->s.snd_chk);
	ck= &ckSndChkWr;	ck->Init("SndChkWr",    &pSet->s.snd_chkwr);
#endif  // setup
	ck= &ckReverb;		ck->Init("ChkReverb",   &pSet->s.snd_reverb);
	sv= &svVolHud;		sv->Init("VolHud",		&pSet->s.vol_hud,    0.f, 2.f);  sv->DefaultF(0.75f);  SevC(VolHud);

	sv= &svSndBuf;      sv->Init("SndBuf",      &pSet->s.cnt_buffers, 256, 2048, 2); sv->DefaultI(1024);
	sv= &svSndSrc;      sv->Init("SndSrc",      &pSet->s.cnt_sources, 64, 1024, 2); sv->DefaultI(244);
	sv= &svSndSrcDyn;   sv->Init("SndSrcDyn",   &pSet->s.cnt_dynamics, 6, 256, 2);  sv->DefaultI(12);

	sv= &svSndRefDist;	sv->Init("SndRefDist", &pSet->s.ref_dist, 0.01f, 100.f, 2.f);  sv->DefaultF(1.f);
	sv= &svSndMaxDist;	sv->Init("SndMaxDist", &pSet->s.max_dist, 0.01f, 1500.f, 2.f);  sv->DefaultF(500.f);
	sv= &svSndRolloff;	sv->Init("SndRolloff", &pSet->s.rolloff,  0.00f, 1.f, 2.f);  sv->DefaultF(0.05f);


	//  ⚙️ Settings  🆕 Startup common 
	//------------------------------------------------------------
	// ck= &ckVRmode;  ck->Init("VRmode", &pSet->vr_mode);  // todo: meh

	ck= &ckStartInMain;	ck->Init("StartInMain", &pSet->startInMain);
	ck= &ckAutoStart;	ck->Init("AutoStart",   &pSet->autostart);
	ck= &ckEscQuits;	ck->Init("EscQuits",    &pSet->escquit);

	ck= &ckOgreDialog;	ck->Init("OgreDialog",  &pSet->ogre_dialog);
	ck= &ckMouseCapture;ck->Init("MouseCapture",&pSet->mouse_capture);
	//  hlms
	ck= &ckCacheHlms;		ck->Init("CacheHlms",      &pSet->cache_hlms);
	ck= &ckCacheShaders;	ck->Init("CacheShaders",   &pSet->cache_shaders);

 	ck= &ckDebugShaders;	ck->Init("DebugShaders",   &pSet->debug_shaders);
 	ck= &ckDebugProperties;	ck->Init("DebugProperties",&pSet->debug_properties);
}


//  📈 Fps  . . . . . . . . . . . . . . . . . . . . . . . . 
void CGuiCom::nextFps()
{
	++pSet->fps_bar;
	if (pSet->fps_bar > FPS_Modes)  pSet->fps_bar = 0;
	svFps.Upd();  slFps(0);
}

void CGuiCom::slFps(SV*)
{
	int f = max(0, min(FPS_Modes, pSet->fps_bar));
	app->txFps->setVisible(f > 0);
	app->bckFps->setVisible(f > 0);

	const static int dim[FPS_Modes+1][2] = {  // dim
		{ 16, 16},  // off
		{ 76, 28},  // fps
		{234, 28},  // 3
		{234, 28},  // 3
		{234, 78}   // full
	};
	app->bckFps->setSize(dim[f][0], dim[f][1]);

	const static string ss[FPS_Modes+1] = {  // info
		"#{None}",
		"#{FPS_Fps}",
		"#{FPS_Fps},  #{FPS_Triangles}, #{FPS_Draws}",
		"#{FPS_Fps},  #{FPS_Draws},  #{FPS_Memory} MB",
		"#{FPS_Fps},  #{FPS_Triangles},  #{FPS_Draws}\n"
		"   i #{FPS_Instances},  #{FPS_Vegetation}\n"
		"  G GUI,  w #{FPS_Workspaces},  #{FPS_Memory} MB",
	};
	txFpsInfo->setCaption(TR(ss[f]));
}


//  events
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
//  🌄 GI
void CGuiCom::btnGInext(WP)
{
	app->GInextMethod(1);
	app->GIupdText();
}
void CGuiCom::btnGIhq(WP)
{
	app->GItoggletVctQuality();
	app->GIupdText();
}
void CGuiCom::btnGIvis(WP)
{
	app->GInextVctVis(1);
	app->GIupdText();
}
void CGuiCom::btnGIvis2(WP)
{
	app->GInextIfdProbeVis(1);
	app->GIupdText();
}
void CGuiCom::btnGIbncInc(WP)
{
	++app->iVctBounces;
	if (app->mVCT)
		app->mVCT->update( app->mSceneMgr, app->iVctBounces, app->fVctThinWall );
	if (app->mIFD)
		app->mIFD->reset();
	app->GIupdText();
}
void CGuiCom::btnGIbncDec(WP)
{
	if (app->iVctBounces > 0)
		--app->iVctBounces;
	if (app->mVCT)
		app->mVCT->update( app->mSceneMgr, app->iVctBounces, app->fVctThinWall );
	if (app->mIFD)
		app->mIFD->reset();
	app->GIupdText();
}
void CGuiCom::btnGIiso(WP)
{
	if (app->mVCT)
	{	app->mVCT->setAnisotropic( !app->mVCT->isAnisotropic() );
		app->mVCT->update( app->mSceneMgr, app->iVctBounces, app->fVctThinWall );
	}app->GIupdText();
}
	/*  more -
	auto giMode = getGiMode();
	if (giMode == IfdVct || giMode == IfdOnly)
	{
		mUseRasterIrradianceField = !mUseRasterIrradianceField;
		voxelizeScene();
	}*/
//-----------------------------------------------------------------------------------


//  Tex
void CGuiCom::cmbTexFilter(CMB)
{
	pSet->g.tex_filt = val;
	app->SetAnisotropy();
}

void CGuiCom::slAnisotropy(SV*)
{
	app->SetAnisotropy();
}

//  Lod
void CGuiCom::slViewDist(SV*)
{
	app->scn->UpdSkyScale();
	for (auto& c : app->mCams)
		c.cam->setFarClipDistance(pSet->g.view_distance);
}

void CGuiCom::slTerLod(SV*)
{
	app->scn->updTerLod();
}

void CGuiCom::slLodBias(SV*)
{
	for (auto& c : app->mCams)
		c.cam->setLodBias(pSet->g.lod_bias);
}

//  🌳🪨🌿 veget
void CGuiCom::btnVegetReset(WP)
{
	svTrees.SetValueF(1.5f);
	svGrass.SetValueF(1.f);
	svTreesDist.SetValueF(1.f);
	svGrassDist.SetValueF(2.f);
}

void CGuiCom::btnVegetApply(WP)
{
#ifndef SR_EDITOR  // not in multi..
	pSet->game.trees = pSet->gui.trees;
#else  // ed
	if (!pSet->bTrees)  return;
#endif
	if (!app->scn->imgRoadSize)
		app->scn->LoadRoadDens();
	app->scn->DestroyVegets();
	app->scn->CreateVegets();
	
	app->scn->grass->Destroy();
	app->scn->grass->Create(app);
}


//  🌒 shadows
void CGuiCom::btnShadowsApply(WP)
{
	app->SetupCompositors();  //
	// app->setupShadowCompositor();  // fixme crash ws def..
	app->updShadowFilter();  // works
}

void CGuiCom::slWaterDist(SV*)
{
	// planar refl dist?..
	app->scn->UpdSkyScale();
}

//  🔮 reflection
void CGuiCom::slReflDist(SV*)
{
	app->mCubeCamera->setFarClipDistance( pSet->g.refl_dist );
	app->scn->UpdSkyScale();
}

/*void CGuiCom::slReflMode(SV* sv)
{
}*/
void CGuiCom::slReflIbl(SV* sv)
{
}
void CGuiCom::slReflLod(SV* sv)
{
	app->mCubeCamera->setLodBias( pSet->g.refl_lod );
}

void CGuiCom::btnReflApply(WP)
{
	// app->CreateCubeReflect();
	app->SetupCompositors();  //
}


//  🌊 water  todo
void CGuiCom::chkWater(Ck*)
{
	// setReflect(pSet->water_reflect);
	// setRefract(pSet->water_refract);
	// recreate();
}

void CGuiCom::slWaterSize(SV*)
{
	// setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	// recreate();
}

void CGuiCom::btnWaterApply(WP)
{
	//app->bRecreateFluidsRTT = 1;  // fixme: crash-
	// app->SetupCompositor();  //
}

void CGuiCom::slCarLightBright(SV*)
{
	//UpdSun(0.f);  // not needed
	// upd veh lights..
	#ifndef SR_EDITOR
	app->updCarLightsBright();
	#endif
}


//  mat ed
void CGuiCom::btnMatEditor(WP)
{
#ifdef SR_EDITOR
	app->gui->GuiShortcut(WND_Materials, 1);
#else
	app->gui->GuiShortcut(MN_Materials, 1);
#endif
}

//  🔊 Sound
void CGuiCom::slVolMaster(SV*)
{
#ifndef SR_EDITOR
	app->pGame->ProcessNewSettings();
#else
	// todo ed snd..
#endif
}

void CGuiCom::slVolHud(SV*)
{
#ifndef SR_EDITOR
	app->pGame->UpdHudSndVol();
#else
	// todo ed snd..
#endif
}
