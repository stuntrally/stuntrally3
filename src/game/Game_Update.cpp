#include "pch.h"
#include "Def_Str.h"
#include "CScene.h"
#include "SceneXml.h"
#include "CData.h"
#include "settings.h"
#include "CHud.h"
#include "CGui.h"
#include "GuiCom.h"
#include "CGame.h"
#include "game.h"
#include "SoundMgr.h"
#include "CarPosInfo.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "carcontrolmap_local.h"
#include "CInput.h"
#include "Road.h"
#include "PaceNotes.h"
#include "GraphicsSystem.h"
#include "BtOgreExtras.h"
#include "gameclient.hpp"

#include <OgreVector3.h>
#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindow.h>
#include "Terra.h"
#include <OgreAtmosphere2Npr.h>

#include "ICSInputControlSystem.h"
#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_Window.h>
#include <string>
using namespace Ogre;
using namespace std;


//-----------------------------------------------------------------------------------------------------------------------------
//  💫 Update  frame
//-----------------------------------------------------------------------------------------------------------------------------
void App::update( float dt )
{
	fLastFrameDT = dt;

	#if 0  //** log big dt
	if (dt > 0.02)
		LogO("dt "+fToStr(dt,3,5));
	#endif

	//  fixes white texture flashes
	if (bLoading)
	{
		auto* texMgr = mRoot->getRenderSystem()->getTextureGpuManager();
		texMgr->waitForStreamingCompletion();
	}

	//  🕹️ Input upd  ----
	mInputCtrl->update(dt);
	for (int i=0; i<4; ++i)
		mInputCtrlPlayer[i]->update(dt);


	scn->UpdSun(dt);  // ⛅

	for (int i=0; i < CScene::SK_ALL; ++i)
	if (scn->ndSky[i])
	{	scn->ndSky[i]->setPosition(mCamera->getPosition());
		scn->ndSky[i]->_getFullTransformUpdated();
	}


	//  ⏳ Loading steps --------
	if (bLoading)
	{
		NewGameDoLoad();
		// PROFILER.endBlock(" frameSt");
		// return;  //?
	}//else   //!
	
	{
		///  loading end  ------
		if (!bLoading)
		{
			const int iFr = 3;
			if (iLoad1stFrames >= 0)
			{	++iLoad1stFrames;
				if (iLoad1stFrames == iFr)
				{
					// LoadingOff();  // hide loading overlay
					// mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);
					gui->Ch_LoadEnd();
					bLoadingEnd = true;
					iLoad1stFrames = -1;  // for refl
				}
			}else if (iLoad1stFrames >= -1)
			{
				--iLoad1stFrames;  // -2 end
				LoadingOff();  // hide loading overlay

				//  rpl ctrl show
				if (mWndRpl && !bLoading)
					mWndRpl->setVisible(bRplPlay && bRplWnd);
			}
		}

		///  🎛️ Gui  --------
		if (gui)
			gui->GuiUpdate();

		//  input
		if (isFocGui && pSet->iMenu == MN_Options &&
			mWndTabsOpts->getIndexSelected() == TABo_Input)
			gui->UpdateInputBars();

		
		if (bWindowResized && gcom)
		{	bWindowResized = false;
			LogO("[]-- WindowResized");
			// todo: GraphicsSystem SDL_WINDOWEVENT_RESIZED:  bWindowResized = true;  // set ?

			gcom->ResizeOptWnd();
			gcom->SizeGUI();
			gcom->updTrkListDim();
			gui->updChampListDim();  // resize lists
			gui->slSSS(0);
			gui->listCarChng(gui->carList,0);  // had wrong size
			bRecreateHUD = true;
			
			/*if (mSplitMgr)  //  reassign car cameras from new viewports
			scn->mWaterRTT->setViewerCamera(cam1);
			}*/
			///gui->InitCarPrv();
		}


		//  ⏱️ HUD  ----
		//  hud update sizes, after res change
		if (bRecreateHUD)
		{	bRecreateHUD = false;
			
			hud->Destroy();  hud->Create();
			bSizeHUD = true;
		}
		if (bSizeHUD)
		{	bSizeHUD = false;
			
			hud->Size();
		}

		if (!bLoading)
		{
			for (int c = 0; c < carModels.size(); ++c)
				hud->Update(c, dt);
			hud->Update(-1, dt);
		

			///  📉 graphs update  -._/\_-.
			if (pSet->show_graphs && graphs.size() > 0)
			{
				GraphsNewVals();
				UpdateGraphs();
			}
		}

		//  🚗 Car poses
		if (pGame && iLoad1stFrames == -2)
		{
			updatePoses(dt);

			if (pSet->check_arrow && !bRplPlay && !carModels.empty())
				hud->arrow.Update(carModels[0], dt);

			//  cam info text
			if (pSet->show_cam && !carModels.empty() && hud->txCamInfo)
			{	FollowCamera* cam = carModels[0]->fCam;
				if (cam)
				{	bool vis = cam->updInfo(dt) && !isFocGui;
					if (vis)
						hud->txCamInfo->setCaption(String(cam->ss));
					hud->txCamInfo->setVisible(vis);
			}	}
		}

		
		//  📃 keys up/dn, for gui lists
		//------------------------------------------------------------------------
		static float fUp = 0.f, fDn = 0.f, fPgUp = 0.f, fPgDn = 0.f;
		const float rpt = -0.15f;  // -0.15f s delay
		const int d = alt ? 16 : ctrl ? 4  : 1,
		         pg = alt ? 64 : ctrl ? 32 : 8;

		if (isFocGui && !isTweak() && !bLoading &&
			pSet->iMenu >= MN_Single && pSet->iMenu <= MN_Chall)
		{
			if (keyDown)  fDn += dt;  else  if (keyPgDown)  fPgDn += dt;  else
			if (keyUp)    fUp += dt;  else  if (keyPgUp)    fPgUp += dt;  else
			{	fUp = 0.f;  fDn = 0.f;  fPgUp = 0.f;  fPgDn = 0.f;  }
			if (fUp   > 0.f) {  gui->LNext(-d);   fUp = rpt;  }
			if (fDn   > 0.f) {  gui->LNext( d);   fDn = rpt;  }
			if (fPgUp > 0.f) {  gui->LNext(-pg);  fPgUp = rpt;  }
			if (fPgDn > 0.f) {  gui->LNext( pg);  fPgDn = rpt;  }
		}


		///  Gui updates from Networking
		gui->UpdGuiNetw();

		//  Signal loading finished to the peers
		if (mClient && bLoadingEnd)
		{
			bLoadingEnd = false;
			mClient->loadingFinished();
		}


		//  🌧️ Update rain/snow - depends on camera
		if (pSet->particles)
			scn->UpdateWeather(mCamera, 1.f/dt);
		

		//**  bullet bebug draw
		if (dbgdraw)  // DBG_DrawWireframe
		{
			dbgdraw->setDebugMode(1); //pSet->bltDebug ? 1 /*+(1<<13) 255*/ : 0);
			dbgdraw->step();
		}


		//  🛣️ Road  upd lods  ----
		static float roadUpdTm = 0.f;
		if (scn->road)
		{
			//PROFILER.beginBlock("g.road");  // below 0.0 ms

			//  more than 1: in pre viewport, each frame
			// if (mSplitMgr->mNumViewports == 1)
			{
				roadUpdTm += dt;
				if (roadUpdTm > 0.1f)  // interval [sec]
				{
					roadUpdTm = 0.f;
					for (auto r : scn->roads)
						r->UpdLodVis(pSet->road_dist);
					
					//  trail upd lods
					if (scn->trail && pSet->trail_show && !bHideHudTrail)
						scn->trail->UpdLodVis();
				}
			}
			//PROFILER.endBlock("g.road");
		}

		//  🚦 pace upd vis  ~ ~ ~
		if (scn->pace && !bLoading && !carModels.empty())
		{	
			const CarModel* cm = *carModels.begin();
			Vector3 p = cm->ndMain->getPosition();
			float vel = bRplPlay ? frm[0].vel : cm->pCar->GetSpeedometer();
			scn->pace->carVel = vel;
			scn->pace->rewind = bRplPlay ? false : cm->pCar->bRewind;
			scn->pace->UpdVis(p);
		}

		UpdCubeRefl();  // 🔮💫


		//  🌧️ stop rain/snow when paused
		if (scn->pr && scn->pr2 && pGame)
		{
			if (pGame->pause)
				{	 scn->pr->setSpeedFactor(0.f);  scn->pr2->setSpeedFactor(0.f);  }
			else{	 scn->pr->setSpeedFactor(1.f);  scn->pr2->setSpeedFactor(1.f);  }
		}


		//  📽️ replay forward,backward keys
		if (bRplPlay)
		{
			isFocRpl = ctrl;
			bool le = keyDown || keyPgDown, ri = keyUp || keyPgUp, ctrlN = ctrl && (le || ri);
			int ta = ((le || gui->bRplBack) ? -2 : 0) + ((ri || gui->bRplFwd) ? 2 : 0);
			if (ta)
			{	double tadd = ta;
				tadd *= (shift ? 0.2 : 1) * (ctrlN ? 4 : 1) * (alt ? 8 : 1);  // multipliers
				if (!bRplPause)  tadd -= 1;  // play compensate
				double t = pGame->timer.GetReplayTime(0), len = replay.GetTimeLength();
				t += tadd * dt;  // add
				if (t < 0.0)  t += len;  // cycle
				if (t > len)  t -= len;
				pGame->timer.SetReplayTime(0, t);
			}
		}


		//  ⚫📉
		bool tireEd = updateTireEdit();

		///  ⛰️ Terrain  ----
		if (mGraphicsSystem->getRenderWindow()->isVisible() )
		{
			const float lightEpsilon = 0.0001f;  //** 0.0f slow
			for (auto ter : scn->ters)
			if (ter)	// todo: move to ws listener for splitscr..
				ter->update( !scn->sun ? -Vector3::UNIT_Y :
					scn->sun->getDerivedDirectionUpdated(), 0, lightEpsilon );
		}
	}

	UpdFpsText(dt);
}


//  ⚫📉 Tire Edit keys
//...................................................................
bool App::updateTireEdit()
{
	bool edit =
		(pSet->graphs_type == Gh_TireEdit ||
		 pSet->graphs_type == Gh_Tires4Edit) &&
		carModels.size() > 0 && !mWndTweak->getVisible();

	if (!edit)  return edit;

	int k = (keyMul ? -1 : 0)
		  + (keyDiv ?  1 : 0);
	if (!k)  return edit;

	double mul = shift ? 0.2 : (ctrl ? 4.0 : 1.0);
	mul *= 0.005;  // par

	CARDYNAMICS& cd = carModels[0]->pCar->dynamics;
	CARTIRE* tire = cd.GetTire(FRONT_LEFT);
	if (iEdTire == 1)  // longit |
	{
		Dbl& val = tire->longitudinal[iCurLong];  // modify 1st
		val += mul*k * (1 + abs(val));
		for (int i=1; i<4; ++i)
			cd.GetTire(WHEEL_POSITION(i))->longitudinal[iCurLong] = val;  // copy for rest
	}
	else if (iEdTire == 0)  // lateral --
	{
		Dbl& val = tire->lateral[iCurLat];
		val += mul*k * (1 + abs(val));
		for (int i=1; i<4; ++i)
			cd.GetTire(WHEEL_POSITION(i))->lateral[iCurLat] = val;
	}
	else  // align o
	{
		Dbl& val = tire->aligning[iCurAlign];
		val += mul*k * (1 + abs(val));
		for (int i=1; i<4; ++i)
			cd.GetTire(WHEEL_POSITION(i))->aligning[iCurAlign] = val;
	}

	//  update hat, 1st
	tire->CalculateSigmaHatAlphaHat();
	for (int i=1; i<4; ++i)  // copy for rest
	{	cd.GetTire(WHEEL_POSITION(i))->sigma_hat = tire->sigma_hat;
		cd.GetTire(WHEEL_POSITION(i))->alpha_hat = tire->alpha_hat;
	}
	iUpdTireGr = 1;

	return edit;
}
