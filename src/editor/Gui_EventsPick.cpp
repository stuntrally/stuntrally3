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
#include <OgreHlmsCommon.h>
#include <OgreHlmsPbsDatablock.h>
#include "MultiList2.h"
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  👆 Pick window
//-----------------------------------------------------------------------------------------------------------
int CGui::liNext(Mli2 li, int rel)
{
	int cnt = li->getItemCount()-1;
	if (cnt < 0)  return 0;
	int i = li->getIndexSelected();
	if (i == ITEM_NONE)  i = 0;
	i += rel;  if (i<0) i=0;  if (i>cnt) i=cnt;
	li->setIndexSelected(i);
	li->beginToItemAt(std::min(cnt, std::max(0, i-20)));
	return i;
}

void CGui::keyPickNext(int r)
{
	if (liSky->getVisible())  listPickSky(liSky, liNext(liSky, r));  else
	if (liTex->getVisible())  listPickTex(liTex, liNext(liTex, r));	 else
	if (liGrs->getVisible())  listPickGrs(liGrs, liNext(liGrs, r));  else
	if (liVeg->getVisible())  listPickVeg(liVeg, liNext(liVeg, r));  else
	if (liRd->getVisible())   listPickRd(liRd, liNext(liRd, r));
}

void CGui::wheelSky(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickSky(liSky, liNext(liSky, r));  }
void CGui::wheelTex(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickTex(liTex, liNext(liTex, r));  }
void CGui::wheelGrs(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickGrs(liGrs, liNext(liGrs, r));  }
void CGui::wheelVeg(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickVeg(liVeg, liNext(liVeg, r));  }

void CGui::wheelRd(WP wp, int rel)
{
	idRdPick = atoi(wp->getName().substr(0,1).c_str());
	int r = rel < 0 ? 1 : -1;  listPickRd(liRd, liNext(liRd, r));
}

void CGui::btnPickSky(WP){    PickShow(P_Sky);  }
void CGui::btnPickTex(WP){    PickShow(P_Tex);  }
void CGui::btnPickGrass(WP){  PickShow(P_Grs);  }
void CGui::btnPickVeget(WP){  PickShow(P_Veg);  }

void CGui::btnPickRd(WP wp)
{
	if (!wp) {  PickShow(P_Rd, true);  return;  }
	// int old = idRdPick;
	idRdPick = atoi(wp->getName().substr(0,1).c_str());
	PickShow(P_Rd, false); //old==idRdPick);
}
void CGui::btnPickRoad(WP wp)
{
	idRdType = RdPk_Road;
	btnPickRd(wp);
}
// Btn btnRoad[4], btnPipe[4], btnRoadW =0, btnPipeW =0, btnRoadCol =0;
void CGui::btnPickPipe(WP wp)
{
	idRdType = RdPk_Pipe;
	btnPickRd(wp);
	// String sn = cmb->getName().substr(0,1);
	// int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	// String s = cmb->getItemNameAt(val);
	// app->scn->road->SetMtrPipe(id, s);  app->scn->road->Rebuild(true);
	// UpdSurfList();
}

void CGui::btnPickRoadW(WP wp)
{
	idRdType = RdPk_Wall;
	btnPickRd(wp);
}
void CGui::btnPickPipeW(WP wp)
{
	idRdType = RdPk_PipeW;
	btnPickRd(wp);
}
void CGui::btnPickRoadCol(WP wp)
{
	idRdType = RdPk_Col;
	btnPickRd(wp);
}


//  🪟📃👆 Pick window show
//----------------------------------------------------
void CGui::PickShow(EPick n, bool toggleVis)
{
	liSky->setVisible(n == P_Sky);  liTex->setVisible(n == P_Tex);
	liGrs->setVisible(n == P_Grs);  liVeg->setVisible(n == P_Veg);
	liRd->setVisible(n == P_Rd);
	panPick->setPosition(liPickW[n], 0);

	const int wx = app->mWindow->getWidth(),
			  wy = app->mWindow->getHeight();  ///  pick dim
	app->mWndPick->setCoord(wx * liPickX[n], 12.f, liPickW[n], 0.95f*wy);
	
	if (n == P_Rd)  //  upd pick road
	{	UString s = btnRoad[idRdPick]->getCaption();
		for (int i=0; i < liRd->getItemCount(); ++i)
			if (liRd->getSubItemNameAt(1,i).substr(7) == s)
				liRd->setIndexSelected(i);
	}
	bool vis = app->mWndPick->getVisible();
	if (n != P_Rd || toggleVis || !vis)
		app->mWndPick->setVisible(!vis);
}


///  🛣️ Road  -------------------------------------------------
void CGui::listPickRd(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos > data->pre->rd.size())
	{	liRd->setIndexSelected(0);  pos = 0;  }
	
	string s = liRd->getSubItemNameAt(1,pos);
	if (s.length() < 8)  return;
	s = s.substr(7);
	if (s[0] == '-' || s[0] == '=')  return;  // sep

	//  set
	const PRoad* p = data->pre->GetRoad(s);
	// Btn btnRoad[4], btnPipe[4], btnRoadW =0, btnPipeW =0, btnRoadCol =0;
	switch (idRdType)
	{
	case RdPk_Road:
		scn->road->sMtrRoad[idRdPick] = s;
		btnRoad[idRdPick]->setCaption(s);  break;
	case RdPk_Pipe:
		app->scn->road->SetMtrPipe(idRdPick, s);
		btnPipe[idRdPick]->setCaption(s);  break;

	case RdPk_Wall:
		scn->road->sMtrWall = s;
		btnRoadW->setCaption(s);  break;
	case RdPk_PipeW:
		scn->road->sMtrWallPipe = s;
		btnPipeW->setCaption(s);  break;
	case RdPk_Col:
		scn->road->sMtrCol = s;
		btnRoadCol->setCaption(s);  break;
	}	
	//  preset
	if (pSet->pick_setpar && p && idRdType == RdPk_Road)
	{	TerLayer& l = scn->sc->layerRoad[idRdPick];
		l.surfName = p->surfName;
		l.dust = p->dust;  l.dustS = p->dustS;
		l.mud = p->mud;  l.tclr = p->tclr;
		listSurf(surfList, idSurf);
	}
	//  upd
	app->scn->road->Rebuild(true);
	UpdSurfList();
}


///  ⛅ Sky Mtr  ----------------------------------------------------
void CGui::listPickSky(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->sky.size())
	{	liSky->setIndexSelected(0);  pos = 0;  }
	
	string s = liSky->getSubItemNameAt(1,pos);
	//s = "sky/" + s.substr(7);
	s = s.substr(7);
	const PSky* p = data->pre->GetSky(s);  if (!p)  return;

	//  set
	sc->skyMtr = p->mtr;
	if (pSet->pick_setpar)
	{	sc->ldPitch = p->ldPitch;  svSunPitch.Upd();
		sc->ldYaw = p->ldYaw;  svSunYaw.Upd();
		scn->UpdSun();
	}
	btnSky->setCaption(s);

	//  upd img tex prv
	Hlms *hlms = app->mRoot->getHlmsManager()->getHlms( HLMS_PBS );
	HlmsPbsDatablock *db = static_cast<HlmsPbsDatablock*>(hlms->getDatablock( p->mtr ));
	if (db)
	{	auto* tex = db->getTexture(PBSM_EMISSIVE);
	    if (tex)
		{	String n = tex->getNameStr();
			app->LoadTex(n);
			imgSky->setImageTexture(n);
	}	}

	// app->UpdateTrack();
	scn->DestroyAllAtmo();
	scn->CreateAllAtmo();
}


///  🏔️ Tex Diff  ----------------------------------------------------
void CGui::listPickTex(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->ter.size())
	{	liTex->setIndexSelected(0);  pos = 0;  }
	
	string s = liTex->getSubItemNameAt(1,pos);
	s = s.substr(7) + "_d";  // rem #clr
	const PTer* p = data->pre->GetTer(s);  if (!p)  return;
	s += ".jpg";

	//  set
	TerLayer& l = td().layersAll[idTerLay];
	l.texFile = s;
	
	//  auto norm
	{	String sNorm; //old = StringUtil::replaceAll(s,"_d.","_n.");  //_T
		sNorm = p->texNorm+".jpg";

		//  preset
		if (pSet->pick_setpar)
		{	l.tiling = p->tiling;  svTerLScale.Upd();
			l.triplanar = p->triplanar;  ckTerLayTripl.Upd();
			//angMin angMax
			l.surfName = p->surfName;
			l.dust = p->dust;  l.dustS = p->dustS;
			l.mud = p->mud;  l.tclr = p->tclr;
			l.fDamage = p->dmg;  svTerLDmg.Upd();
			listSurf(surfList, idSurf);
		}

		size_t id = cmbTexNorm->findItemIndexWith(sNorm);
		if (id != ITEM_NONE)  // set only if found
		{
			cmbTexNorm->setIndexSelected(id);
			l.texNorm = sNorm;
	}	}

	//  upd img
	app->LoadTex(s);
	btnTexDiff->setCaption(s.substr(0, s.length()-6));  // rem _d.jpg
    imgTexDiff->setImageTexture(s);
	UpdSurfList();
}


///  🌿 Grass  -------------------------------------------------------
void CGui::listPickGrs(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->gr.size())
	{	liGrs->setIndexSelected(0);  pos = 0;  }
	
	string s = liGrs->getSubItemNameAt(1,pos);
	if (s.length() < 8)  return;
	s = s.substr(7);
	if (s[0] == '-' || s[0] == '=')  return;  // sep

	//  set
	SGrassLayer& l = sc->grLayersAll[idGrLay];
	l.material = s;
	
	//  preset
	const PGrass* p = data->pre->GetGrass(s);
	if (pSet->pick_setpar && p)
	{	l.minSx = p->minSx;  svGrMinX.Upd();
		l.maxSx = p->maxSx;  svGrMaxX.Upd();
		l.minSy = p->minSy;  svGrMinY.Upd();
		l.maxSy = p->maxSy;  svGrMaxY.Upd();
	}

	//  upd img
	app->LoadTex(s + ".png");
	btnGrassMtr->setCaption(s);
	imgGrass->setImageTexture(s + ".png");  // same mtr name as tex
}


///  🌳🪨 Veget Model  -------------------------------------------------
void CGui::listPickVeg(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->veg.size())
	{	liVeg->setIndexSelected(0);  pos = 0;  }
	
	string s = liVeg->getSubItemNameAt(1,pos);
	if (s.length() < 8)  return;
	s = s.substr(7);
	if (s[0] == '-' || s[0] == '=')  return;  // sep
		
	//  set
	const PVeget* p = data->pre->GetVeget(s);
	btnVeget->setCaption(s);
	s += ".mesh";

	auto& l = sc->vegLayersAll[idPgLay];
	l.name = s;
	
	//  preset
	if (pSet->pick_setpar && p)
	{	l.minScale = p->minScale;  svLTrMinSc.Upd();
		l.maxScale = p->maxScale;  svLTrMaxSc.Upd();
		// l.windFx = p->windFx;  svLTrWindFx.Upd();
		// l.windFy = p->windFy;  svLTrWindFy.Upd();
		l.maxTerAng = p->maxTerAng;  svLTrMaxTerAng.Upd();
		l.maxDepth = p->maxDepth;  svLTrFlDepth.Upd();
		l.addRdist = p->addRdist;  svLTrRdDist.Upd();
	}
	Upd3DView(s);
}
