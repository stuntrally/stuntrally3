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
	#if 1
	if (dt > 0.02)  // test big dt
		LogO("dt "+fToStr(dt,3,5));
	#endif


	//  🕹️ Input upd  ----
	mInputCtrl->update(dt);
	for (int i=0; i<4; ++i)
		mInputCtrlPlayer[i]->update(dt);


	scn->UpdSun(dt);  // ⛅

	if (scn->ndSky)
		scn->ndSky->setPosition(mCamera->getPosition());


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

			gcom->ResizeOptWnd();
			gcom->SizeGUI();
			gcom->updTrkListDim();
			gui->updChampListDim();  // resize lists
			gui->slSSS(0);
			gui->listCarChng(gui->carList,0);  // had wrong size
			//; bRecreateHUD = true;
			
			/*if (mSplitMgr)  //  reassign car cameras from new viewports
			scn->mWaterRTT->setViewerCamera(cam1);
			}*/
			///gui->InitCarPrv();
		}


		//  ⏱️ HUD  ----
		if (bSizeHUD)
		{	bSizeHUD = false;
			
			hud->Size();
		}

		for (int c = 0; c < carModels.size(); ++c)
			hud->Update(c, dt);
		hud->Update(-1, dt);
	

		///  📉 graphs update  -._/\_-.
		if (pSet->show_graphs && graphs.size() > 0)
		{
			GraphsNewVals();
			UpdateGraphs();
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
		const float rpt = -0.1f;  // -0.15f s delay
		const int d = alt ? 16 : ctrl ? 4  : 1,
		         pg = alt ? 64 : ctrl ? 32 : 8;

		if (isFocGui && !isTweak() && !bLoading &&
			pSet->iMenu >= MN_Single && pSet->iMenu <= MN_Chall)
		{
			if (down)  fDn += dt;  else  if (pgdown)  fPgDn += dt;  else
			if (up)    fUp += dt;  else  if (pgup)    fPgUp += dt;  else
			{	fUp = 0.f;  fDn = 0.f;  fPgUp = 0.f;  fPgDn = 0.f;  }
			if (fUp   > 0.f) {  gui->LNext(-d);   fUp = rpt;  }
			if (fDn   > 0.f) {  gui->LNext( d);   fDn = rpt;  }
			if (fPgUp > 0.f) {  gui->LNext(-pg);  fPgUp = rpt;  }
			if (fPgDn > 0.f) {  gui->LNext( pg);  fPgDn = rpt;  }
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


		//  🔧 Keys  params  ----
		#if 0  // todo: move to ed gui
		float mul = shift ? 0.2f : ctrl ? 3.f : 1.f;
		int d = right ? 1 : left ? -1 : 0;
		if (0 && d && scn->atmo)  // todo on gui-
		{
			SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
			Atmosphere2Npr *atmo = static_cast<Atmosphere2Npr*>( sceneManager->getAtmosphere() );
			if (atmo)
			{
			Atmosphere2Npr::Preset p = atmo->getPreset();

			float mul1 = 1.f + 0.01f * mul * d;  //par
			switch (param)
			{
			// p.fogHcolor.xyz  fogHparams.xy
			case 0:  p.fogDensity *= mul1;  break;
			case 1:  p.densityCoeff *= mul1;  break;
			case 2:  p.densityDiffusion *= mul1;  break;
			case 3:  p.horizonLimit *= mul1;  break;
			case 4:  p.sunPower *= mul1;  break;
			case 5:  p.skyPower *= mul1;  break;
			case 6:  p.skyColour.x *= mul1;  break;
			case 7:  p.skyColour.y *= mul1;  break;
			case 8:  p.skyColour.z *= mul1;  break;
			case 9:   p.fogBreakMinBrightness *= mul1;  break;
			case 10:  p.fogBreakFalloff *= mul1;  break;
			case 11:  p.linkedLightPower *= mul1;  break;
			case 12:  p.linkedSceneAmbientUpperPower *= mul1;  break;
			case 13:  p.linkedSceneAmbientLowerPower *= mul1;  break;
			case 14:  p.envmapScale *= mul1;  break;
			}
			atmo->setPreset(p);
		}	}
		#endif

		//  ⚫📉
		bool tireEd = updateTireEdit();

		///  ⛰️ Terrain  ----
		if (mGraphicsSystem->getRenderWindow()->isVisible() )
		{
			const float lightEpsilon = 0.0001f;  //** 0.0f slow
			for (auto ter : scn->ters)
			if (ter)
				// Force update the shadow map every frame to avoid the feeling we're "cheating" the
				// user in this sample with higher framerates than what he may encounter in many of
				// his possible uses.
				ter->update( !scn->sun ? -Vector3::UNIT_Y :
					scn->sun->getDerivedDirectionUpdated(), lightEpsilon );
		}
	}

	UpdFpsText();
	updDebugText();
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

	int k = (mKeys[2] ? -1 : 0)
		  + (mKeys[3] ?  1 : 0);
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
