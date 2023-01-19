#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "CData.h"
#include "PresetsXml.h"
#include "Road.h"
#include <fstream>
#include "Gui_Def.h"
#include "Slider.h"
#include "MultiList2.h"
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  🌈 used value colors  blue,green,yellow,orange,red,black  ..
const Colour CGui::sUsedClr[8] = {
	Colour(0.2,0.6,1), Colour(0,1,0.6), Colour(0,1,0), Colour(0.5,1,0),
	Colour(1,1,0), Colour(1,0.5,0), Colour(1,0,0), Colour(0.9,0.2,0.2)};

void CGui::SetUsedStr(Txt valUsed, int cnt, int yellowAt)
{
	if (!valUsed)  return;
	valUsed->setCaption(TR("#{Used}") + ": " + toStr(cnt));
	valUsed->setTextColour(sUsedClr[ (int)( std::min(7.f, 4.f*float(cnt)/yellowAt )) ]);
}


///  Gui Events

//  ⛅ Sky
//-----------------------------------------------------------------------------------------------------------

void CGui::comboSky(Cmb cmb, size_t val)  // sky materials
{
	String s = cmb->getItemNameAt(val);
	sc->skyMtr = s;  app->UpdateTrack();
}

void CGui::comboRain1(Cmb cmb, size_t val)  // rain types
{
	String s = cmb->getItemNameAt(val);  sc->rainName = s;
	app->scn->DestroyWeather();  app->scn->CreateWeather();
}
void CGui::comboRain2(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);  sc->rain2Name = s;
	app->scn->DestroyWeather();  app->scn->CreateWeather();
}

void CGui::slUpdSky(SV*){	scn->UpdSky();	}
void CGui::slUpdSun(SV*){	scn->UpdSun();	}

//  fog
void CGui::slUpdFog(SV*)
{
	scn->UpdFog();
}

//  chk disable
void CGui::chkFog(Ck*)
{
	scn->UpdFog();
}


//  🌳🪨🌿 Vegetation
//-----------------------------------------------------------------------------------------------------------

void CGui::editTrGr(Ed ed)
{
	Real r = s2r(ed->getCaption());
	String n = ed->getName();
	SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	     if (n=="GrPage")  sc->grPage = r;   else if (n=="GrDist")  sc->grDist = r;
	else if (n=="TrPage")  sc->trPage = r;   else if (n=="TrDist")  sc->trDist = r;
	else if (n=="TrImpDist")  sc->trDistImp = r;

	else if (n=="GrSwayDistr")  g0->swayDistr = r;
	else if (n=="GrSwayLen")  g0->swayLen = r;
	else if (n=="GrSwaySpd")  g0->swaySpeed = r;
}

void CGui::comboGrassClr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	SGrassLayer* gr = &sc->grLayersAll[idGrLay];
	gr->colorMap = s;
	imgGrClr->setImageTexture(gr->colorMap);
}


///  🌿 Grass layers  ----------------------------------------------------------

void CGui::tabGrLayers(Tab wp, size_t id)
{
	idGrLay = id;  // help var
	SldUpd_GrL();
	SldUpd_GrChan();
	const SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
	imgGrClr->setImageTexture(gr->colorMap);

	int used=0;
	for (int i=0; i < sc->ciNumGrLay; ++i)
		if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);

	#define _Ed(name, val)  ed##name->setCaption(toStr(val));
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );

	btnGrassMtr->setCaption(gr->material);
	for (int i=0; i < liGrs->getItemCount(); ++i)  // upd pick
		if (liGrs->getSubItemNameAt(1,i).substr(7) == gr->material)
			liGrs->setIndexSelected(i);
	_Cmb(cmbGrassClr, gr->colorMap);

	_Ed(GrSwayDistr, g0->swayDistr);
	_Ed(GrSwayLen, g0->swayLen);
	_Ed(GrSwaySpd, g0->swaySpeed);  //g0-
}

//  tab changed, set slider pointer values, and update
void CGui::SldUpd_GrL()
{
	SGrassLayer& gr =  sc->grLayersAll[idGrLay];
	ckGrLayOn.Upd(&gr.on);
	svGrChan.UpdI(&gr.iChan);
	svLGrDens.UpdF(&gr.dens);

	svGrMinX.UpdF(&gr.minSx);  svGrMaxX.UpdF(&gr.maxSx);
	svGrMinY.UpdF(&gr.minSy);  svGrMaxY.UpdF(&gr.maxSy);
}

///  channels
void CGui::tabGrChan(Tab wp, size_t id)
{
	idGrChan = id;  // help var
	SldUpd_GrChan();
}

void CGui::SldUpd_GrChan()
{
	SGrassChannel& gr =  sc->grChan[idGrChan];
	svGrChAngMin.UpdF(&gr.angMin);  svGrChAngMax.UpdF(&gr.angMax);  svGrChAngSm.UpdF(&gr.angSm);
	svGrChHMin.UpdF(&gr.hMin);  svGrChHMax.UpdF(&gr.hMax);  svGrChHSm.UpdF(&gr.hSm);
	svGrChRdPow.UpdF(&gr.rdPow);
	svGrChNoise.UpdF(&gr.noise);  svGrChNfreq.UpdF(&gr.nFreq);
	svGrChNoct.UpdI(&gr.nOct);  svGrChNpers.UpdF(&gr.nPers);  svGrChNpow.UpdF(&gr.nPow);
}

void CGui::chkGrLayOn(Ck*)
{
	int used=0;
	for (int i=0; i < sc->ciNumGrLay; ++i)
		if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);
}


///  🌳🪨 Vegetation layers  -----------------------------------------------------

void CGui::tabPgLayers(Tab wp, size_t id)
{
	idPgLay = id;  // help var
	SldUpd_PgL();

	const PagedLayer& lay = sc->pgLayersAll[idPgLay];
	string s = lay.name.substr(0, lay.name.length()-5);

	btnVeget->setCaption(s);
	for (int i=0; i < liVeg->getItemCount(); ++i)  // upd pick
		if (liVeg->getSubItemNameAt(1,i).substr(7) == s)
			liVeg->setIndexSelected(i);
			
	Upd3DView(lay.name);
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 6);
	txVCnt->setCaption(toStr(lay.cnt));
}

void CGui::updVegetInfo()
{
	Vector3 va = viewSc * svLTrMinSc.getF(),
			vb = viewSc * svLTrMaxSc.getF();
	float wa = std::max(va.x, va.z), wb = std::max(vb.x, vb.z);
	txVHmin->setCaption(fToStr(va.y, 1,4));
	txVHmax->setCaption(fToStr(vb.y, 1,4));
	txVWmin->setCaption(fToStr(wa, 1,4));
	txVWmax->setCaption(fToStr(wb, 1,4));
}

void CGui::slLTrSc(SV*)
{
	updVegetInfo();
}

//  tab changed
void CGui::SldUpd_PgL()
{
	PagedLayer& lay = sc->pgLayersAll[idPgLay];
	ckPgLayOn.Upd(&lay.on);
	svLTrDens.UpdF(&lay.dens);

	svLTrRdDist.UpdI(&lay.addRdist);
	svLTrRdDistMax.UpdI(&lay.maxRdist);

	svLTrMinSc.UpdF(&lay.minScale);
	svLTrMaxSc.UpdF(&lay.maxScale);

	svLTrWindFx.UpdF(&lay.windFx);
	svLTrWindFy.UpdF(&lay.windFy);

	svLTrMaxTerAng.UpdF(&lay.maxTerAng);
	svLTrMinTerH.UpdF(&lay.minTerH);
	svLTrMaxTerH.UpdF(&lay.maxTerH);
	svLTrFlDepth.UpdF(&lay.maxDepth);
}

void CGui::chkPgLayOn(Ck*)
{
	sc->UpdPgLayers();
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 6);
}

void CGui::Upd3DView(String mesh)
{
	viewMesh = mesh;
	tiViewUpd = 0.f;
}


//  🛣️ Road
//-----------------------------------------------------------------------------------------------------------
void CGui::btnRoadsPrev(WP)
{
	app->RoadsNext(-1);
}
void CGui::btnRoadsNext(WP)
{
	app->RoadsNext(1);
}
void CGui::btnRoadsDel(WP)
{
	app->RoadsDel();
}
void CGui::btnRoadsAdd(WP)
{
	app->RoadsAdd();
}

void CGui::updRoadsTxt()
{
	int all = scn->roads.size();
	if (txRoadsCur)
		txRoadsCur->setCaption(TR("#{RplCurrent}: " + toStr(scn->rdCur+1) +"/"+ toStr(all)));
}

void CGui::editTrkDescr(Ed ed)
{
	app->scn->road->sTxtDescr = ed->getCaption();
}
void CGui::editTrkAdvice(Ed ed)
{
	app->scn->road->sTxtAdvice = ed->getCaption();
}

void CGui::editRoad(Ed ed)
{
	if (!app->scn->road)  return;
	Real r = s2r(ed->getCaption());
	String n = ed->getName();

		 if (n=="RdHeightOfs")	app->scn->road->g_Height = r;
	else if (n=="RdSkirtLen")	app->scn->road->g_SkirtLen = r;
	else if (n=="RdSkirtH")		app->scn->road->g_SkirtH = r;
	//app->scn->road->RebuildRoad(true);  //on Enter ?..
}

//  set slider pointer values, and update
void CGui::SldUpd_Road()
{
	if (!app->scn->road)  return;
	SplineRoad& r = *app->scn->road;
	
	svRdTcMul.UpdF(&r.g_tcMul);       svRdTcMulW.UpdF(&r.g_tcMulW);
	svRdTcMulP.UpdF(&r.g_tcMulP);     svRdTcMulPW.UpdF(&r.g_tcMulPW);
	svRdTcMulC.UpdF(&r.g_tcMulC);

	svRdLenDim.UpdF(&r.g_LenDim0);    svRdWidthSteps.UpdI(&r.g_iWidthDiv0);
	svRdPwsM.UpdF(&r.g_P_iw_mul);     svRdPlsM.UpdF(&r.g_P_il_mul);
	svRdColN.UpdI(&r.g_ColNSides);    svRdColR.UpdF(&r.g_ColRadius);

	svRdMergeLen.UpdF(&r.g_MergeLen); svRdLodPLen.UpdF(&r.g_LodPntLen);
	svRdVisDist.UpdF(&r.g_VisDist);   svRdVisBehind.UpdF(&r.g_VisBehind);
}


//  🚗 Game
//-----------------------------------------------------------------------------------------------------------
void CGui::SldUpd_Game()
{
	svDamage.UpdF(&sc->damageMul);
	svWind.UpdF(&sc->windAmt);
	svGravity.UpdF(&sc->gravity);
}
void CGui::comboReverbs(Cmb cmb, size_t val)  // reverb sets
{
	String s = cmb->getItemNameAt(val);
	sc->sReverbs = s;
	UpdRevDescr();
}
void CGui::UpdRevDescr()
{
	sc->UpdRevSet();
	txtRevebDescr->setCaption(sc->revSet.descr);
}

//  🚦 Pacenotes
void CGui::slUpd_Pace(SV*)
{
	//; scn->UpdPaceParams();
}

void CGui::chkTrkReverse(Ck*)
{
	scn->road->Rebuild(true);
}


//  ⚙️ Settings  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void CGui::chkCamPos(Ck*){     app->txCamPos->setVisible(pSet->camPos);  }
void CGui::chkInputBar(Ck*){   app->bckInput->setVisible(pSet->inputBar);  }

void CGui::chkFps(Ck*){        app->bckFps->setVisible(pSet->show_fps);
								app->txFps->setVisible(pSet->show_fps);  }
void CGui::chkWireframe(Ck*){  app->SetWireframe();  }

void CGui::slSizeRoadP(SV*)
{
	if (app->scn->road)
	{	app->scn->road->fMarkerScale = pSet->road_sphr;
		app->scn->road->UpdAllMarkers();  }
}

void CGui::slSizeMinimap(SV*)
{
	app->UpdMiniSize();
}

void CGui::chkMinimap(Ck*)
{
	app->UpdMiniVis();
}


//  🎥 set camera in settings at exit
void App::SaveCam()
{
	if (!mCamera)  return;
	Vector3 p = mCamera->getPosition(), d = mCamera->getDirection();
	if (gui->bTopView)  {  p = gui->oldPos;  d = gui->oldRot;  }
	pSet->cam_x  = p.x;  pSet->cam_y  = p.y;  pSet->cam_z  = p.z;
	pSet->cam_dx = d.x;  pSet->cam_dy = d.y;  pSet->cam_dz = d.z;
}

//  🎥 set predefined camera view
void CGui::btnSetCam(WP wp)
{
	String s = wp->getName();
	Real y0 = 20, xz = sc->td.fTerWorldSize*0.5f, r = 45.f * 0.5f*PI_d/180.f, yt = xz / Math::Tan(r);
	Camera* cam = app->mCamera;

		 if (s=="CamView1")	{	cam->setPosition(xz*0.8,60,0);  cam->setDirection(-1,-0.3,0);  }
	else if (s=="CamView2")	{	cam->setPosition(xz*0.6,80,xz*0.6);  cam->setDirection(-1,-0.5,-1);  }
	else if (s=="CamView3")	{	cam->setPosition(-xz*0.7,80,-xz*0.5);  cam->setDirection(0.8,-0.5,0.5);  }
	else if (s=="CamView4")	{
		Vector3 cp = app->ndCar->getPosition();  float cy = app->ndCar->getOrientation().getYaw().valueRadians();
		Vector3 cd = Vector3(cosf(cy),0,-sinf(cy));
		cam->setPosition(cp - cd * 15 + Vector3(0,7,0));  cd.y = -0.3f;
		cam->setDirection(cd);  }

	else if (s=="CamTop")	{	cam->setPosition(0,yt,0);  cam->setDirection(-0.0001,-1,0);  }
	else if (s=="CamLeft")	{	cam->setPosition(0,y0, xz);  cam->setDirection(0,0,-1);  }
	else if (s=="CamRight")	{	cam->setPosition(0,y0,-xz);  cam->setDirection(0,0, 1);  }
	else if (s=="CamFront")	{	cam->setPosition( xz,y0,0);  cam->setDirection(-1,0,0);  }
	else if (s=="CamBack")	{	cam->setPosition(-xz,y0,0);  cam->setDirection( 1,0,0);  }
}

//  🎥 toggle top view camera
void CGui::toggleTopView()
{
	bTopView = !bTopView;
	Camera* cam = app->mCamera;
	
	if (bTopView)
	{	// store old
		oldPos = cam->getPosition();
		oldRot = cam->getDirection();
		
		Real xz = sc->td.fTerWorldSize*0.5f, r = 45.f * 0.5f*PI_d/180.f, yt = xz / Math::Tan(r);
		cam->setPosition(0,yt,0);  cam->setDirection(-0.0001,-1,0);

		oldFog = pSet->bFog;
		pSet->bFog = true;  ckFog.Upd();  scn->UpdFog();
	}else
	{	// restore
		cam->setPosition(oldPos);
		cam->setDirection(oldRot);

		pSet->bFog = oldFog;  ckFog.Upd();  scn->UpdFog();
	}
}


//  ⚫💭 Surface
//-----------------------------------------------------------------------------------------------------------

TerLayer* CGui::GetTerRdLay()
{
	if (idSurf < 4)  //  terrain
	{	if (idSurf >= sc->td.layers.size())  // could change by on/off ter layers
		{	idSurf = 0;  if (surfList)  surfList->setIndexSelected(idSurf);  }
		return &sc->td.layersAll[sc->td.layers[idSurf]];
	}
	//  road
	return  &sc->td.layerRoad[sc->td.road1mtr ? 0 : idSurf-4];
}

void CGui::listSurf(Li, size_t id)
{
	if (id == ITEM_NONE) {  id = 0;  surfList->setIndexSelected(0);  }
	if (id < 4 && id >= sc->td.layers.size()) {  id = 0;  surfList->setIndexSelected(id);  }  // more than used
	if (id >= 8) {  id = 4;  surfList->setIndexSelected(id);  }
	//TODO: own pipe mtrs..

	idSurf = id;  // help var
	TerLayer* l = GetTerRdLay();
	SldUpd_Surf();

	Vector3 c;  c = l->tclr.GetRGB1();
	clrTrail->setColour(Colour(c.x, c.y, c.z));
	
	//  Surface
	cmbSurface->setIndexSelected( cmbSurface->findItemIndexWith(l->surfName));
	UpdSurfInfo();
}

void CGui::SldUpd_Surf()
{
	TerLayer* l = GetTerRdLay();

	svLDust.UpdF(&l->dust);  svLDustS.UpdF(&l->dustS);
	svLMud.UpdF(&l->mud);    svLSmoke.UpdF(&l->smoke);
}

void CGui::UpdSurfList()
{
	if (!surfList || surfList->getItemCount() != 12)  return;

	for (int n=0; n < 4; ++n)
	{
		String s = n >= app->scn->sc->td.layers.size() ? "" : StringUtil::replaceAll(
			app->scn->sc->td.layersAll[app->scn->sc->td.layers[n]].texFile, "_d.jpg","");
		surfList->setItemNameAt(n  , "#80FF00"+TR("#{Layer} ")+toStr(n+1)+"  "+ s);
		surfList->setItemNameAt(n+4, "#FFB020"+TR("#{Road} ") +toStr(n+1)+"  "+ app->scn->road->sMtrRoad[n]);
		surfList->setItemNameAt(n+8, "#FFFF80"+TR("#{Pipe} ") +toStr(n+1)+"  "+ app->scn->road->sMtrPipe[n]);
	}
	GetTerRdLay();  // fix list pos if cur gone
}

//  upd ed info txt
void CGui::UpdEdInfo()
{	
	int t = scn->sc->secEdited;
	ostringstream s;  s.fill('0');
	s << fixed << "#C0F0F8" << t/3600 << ":#A0D8E0" << setw(2) << t/60%60 << ":#70A8B0" << setw(2) << t%60;
	txtEdInfo->setCaption(
		TR("#A0C0D0#{Time} [h:m:s]: ") + s.str()+"\n"+
		TR("#808080Base track: ") + scn->sc->baseTrk);
}
