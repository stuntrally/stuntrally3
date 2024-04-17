#include "pch.h"

#if 0  //  OLD  Ogre 1.x  file reference  ------
#include "Def_Str.h"
#include "settings.h"
#include "BaseApp.h"
#include "CApp.h" //
#include "paths.h"
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
		mRoot->loadPlugin(PATHS::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->loadPlugin(PATHS::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		#endif
	}else{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHS::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHS::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
	}

	mRoot->loadPlugin(PATHS::OgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
#if defined(OGRE_VERSION) && OGRE_VERSION >= 0x10B00
    //mRoot->loadPlugin(PATHS::OgrePluginDir() + "/Codec_STBI" + D_SUFFIX);  // only png
    mRoot->loadPlugin(PATHS::OgrePluginDir() + "/Codec_FreeImage" + D_SUFFIX);  // for jpg screenshots
#endif

	setupResources();

	if (!configure())
		return false;

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	mSceneMgr->setFog(FOG_NONE);

	loadResources();

	baseInitGui();

	createFrameListener();

	LogO(String(":::* Time Ogre Start: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	return true;
}


//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	//  Load resource paths from config file
	ConfigFile cf;
	string s = PATHS::GameConfigDir() +
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
				PATHS::Data() + "/" + archName, typeName, secName);
		}
	}
}

void BaseApp::loadResources()
{
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}



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

#endif
