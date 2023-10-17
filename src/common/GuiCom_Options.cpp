#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "App.h"
#include "Cam.h"
#include "Road.h"
#include "Grass.h"
#ifndef SR_EDITOR
	#include "game.h"
#endif
#include "CGui.h"
#include "Slider.h"
#include "Terra.h"
#include <OgreCamera.h>
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
	txFpsInfo = fTxt("FpsInfo");
	slFps(0);

	//  🧊 Detail, far geometry
	sv= &svLodBias;		sv->Init("LodBias",		&pSet->lod_bias,	0.f,4.f, 1.5f);  SevC(LodBias);  sv->DefaultF(1.f);
	sv= &svTerDetail;	sv->Init("TerDetail",	&pSet->ter_detail,	0,5);  SevC(TerDetail);  sv->DefaultI(2);
	sv= &svRoadDist;	sv->Init("RoadDist",	&pSet->road_dist,	0.f,5.f, 2.f, 2,5);  sv->DefaultF(1.6f);
	sv= &svViewDist;	sv->Init("ViewDist",	&pSet->view_distance, 50.f,60000.f, 2.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																			SevC(ViewDist);  sv->DefaultF(8000.f);
	sv= &svHorizons;	sv->Init("Horizons",	&pSet->horizons,	0,3);  sv->DefaultI(3);


	//  🖼️ Textures filtering
	CmbC(cmb, "TexFiltering", comboTexFilter);
	cmb->removeAllItems();
	cmb->addItem(TR("#{Bilinear}"));     cmb->addItem(TR("#{Trilinear}"));
	cmb->addItem(TR("#{Anisotropic}"));  cmb->addItem(TR("#{Anisotropic} #{RplAll}"));
	cmb->setIndexSelected(pSet->tex_filt);  comboTexFilter(cmb, pSet->tex_filt);

	sv= &svAnisotropy;	sv->Init("Anisotropy",	&pSet->anisotropy,	0,16);		SevC(Anisotropy);  sv->DefaultI(4);
	// BtnC("ApplyShaders", btnShaders);

	//  ⛰️ Terrain
	/*sv= &svTerMtr;  // drop-
		sv->strMap[0] = TR("#{GraphicsAll_Lowest}");	sv->strMap[1] = TR("#{GraphicsAll_Medium}");
		sv->strMap[2] = TR("#{GraphicsAll_High}");		//sv->strMap[3] = "Parallax-";
						sv->Init("TerMtr",		&pSet->ter_mtr,		0,2);	sv->DefaultI(2);*/
	auto triplMap = [&](){  //sv->strMap[1] = TR("#{max} 2");  sv->strMap[2] = TR("#{Any}");
		sv->strMap[0] = TR("0 #{None}");		sv->strMap[1] = TR("1 #{Diffuse}");
		sv->strMap[2] = TR("2 #{Specular}");	sv->strMap[3] = TR("3 #{Normal}");  };
	sv= &svTerTripl;	triplMap();
						sv->Init("TerTripl",	&pSet->ter_tripl,	0,3);	sv->DefaultI(1);
	sv= &svTriplHoriz;	sv->Init("TriplHoriz",	&pSet->tripl_horiz,	0,3);	sv->DefaultI(0);


	//  🌳🪨 Veget  🌿 grass
	sv= &svTrees;		sv->Init("Trees",		&pSet->gui.trees,	0.f, 8.f, 2.f);  sv->DefaultF(1.5f);
	sv= &svGrass;		sv->Init("Grass",		&pSet->grass,		0.f, 8.f, 2.f);  sv->DefaultF(1.f);
	sv= &svTreesDist;	sv->Init("TreesDist",   &pSet->trees_dist,	0.1f,8.f, 2.f);  sv->DefaultF(1.f);
	sv= &svGrassDist;	sv->Init("GrassDist",   &pSet->grass_dist,	0.1f,8.f, 2.f);  sv->DefaultF(2.f);
	BtnC("ApplyVeget",  btnVegetApply);   BtnC("VegetReset",  btnVegetReset);


	//  🌒 Shadows
	sv= &svShadowType;
		sv->strMap[0] = TR("#{None}");	sv->strMap[1] = "Depth";	sv->strMap[2] = "Soft-";
						sv->Init("ShadowType",	&pSet->shadow_type,  0,2);  sv->DefaultI(1);
	sv= &svShadowCount;	sv->Init("ShadowCount",	&pSet->shadow_count, 1,4);  sv->DefaultI(3);
	sv= &svShadowSize;
		for (i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);
						sv->Init("ShadowSize",	&pSet->shadow_size, 0,NumTexSizes-1);   sv->DefaultI(3);
	sv= &svShadowDist;	sv->Init("ShadowDist",	&pSet->shadow_dist, 20.f,5000.f, 3.f, 0,3, 1.f," m");  sv->DefaultF(1300.f);  // todo:
	BtnC("ApplyShadows", btnShadowsApply);

	//  💡 Lights
	ck= &ckCarLights;        ck->Init("CarLights",          &pSet->car_lights);  //CevC(Lights);
	ck= &ckCarLightsShadows; ck->Init("CarLightsShadows",   &pSet->car_light_shadows);


	//  🔮 Reflection
	sv= &svReflSkip;	sv->Init("ReflSkip",	&pSet->refl_skip,    0,1000, 2.f);  sv->DefaultI(0);
	sv= &svReflFaces;	sv->Init("ReflFaces",	&pSet->refl_faces,   1,6);  sv->DefaultI(1);
	sv= &svReflSize;
		for (i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);  //v way too big
						sv->Init("ReflSize",	&pSet->refl_size,    0,NumTexSizes-1 -1, 2.f);  sv->DefaultI(0);

	sv= &svReflDist;	sv->Init("ReflDist",	&pSet->refl_dist,	20.f,60000.f, 2.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																	SevC(ReflDist);  sv->DefaultF(1000.f);
	sv= &svReflIbl;		sv->Init("ReflIbl",		&pSet->refl_ibl,	0,8, 2.f);  SevC(ReflIbl);  sv->DefaultI(5);
	sv= &svReflLod;		sv->Init("ReflLod",		&pSet->refl_lod,	0.f,4.f, 1.5f);  SevC(ReflLod);  sv->DefaultF(0.4f);
	BtnC("ApplyRefl", btnReflApply);
	/*sv= &svReflMode;  // todo: 1 cube refl for each car..
	sv->strMap[0] = TR("#{ReflMode_static}");  sv->strMap[1] = TR("#{ReflMode_single}");
	sv->strMap[2] = TR("#{ReflMode_full}");
	sv->Init("ReflMode",	&pSet->refl_mode,   0,2);  SevC(ReflMode);  sv->DefaultI(1);*/

	//  🌊 Water  // todo:
	ck= &ckWaterReflect; ck->Init("WaterReflection", &pSet->water_reflect);  CevC(Water);
	ck= &ckWaterRefract; ck->Init("WaterRefraction", &pSet->water_refract);  CevC(Water);
	sv= &svWaterSize;
		for (int i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);
						sv->Init("WaterSize",	&pSet->water_size, 0,NumTexSizes-1);  sv->DefaultI(0);  SevC(WaterSize);

	sv= &svWaterSkip;	sv->Init("WaterSkip",	&pSet->water_skip,	0,20, 2.f);  sv->DefaultI(0);
	sv= &svWaterDist;	sv->Init("WaterDist",	&pSet->water_dist,	20.f,60000.f, 4.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																	SevC(WaterDist);  sv->DefaultF(1000.f);
	sv= &svWaterLod;	sv->Init("WaterLod",	&pSet->water_lod,	0.f,2.f, 2.f);  sv->DefaultF(0.4f);
	BtnC("ApplyShadersWater", btnWaterApply);


	//  Presets
	CmbC(cmb, "CmbGraphicsAll", comboGraphicsAll);
	if (cmb)
	{	cmb->removeAllItems();
		cmb->addItem(TR("#{GraphicsAll_Lowest}"));  cmb->addItem(TR("#{GraphicsAll_Low}"));
		cmb->addItem(TR("#{GraphicsAll_Medium}"));  cmb->addItem(TR("#{GraphicsAll_High}"));
		cmb->addItem(TR("#{GraphicsAll_Higher}"));  cmb->addItem(TR("#{GraphicsAll_VeryHigh}"));
		cmb->addItem(TR("#{GraphicsAll_Highest}")); cmb->addItem(TR("#{GraphicsAll_Ultra}"));
		cmb->setIndexSelected(pSet->preset);
	}
	
	//  Video  // todo?
	// ck= &ckLimitFps;   ck->Init("LimitFpsOn", &pSet->limit_fps);
	// sv= &svLimitFps;   sv->Init("LimitFps",   &pSet->limit_fps_val,	20.f,144.f);  sv->DefaultF(60.f);
	// sv= &svLimitSleep; sv->Init("LimitSleep", &pSet->limit_sleep,  -2,20, 1.5f);  sv->DefaultI(-1);
	
	//  🖥️ Screen
	// ck= &ckVRmode;  ck->Init("VRmode", &pSet->vr_mode);  // todo:

	//  💡 brightness
	float bright = 1.f, contrast = 1.f;
	sv= &svBright;		sv->Init("Bright",		&pSet->bright,   0.2,3.f);  sv->DefaultF(1.f);
	sv= &svContrast;	sv->Init("Contrast",	&pSet->contrast, 0.2,3.f);  sv->DefaultF(1.f);

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
}


//  Fps . . . . . . . . . . . . . . . . . . . . . . . . 
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


//  events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void CGuiCom::comboTexFilter(CMB)
{
	pSet->tex_filt = val;
}

void CGuiCom::slAnisotropy(SV*)
{
	// todo: ugh
}

void CGuiCom::slViewDist(SV*)
{
	app->scn->UpdSkyScale();
	for (auto& c : app->mCams)
		c.cam->setFarClipDistance(pSet->view_distance);
}

void CGuiCom::slTerDetail(SV*)
{
	app->scn->updTerLod();
}

void CGuiCom::slLodBias(SV*)
{
	for (auto& c : app->mCams)
		c.cam->setLodBias(pSet->lod_bias);
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
	app->scn->DestroyTrees();
	app->scn->CreateTrees();
	
	app->scn->grass->Destroy();
	app->scn->grass->Create(app);
}


//  🌒 shadows
void CGuiCom::btnShadowsApply(WP)
{
	// app->scn->changeShadows();  // todo ..
}

void CGuiCom::btnShadersApply(WP)
{
}

void CGuiCom::slWaterDist(SV*)
{
	// planar refl dist?..
	app->scn->UpdSkyScale();
}

//  🔮 reflection
void CGuiCom::slReflDist(SV*)
{
	app->mCubeCamera->setFarClipDistance( pSet->refl_dist );
	app->scn->UpdSkyScale();
}

void CGuiCom::slReflMode(SV* sv)
{
}
void CGuiCom::slReflIbl(SV* sv)
{
}
void CGuiCom::slReflLod(SV* sv)
{
	app->mCubeCamera->setLodBias( pSet->refl_lod );
}

void CGuiCom::btnReflApply(WP)
{
	app->CreateCubeReflect();
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
}


void CGuiCom::btnMatEditor(WP)
{
#ifdef SR_EDITOR
	app->gui->GuiShortcut(WND_Materials, 1);
#else
	app->gui->GuiShortcut(MN_Materials, 1);
#endif
}
