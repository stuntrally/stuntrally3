#include "pch.h"
#include "CHud.h"
#include "OgreVector3.h"
#include "Game.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "SDL_scancode.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreWindow.h"

#include "Terra/Terra.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include "OgreGpuProgramManager.h"

#include "OgreAtmosphere2Npr.h"

#include "game.h"  // snd
#include "SoundMgr.h"
#include "CarPosInfo.h"
#include "CGame.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "carcontrolmap_local.h"
#include "Road.h"

#include "Def_Str.h"
#include "CScene.h"
#include "SceneXml.h"
#include "TracksXml.h"
#include "CData.h"
#include "SceneClasses.h"
#include "settings.h"
using namespace Ogre;


//  Update  frame
//-----------------------------------------------------------------------------------------------------------------------------
void OgreGame::update( float dt )
{
	pApp->scn->UpdSun();

	if (pApp->bLoading)
	{
		pApp->NewGameDoLoad();
		// PROFILER.endBlock(" frameSt");
		// return;  //?
	}
	else 
	{
		///  loading end  ------
		const int iFr = 3;
		if (pApp->iLoad1stFrames >= 0)
		{	++pApp->iLoad1stFrames;
			if (pApp->iLoad1stFrames == iFr)
			{
				// LoadingOff();  // hide loading overlay
				// mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);
				// gui->Ch_LoadEnd();
				pApp->bLoadingEnd = true;
				// iLoad1stFrames = -1;  // for refl
			}
		}else if (pApp->iLoad1stFrames >= -1)
		{
			--pApp->iLoad1stFrames;  // -2 end

			// imgLoad->setVisible(false);  // hide back imgs
			// if (imgBack)
			//     imgBack->setVisible(false);
		}

		//  HUD
		if (pApp->bSizeHUD)
		{	pApp->bSizeHUD = false;
			
			pApp->hud->Size();
		}
		pApp->hud->Update(0, dt);
		pApp->hud->Update(-1, dt);
	}


	if (pGame && pApp && !pApp->bLoading)//pApp->iLoad1stFrames == -2)
	{
		//  set game inputs .. // todo: use oics
		pApp->inputs[A_Throttle] = mArrows[2];
		pApp->inputs[A_Brake] = mArrows[3];
		pApp->inputs[A_Steering] = 0.5f * (1 + mArrows[1] - mArrows[0]);
		pApp->inputs[A_HandBrake] = mArrows[4];
		pApp->inputs[A_Boost] = mArrows[5];
		pApp->inputs[A_Flip] = 0.5f * (1 + mArrows[7] - mArrows[6]);
		pApp->inputs[A_NextCamera] = mArrows[8];
		pApp->inputs[A_PrevCamera] = mArrows[9];
		pApp->inputs[A_Rewind] = mArrows[10];

		//  multi thread
		// if (pSet->multi_thr == 1 && pGame && !bLoading)
		{
			pApp->updatePoses(dt);
		}
	}


	if (pSet->particles)
		pApp->scn->UpdateWeather(mGraphicsSystem->getCamera());  //, 1.f/dt
	

	//**  bullet bebug draw
	if (pApp->dbgdraw)  // DBG_DrawWireframe
	{
		pApp->dbgdraw->setDebugMode(1); //pSet->bltDebug ? 1 /*+(1<<13) 255*/ : 0);
		pApp->dbgdraw->step();
	}

	//  road upd lods
	static float roadUpdTm = 0.f;
	if (pApp->scn->road)
	{
		//PROFILER.beginBlock("g.road");  // below 0.0 ms

		//  more than 1: in pre viewport, each frame
		// if (mSplitMgr->mNumViewports == 1)
		{
			roadUpdTm += dt;
			if (roadUpdTm > 0.1f)  // interval [sec]
			{
				roadUpdTm = 0.f;
				for (auto r : pApp->scn->roads)
					r->UpdLodVis(pSet->road_dist);
				
				//  trail upd lods
				// if (scn->trail && pSet->trail_show && !bHideHudTrail)
					// scn->trail->UpdLodVis();
			}
		}
		//PROFILER.endBlock("g.road");
	}
	

	//  Keys  params  ----
	float mul = shift ? 0.2f : ctrl ? 3.f : 1.f;
	int d = right ? 1 : left ? -1 : 0;
	if (d && pApp->scn->atmo)
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
	auto sun = pApp->scn->sun;
	if (any)
		pApp->scn->UpdSun();

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

	generateDebugText();

	TutorialGameState::update( dt );
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
//  Key events
//-----------------------------------------------------------------------------------------------------------------------------
void OgreGame::keyPressed( const SDL_KeyboardEvent &arg )
{
	int itrk = 0, icar = 0;
	switch (arg.keysym.scancode)
	{
	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT:  shift = true;  break;  // mod
	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RCTRL:   ctrl = true;   break;


	case SDL_SCANCODE_LEFT:   mArrows[0] = 1;  break;  // car
	case SDL_SCANCODE_RIGHT:  mArrows[1] = 1;  break;
	case SDL_SCANCODE_UP:     mArrows[2] = 1;  break;
	case SDL_SCANCODE_DOWN:   mArrows[3] = 1;  break;

	case SDL_SCANCODE_SPACE:  mArrows[4] = 1;  break;
	case SDL_SCANCODE_LCTRL:  mArrows[5] = 1;  break;
	case SDL_SCANCODE_Q:      mArrows[6] = 1;  break;
	case SDL_SCANCODE_W:      mArrows[7] = 1;  break;

	case SDL_SCANCODE_C:      mArrows[8] = 1;  break;
	case SDL_SCANCODE_X:      mArrows[9] = 1;  break;
	case SDL_SCANCODE_INSERT: mArrows[10] = 1;  break;


	case SDL_SCANCODE_1:  itrk = -1;  break;
	case SDL_SCANCODE_2:  itrk =  1;  break;
	case SDL_SCANCODE_3:  icar = -1;  break;
	case SDL_SCANCODE_4:  icar =  1;  break;


	case SDL_SCANCODE_HOME:  left  = true;  break;  // params
	case SDL_SCANCODE_END:   right = true;  break;
	case SDL_SCANCODE_PAGEUP:     --param;  break;
	case SDL_SCANCODE_PAGEDOWN:   ++param;  break;

	case SDL_SCANCODE_KP_PLUS:      mKeys[0] = 1;  break;  // sun
	case SDL_SCANCODE_KP_MINUS:     mKeys[1] = 1;  break;
	case SDL_SCANCODE_KP_MULTIPLY:  mKeys[2] = 1;  break;
	case SDL_SCANCODE_KP_DIVIDE:    mKeys[3] = 1;  break;

	// case SDL_SCANCODE_SPACE:  // snd test
		// pGame->snd_lapbest->start();  //)
		// break;

	case SDL_SCANCODE_ESCAPE:
		if (pSet->escquit)
		{	pApp->mShutDown = true;
			mGraphicsSystem->setQuit();
		}	break;


	//###  restart game, new track or car
	case SDL_SCANCODE_RETURN:
	case SDL_SCANCODE_KP_ENTER:
		pApp->NewGame(shift);  return;

	//###  reset game - fast (same track & cars)
	case SDL_SCANCODE_BACKSPACE:
	{
		for (int c=0; c < pApp->carModels.size(); ++c)
		{
			CarModel* cm = pApp->carModels[c];
			if (cm->pCar)
				cm->pCar->bResetPos = true;
			if (cm->iLoopLastCam != -1 && cm->fCam)
			{	cm->fCam->setCamera(cm->iLoopLastCam);  //o restore
				cm->iLoopLastCam = -1;  }
			cm->First();
			cm->ResetChecks();
			cm->iWonPlace = 0;  cm->iWonPlaceOld = 0;
			cm->iWonMsgTime = 0.f;
		}
		pGame->timer.Reset(-1);
		// pGame->timer.pretime = mClient ? 2.0f : pSet->game.pre_time;  // same for all multi players

		// carIdWin = 1;  //
		//; ghost.Clear();  replay.Clear();
	}	return;;


	//**  terrain wireframe toggle
	case SDL_SCANCODE_R:
	{
		wireTerrain = !wireTerrain;
		Root *root = mGraphicsSystem->getRoot();
		HlmsManager *hlmsManager = root->getHlmsManager();
		HlmsDatablock *datablock = 0;
		datablock = hlmsManager->getDatablock( "TerraExampleMaterial2" );
		if (wireTerrain)
		{
			datablock = hlmsManager->getHlms( HLMS_USER3 )->getDefaultDatablock();
			datablock->setMacroblock( macroblockWire );
		}
		mTerra->setDatablock( datablock );
	}   break;


	//  sky
	case SDL_SCANCODE_K:  
		if (pApp->scn->ndSky)
			pApp->scn->DestroySkyDome();
		else
			pApp->scn->CreateSkyDome("sky-clearday1", 0.f);
		break;
	}


	//***  pick track, car  ***
	int mul4 = shift ? 10 : ctrl ? 4 : 1;
	const auto* data = pApp->scn->data;
	int tracks = data->tracks->trks.size();
	int cars = data->cars->cars.size();

	if (itrk)
	{	idTrack = (idTrack + mul4 * itrk + tracks) % tracks;
		pSet->gui.track = data->tracks->trks[idTrack].name;
	}
	if (icar)
	{	idCar = (idCar + mul4 * icar + cars) % cars;
		pSet->gui.car[0] = data->cars->cars[idCar].id;
	}

	TutorialGameState::keyPressed( arg );
}


void OgreGame::keyReleased( const SDL_KeyboardEvent &arg )
{
	switch (arg.keysym.scancode)
	{
	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT:  shift = false;  break;  // mod
	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RCTRL:   ctrl = false;   break;


	case SDL_SCANCODE_LEFT:   mArrows[0] = 0;  break;  // car
	case SDL_SCANCODE_RIGHT:  mArrows[1] = 0;  break;
	case SDL_SCANCODE_UP:     mArrows[2] = 0;  break;
	case SDL_SCANCODE_DOWN:   mArrows[3] = 0;  break;

	case SDL_SCANCODE_SPACE:  mArrows[4] = 0;  break;
	case SDL_SCANCODE_LCTRL:  mArrows[5] = 0;  break;
	case SDL_SCANCODE_Q:      mArrows[6] = 0;  break;
	case SDL_SCANCODE_W:      mArrows[7] = 0;  break;
	
	case SDL_SCANCODE_C:      mArrows[8] = 0;  break;
	case SDL_SCANCODE_X:      mArrows[9] = 0;  break;
	case SDL_SCANCODE_INSERT: mArrows[10] = 0;  break;


	case SDL_SCANCODE_HOME:  left = false;   break;  // params
	case SDL_SCANCODE_END:   right = false;  break;

	case SDL_SCANCODE_KP_PLUS:      mKeys[0] = 0;  break;  // sun
	case SDL_SCANCODE_KP_MINUS:     mKeys[1] = 0;  break;
	case SDL_SCANCODE_KP_MULTIPLY:  mKeys[2] = 0;  break;
	case SDL_SCANCODE_KP_DIVIDE:    mKeys[3] = 0;  break;


	case SDL_SCANCODE_F4:
	if (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
	{
		// Hot reload of Terra shaders
		Root *root = mGraphicsSystem->getRoot();
		HlmsManager *hlmsManager = root->getHlmsManager();

		Hlms *hlms = hlmsManager->getHlms( HLMS_USER3 );
		GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}   break;
	}

	TutorialGameState::keyReleased( arg );
}

#pragma GCC diagnostic pop
