#include "pch.h"
#include "Def_Str.h"
#include "BaseApp.h"
#include "CarModel.h"
#include "FollowCamera.h"
// #include "pathmanager.h"
// #include "SplitScreen.h"

// #include "MyGUI_Prerequest.h"
// #include "MyGUI_PointerManager.h"
#include "ICSInputControlSystem.h"
#include "SDL_events.h"

#include <MyGUI.h>
// #include <MyGUI_Ogre2Platform.h>
using namespace Ogre;


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


//  input events
//-----------------------------------------------------------------------------------
MyGUI::MouseButton sdlButtonToMyGUI(Uint8 button)
{
	//  The right button is the second button, according to MyGUI
	if (button == SDL_BUTTON_RIGHT)
		button = SDL_BUTTON_MIDDLE;
	else if (button == SDL_BUTTON_MIDDLE)
		button = SDL_BUTTON_RIGHT;
	//  MyGUI's buttons are 0 indexed
	return MyGUI::MouseButton::Enum(button - 1);
}

#if 0
void BaseApp::mouseMoved( const SDL_Event &arg )
{
// if (bGuiFocus)
	// LogO(toStr(arg.motion.x)+" "+toStr(arg.motion.y)+" "+toStr(arg.wheel.y));
	MyGUI::InputManager::getInstance().injectMouseMove(
		arg.motion.x, arg.motion.y, 0/*arg.wheel.y*/);

	GameState::mouseMoved( arg );
}

void BaseApp::mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id )
{
	MyGUI::InputManager::getInstance().injectMousePress(
		arg.x, arg.y, sdlButtonToMyGUI(arg.button));
}
void BaseApp::mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id )
{
	MyGUI::InputManager::getInstance().injectMouseRelease(
		arg.x, arg.y, sdlButtonToMyGUI(arg.button));
}
#endif

//-------------------------------------------------------------------------------------
//  key, mouse, window
//-------------------------------------------------------------------------------------
/*
void BaseApp::keyReleased(const SDL_KeyboardEvent& arg)
{
	mInputCtrl->keyReleased(arg);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->keyReleased(arg);

	if (bAssignKey)  return;

	if (mGui && (isFocGui || isTweak()))
	{
		// OIS::KeyCode kc = mInputWrapper->sdl2OISKeyCode(arg.keysym.sym);
		// MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(kc));
	}
}*/

//  Mouse events
//-------------------------------------------------------------------------------------

void BaseApp::mouseMoved(const SDL_Event &arg)
{
	int mx = 0, my = 0, mz = 0;
	int rx = 0, ry = 0, rz = 0;

	if (arg.type == SDL_MOUSEMOTION)
	{
		mInputCtrl->mouseMoved(arg.motion);
		for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mouseMoved(arg.motion);

		mx = arg.motion.x;	rx = arg.motion.xrel;
		my = arg.motion.y;	ry = arg.motion.yrel;
	}
	else if (arg.type == SDL_MOUSEWHEEL)
	{
		mz = arg.wheel.y;  rz = mz; //arg.wheel.yrel;
	}

	if (bAssignKey)  return;

	if (IsFocGui() && mGui)
	{
		MyGUI::InputManager::getInstance().injectMouseMove(mx, my, mz);
		return;
	}

	///  Follow Camera Controls
	int i = 0;  //Log("cam: "+toStr(iCurCam));
	for (auto it = carModels.begin(); it != carModels.end(); ++it,++i)
		if (i == iCurCam && (*it)->fCam)
			(*it)->fCam->Move( mbLeft, mbRight, mbMiddle, shift, rx, ry, rz );

	// mMouseX = arg.x;
	// mMouseY = arg.y;
}

void BaseApp::mousePressed( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	mInputCtrl->mousePressed(arg, id);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mousePressed(arg, id);

	if (bAssignKey)  return;

	if (IsFocGui() && mGui)
	{
		MyGUI::InputManager::getInstance().injectMousePress(arg.x, arg.y, sdlButtonToMyGUI(id));
		return;
	}
	if		(id == SDL_BUTTON_LEFT)		mbLeft = true;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = true;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = true;
}

void BaseApp::mouseReleased( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	mInputCtrl->mouseReleased(arg, id);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mouseReleased(arg, id);

	if (bAssignKey)  return;

	if (IsFocGui() && mGui)
	{	MyGUI::InputManager::getInstance().injectMouseRelease(arg.x, arg.y, sdlButtonToMyGUI(id));
		return;
	}
	if		(id == SDL_BUTTON_LEFT)		mbLeft = false;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = false;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = false;
}

/*void BaseApp::textInput(const SDL_TextInputEvent &arg)
{
	const char* text = &arg.text[0];
	std::vector<unsigned long> unicode = utf8ToUnicode(std::string(text));

	if (isFocGui || isTweak())
	for (auto it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::None, *it);
}*/

void BaseApp::joyAxisMoved(const SDL_JoyAxisEvent &arg, int axis )
{
	mInputCtrl->axisMoved(arg, axis);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->axisMoved(arg, axis);
}

void BaseApp::joyButtonPressed(const SDL_JoyButtonEvent &evt, int button)
{
	mInputCtrl->buttonPressed(evt, button);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->buttonPressed(evt, button);
}

void BaseApp::joyButtonReleased(const SDL_JoyButtonEvent &evt, int button)
{
	mInputCtrl->buttonReleased(evt, button);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->buttonReleased(evt, button);
}


#if 0
//  mouse cursor
//-------------------------------------------------------
void BaseApp::showMouse()
{	
	mInputWrapper->setMouseVisible(true);
}
void BaseApp::hideMouse()
{                
	mInputWrapper->setMouseVisible(false);
}

void BaseApp::updMouse()
{
	if (IsFocGui())	showMouse();
	else			hideMouse();

	mInputWrapper->setAllowGrab(pSet->mouse_capture);

	mInputWrapper->setMouseRelative(!IsFocGui());
	mInputWrapper->setGrabPointer(!IsFocGui());
}

void BaseApp::onCursorChange(const std::string &name)
{
	if (!mCursorManager->cursorChanged(name))
		return; //the cursor manager doesn't want any more info about this cursor
	//See if we can get the information we need out of the cursor resource
	ResourceImageSetPointerFix* imgSetPtr = dynamic_cast<ResourceImageSetPointerFix*>(MyGUI::PointerManager::getInstance().getByName(name));
	if (imgSetPtr != NULL)
	{
		MyGUI::ResourceImageSet* imgSet = imgSetPtr->getImageSet();
		std::string tex_name = imgSet->getIndexInfo(0,0).texture;
		TexturePtr tex = TextureManager::getSingleton().getByName(tex_name);

		//everything looks good, send it to the cursor manager
		if (tex)
		{
			Uint8 size_x = imgSetPtr->getSize().width;
			Uint8 size_y = imgSetPtr->getSize().height;
			Uint8 left = imgSetPtr->getTexturePosition().left;
			Uint8 top = imgSetPtr->getTexturePosition().top;
			Uint8 hotspot_x = imgSetPtr->getHotSpot().left;
			Uint8 hotspot_y = imgSetPtr->getHotSpot().top;

			mCursorManager->receiveCursorInfo(name, tex, left, top, size_x, size_y, hotspot_x, hotspot_y);
	}	}
}

void BaseApp::windowResized(int x, int y)
{
	pSet->windowx = x;  pSet->windowy = y;
	bWindowResized = true;
	
	// Adjust viewports
	mSplitMgr->Align();
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);
}

void BaseApp::windowClosed()
{
	Root::getSingleton().queueEndRendering();
}
#endif
