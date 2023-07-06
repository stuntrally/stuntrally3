#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
#include "TracksXml.h"
#include "CScene.h"

#include "game.h"
#include "settings.h"
#include "quickprof.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
// #include "CGui.h"
// #include "SplitScreen.h"
#include "FollowCamera.h"
#include "CarModel.h"
// #include "MultiList2.h"
// #include "GraphView.h"

// #include <OgreWindow.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreManualObject2.h>
#include <OgreSceneManager.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>

using namespace Ogre;
// using namespace MyGUI;


///---------------------------------------------------------------------------------------------------------------
//  ⏱️💫 Update HUD
///---------------------------------------------------------------------------------------------------------------
void CHud::Update(int carId, float time)
{
	PROFILER.beginBlock("g.hud");

	
	//  update HUD elements for all cars that have a viewport (local or replay)
	int cnt = std::min(6, (int)app->carModels.size());  // all cars
	int cntC = std::min(4, cnt - (app->isGhost2nd && !app->bRplPlay ? 1 : 0));  // all vis plr
	
	UpdPosElems(cnt, cntC, carId);


	//  track %  local, updated always
	for (int c = 0; c < cntC; ++c)
	{	CarModel* cm = app->carModels[c];
		if (cm->cType == CarModel::CT_LOCAL ||
			cm->cType == CarModel::CT_REPLAY)
			cm->UpdTrackPercent();
	}
	
	if (carId == -1 || app->carModels.empty())
	{
		PROFILER.endBlock("g.hud");
		return;
	}

	#ifdef DEBUG
	assert(carId >= 0);
	assert(carId < app->carModels.size());
	#endif


	//  var
	CarModel* pCarM = app->carModels[carId];
	CAR* pCar = pCarM ? pCarM->pCar : 0;
	Hud& h = hud[carId];


	///  multiplayer
	if (app->mClient)
		UpdMultiplayer(app->gui, cnt, time);


	///  opponents list
	// bool visOpp = h.txOpp[0] && pSet->show_opponents;
	// if (visOpp && pCarM && pCarM->pMainNode)
	// 	UpdOpponents(h, cnt, pCarM);


	//  motion blur intensity
	// if (pSet->blur)
	// 	UpdMotBlur(pCar, time);


	///  gear, vel texts
	UpdCarTexts(carId, h, time, pCar);

	
	///  Times, race pos
	if (pSet->show_times && pCar)
		UpdTimes(carId, h, time, pCar, pCarM);


	//  camera cur
	/*if (h.txCam)
	{	FollowCamera* cam = pCarM->fCam;
		if (cam && cam->updName)
		{	cam->updName = false;
			h.txCam->setCaption(cam->sName);
	}	}*/


	///  debug infos
	UpdDebug(pCar, pCarM);


	PROFILER.endBlock("g.hud");
}


//---------------------------------------------------------------------------------------------------------------
///  ⏲️🌍🔺 Update HUD minimap poses, vertices, etc
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdPosElems(int cnt, int cntC, int carId)
{
	//LogO(toStr(cnt)+" "+toStr(cntC)+" "+toStr(carId));
	int c;
	//  gui viewport - done once for all
	if (carId == -1)
	for (c = 0; c < cntC; ++c)
	if (app->carModels[c]->cType == CarModel::CT_LOCAL)
	{
		//  hud rpm,vel
		float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
		GetCarVals(c,&vel,&rpm,&clutch,&gear);

		//  update all mini pos tri
		for (int i=0; i < cnt; ++i)
			UpdRotElems(c, i, vel, rpm);
	}


	///  🌍🔺 all minimap car pos-es rot
	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};
	const float z = pSet->size_minipos;  // tri size
	
	if (carId == -1 && hrPos)
	{	hrPos->begin();

		const int plr = 1;  //; app->mSplitMgr->mNumViewports;
		for (int v = 0; v < plr; ++v)  // all viewports
		{
			const Hud& h = hud[v];
			const float sc = pSet->size_minimap * app->mDims[v].avgsize1;
			const Vector3& pos = h.ndMap->getPosition();
			
			for (c = 0; c < cntC; ++c)  // all mini pos for one car
			{
				const SMiniPos& sp = h.vMiniPos[c];
				const ColourValue& clr = app->carModels[c]->color;

				for (int p=0; p < 4; ++p)  // all 4 points
				{
					float x = pos.x + (sp.x + sp.px[p]*z)*sc;
					float y = pos.y + (sp.y + sp.py[p]*z)*sc*asp;
					hrPos->position(x, y, 0);
					hrPos->texUV(tc[p][0], tc[p][1]);
					hrPos->color(clr.r, clr.g, clr.b, clr.a);
		}	}	}
		
		hrPos->end();
	}
}

//---------------------------------------------------------------------------------------------------------------
///  Update HUD rotated elems - for carId, in baseCarId's space
///  ⏲️ rpm,vel gauges  and 🌍🔺 minimap triangles
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdRotElems(int baseCarId, int carId, float vel, float rpm)
{
	//if (carId == -1)  return;
	int base = baseCarId, id = carId;
	bool main = base == id;
	// LogO(toStr(b)+" b "+toStr(c)+" c");
	#ifdef DEBUG
	assert(id >= 0);
	assert(base >= 0);
	assert(base < hud.size());  // only b
	assert(id < app->carModels.size());
	assert(base < app->carModels.size());
	assert(id < hud[base].vMiniPos.size());
	#endif
	float angBase = app->carModels[base]->angCarY;
	
	bool bZoom = pSet->mini_zoomed,
		bRot = pSet->mini_rotated;

	const float vmin[2] = {0.f,-45.f}, rmin[2] = {0.f,-45.f},
		vsc_mph[2] = {-180.f/100.f, -(180.f+vmin[1])/90.f},
		vsc_kmh[2] = {-180.f/160.f, -(180.f+vmin[1])/120.f},
		sc_rpm[2] = {-180.f/6000.f, -(180.f+rmin[1])/5000.f};
	const int ig = pSet->gauges_type > 0 ? 1 : 0;

	//  angles
	float angrmp = rpm*sc_rpm[ig] + rmin[ig];
	float vsc = pSet->show_mph ? vsc_mph[ig] : vsc_kmh[ig];
	float angvel = fabs(vel)*vsc + vmin[ig];
	float angrot = app->carModels[base]->angCarY;
	if (bRot && bZoom && !main)
		angrot -= angBase-180.f;

	Hud& h = hud[base];  int p;
	float sx = 1.4f * h.fScale, sy = sx*asp;  // *par len

	//  4 points, 2d pos
	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};  // defaults, no rot
	const static Real tn[4][2] = {{0.5f,1.f}, {1.f,1.f}, {0.5,0.5f}, {1.f,0.5f}};  // tc needle
	const static Real tp[4][2] = {{-1,-1}, {1,-1}, {-1,1}, {1,1}};
	const static float d2r = PI_d/180.f;
	const static Real ang[4] = {0.f,90.f,270.f,180.f};

	float rx[4],ry[4], vx[4],vy[4], px[4],py[4], cx[4],cy[4];  // rpm,vel, pos,crc
	for (int i=0; i < 4; ++i)  // 4 verts, each +90deg
	{
		//  needles
		float ia = 45.f + ang[i];
		if (main)
		{	float r = -(angrmp + ia) * d2r;   rx[i] = sx*cosf(r);  ry[i] =-sy*sinf(r);
			float v = -(angvel + ia) * d2r;   vx[i] = sx*cosf(v);  vy[i] =-sy*sinf(v);
		}
		float p = -(angrot + ia) * d2r;	  float cp = cosf(p), sp = sinf(p);

		//  mini
		if (bRot && bZoom && main)
			{  px[i] = tp[i][0];  py[i] = tp[i][1];  }
		else{  px[i] = cp*1.4f;   py[i] =-sp*1.4f;   }

		float z = bRot ? 0.70f/pSet->zoom_minimap : 0.5f/pSet->zoom_minimap;
		if (!bRot)
			{  cx[i] = tp[i][0]*z;  cy[i] = tp[i][1]*z-1.f;  }
		else{  cx[i] =       cp*z;  cy[i] =      -sp*z-1.f;  }
	}
	    
    //  ⏲️ rpm,vel needles
    const float r = 0.55f, v = 0.85f;
	const bool bRpm = app->carModels[id]->hasRpm();

	//  rpm,vel gauges backgr
	HudRenderable* hr = h.hrGauges;
	if (main && hr)
	{
		Real ofs = pSet->show_mph ? 0.5f : 0.f;
	
		hr->begin();
		//  backgr
		if (bRpm)
		for (p=0; p < 4; ++p)
		{	hr->position(
				h.vcRpm.x + tp[p][0]*h.fScale*r,
				h.vcRpm.y + tp[p][1]*h.fScale*asp*r, 0);
			hr->texUV(tc[p][0]*0.5f, tc[p][1]*0.5f + 0.5f);
		}
		for (p=0; p < 4; ++p)
		{	hr->position(
				h.vcVel.x + tp[p][0]*h.fScale*v,
				h.vcVel.y + tp[p][1]*h.fScale*asp*v, 0);
			hr->texUV(tc[p][0]*0.5f +ofs, tc[p][1]*0.5f);
		}

		//  needles
		if (bRpm)
		for (p=0; p < 4; ++p)
		{	hr->position(
				h.vcRpm.x + rx[p]*r,
				h.vcRpm.y + ry[p]*r, 0);
			hr->texUV(tn[p][0], tn[p][1]);
		}
		for (p=0; p < 4; ++p)
		{	hr->position(
				h.vcVel.x + vx[p]*v,
				h.vcVel.y + vy[p]*v, 0);
			hr->texUV(tn[p][0], tn[p][1]);
		}
		hr->end();
	}


	//---------------------------------------------------------------------------------------------------------------
	///  minimap car pos-es rot
	for (p=0; p < 4; ++p)
	{	
		h.vMiniPos[id].px[p] = px[p];
		h.vMiniPos[id].py[p] = py[p];
	}
	
	//  🌍 minimap  circle/rect rot
	int qb = app->iCurPoses[base], qc = app->iCurPoses[id];
	if (h.hrMap && pSet->trackmap && main)
	{
		h.hrMap->begin();
		if (!bZoom)
			for (p=0; p < 4; ++p)
			{	h.hrMap->position(tp[p][0],tp[p][1], 0);
				h.hrMap->texUV(tc[p][0], tc[p][1]);
				// h.moMap->texUV(tc[p][0],tc[p][1], 0);  // uv2
			}
		else
		{	Vector2 mp(-app->carPoses[qb][base].pos[2], app->carPoses[qb][base].pos[0]);
			float t = app->sc->tds[0].fTerWorldSize;  // 1st ter-
			float t2 = t * 0.5;
			float xc =  (mp.x + t2) / t,
				  yc = -(mp.y + t2) / t + 1.f;
			
			for (p=0; p < 4; ++p)
			{	h.hrMap->position(tp[p][0],tp[p][1], 0);
				float u = cx[p] +xc, v = -cy[p] -yc;
				h.hrMap->texUV(u, v);
				// h.moMap->texUV(tc[p][0],tc[p][1], 1);  // uv2
		}	}
		h.hrMap->end();
	}


	///  🌍🔺 minimap car pos  x,y = -1..1
	Vector2 mp(-app->carPoses[qc][id].pos[2], app->carPoses[qc][id].pos[0]);

	//  other cars in player's car view space
	if (!main && bZoom)
	{
		Vector2 plr(-app->carPoses[qb][base].pos[2], app->carPoses[qb][base].pos[0]);
		mp -= plr;  mp *= pSet->zoom_minimap;

		if (bRot)
		{
			float a = angBase * PI_d/180.f;  Vector2 np;
			np.x = mp.x*cosf(a) - mp.y*sinf(a);  // rotate
			np.y = mp.x*sinf(a) + mp.y*cosf(a);  mp = -np;
		}
	}
	float xp =  (mp.x - minX)*scX*2.f-1.f,
		  yp = -(mp.y - minY)*scY*2.f+1.f;

	//  clamp to circle
	if (bZoom /*&& bRot*/)
	{
		float d = xp*xp + yp*yp;
		const float dd = pSet->mini_border ? 0.95f : 0.85f;
		if (d > dd*dd)
		{	d = dd/sqrt(d);
			xp *= d;  yp *= d;
		}
	}else
	{	// clamp to square
		xp = std::min(1.f, std::max(-1.f, xp));
		yp = std::min(1.f, std::max(-1.f, yp));
	}
	
	//  visible
	int cg = app->isGhost2nd && !app->bRplPlay &&
		app->carModels[id]->cType == CarModel::CT_GHOST &&
		id < app->carModels.size()-1 ? 1 : 0;

	bool hide = !app->carModels[id+cg]->bVisible;
	if (hide)
	{	h.vMiniPos[id].x = -100.f;
		h.vMiniPos[id].y = 0.f;
	}
	else if (bZoom && main)
	{	h.vMiniPos[id].x = 0.f;
		h.vMiniPos[id].y = 0.f;
	}else
	{	h.vMiniPos[id].x = xp;
		h.vMiniPos[id].y = yp;
	}
}
