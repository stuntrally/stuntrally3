#include "pch.h"
#include "CHud.h"
#include "CGui.h"
#include "GuiCom.h"
#include "CGame.h"
#include "GraphicsSystem.h"

#include "game.h"  // snd
#include "SoundMgr.h"
#include "CarPosInfo.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "carcontrolmap_local.h"
#include "CInput.h"
#include "Road.h"

#include "Def_Str.h"
#include "CScene.h"
#include "SceneXml.h"
#include "CData.h"
#include "settings.h"

#include <OgreVector3.h>
#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindow.h>

#include "Terra/Terra.h"
#include <OgreAtmosphere2Npr.h>

#include "ICSInputControlSystem.h"
#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_TextBox.h>
#include <string>
using namespace Ogre;
using namespace std;


//  💫 Update  frame
//-----------------------------------------------------------------------------------------------------------------------------
void App::update( float dt )
{
	if (mShutDown)
	{
		mGraphicsSystem->setQuit();
		return;
	}

	//  🕹️ Input upd  ----
	mInputCtrl->update(dt);
	for (int i=0; i<4; ++i)
		mInputCtrlPlayer[i]->update(dt);


	scn->UpdSun();

	//  ⏳ Loading steps --------
	if (bLoading)
	{
		NewGameDoLoad();
		// PROFILER.endBlock(" frameSt");
		// return;  //?
	}else 
	{
		///  loading end  ------
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
			{	std::list<Camera*>::iterator it = mSplitMgr->mCameras.begin();
				for (int i=0; i < carModels.size(); ++i)
					if (carModels[i]->fCam && it != mSplitMgr->mCameras.end())
					{	carModels[i]->fCam->mCamera = *it;  ++it;  }
			}
			if (!mSplitMgr->mCameras.empty())
			{
				Camera* cam1 = *mSplitMgr->mCameras.begin();
				scn->mWaterRTT->setViewerCamera(cam1);
				if (scn->grass)  scn->grass->setCamera(cam1);
				if (scn->trees)  scn->trees->setCamera(cam1);
			}*/
			///gui->InitCarPrv();
		}


		//  ⏱️ HUD  ----
		if (bSizeHUD)
		{	bSizeHUD = false;
			
			hud->Size();
		}
		hud->Update(0, dt);
		hud->Update(-1, dt);
	

		//  car pos
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

		
		//  keys up/dn, for gui lists  ----
		static float dirU = 0.f,dirD = 0.f;
		if (isFocGui && !isTweak() &&
			pSet->iMenu >= MN_Single &&
			pSet->iMenu <= MN_Chall)
		{
			if (down)  dirD += dt;  else
			if (up)    dirU += dt;  else
			{	dirU = 0.f;  dirD = 0.f;  }
			int d = ctrl ? 4 : 1;
			if (dirU > 0.0f) {  gui->LNext(-d);  dirU = -0.15f;  }
			if (dirD > 0.0f) {  gui->LNext( d);  dirD = -0.15f;  }
		}


		if (pSet->particles)
			scn->UpdateWeather(mGraphicsSystem->getCamera());  //, 1.f/dt
		

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


		//  Keys  params  ----
		float mul = shift ? 0.2f : ctrl ? 3.f : 1.f;
		int d = right ? 1 : left ? -1 : 0;
		if (d && scn->atmo)
		{
			SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
			Atmosphere2Npr *atmo = static_cast<Atmosphere2Npr*>( sceneManager->getAtmosphere() );
			if (atmo)
			{
			Atmosphere2Npr::Preset p = atmo->getPreset();

			float mul1 = 1.f + 0.01f * mul * d;  //par
			switch (param)
			{
			// mCamera->setLodBias(0.1);  //** todo: par
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

		//  Light  sun dir  ----
		bool any = false;
		d = mKeys[0] - mKeys[1];
		if (d)
		{	any = true;
			sc->ldPitch += d * mul * 20.f * dt;
			sc->ldPitch = std::max( 0.f, std::min( sc->ldPitch, 180.f ) );
		}
		d = mKeys[2] - mKeys[3];
		if (d)
		{	any = true;
			sc->ldYaw += d * mul * 30.f * dt;
			sc->ldYaw = fmodf( sc->ldYaw, 360.f );
			if( sc->ldYaw < 0.f )
				sc->ldYaw = 360.f + sc->ldYaw;
		}
		auto sun = scn->sun;
		if (any)
			scn->UpdSun();

		///  Terrain  ----
		if (mTerra && mGraphicsSystem->getRenderWindow()->isVisible() )
		{
			// Force update the shadow map every frame to avoid the feeling we're "cheating" the
			// user in this sample with higher framerates than what he may encounter in many of
			// his possible uses.
			const float lightEpsilon = 0.0001f;  //** 0.0f slow
			mTerra->update( !sun ? -Vector3::UNIT_Y :
				sun->getDerivedDirectionUpdated(), lightEpsilon );
		}

	}

	updFpsText();
	updDebugText();
}
