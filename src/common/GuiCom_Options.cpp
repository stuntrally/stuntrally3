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
	// #include "SplitScreen.h"
#endif
#include "Slider.h"
// #include "WaterRTT.h"
// #include "Terra.h"
#include <OgreCamera.h>
// #include <OgreMaterialManager.h>
// #include <OgreSceneNode.h>
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
	
	//  🧊 Detail, far geometry
	sv= &svLodBias;		sv->Init("LodBias",		&pSet->lod_bias,	0.f,4.f, 1.5f);  SevC(LodBias);  sv->DefaultF(1.f);
	sv= &svTerDetail;	sv->Init("TerDetail",	&pSet->ter_detail,	0.f,4.f, 1.5f);  SevC(TerDetail);  sv->DefaultF(1.f);
	sv= &svRoadDist;	sv->Init("RoadDist",	&pSet->road_dist,	0.f,4.f, 2.f, 2,5);  sv->DefaultF(1.6f);
	sv= &svViewDist;	sv->Init("ViewDist",	&pSet->view_distance, 50.f,20000.f, 2.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																				SevC(ViewDist);  sv->DefaultF(8000.f);

	//  🖼️ Textures filtering
	CmbC(cmb, "TexFiltering", comboTexFilter);
	cmb->removeAllItems();
	cmb->addItem(TR("#{Bilinear}"));     cmb->addItem(TR("#{Trilinear}"));
	cmb->addItem(TR("#{Anisotropic}"));  cmb->addItem(TR("#{Anisotropic} #{RplAll}"));
	cmb->setIndexSelected(pSet->tex_filt);  comboTexFilter(cmb, pSet->tex_filt);

	sv= &svAnisotropy;	sv->Init("Anisotropy",	&pSet->anisotropy,	0,16);		SevC(Anisotropy);  sv->DefaultI(4);
	// BtnC("ApplyShaders", btnShaders);

	//  ⛰️ Terrain
	/*sv= &svTerMtr;
		sv->strMap[0] = TR("#{GraphicsAll_Lowest}");	sv->strMap[1] = TR("#{GraphicsAll_Medium}");
		sv->strMap[2] = TR("#{GraphicsAll_High}");		//sv->strMap[3] = "Parallax-";
						sv->Init("TerMtr",		&pSet->ter_mtr,		0,2);	sv->DefaultI(2);*/
	/*sv= &svTerTripl;
		sv->strMap[0] = TR("#{None}");  sv->strMap[1] = TR("#{max} 2");  sv->strMap[2] = TR("#{Any}");
						sv->Init("TerTripl",	&pSet->ter_tripl,	0,2);	sv->DefaultF(1);*/

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

	//  🔮 Reflection  // todo:
	sv= &svReflSkip;	sv->Init("ReflSkip",	&pSet->refl_skip,    0,1000, 2.f);  sv->DefaultI(0);
	sv= &svReflFaces;	sv->Init("ReflFaces",	&pSet->refl_faces,   1,6);  sv->DefaultI(1);
	sv= &svReflSize;
		for (i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);
						sv->Init("ReflSize",	&pSet->refl_size,    0,NumTexSizes-1);  sv->DefaultI(1);

	sv= &svReflDist;	sv->Init("ReflDist",	&pSet->refl_dist,	20.f,1500.f, 2.f, 0,4, 1.f," m");
																	SevC(ReflDist);  sv->DefaultF(300.f);
	sv= &svReflMode;
	sv->strMap[0] = TR("#{ReflMode_static}");  sv->strMap[1] = TR("#{ReflMode_single}");
	sv->strMap[2] = TR("#{ReflMode_full}");
	sv->Init("ReflMode",	&pSet->refl_mode,   0,2);  SevC(ReflMode);  sv->DefaultI(1);

	//  🌊 Water  // todo:
	// ck= &ckWaterReflect; ck->Init("WaterReflection", &pSet->water_reflect);  CevC(Water);
	// ck= &ckWaterRefract; ck->Init("WaterRefraction", &pSet->water_refract);  CevC(Water);
	// sv= &svWaterSize;
	// 	for (int i=0; i < NumTexSizes; ++i)  sv->strMap[i] = toStr(cTexSizes[i]);
	// 					sv->Init("WaterSize",	&pSet->water_rttsize, 0,NumTexSizes-1;  sv->DefaultI(0);  SevC(WaterSize);
	// BtnC("ApplyShadersWater", btnShaders);


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
	ck= &ckVRmode;  ck->Init("VRmode", &pSet->vr_mode);

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
	// todo:
	// app->mTerra->iLodMax = pSet->terdetail
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
	app->scn->grass->Create();
}


//  🌒 shadows
void CGuiCom::btnShadowsApply(WP)
{
	// app->scn->changeShadows();  // todo ..
}

void CGuiCom::btnShadersApply(WP)
{
	//app->scn->changeShadows();  //?
}

//  🔮 reflection
void CGuiCom::slReflDist(SV*)
{
	// app->recreateReflections();  // todo ..
}
void CGuiCom::slReflMode(SV* sv)
{
	// if (app->gui->bGI)
	/*switch (pSet->refl_mode)
	{
		case 0: sv->setTextClr(0.0, 1.0, 0.0);  break;
		case 1: sv->setTextClr(1.0, 0.5, 0.0);  break;
		case 2: sv->setTextClr(1.0, 0.0, 0.0);  break;
	}*/
	// app->recreateReflections();
}


//  🌊 water
void CGuiCom::chkWater(Ck*)
{
	// app->scn->mWaterRTT->setReflect(pSet->water_reflect);
	// app->scn->mWaterRTT->setRefract(pSet->water_refract);
	// app->scn->changeShadows();
	// app->scn->mWaterRTT->recreate();
}

void CGuiCom::slWaterSize(SV*)
{
	// app->scn->mWaterRTT->setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	// app->scn->mWaterRTT->recreate();
}
