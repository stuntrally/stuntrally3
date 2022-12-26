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
// #include <OgreWindowEventUtilities.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;

namespace
{
	std::vector<unsigned long> utf8ToUnicode(const string& utf8)
	{
		std::vector<unsigned long> unicode;
		size_t i = 0;
		while (i < utf8.size())
		{
			unsigned long uni;  size_t todo;
			unsigned char ch = utf8[i++];

				 if (ch <= 0x7F){	uni = ch;	todo = 0;	}
			else if (ch <= 0xBF){	throw logic_error("not a UTF-8 string");	}
			else if (ch <= 0xDF){	uni = ch&0x1F;	todo = 1;	}
			else if (ch <= 0xEF){	uni = ch&0x0F;	todo = 2;	}
			else if (ch <= 0xF7){	uni = ch&0x07;	todo = 3;	}
			else				{	throw logic_error("not a UTF-8 string");	}

			for (size_t j = 0; j < todo; ++j)
			{
				if (i == utf8.size())	throw logic_error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)  throw logic_error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF)  throw logic_error("not a UTF-8 string");
			if (uni > 0x10FFFF)  throw logic_error("not a UTF-8 string");
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


/*
//  Camera
//-------------------------------------------------------------------------------------
void BaseApp::createCamera()
{
	mCamera = mSceneMgr->createCamera("Cam");
	mCamera->setPosition(Vector3(0,00,100));
	mCamera->lookAt(Vector3(0,0,0));
	mCamera->setNearClipDistance(0.5f);

	mViewport = mWindow->addViewport(mCamera);
	//mViewport->setBackgroundColour(ColourValue(0.5,0.65,0.8));  //`
	mViewport->setBackgroundColour(ColourValue(0.2,0.3,0.4));  //`
	Real asp = Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight());
	mCamera->setAspectRatio(asp);
}


//  Frame
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing ***");

	OverlayManager& ovr = OverlayManager::getSingleton();
	//  overlays-
	ovBrushPrv = ovr.getByName("Editor/BrushPrvOverlay");
	ovBrushMtr = ovr.getOverlayElement("Editor/BrushPrvPanel");
	ovTerPrv = ovr.getByName("Editor/TerPrvOverlay");  ovTerPrv->hide();
	ovTerMtr = ovr.getOverlayElement("Editor/TerPrvPanel");

	//  input
	mInputWrapper = new SFO::InputWrapper(mSDLWindow, mWindow);
	mInputWrapper->setMouseEventCallback(this);
	mInputWrapper->setKeyboardEventCallback(this);
	mInputWrapper->setWindowEventCallback(this);
	mCursorManager = new SFO::SDLCursorManager();
	onCursorChange(MyGUI::PointerManager::getInstance().getDefaultPointer());
	mCursorManager->setEnabled(true);

	mRoot->addFrameListener(this);
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
				boost::this_thread::sleep(boost::posix_time::milliseconds(pSet->limit_sleep));
	}	}

	destroyScene();
}
*/

//  dtor
//-------------------------------------------------------------------------------------
BaseApp::~BaseApp()
{
	// delete mCursorManager;  mCursorManager = 0;
	// delete mInputWrapper;  mInputWrapper = 0;
	
	if (mGui)  {
		mGui->shutdown();	delete mGui;	mGui = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	// OGRE_DELETE mRoot;
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

/*void BaseApp::keyReleased( const SDL_KeyboardEvent &arg )
{
	//if (bGuiFocus)
		// MyGUI::InputManager::getInstance().injectKeyRelease(
		// 	MyGUI::KeyCode::Enum(mInputWrapper->sdl2OISKeyCode(arg.keysym.sym)));
}*/

void BaseApp::textInput(const SDL_TextInputEvent &arg)
{
	/*const char* text = &arg.text[0];
	if (*text == '`')  return;
	std::vector<unsigned long> unicode = utf8ToUnicode(string(text));
	if (bGuiFocus)
	for (auto it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::None, *it);*/
}

//  ⌨️ Key Released
//-------------------------------------------------------------------------------------
#define key(a)  SDL_SCANCODE_##a
void BaseApp::keyReleased( const SDL_KeyboardEvent &arg )
{
	switch (arg.keysym.scancode)
	{
	case key(LSHIFT):  case key(RSHIFT):  shift = false;  break;  // mods
	case key(LCTRL):   case key(RCTRL):   ctrl = false;   break;
	case key(LALT):    case key(RALT):    alt = false;    break;

	case key(W):  mKeys[0] = 0;  break;  case key(S):  mKeys[1] = 0;  break;  // cam move
	case key(A):  mKeys[2] = 0;  break;  case key(D):  mKeys[3] = 0;  break;
	case key(Q):  mKeys[4] = 0;  break;  case key(E):  mKeys[5] = 0;  break;

	case key(UP):     mKeys[6] = 0;  break;  case key(DOWN):   mKeys[7] = 0;  break;  // cam rot
	case key(LEFT):   mKeys[8] = 0;  break;  case key(RIGHT):  mKeys[9] = 0;  break;
	default:  break;
	}
}

//  ⌨️ Key pressed
//-------------------------------------------------------------------------------------
void BaseApp::BaseKeyPressed(const SDL_KeyboardEvent &arg)
{	
	switch (arg.keysym.scancode)
	{
	case key(LSHIFT):  case key(RSHIFT):  shift = false;  break;  // mods
	case key(LCTRL):   case key(RCTRL):   ctrl = false;   break;
	case key(LALT):    case key(RALT):    alt = false;    break;

	case key(W):  mKeys[0] = 1;  break;  case key(S):  mKeys[1] = 1;  break;  // cam move
	case key(A):  mKeys[2] = 1;  break;  case key(D):  mKeys[3] = 1;  break;
	case key(Q):  mKeys[4] = 1;  break;  case key(E):  mKeys[5] = 1;  break;

	case key(UP):     mKeys[6] = 1;  break;  case key(DOWN):   mKeys[7] = 1;  break;  // cam rot
	case key(LEFT):   mKeys[8] = 1;  break;  case key(RIGHT):  mKeys[9] = 1;  break;
	default:  break;
	}
}
#undef key


//  Mouse
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
//--------------------------------------------------------------------------------------------------------------
void BaseApp::baseInitGui()
{
	using namespace MyGUI;
	//  Gui
	mPlatform = new Ogre2Platform();

	mPlatform->initialise(
		mWindow, mSceneMgr, // mGraphicsSystem->getSceneManager(),
		"Essential",
		PATHMANAGER::UserConfigDir() + "/MyGUI_ed.log");
	mGui = new Gui();

	mGui->initialise("core.xml");

	// FactoryManager::getInstance().registerFactory<ResourceImageSetPointerFix>("Resource", "ResourceImageSetPointer");
	// MyGUI::ResourceManager::getInstance().load("core.xml");

	// PointerManager::getInstance().eventChangeMousePointer += newDelegate(this, &BaseApp::onCursorChange);
	//; PointerManager::getInstance().setVisible(false);
	
	//------------------------ language
	/*if (pSet->language == "")  // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }*/
	
	if (!PATHMANAGER::FileExists(PATHMANAGER::Data() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";  // use en if not found
	
	LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	//------------------------


	///  create widgets
	//------------------------------------------------
	//  Fps
	bckFps = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 212,25, Align::Default, "Pointer", "FpsB");
	bckFps->setImageTexture("back_fps.png");

	txFps = bckFps->createWidget<TextBox>("TextBox",
		1,1, 212,25, Align::Default, "FpsT");
	txFps->setFontName("hud.fps");  bckFps->setVisible(false);

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
