#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "CGui.h"
#include "CGame.h"
#include "game.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "ICSInputControlSystem.h"
#include <SDL_scancode.h>
#include <SDL_keycode.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

#define key(a)  SDL_SCANCODE_##a


//  Key Pressed
//-----------------------------------------------------------------------------------------------------------------------------
void App::keyPressed( const SDL_KeyboardEvent &arg )
{
	//  input
	// mInputCtrl->keyPressed(arg);
	// for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->keyPressed(arg);

	if (!mInputCtrl->keyPressed(arg))
		return;
	for (int i=0; i<4; ++i)
	{
		if (!mInputCtrlPlayer[i]->keyPressed(arg))
			return;
	}

	switch (arg.keysym.scancode)
	{
	case key(LSHIFT):
	case key(RSHIFT):  shift = true;  break;  // mods
	case key(LCTRL):
	case key(RCTRL):  ctrl = true;  break;
	case key(LALT):
	case key(RALT):  alt = true;  break;
	}

	if (arg.keysym.mod == KMOD_LALT)  // alt
	{
		switch (arg.keysym.scancode)
		{
			case key(Q):	gui->GuiShortcut(MN_Single, TAB_Track); break;  // Q Track
			case key(C):	gui->GuiShortcut(MN_Single, TAB_Car);	break;  // C Car

			case key(W):	gui->GuiShortcut(MN_Single, TAB_Setup); break;  // W Setup
			case key(G):	gui->GuiShortcut(MN_Single, TAB_Split);	break;  // G SplitScreen

			case key(J):	gui->GuiShortcut(MN_Tutorial, TAB_Champs);	break;  // J Tutorials
			case key(H):	gui->GuiShortcut(MN_Champ,    TAB_Champs);	break;  // H Champs
			case key(L):	gui->GuiShortcut(MN_Chall,    TAB_Champs);	break;  // L Challenges

			case key(U):	gui->GuiShortcut(MN_Single,   TAB_Multi);	break;  // U Multiplayer
			case key(R):	gui->GuiShortcut(MN_Replays,  1);	break;		  // R Replays

			case key(S):	gui->GuiShortcut(MN_Options, TABo_Screen);	break;  // S Screen
			case key(I):	gui->GuiShortcut(MN_Options, TABo_Input);	break;  // I Input
			case key(V):	gui->GuiShortcut(MN_Options, TABo_View);	break;  // V View

			case key(A):	gui->GuiShortcut(MN_Options, TABo_Graphics);	break;  // A Graphics
			// case key(E):	gui->GuiShortcut(MN_Options, TABo_Graphics,2);	break;  // E -Effects

			case key(T):	gui->GuiShortcut(MN_Options, TABo_Settings);	break;  // T Settings
			case key(O):	gui->GuiShortcut(MN_Options, TABo_Sound);	break;  // O Sound
			case key(K):	gui->GuiShortcut(MN_Options, TABo_Tweak);  break;  // K Tweak
		}
		return;
	}
	else if (arg.keysym.mod == (KMOD_LALT | KMOD_LSHIFT))  // alt shift
	//>--  dev shortcuts, alt-shift - start test tracks
	// if (pSet->dev_keys && alt && shift && !mClient)
	{
		string t;
		switch (arg.keysym.scancode)
		{
			case key(1):  t = "Test2-Asphalt";  break;		case key(2):  t = "TestC9-Jumps";  break;
			case key(3):  t = "Test3-Bumps";  break;		case key(4):  t = "TestC4-Ow";  break;
			case key(5):  t = "TestC7-Oc";  break;
			case key(6):  t = "TestC6-Temp";  break;		case key(7):  t = "Test10-FlatPerf";  break;
			
			case key(Q):  t = "Test6-Fluids";  break;		case key(W):  t = "Test4-TerrainBig";  break;
			case key(E):  t = "TestC8-Align";  break;		case key(R):  t = "Test12-Snow";  break;

			case key(A):  t = "Test1-Flat";  break;			case key(S):  t = "Test7-FluidsSmall";  break;
			case key(D):  t = "TestC13-Rivers";  break;		case key(F):  t = "Test3-Bumps";  break;

			case key(X):  t = "Test8-Objects";  break;		case key(C):  t = "TestC6-Temp";  break;
			case key(V):  t = "TestC10-Pace";  break;
		} 
		if (!t.empty())
		{
			gui->BackFromChs();
			pSet->gui.track = t;  //bPerfTest = false;
			pSet->gui.track_user = false;
			NewGame();
		}
		return;
	}

	switch (arg.keysym.scancode)
	{
	case key(TAB):
		gui->toggleGui(true);  break;  // gui on/off

	case key(DOWN):  down = true;   break;
	case key(UP):    up = true;  break;
	case key(HOME):  left  = true;  break;  // params
	case key(END):   right = true;  break;
	case key(PAGEUP):     --param;  break;
	case key(PAGEDOWN):   ++param;  break;

	case key(KP_PLUS):      mKeys[0] = 1;  break;  // sun
	case key(KP_MINUS):     mKeys[1] = 1;  break;
	case key(KP_MULTIPLY):  mKeys[2] = 1;  break;
	case key(KP_DIVIDE):    mKeys[3] = 1;  break;

	case key(ESCAPE):
		if (pSet->escquit)
			mShutDown = true;
		break;

	//###  restart game, new track or car
	case key(F5):
	case key(RETURN)://
	case key(KP_ENTER):
		NewGame(shift);  return;

	//###  reset game - fast (same track & cars)
	case key(F4):
	{
		for (int c=0; c < carModels.size(); ++c)
		{
			CarModel* cm = carModels[c];
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
		ghost.Clear();  replay.Clear();
	}	return;


	//  wireframe
	case key(F10):
		gui->ckWireframe.Invert();
		break;

	//  Fps
	case key(F11):
		gui->ckFps.Invert();
		bckFps->setVisible(pSet->show_fps);
		txFps->setVisible(pSet->show_fps);
		break;
	}
}


//  Key Released
//-----------------------------------------------------------------------------------------------------------------------------
void App::keyReleased( const SDL_KeyboardEvent &arg )
{
	//  input
	mInputCtrl->keyReleased(arg);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->keyReleased(arg);

	switch (arg.keysym.scancode)
	{
	case key(LSHIFT):
	case key(RSHIFT):  shift = false;  break;  // mods
	case key(LCTRL):
	case key(RCTRL):  ctrl = false;  break;
	case key(LALT):
	case key(RALT):  alt = false;  break;

	case key(DOWN):  down = false;   break;
	case key(UP):    up = false;     break;
	case key(HOME):  left = false;   break;  // params
	case key(END):   right = false;  break;

	case key(KP_PLUS):      mKeys[0] = 0;  break;  // sun
	case key(KP_MINUS):     mKeys[1] = 0;  break;
	case key(KP_MULTIPLY):  mKeys[2] = 0;  break;
	case key(KP_DIVIDE):    mKeys[3] = 0;  break;

	/*case key(F4):
	if (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
	{
		// Hot reload of Terra shaders
		Root *root = mGraphicsSystem->getRoot();
		HlmsManager *hlmsManager = root->getHlmsManager();

		Hlms *hlms = hlmsManager->getHlms( HLMS_USER3 );
		GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}   break;*/
	}
}

#undef key
#pragma GCC diagnostic pop


//  Key events input
//-----------------------------------------------------------------------------------------------------------------------------

void App::channelChanged(ICS::Channel *channel, float currentValue, float previousValue)
{

}
