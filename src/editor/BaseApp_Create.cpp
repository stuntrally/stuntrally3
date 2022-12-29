#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "BaseApp.h"
#include "CApp.h" //
#include "pathmanager.h"
// #include "Localization.h"

#include <thread>
#include <filesystem>
#include <OgreConfigFile.h>
#include <OgreOverlaySystem.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreTimer.h>
// #include "PointerFix.h"
#include <MyGUI_PointerManager.h>
#include <MyGUI_Gui.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_FactoryManager.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_Ogre2Platform.h>
#include <SDL_events.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;



/*
//  Frame
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	OverlayManager& ovr = OverlayManager::getSingleton();
	//  overlays-
	ovBrushPrv = ovr.getByName("Editor/BrushPrvOverlay");
	ovBrushMtr = ovr.getOverlayElement("Editor/BrushPrvPanel");
	ovTerPrv = ovr.getByName("Editor/TerPrvOverlay");  ovTerPrv->hide();
	ovTerMtr = ovr.getOverlayElement("Editor/TerPrvPanel");

	onCursorChange(MyGUI::PointerManager::getInstance().getDefaultPointer());
}


//  Run
//-------------------------------------------------------------------------------------
void BaseApp::Run( bool showDialog )
{
	mShowDialog = showDialog;
	if (!setup())
		return;

	if (!pSet->limit_fps)
		mRoot->startRendering();  // default
	else
	{	Ogre::Timer tim;
		while (1)
		{
			WindowEventUtilities::messagePump();
			if (tim.getMicroseconds() > 1000000.0 / pSet->limit_fps_val)
			{
				tim.reset();
				if (!mRoot->renderOneFrame())
					break;
			}else
			if (pSet->limit_sleep >= 0)
				sleep(milliseconds(pSet->limit_sleep));
	}	}

	destroyScene();
}
*/

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
	// delete mCursorManager;  mCursorManager = 0;
	
	DestroyGui();
}

#if 0
//  config
//-------------------------------------------------------------------------------------
bool BaseApp::configure()
{
	RenderSystem* rs;
	if (rs = mRoot->getRenderSystemByName(pSet->rendersystem))
	{
		mRoot->setRenderSystem(rs);
	}else{
		LogO("RenderSystem '" + pSet->rendersystem + "' is not available. Exiting.");
		return false;
	}
	if (pSet->rendersystem == "OpenGL Rendering Subsystem")  // not on dx
		mRoot->getRenderSystem()->setConfigOption("RTT Preferred Mode", pSet->buffer);

	mRoot->initialise(false);

	Uint32 flags = SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE;
	if (SDL_WasInit(flags) == 0)
	{
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
		if (SDL_Init(flags) != 0)
			throw runtime_error("Could not initialize SDL! " + string(SDL_GetError()));
	}
	SDL_StartTextInput();


	NameValuePairList params;
	params.insert(make_pair("title", "SR Editor"));
	params.insert(make_pair("FSAA", toStr(pSet->fsaa)));
	params.insert(make_pair("vsync", pSet->vsync ? "true" : "false"));

	int pos_x = SDL_WINDOWPOS_UNDEFINED,
		pos_y = SDL_WINDOWPOS_UNDEFINED;

	/// \todo For multiple monitors, WINDOWPOS_UNDEFINED is not the best idea. Needs a setting which screen to launch on,
	/// then place the window on that screen (derive x&y pos from SDL_GetDisplayBounds)+
	/*
	if (pSet->fullscreen)
	{
		SDL_Rect display_bounds;
		if (SDL_GetDisplayBounds(settings.screen, &display_bounds) != 0)
			throw runtime_error("Couldn't get display bounds!");
		pos_x = display_bounds.x;
		pos_y = display_bounds.y;
	}
	*/

	//  Create window
	mSDLWindow = SDL_CreateWindow(
		"SR Editor", pos_x, pos_y, pSet->windowx, pSet->windowy,
		SDL_WINDOW_SHOWN | (pSet->fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_RESIZABLE);

	SFO::SDLWindowHelper helper(mSDLWindow, pSet->windowx, pSet->windowy, "SR Editor", pSet->fullscreen, params);
	helper.setWindowIcon("sr-editor.png");
	mWindow = helper.getWindow();

	return true;
}


//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
	Ogre::Timer ti;
	LogO("*** start setup ***");

	if (pSet->rendersystem == "Default")
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		pSet->rendersystem = "Direct3D9 Rendering Subsystem";
		#else
		pSet->rendersystem = "OpenGL Rendering Subsystem";
		#endif
	}
	#ifdef _DEBUG
	Ogre::LogManager::getSingleton().setMinLogLevel(LML_TRIVIAL);  // all
	#endif

	#ifdef _DEBUG
	#define D_SUFFIX ""  // "_d"
	#else
	#define D_SUFFIX ""
	#endif

	//  when show ogre dialog is on, load both rendersystems so user can select
	if (pSet->ogre_dialog)
	{
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		#endif
	}else{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
	}

	mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
#if defined(OGRE_VERSION) && OGRE_VERSION >= 0x10B00
    //mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Codec_STBI" + D_SUFFIX);  // only png
    mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Codec_FreeImage" + D_SUFFIX);  // for jpg screenshots
#endif

	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager("DefaultSceneManager");

	#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 9, 0) 
	OverlaySystem* pOverlaySystem = new OverlaySystem();
	mSceneMgr->addRenderQueueListener(pOverlaySystem);
	#endif

	createCamera();

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	mSceneMgr->setFog(FOG_NONE);

    postInit();
	loadResources();

	baseInitGui();

	createFrameListener();

	LogO(String(":::* Time Ogre Start: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");

	createScene();

	return true;
}

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	//  Load resource paths from config file
	ConfigFile cf;
	string s = PATHMANAGER::GameConfigDir() +
		(pSet->tex_size > 0 ? "/resources_ed.cfg" : "/resources_s_ed.cfg");
	cf.load(s);

	//  Go through all sections & settings in the file
	auto seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		auto *settings = seci.getNext();
		for (auto i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(
				PATHMANAGER::Data() + "/" + archName, typeName, secName);
		}
	}
}

void BaseApp::loadResources()
{
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
#endif

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

//  ⌨️ Key pressed
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

	//if (bGuiFocus)
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

/*
void BaseApp::onCursorChange(const string &name)
{
	if (!mCursorManager->cursorChanged(name))
		return;  // the cursor manager doesn't want any more info about this cursor
	//  See if we can get the information we need out of the cursor resource
	ResourceImageSetPointerFix* imgSetPtr = dynamic_cast<ResourceImageSetPointerFix*>(MyGUI::PointerManager::getInstance().getByName(name));
	if (imgSetPtr != NULL)
	{
		MyGUI::ResourceImageSet* imgSet = imgSetPtr->getImageSet();
		string tex_name = imgSet->getIndexInfo(0,0).texture;
		TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(tex_name);

		//  everything looks good, send it to the cursor manager
		if (tex)
		{
			Uint8 size_x = imgSetPtr->getSize().width;
			Uint8 size_y = imgSetPtr->getSize().height;
			Uint8 left = imgSetPtr->getTexturePosition().left;
			Uint8 top = imgSetPtr->getTexturePosition().top;
			Uint8 hotspot_x = imgSetPtr->getHotSpot().left;
			Uint8 hotspot_y = imgSetPtr->getHotSpot().top;

			mCursorManager->receiveCursorInfo(name, tex, left, top, size_x, size_y, hotspot_x, hotspot_y);
		}
	}
}

void BaseApp::windowResized(int x, int y)
{
	pSet->windowx = x;  pSet->windowy = y;
	bWindowResized = true;

	// adjust camera asp. ratio
	if (mCamera && mViewport)
		mCamera->setAspectRatio( float(x) / float(y));
	mPlatform->getRenderManagerPtr()->setActiveViewport(0);
}

void BaseApp::windowClosed()
{
	Root::getSingleton().queueEndRendering();
}
*/


///  base Init Gui
//-------------------------------------------------------------------------------------
void BaseApp::baseInitGui()
{
	//  Cam Pos
	txCamPos = mGui->createWidget<TextBox>("TextBox",
		208,2, 600,40, Align::Default, "Pointer", "CamT");
	txCamPos->setFontName("hud.fps");
	txCamPos->setTextShadow(true);  txCamPos->setVisible(false);

	//  Input bar
	bckInput = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 640,64, Align::Default, "Pointer", "InpB");
	bckInput->setImageTexture("border_rect.png");

	txInput = bckInput->createWidget<TextBox>("TextBox",
		40,8, 630,60, Align::Default, "InpT");
	txInput->setFontName("hud.text");
	txInput->setFontHeight(40);
	txInput->setTextShadow(true);  bckInput->setVisible(false);
}
