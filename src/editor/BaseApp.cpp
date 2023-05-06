#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "BaseApp.h"
#include "CApp.h" //

#include <OgreTimer.h>
#include <OgreCamera.h>
// #include "PointerFix.h"
#include <MyGUI_Gui.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_Ogre2Platform.h>
#include <SDL_events.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


//  🌟 ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp()
{
	for (int k=0; k < SDL_NUM_SCANCODES; ++k)
		iKeys[k] = false;
}

//  💥 dtor
BaseApp::~BaseApp()
{
	DestroyGui();
}


//  key, mouse, window
//-------------------------------------------------------------------------------------
void BaseApp::textInput(const SDL_TextInputEvent &arg)
{
	const char* text = &arg.text[0];
	auto unicode = utf8ToUnicode(std::string(text));

	if (bGuiFocus)
	for (auto it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(
			MyGUI::KeyCode::None, *it);
}

//  ⌨️ Key Released
//-------------------------------------------------------------------------------------
#define key(a)  SDL_SCANCODE_##a
void BaseApp::keyReleased( const SDL_KeyboardEvent &arg )
{
	auto k = arg.keysym.scancode;
	iKeys[k] = 0;
	switch (k)
	{
	case key(LSHIFT):  case key(RSHIFT):  shift = false;  break;  // mods
	case key(LCTRL):   case key(RCTRL):   ctrl = false;   break;
	case key(LALT):    case key(RALT):    alt = false;    break;
	default:  break;
	}

	if (bGuiFocus)
	{
		MyGUI::KeyCode kc = SDL2toGUIKey(arg.keysym.sym);
		MyGUI::InputManager::getInstance().injectKeyRelease(kc);
	}
}

//  ⌨️ Key Pressed
//-------------------------------------------------------------------------------------
void BaseApp::BaseKeyPressed(const SDL_KeyboardEvent &arg)
{	
	auto k = arg.keysym.scancode;
	iKeys[k] = 1;
	switch (k)
	{
	case key(LSHIFT):  case key(RSHIFT):  shift = true;  break;  // mods
	case key(LCTRL):   case key(RCTRL):   ctrl = true;   break;
	case key(LALT):    case key(RALT):    alt = true;    break;
	default:  break;
	}
}
#undef key


//  🖱️ Mouse
//-------------------------------------------------------------------------------------
void BaseApp::mouseMoved( const SDL_Event &arg )
{
	static int xAbs = 0, yAbs = 0, whAbs = 0;  // abs

	if (arg.type == SDL_MOUSEMOTION)
	{
		xAbs = arg.motion.x;  mx = arg.motion.xrel;
		yAbs = arg.motion.y;  my = arg.motion.yrel;
	}
	else if (arg.type == SDL_MOUSEWHEEL)
	{
		mz = arg.wheel.y;  whAbs += mz;
	}

	//if (bGuiFocus)  // todo: faster wheel
		MyGUI::InputManager::getInstance().injectMouseMove( xAbs, yAbs, whAbs );
}

void BaseApp::mousePressed( const SDL_MouseButtonEvent &arg, Uint8 id )
{
	if (bGuiFocus)
		MyGUI::InputManager::getInstance().injectMousePress(
			arg.x, arg.y, sdlButtonToMyGUI(id));

	if      (id == SDL_BUTTON_LEFT)		mbLeft = true;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = true;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = true;
}

void BaseApp::mouseReleased( const SDL_MouseButtonEvent &arg, Uint8 id )
{
	//if (bGuiFocus)
		MyGUI::InputManager::getInstance().injectMouseRelease(
			arg.x, arg.y, sdlButtonToMyGUI(id));

	if      (id == SDL_BUTTON_LEFT)		mbLeft = false;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = false;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = false;
}


///  base Init Gui
//-------------------------------------------------------------------------------------
void BaseApp::baseInitGui()
{
	//  Cam Pos
	bckCamPos = mGui->createWidget<ImageBox>("ImageBox",
		248,22, 616,96, Align::Default, "Pointer", "CamB");
	bckCamPos->setImageTexture("back_times.png");

	txCamPos = bckCamPos->createWidget<TextBox>("TextBox",
		16,8, 600,80, Align::Default, "CamT");
	txCamPos->setFontName("hud.fps");  txCamPos->setTextShadow(true);
	bckCamPos->setVisible(pSet->camPos);

	//  Input bar
	bckInput = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 640,64, Align::Default, "Pointer", "InpB");
	bckInput->setImageTexture("back_times.png");  //menu.png");

	txInput = bckInput->createWidget<TextBox>("TextBox",
		40,8, 630,60, Align::Default, "InpT");
	txInput->setFontName("hud.text");
	txInput->setFontHeight(40);  txInput->setTextShadow(true);
	bckInput->setVisible(pSet->inputBar);
}
