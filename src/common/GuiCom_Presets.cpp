#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "settings_com.h"
#include "CGui.h"
#ifndef SR_EDITOR
	#include "game.h"
	#include "CGame.h"
	// #include "SplitScreen.h"
#else
	#include "CApp.h"
	#include "settings.h"
#endif
#include "Road.h"

#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <MyGUI.h>
#include "Slider.h"
using namespace MyGUI;
using namespace Ogre;


///  set simulation settings for preset
///.........................................................
#ifndef SR_EDITOR  ///  Game only
void CGuiCom::UpdSimQuality()
{
	auto& s = *pSet;
	switch (s.g.sim_quality)
	{
	case 0:  s.game_fq =  80.f;  s.blt_fq =  80.f;  s.blt_iter = 12;  s.dyn_iter = 30;  break;  // lowest
	case 1:  s.game_fq = 120.f;  s.blt_fq = 120.f;  s.blt_iter = 18;  s.dyn_iter = 45;  break;
	case 2:  s.game_fq = 160.f;  s.blt_fq = 160.f;  s.blt_iter = 24;  s.dyn_iter = 60;  break;  // normal
	case 3:  s.game_fq = 320.f;  s.blt_fq = 320.f;  s.blt_iter = 24;  s.dyn_iter = 60;  break;
	}
	String t;  // sim info txt
	t += "Game frequency:  "+fToStr(s.game_fq,0)+" Hz\n";
	t += "Bullet frequency:  "+fToStr(s.blt_fq,0)+" Hz\n";
	t += " Bullet iterations:  "+iToStr(s.blt_iter)+"\n";
	t += " Car Dynamics iterations:  "+iToStr(s.dyn_iter)+"\n";
	t += "Car Dynamics frequency:  "+fToStr(s.blt_fq * s.dyn_iter,0)+" Hz\n";
	app->gui->txSimDetail->setCaption(t);
}
#endif

///  change all Graphics settings
///..............................................................................................................................
void CGuiCom::cmbGraphicsPreset(CMB)
{
const static SETTINGS::Lights lights[8] = {  /* 💡 Lights only
	front     boost  thrust
	|shdw   undr |  / collect
	|  |  rev |  | |  | track
	|  | brk| |  | |  | |  grid */
	{0,0, 0,0,0, 0,0, 0,0, 0 },  // Lowest
	{0,0, 0,0,0, 0,0, 0,0, 0 },  // Low 
	{1,0, 0,0,0, 0,0, 0,0, 0 },  // Medium ~
	{1,1, 0,0,0, 1,0, 0,1, 0 },  // High
	{1,1, 1,0,0, 1,1, 0,1, 1 },  // Higher *
	{1,1, 1,1,1, 1,1, 1,1, 2 },  // Very high
	{1,1, 1,1,1, 1,1, 1,1, 2 },  // Highest
	{1,1, 1,1,1, 1,1, 1,1, 3 },  // Ultra
};
const static SETTINGS::Veget veget[8] = {  /* 🌳🪨🪴🌿 Veget only
 	🌳Trees  mul                    Horiz mul           
 	|          🪴Bushes             🌳Trees   🪴Bushes  
	|     dist |    dist 🌿Grass    |    dist |    dist 
	|     |    |    |    |    dist  |    |    |    |    */
	{0.0f,0.5f,0.0f,0.5f,0.0f,0.5f, 0.0f,0.0f,0.0f,0.0f,   000 },  // Lowest
	{0.5f,0.5f,0.0f,0.5f,0.0f,0.5f, 0.0f,0.0f,0.0f,0.0f,  1000 },  // Low 
	{1.0f,0.7f,0.5f,0.7f,0.5f,0.7f, 0.0f,0.0f,0.0f,0.0f,  5000 },  // Medium ~
	{1.0f,1.0f,1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f,0.0f,0.0f, 10000 },  // High
	{1.5f,1.2f,1.0f,1.4f,1.0f,1.4f, 0.5f,0.0f,0.0f,0.0f, 20000 },  // Higher *
	{1.5f,1.5f,1.5f,1.7f,1.5f,1.7f, 0.6f,0.0f,0.5f,0.1f, 30000 },  // Very high
	{2.0f,2.0f,2.0f,2.0f,2.0f,2.0f, 0.8f,0.5f,0.5f,0.5f, 40000 },  // Highest
	{2.5f,3.0f,2.5f,3.0f,2.5f,3.0f, 1.0f,1.0f,1.0f,1.0f, 60000 },  // Ultra
};
const static SETTINGS::Detail presets[8] = {  /*
	   🖼️           LOD      ⛰️ Terain  🌒 Shadow     🌊 Water          🔮 Reflect cube     
Sim	   Anisot       Obj,Veg  Tripl,hor   Size    filt  Size              Size              
Qlty   | Tex        |        | LOD,hor   | Cnt Dist |  | Reflect         | Skip            
	\  \ fil Visibl |  Road  | Horiz |   | | Typ |  |  | | refra         | | Faces     IBL 
	 |  | |   Dist  |   |🛣️  |\  |  /|   | | |   |  |  | | |  Dist LOD   | | |  Dist LOD | */
	{0, 0,1,  5000, 0.2,0.9, 0,0,0,0,0,  0,1,0, 100,0, 0,0,0,  100,0.1,  0,600,0,100,0.1,0 },  // Lowest
	{1, 2,2, 10000, 0.6,1.5, 0,0,0,1,0,  1,3,1, 200,1, 0,0,0,  400,0.2,  0,60,1, 300,0.2,0 },  // Low 
	{1, 4,2, 20000, 1.0,2.0, 0,0,1,1,1,  2,3,1, 300,2, 1,1,0,  700,0.3,  1,4,1,  500,0.3,0 },  // Medium ~
	{2, 4,2, 40000, 1.2,2.5, 1,0,1,2,2,  3,3,1, 500,2, 1,1,1, 1000,0.4,  1,2,1,  700,0.4,0 },  // High
	{2, 4,3, 60000, 1.5,3.0, 1,0,2,2,2,  3,3,1, 700,3, 1,1,1, 3000,0.4,  2,0,1, 1000,0.4,0 },  // Higher *
	{2, 8,3, 60000, 1.8,3.5, 2,1,3,3,3,  3,3,1,1000,3, 2,1,1, 6000,0.6,  2,0,2, 2000,0.6,2 },  // Very high
	{2, 16,3,60000, 2.4,4.0, 2,2,3,3,3,  4,3,1,1300,4, 3,1,1,20000,0.8,  3,0,3,20000,0.8,2 },  // Highest
	{3, 16,3,60000, 2.4,4.0, 2,2,3,4,4,  4,3,1,2000,4, 4,1,1,60000,1.0,  3,0,4,60000,1.0,4 },  // Ultra
};

	pSet->preset = val;  // for info
	SETTINGS& s = *pSet;
	s.g = presets[val];  // set all
	s.li = lights[val];
	s.veg = veget[val];
	s.gui.trees = s.veg.trees;
	s.gui.bushes = s.veg.bushes;  //?-

#ifndef SR_EDITOR  ///  Game only
	UpdSimQuality();

	s.particles = val >= 1;  s.trails = val >= 1;
	
	s.rpl_rec   = val >= 1;  //? rpl-
	s.rpl_ghost = val >= 1;
	
#else  ///  Editor only  todo: skip
	switch (val)
	{
	case 0:  // Lowest
		s.trackmap = 0;  s.brush_prv = 0;  s.ter_skip = 20; s.road_skip = 12;  s.mini_skip = 20;  break;
	case 1:  // Low
		s.trackmap = 1;  s.brush_prv = 0;  s.ter_skip = 10; s.road_skip = 12;  s.mini_skip = 20;  break;
	case 2:  // Medium
		s.trackmap = 1;  s.brush_prv = 1;  s.ter_skip = 3;  s.road_skip = 12;  s.mini_skip = 6;  break;
	case 3:  // High
		s.trackmap = 1;  s.brush_prv = 1;  s.ter_skip = 2;  s.road_skip = 12;  s.mini_skip = 4;  break;
	case 4:  // Higher
		s.trackmap = 1;  s.brush_prv = 1;  s.ter_skip = 2;  s.road_skip = 12;  s.mini_skip = 2;  break;
	case 5:  // Very High
		s.trackmap = 1;  s.brush_prv = 1;  s.ter_skip = 1;  s.road_skip = 12;  s.mini_skip = 2;  break;
	case 6:  // Highest
		s.trackmap = 1;  s.brush_prv = 1;  s.ter_skip = 1;  s.road_skip = 12;  s.mini_skip = 1;  break;
	case 7:  // Ultra
		s.trackmap = 1;  s.brush_prv = 1;  s.ter_skip = 1;  s.road_skip = 12;  s.mini_skip = 1;  break;
	}
#endif

	//  🔁 Update Gui  cmb,sld,val,chk
	//----  same as SETcom struct Detail
	//-----------------------------------------------------------------
	//  🖼️ textures
	svAnisotropy.Upd();
	cbTexFilter->setIndexSelected(pSet->g.tex_filt);
	//  geom detail  📦🪨 🛣️
	svViewDist.Upd();
	svLodBias.Upd();  svRoadDist.Upd();

	//  ⛰️ terrain
	svTerTripl.Upd();  svTriplHoriz.Upd();
	svHorizons.Upd();
	svTerLod.Upd();  svHorizLod.Upd();
	
	//  🌳🪨🌿 veget
	svTrees.Upd();  svTreesDist.Upd();
	svGrass.Upd();	svGrassDist.Upd();  //btnVegetApply(0);

	//  🌒 shadow
	svShadowSize.Upd();  svShadowCount.Upd();  svShadowType.Upd();
	svShadowDist.Upd();  svShadowFilter.Upd();  //btnShadowsApply(0);
	//  💡 lights
#ifndef SR_EDITOR
	ckLiFront.Upd();  ckLiFrontShadows.Upd();
	ckLiBrake.Upd();  ckLiRevese.Upd();
	ckLiBoost.Upd();  ckLiThrust.Upd();
#endif
	ckLiCollect.Upd();  ckLiTrack.Upd();

	//  🌊 water
	svWaterSize.Upd();  //svWaterSkip.Upd();
	ckWaterReflect.Upd();  ckWaterRefract.Upd();
	svWaterDist.Upd();  svWaterLod.Upd();  //btnWaterApply(0);

	//  🔮 reflection
	svReflSize.Upd();  svReflSkip.Upd();  svReflFaces.Upd();
	svReflDist.Upd();  svReflLod.Upd();  //svReflMode.Upd();
	svReflIbl.Upd();   // btnReflApply(0);


	auto* g = app->gui;
#ifndef SR_EDITOR   ///  game only
	g->svSimQuality.Upd();
	UpdSimQuality();

	g->ckParticles.Upd();  g->ckTrails.Upd();
	g->svParticles.Upd();  g->svTrails.Upd();

	//ckAllEffects.Upd();
	
	g->ckRplAutoRec.Upd();  // rpl-
	g->ckRplGhost.Upd();
#else
	///  editor only
	g->ckMinimap.Upd();  g->svMiniUpd.Upd();
	g->svTerUpd.Upd();  g->svRoadUpd.Upd();
#endif

	// Apply  meh todo
	btnVegetApply(0);
	// chkWater(0);
	//^app->changeShadows();  // apply shadow, material factory generate
}
