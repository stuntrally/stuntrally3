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


///  change all Graphics settings
///..............................................................................................................................
void CGuiCom::cmbGraphicsPreset(CMB)
{
	const static SETTINGS::Detail presets[8] = {  /*
🖼️           LOD      ⛰️ Terain         Veget        🌒 Shadow     💡   🌊 Water          🔮 Reflect cube     
Anisot       Obj,Veg  Tripl,hor  🌳Trees mul          Size    filt  Veh  Size              Size              
| Tex        |        | LOD,hor  | 🌿Grass    Dist    | Cnt Dist | Light | Reflect         | Skip            
| fil Visibl |  Road  | Horiz |  |    |  trees grass  | | Typ |  |  |shd | | refra         | | Faces     IBL 
|  |   Dist  |   |🛣️  |\  |  /|  |    |    |    |     | | |   |  |  | |  | | |  Dist LOD   | | |  Dist LOD | */
{0,1,  5000, 0.2,0.9, 0,0,0,0,0, 0.0f,0.0f,0.5f,0.5f, 0,1,0, 100,0, 0,0, 0,0,0,  100,0.1,  0,600,0,100,0.1,0 },  // Lowest
{2,2, 10000, 0.6,1.5, 0,0,0,1,0, 0.5f,0.0f,0.5f,0.5f, 1,3,1, 200,1, 0,0, 0,0,0,  400,0.2,  0,60,1, 300,0.2,0 },  // Low 

{4,2, 20000, 1.0,2.0, 0,0,1,1,1, 1.0f,0.5f,0.7f,0.7f, 2,3,1, 300,2, 0,0, 1,1,0,  700,0.3,  1,4,1,  500,0.3,0 },  // Medium
{4,2, 40000, 1.2,2.5, 1,0,1,2,2, 1.0f,1.0f,1.0f,1.0f, 3,3,1, 500,2, 0,0, 1,1,1, 1000,0.4,  1,2,1,  700,0.4,0 },  // High
{4,3, 60000, 1.5,3.0, 1,0,2,2,2, 1.5f,1.0f,1.2f,1.4f, 3,3,1, 700,3, 0,0, 1,1,1, 3000,0.4,  2,0,1, 1000,0.4,0 },  // Higher *

{8,3, 60000, 1.8,3.5, 2,1,3,3,3, 1.5f,1.5f,1.5f,1.7f, 3,3,1,1000,3, 0,0, 2,1,1, 6000,0.6,  2,0,2, 2000,0.6,2 },  // Very high
{16,3,60000, 2.4,4.0, 2,2,3,3,3, 2.0f,2.0f,2.0f,2.0f, 4,3,1,1300,4, 0,0, 3,1,1,20000,0.8,  3,0,3,20000,0.8,2 },  // Highest
{16,3,60000, 2.4,4.0, 2,2,3,4,4, 2.5f,2.5f,3.0f,3.0f, 4,3,1,2000,4, 0,0, 4,1,1,60000,1.0,  3,0,4,60000,1.0,4 },  // Ultra
	};

	pSet->preset = val;  // for info
	SETTINGS& s = *pSet;
	s.g = presets[val];  // set all
	s.gui.trees = s.g.trees;

#ifndef SR_EDITOR  ///  Game only
	//game_fq = 100.f;  blt_fq = 60.f;  blt_iter = 7;  dyn_iter = 10;  mult_thr = 0;  //sim low
	//veget_collis = true;  car_collis = false;

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
	ckCarLights.Upd();  ckCarLightsShadows.Upd();

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
