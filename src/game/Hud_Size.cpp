#include "pch.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
#include "RenderConst.h"
// #include "GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "game.h"
#include "Road.h"
// #include "SplitScreen.h"

#include <OgreWindow.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
// #include <OgreOverlayManager.h>
// #include <OgreOverlayElement.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;
using namespace MyGUI;


///  🗜️ HUD resize
//---------------------------------------------------------------------------------------------------------------
void CHud::Size()
{
	int cnt = pSet->game.local_players;
	#ifdef DEBUG
	assert(cnt <= hud.size());
	#endif
	//  for each car
	for (int c=0; c < cnt; ++c)
	{
		const auto& dim = app->mDims[c];
		Hud& h = hud[c];
		float wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
		asp = wx/wy;

		//  ⏲️ gauges
		Real xcRpm=0.f,ycRpm=0.f, xcRpmL=0.f,
			 xcVel=0.f,ycVel=0.f, ygMax=0.f, xBFuel=0.f;  // -1..1
		{
			Real sc = pSet->size_gauges * dim.avgsize1;
			Real spx = sc * 1.1f, spy = spx*asp;
			//xcRpm = dim.left + spx;   ycRpm =-dim.bottom + spy;
			xcRpm = dim.right1 - spx*0.5f;  ycRpm =-dim.bottom1 + spy*2.f;
			xcRpmL= dim.right1 - spx;
			xcVel = dim.right1 - spx;       ycVel =-dim.bottom1 + spy*0.9f;
			ygMax = ycVel - sc;  xBFuel = xcVel - sc;

			h.vcRpm = Vector2(xcRpm,ycRpm);  // store for hr updates
			h.vcVel = Vector2(xcVel,ycVel);
			h.fScale = sc;
		}
		
		//  🌍 minimap
		Real sc = pSet->size_minimap * dim.avgsize1;
		const Real marg = 1.3f; //1.05f;  // from border
		Real fMiniX = dim.left1 + sc * marg;  //(dim.right - sc * marg);
		Real fMiniY =-dim.bottom1 + sc*asp * marg;  //-dim.top - sc*asp * marg;
		Real miniTopY = fMiniY + sc*asp;

		if (h.ndMap)
		{	h.ndMap->setScale(sc, sc*asp,1);
			h.ndMap->setPosition(Vector3(fMiniX,fMiniY,0.f));
		}
	
		//  current viewport  min,max x,y in pixels  0..1 * wx,wy
		int xMin = dim.left0* wx, xMax = dim.right0 * wx,
			yMin = dim.top0 * wy, yMax = dim.bottom0* wy;
		int my = (1.f-ygMax)*0.5f*wy;  // gauge bottom y

		if (!h.txVel)  continue;

		//  positioning, min yMax - dont go below viewport bottom
		int vv = pSet->gauges_type > 0 ? -45 : 40;
		int gx = (xcRpm+1.f)*0.5f*wx - 10, gy = (-ycRpm+1.f)*0.5f*wy +22;
		int gxL=(xcRpmL+1.f)*0.5f*wx - 10;
		int vx = (xcVel+1.f)*0.5f*wx + vv, vy = std::min(yMax -91, my - 15);
		int bx =(xBFuel+1.f)*0.5f*wx - 10, by = std::min(yMax -36, my + 5);
			vx = std::min(vx, xMax -100);
			bx = std::min(bx, xMax -180);  // not too near to vel

		//  ⏲️ gear, vel
		if (h.txGear)  h.txGear->setPosition(gx,gy +10);
		if (h.bckGear) h.bckGear->setPosition(gx-12,gy +10);

		if (h.bckVel)  h.bckVel->setPosition(vx-32,vy-6);
		h.txVel->setPosition(vx,vy);  //h.bckVel

		#if 0
		h.txRewind ->setPosition(bx,   by);
		h.icoRewind->setPosition(bx+50,by-5);
		#endif

		if (h.txDamage)  // 🔨
		{	h.txDamage ->setPosition(gxL-83-72, gy+10-5);
			h.icoDamage->setPosition(gxL-83+57, gy+10-5);
			h.imgDamage->setPosition(gxL-83-26, gy+10-7);
		}
		if (h.txBFuel)  // 💨
		{	h.txBFuel ->setPosition(gxL-83-74, gy-60);
			h.icoBFuel->setPosition(gxL-83+57, gy-60);
			//h.icoBInf->setPosition(gxL-83+14,gy-60-5+2);
		}

		//  ⏱️ Times
		bool hasLaps = pSet->game.hasLaps();
		int w = 160, tx = xMin + 40, ty = yMin + 55;  //40
		h.bckTimes->setPosition(tx-20,ty);
		//tx = 24;  ty = (hasLaps ? 16 : 4);
		h.txTimTxt->setPosition(tx,ty);
		h.txTimes->setPosition(tx+w,ty);

		//  🏁 lap result
		int lx = xMax - 360, ly = ty;
		h.bckLap->setPosition(lx-14,ly-8);
		h.txLapTxt->setPosition(lx,ly);
		h.txLap->setPosition(lx+w,ly);
	/*		
		//  opp list
		//int ox = itx + 5, oy = (ycRpm+1.f)*0.5f*wy - 10;
		int ox = xMin + 40, oy = (-miniTopY+1.f)*0.5f*wy - 5;  //par
		h.xOpp = ox;  h.yOpp = oy;
		h.lastOppH = -1;
		//h.bckOpp->setPosition(ox,oy -2);  //h.bckOpp->setSize(230, plr*25 +4);
		//for (int n=0; n<3; ++n)
		//	h.txOpp[n]->setPosition(n*65+5,0);
	*/
		//  ❌ warn  🥇 win
		int ox = (xMax-xMin)/2 + xMin - 200;  int oy = yMin + 15;
		h.bckWarn->setPosition(ox,oy);
		h.bckPlace->setPosition(ox,oy + 40);
		
		h.txCountdown->setPosition((xMax-xMin)/2 -100, (yMax-yMin)/2 -60);

		//  🎥 camera info
		if (h.txCam)
			h.txCam->setPosition(xMax-300,yMax-10);
		//  abs,tcs
		h.txAbs->setPosition(xMin+160,yMax-30);
		h.txTcs->setPosition(xMin+220,yMax-30);
	}
	if (txCamInfo)
	{	txCamInfo->setPosition(270, app->mWindow->getHeight() -100);
		bckMsg->setPosition(256,0);
	}
	//  debug txt  === ===
	//..
}


//  🔁 HUD show/hide
//---------------------------------------------------------------------------------------------------------------
void CHud::Show(bool hideAll)
{
	if (!txDbgSurf)  // not inited
		return;
	if (hideAll /*|| app->iLoad1stFrames > -1*/)  // still loading
	{
		app->bckFps->setVisible(false);
		app->txFps->setVisible(false);
		if (bckMsg)
		{
			txCamInfo->setVisible(false);
			bckMsg->setVisible(false);

			for (auto h:hud)
				if (h.parent)
					h.parent->setVisible(false);
		}
		app->hideMouse();
		if (app->mWndRpl)  app->mWndRpl->setVisible(false);
		if (app->mWndRplTxt)  app->mWndRplTxt->setVisible(false);
		return;
	}

	for (int i=0; i < 4; ++i)
		txDbgCar[i]->setVisible(pSet->car_dbgtxt);
	txDbgSurf->setVisible(pSet->car_dbgsurf);
	
	txDbgProfTim->setVisible(pSet->profilerTxt);
	txDbgProfBlt->setVisible(pSet->bltProfilerTxt);

	app->bckFps->setVisible(pSet->fps_bar);
	app->txFps->setVisible(pSet->fps_bar);
	if (bckMsg)
	{
		bool cam = pSet->show_cam && !app->isFocGui, times = pSet->show_times;
		//bool opp = pSet->show_opponents && (app->scn->road && app->scn->road->getNumPoints() > 0);
		bool bfuel = pSet->game.boost_type >= 1; // && pSet->game.boost_type <= 3;
		bool btxt = pSet->game.boost_type == 1 || pSet->game.boost_type == 2;
		//bool binf = pSet->game.boost_type == 3;
		bool bdmg = pSet->game.damage_type > 0;
		txCamInfo->setVisible(cam);

		bool show = pSet->show_gauges;
		for (int c=0; c < hud.size(); ++c)
		{	Hud& h = hud[c];

			if (h.parent)
			{	h.parent->setVisible(true);
			
				if (h.bckGear) h.bckGear->setVisible(pSet->show_digits);
				if (h.txGear)  h.txGear->setVisible(pSet->show_digits);

				if (h.bckVel)  h.bckVel->setVisible(pSet->show_digits);
				h.txVel->setVisible(pSet->show_digits);
				
				if (h.txBFuel){  h.txBFuel->setVisible(show && btxt);
					h.icoBFuel->setVisible(show && bfuel);  /*h.icoBInf->setVisible(show && binf);*/  }
				
				if (h.txDamage){  h.txDamage->setVisible(show && bdmg);
					h.icoDamage->setVisible(show && bdmg);
					h.imgDamage->setVisible(show && bdmg);  }
				//txRewind; icoRewind;

				h.ndGauges->setVisible(show);
				h.ndMap->setVisible(pSet->trackmap);
				h.txTimes->setVisible(times);  h.txTimTxt->setVisible(times);  h.bckTimes->setVisible(times);
				h.txLap->setVisible(times);  h.txLapTxt->setVisible(times);  h.bckLap->setVisible(times);

				/*h.bckOpp->setVisible(opp);
				h.txOpp[0]->setVisible(opp);  h.txOpp[1]->setVisible(opp);  h.txOpp[2]->setVisible(opp);*/
				if (h.txCam)  h.txCam->setVisible(cam);
		}	}
	}
	if (ndPos)  ndPos->setVisible(pSet->trackmap);
	
	app->updMouse();
	
	//  📽️ replay controls
	if (app->mWndRpl && !app->bLoading)
		app->mWndRpl->setVisible(app->bRplPlay && app->bRplWnd);
	//  lesson replay  >> >
	if (app->mWndRplTxt && !app->bLoading && app->gui->bLesson)
		app->mWndRplTxt->setVisible(app->bRplPlay);
}


/*void CHud::ShowVp(bool vp)	//?
{
	// show/hide for render viewport / gui viewport
	// first show everything
	Show(false);  // todo: don't here
	// now hide things we dont want
	if (!vp)
	{
		/// for gui viewport ----------------------
		//if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
	}else{
		/// for render viewport ---------
		//if (ovCam)  ovCam->hide();
		//bckFps->setVisible(false);
	}
}
*/