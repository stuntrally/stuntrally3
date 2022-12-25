#include "pch.h"
#include "Def_Str.h"
#include "BaseApp.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "settings.h"
// #include "pathmanager.h"
// #include "SplitScreen.h"

#include "GraphicsSystem.h"
#include "SdlInputHandler.h"
#include "ICSInputControlSystem.h"
#include <SDL_events.h>

#include <MyGUI.h>
#include <MyGUI_KeyCode.h>
#include <SDL_keycode.h>
// #include <MyGUI_Prerequest.h>
// #include <MyGUI_PointerManager.h>
// #include <MyGUI_Ogre2Platform.h>
using namespace Ogre;


namespace
{
	std::vector<unsigned long> utf8ToUnicode(const std::string& utf8)
	{
		std::vector<unsigned long> unicode;
		size_t i = 0;
		while (i < utf8.size())
		{
			unsigned long uni;  size_t todo;
			unsigned char ch = utf8[i++];

				 if (ch <= 0x7F){	uni = ch;	todo = 0;	}
			else if (ch <= 0xBF){	throw std::logic_error("not a UTF-8 string");	}
			else if (ch <= 0xDF){	uni = ch&0x1F;	todo = 1;	}
			else if (ch <= 0xEF){	uni = ch&0x0F;	todo = 2;	}
			else if (ch <= 0xF7){	uni = ch&0x07;	todo = 3;	}
			else				{	throw std::logic_error("not a UTF-8 string");	}

			for (size_t j = 0; j < todo; ++j)
			{
				if (i == utf8.size())	throw std::logic_error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)  throw std::logic_error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF)  throw std::logic_error("not a UTF-8 string");
			if (uni > 0x10FFFF)  throw std::logic_error("not a UTF-8 string");
			unicode.push_back(uni);
		}
		return unicode;
	}

	MyGUI::MouseButton sdlButtonToMyGUI(Uint8 button)
	{
		//  The right button is the second button, according to MyGUI
		if (button == SDL_BUTTON_RIGHT)  button = SDL_BUTTON_MIDDLE;
		else if (button == SDL_BUTTON_MIDDLE)  button = SDL_BUTTON_RIGHT;
		//  MyGUI's buttons are 0 indexed
		return MyGUI::MouseButton::Enum(button - 1);
	}
}

#if 0
//  rendering
bool BaseApp::frameRenderingQueued(const FrameEvent& evt)
{
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


//-------------------------------------------------------------------------------------
//  🎛️ input events  🖱️ mouse
//-------------------------------------------------------------------------------------
void BaseApp::mouseMoved(const SDL_Event &arg)
{
	static int xAbs = 0, yAbs = 0, whAbs = 0;  // abs
	int xRel = 0, yRel = 0, whRel = 0;  // rel

	if (arg.type == SDL_MOUSEMOTION)
	{
		mInputCtrl->mouseMoved(arg.motion);
		for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mouseMoved(arg.motion);

		xAbs = arg.motion.x;  xRel = arg.motion.xrel;
		yAbs = arg.motion.y;  yRel = arg.motion.yrel;
	}
	else if (arg.type == SDL_MOUSEWHEEL)
	{
		whRel = arg.wheel.y;  whAbs += whRel;
	}

	if (bAssignKey)  return;

	if (IsFocGui() && mGui)
	{
		MyGUI::InputManager::getInstance().injectMouseMove( xAbs, yAbs, whAbs );
		return;
	}

	///  🎥 Follow Camera Controls  ----
	int i = 0;  //LogO("cam: "+toStr(iCurCam));
	for (auto it = carModels.begin(); it != carModels.end(); ++it,++i)
		if (i == iCurCam && (*it)->fCam)
			(*it)->fCam->Move( mbLeft, mbRight, mbMiddle, shift, xRel, yRel, whRel );
}

void BaseApp::mousePressed( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	mInputCtrl->mousePressed(arg, id);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mousePressed(arg, id);

	if (bAssignKey)  return;

	if (IsFocGui() && mGui)
	{
		MyGUI::InputManager::getInstance().injectMousePress(
			arg.x, arg.y, sdlButtonToMyGUI(id));
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
	{	MyGUI::InputManager::getInstance().injectMouseRelease(
			arg.x, arg.y, sdlButtonToMyGUI(id));
		return;
	}
	if		(id == SDL_BUTTON_LEFT)		mbLeft = false;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = false;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = false;
}

//  edit text
void BaseApp::textInput(const SDL_TextInputEvent &arg)
{
	const char* text = &arg.text[0];
	std::vector<unsigned long> unicode = utf8ToUnicode(std::string(text));

	if (isFocGui || isTweak())
	for (auto it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(
			MyGUI::KeyCode::None, *it);
}

//  joy 🕹️
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



//  🖱️ mouse cursor
//-------------------------------------------------------
void BaseApp::showMouse()
{
	MyGUI::PointerManager::getInstance().setVisible(1);
}
void BaseApp::hideMouse()
{                
	MyGUI::PointerManager::getInstance().setVisible(0);
}

void BaseApp::updMouse()
{
	bool gui = IsFocGui();
	if (gui)  showMouse();
	else      hideMouse();

	auto* inp = mGraphicsSystem->getInputHandler();
	inp->setGrabMousePointer( pSet->mouse_capture && !gui );
	inp->setMouseRelative( pSet->mouse_capture && !gui );
	// inp->setMouseVisible( pSet->mouse_capture );
	// mInputWrapper->setAllowGrab(pSet->mouse_capture);
}

#if 0
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


//  util
//-----------------------------------------------------------------------------------------------------------
MyGUI::KeyCode BaseApp::SDL2toGUIKey(SDL_Keycode code)
{
	MyGUI::KeyCode kc = MyGUI::KeyCode::None;

	auto key = mKeyMap.find(code);
	if (key != mKeyMap.end())
		kc = key->second;

	return kc;
}

void BaseApp::SetupKeysForGUI()
{
	mKeyMap.clear();

	mKeyMap[SDLK_HOME] = MyGUI::KeyCode::Home;
	mKeyMap[SDLK_END] = MyGUI::KeyCode::End;
	mKeyMap[SDLK_PAGEUP] = MyGUI::KeyCode::PageUp;
	mKeyMap[SDLK_PAGEDOWN] = MyGUI::KeyCode::PageDown;
	mKeyMap[SDLK_UP] = MyGUI::KeyCode::ArrowUp;
	mKeyMap[SDLK_DOWN] = MyGUI::KeyCode::ArrowDown;
	mKeyMap[SDLK_LEFT] = MyGUI::KeyCode::ArrowLeft;
	mKeyMap[SDLK_RIGHT] = MyGUI::KeyCode::ArrowRight;

	mKeyMap[SDLK_DELETE] = MyGUI::KeyCode::Delete;
    mKeyMap[SDLK_BACKSPACE] = MyGUI::KeyCode::Backspace;
	mKeyMap[SDLK_SPACE] = MyGUI::KeyCode::Space;
    mKeyMap[SDLK_ESCAPE] = MyGUI::KeyCode::Escape;
	mKeyMap[SDLK_INSERT] = MyGUI::KeyCode::Insert;
	mKeyMap[SDLK_RETURN] = MyGUI::KeyCode::Return;
	// mKeyMap[SDLK_KP_0] = MyGUI::KeyCode::Numpad0;
	// mKeyMap[SDLK_KP_ENTER] = MyGUI::KeyCode::NumpadEnter;

	// Don't need more, rest comes in textInput
}
