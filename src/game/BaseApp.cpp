#include "pch.h"
#include "Def_Str.h"
#include "BaseApp.h"
// #include "FollowCamera.h"
// #include "pathmanager.h"
// #include "SplitScreen.h"

// #include "MyGUI_Prerequest.h"
// #include "MyGUI_PointerManager.h"

#include <MyGUI.h>
// #include <MyGUI_Ogre2Platform.h>
using namespace Ogre;

// #include "sdlinputwrapper.hpp"
// #include "ICSInputControlSystem.h"

#if 0
//  rendering
//-------------------------------------------------------------------------------------
bool BaseApp::frameRenderingQueued(const FrameEvent& evt)
{
	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	//  update each device
	mInputWrapper->capture(false);

	mInputCtrl->update(evt.timeSinceLastFrame);
	for (int i=0; i<4; ++i)
		mInputCtrlPlayer[i]->update(evt.timeSinceLastFrame);

	   // key modifiers
	alt = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_ALT));
	ctrl = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_CTRL));
	shift = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_SHIFT));

	updateStats();
	
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	frameStart(time);
	//* */if (!frameStart())
	//	return false;
	
	return true;
}

bool BaseApp::frameEnded(const FrameEvent& evt)
{
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	return frameEnd(time);
	//return true;
}
#endif

bool BaseApp::IsFocGui()
{
	return isFocGui || isFocRpl ||
		(mWndChampStage && mWndChampStage->getVisible()) || (mWndChampEnd && mWndChampEnd->getVisible()) ||
		(mWndChallStage && mWndChallStage->getVisible()) || (mWndChallEnd && mWndChallEnd->getVisible()) ||
		(mWndNetEnd && mWndNetEnd->getVisible()) || (mWndTweak && mWndTweak->getVisible());
}

bool BaseApp::isTweak()
{
	return mWndTweak && mWndTweak->getVisible();
}
