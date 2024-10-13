#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "paths.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "MessageBox/MessageBox.h"
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  tools  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


void CGui::btnTrkCopySel(WP)  // set copy source
{
	sCopyTrack = gcom->sListTrack;
	bCopyTrackU = gcom->bListTrackU;
	if (valTrkCpySel)  valTrkCpySel->setCaption(sCopyTrack);
}

bool CGui::ChkTrkCopy()
{
	if (sCopyTrack == "")  // none
	{
		Message::createMessageBox(
			"Message", TR("#{Track} - #{Copy}"), TR("#{CopyTrackNoSource}"),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	if (sCopyTrack == pSet->gui.track && bCopyTrackU == (pSet->gui.track_user ? 1 : 0))
	{
		Message::createMessageBox(
			"Message", TR("#{Track} - #{Copy}"), TR("#{CopyTrackSourceSame}"),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	return true;
}

///  copy Hmap
void CGui::btnCopyTerHmap(WP)
{
	/*if (!ChkTrkCopy())  return;

	String from = PathCopyTrk(),
		name = gcom->TrkDir() + "heightmap-new.f32";
	Copy(from + "/heightmap.f32", name);
	
	Scene sF;  sF.LoadXml(from + "/scene.xml");
	td().getFileSize(from);  // sets sc->td.iVertsX
	td().fTriangleSize = sF.tds[0].fTriangleSize;
	td().UpdVals();
	app->bNewHmap = true;
	SetGuiFromXmls();	app->UpdateTrack();
	if (scn->road)  scn->road->UpdAllMarkers();*/
}

//  copy Sun, etc.  can you copy a star
void CGui::btnCopySun(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->skyMtr = sF.skyMtr;  // sky
	sc->skyYaw = sF.skyYaw;
	for (int i=0; i < NumWeather; ++i)
	{	sc->rainEmit[i] = sF.rainEmit[i];
		sc->rainName[i] = sF.rainName[i];
	}
	sc->fogClr = sF.fogClr;  sc->fogClr2 = sF.fogClr2;  sc->fogClrH = sF.fogClrH;
	sc->fogStart = sF.fogStart;  sc->fogEnd = sF.fogEnd;
	sc->fogHStart = sF.fogHStart;  sc->fogHEnd = sF.fogHEnd;
	sc->fogHDensity = sF.fogHDensity;  sc->fogHeight = sF.fogHeight;

	sc->ldPitch = sF.ldPitch;  sc->ldYaw = sF.ldYaw;  // light
	sc->lAmb = sF.lAmb;  sc->lDiff = sF.lDiff;  sc->lSpec = sF.lSpec;

	SetGuiFromXmls();	app->UpdateTrack();
	scn->DestroyWeather();  scn->CreateWeather();
}

//  copy Ter layers
void CGui::btnCopyTerLayers(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	for (int i=0; i < td().ciNumLay; ++i)
		td().layersAll[i] = sF.tds[0].layersAll[i];  // 1st ter- todo: slider..
	sc->sParDust = sF.sParDust;  sc->sParMud = sF.sParMud;
	sc->sParSmoke = sF.sParSmoke;
	td().UpdLayers();

	SetGuiFromXmls();	app->UpdateTrack();
}

//  copy Veget
void CGui::btnCopyVeget(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->densGrass = sF.densGrass;  sc->densTrees = sF.densTrees;
	sc->trRdDist = sF.trRdDist;

	for (int i=0; i < sc->ciNumGrLay; ++i)
		sc->grLayersAll[i] = sF.grLayersAll[i];

	for (int i=0; i < sc->ciNumVegLay; ++i)
		sc->vegLayersAll[i] = sF.vegLayersAll[i];

	SetGuiFromXmls();	app->UpdateTrack();
}

//  copy Road
void CGui::btnCopyRoad(WP)
{
	if (!ChkTrkCopy() || !scn->road)  return;
	String from = PathCopyTrk();
	scn->road->LoadFile(from + "/road.xml");  // todo: other roads cmb?

	SetGuiFromXmls();	scn->road->Rebuild(true);
	scn->road->UpdAllMarkers();
}

//  copy Road pars
void CGui::btnCopyRoadPars(WP)
{
	SplineRoad* r = scn->road;
	if (!ChkTrkCopy() || !r)  return;
	String from = PathCopyTrk();
	SplineRoad rd(app);
	rd.LoadFile(from + "/road.xml",false);  // todo: other roads

	for (int i=0; i < MTRs; ++i)
	{	r->sMtrRoad[i] = rd.sMtrRoad[i];
		r->SetMtrPipe(i, rd.sMtrPipe[i]);
	}
	r->g_tcMul  = rd.g_tcMul;	r->g_tcMulW = rd.g_tcMulW;
	r->g_tcMulP = rd.g_tcMulP;	r->g_tcMulPW= rd.g_tcMulPW;
	r->g_tcMulC = rd.g_tcMulC;
	r->g_LenDim0 = rd.g_LenDim0;    r->g_iWidthDiv0 = rd.g_iWidthDiv0;
	r->g_ColNSides = rd.g_ColNSides;  r->g_ColRadius = rd.g_ColRadius;
	r->g_P_iw_mul = rd.g_P_iw_mul;  r->g_P_il_mul = rd.g_P_il_mul;
	r->g_Height = rd.g_Height;
	r->g_SkirtLen = rd.g_SkirtLen;  r->g_SkirtH = rd.g_SkirtH;
	r->g_MergeLen = rd.g_MergeLen;  r->g_LodPntLen = rd.g_LodPntLen;
	r->g_VisDist = rd.g_VisDist;    r->g_VisBehind = rd.g_VisBehind;

	SetGuiFromXmls();
	scn->road->Rebuild(true);
	scn->road->UpdAllMarkers();
}


///  tools 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  deletes all roads points, leaves setups
void CGui::btnDeleteRoad(WP)
{
	//scn->DestroyRoads();
	//scn->road = new

	for (auto* rd : scn->roads)
	{	int l = rd->getNumPoints();
		for (int i=0; i < l; ++i)
		{
			rd->iChosen = rd->getNumPoints()-1;
			rd->Delete();
	}	}
	//scn->road->Rebuild(true);
}
void CGui::btnDeleteFluids(WP)
{
	sc->fluids.clear();
	app->bRecreateFluids = true;
}
void CGui::btnDeleteObjects(WP)
{
	app->DestroyObjects(true);
	app->iObjCur = -1;
}
void CGui::btnDeleteParticles(WP)
{
	scn->DestroyEmitters(true);
}


//  Scale track  --------------------------------
void CGui::btnScaleAll(WP)
{
	if (!scn->road)  return;
	Real sf = std::max(0.1f, fScale);  // scale mul
	int i;
	
	//  roads
	for (auto& r : scn->roads)
	for (i=0; i < r->getNumPoints(); ++i)
	{
		r->Scale1(i, sf, 0.f);
		r->mP[i].width *= sf;
	}
	scn->road->bSelChng = true;
	
	//  fluids
	for (i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.x *= sf;  fb.pos.z *= sf;
		fb.size.x *= sf;  fb.size.z *= sf;
	}
	
	//  objs
	for (i=0; i < sc->objects.size(); ++i)
	{
		Object& o = sc->objects[i];
		o.pos[0] *= sf;  o.pos[1] *= sf;
		o.SetFromBlt();
	}

	//  ter  ---
	for (auto& td : sc->tds)
	{	td.fTriangleSize *= sf;  td.UpdVals();  }
	
	SetGuiFromXmls();	app->UpdateTrack();
	
	//  road upd
	if (0) //road)  // doesnt work here..
	{	scn->road->UpdPointsH();
		scn->road->Rebuild(true);
	}

	//  start,end pos
	for (i=0; i < 2; ++i)
	{	scn->sc->startPos[i][0] *= sf;
		scn->sc->startPos[i][1] *= sf;  }
	app->UpdStartPos();
}


///  track 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .
String CGui::PathCopyTrk(int user) {
	int u = user == -1 ? bCopyTrackU : user;	return gcom->pathTrk[u] + sCopyTrack;  }

///  New (duplicate)
//-----------------------------------------------------------------------------------------------------------
void CGui::btnTrackNew(WP)
{
	String name = trkName->getCaption();
	name = StringUtil::replaceAll(name, "*", "");

	if (gcom->TrackExists(name))  {
		Message::createMessageBox(
			"Message", TR("#{Track} - #{NewDup}"), TR("#{Track}: ") + name + TR(" #{AlreadyExists}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }

	//  paths
	sc->baseTrk = gcom->sListTrack;
	String from = gcom->PathListTrk(),  to = gcom->pathTrk[1] + name;
	sc->secEdited = 0;

	//  dirs
	const String subdirs[3] = {"", "/objects", "/preview"};
	CreateDir(to);  CreateDir(to +subdirs[1]);  CreateDir(to +subdirs[2]);

	//  list all, copy  ----
	// LogO("NEW all, copy  ----");
	for (auto& d : subdirs)
	{
		strlist files;
		PATHS::DirList(from + d, files);
		
		for (auto& f : files)
		if (f.find(".") != string::npos)  // files only, with .
		{
			// LogO(d+"/"+f);
			Copy(from + d+"/"+f, to + d+"/"+f);
		}
	}

	gcom->sListTrack = name;  pSet->gui.track = name;  pSet->gui.track_user = 1;
	app->UpdWndTitle();
	gcom->FillTrackLists();  gcom->TrackListUpd();
}

///  Rename
void CGui::btnTrackRename(WP)
{
	String name = trkName->getCaption();
	if (name == gcom->sListTrack)  return;

	if (!pSet->allow_save)  // could force when originals writable..
	if (gcom->bListTrackU==0)  {
		return;  }

	if (gcom->TrackExists(name))  {
		Message::createMessageBox(
			"Message", TR("#{Track} - #{Rename}"), TR("#{AlreadyExists}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }
	
	//  Rename
	Rename(gcom->PathListTrk(), gcom->pathTrk[/*1*/gcom->bListTrackU ? 1 : 0] + name);
	
	gcom->sListTrack = name;  pSet->gui.track = name;  pSet->gui.track_user = 1;/**/
	app->UpdWndTitle();
	gcom->FillTrackLists();  gcom->TrackListUpd();  //gcom->listTrackChng(trkList,0);
}

///  Delete
//-----------------------------------------------------------------------------------------------------------
void CGui::btnTrackDel(WP)
{
	if (!gcom->bListTrackU && !pSet->allow_save)
		return;  // users don't delete original

	Message* message = Message::createMessageBox(
		"Message", TR("#{DeleteTrack}"), gcom->sListTrack,
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult += newDelegate(this, &CGui::msgTrackDel);
	//message->setUserString("FileName", fileName);
}
void CGui::msgTrackDel(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)
		return;

	//  dirs
	const String subdirs[3] = {"", "/objects", "/preview"};
	string to = gcom->PathListTrk();

	//  list all, delete  ----
	// LogO("DEL all  ----");
	for (auto& d : subdirs)
	{
		strlist files;
		PATHS::DirList(to + d, files);
		
		for (auto& f : files)
		if (f.find(".") != string::npos)  // files only, with .
		{
			// LogO(d+"/"+f);
			Delete(to + d+"/"+f);
		}
	}
	DeleteDir(to +subdirs[1]);  DeleteDir(to +subdirs[2]);  DeleteDir(to);

	String st = pSet->gui.track;
	gcom->FillTrackLists();
	gcom->TrackListUpd();
	if (st != pSet->gui.track)
		app->LoadTrack();  //load 1st if deleted cur
}
