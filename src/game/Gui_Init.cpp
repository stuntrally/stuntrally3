#include "CHud.h"
#include "Game.h"
#include "CGame.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"

#include "OgreHlmsPbs.h"

#include "OgreTextAreaOverlayElement.h"
#include "OgreFrameStats.h"

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/Pass/OgreCompositorPass.h>
#include <Compositor/Pass/OgreCompositorPassDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <Compositor/Pass/PassClear/OgreCompositorPassClearDef.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorManager2.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>

using namespace MyGUI;
using namespace Ogre;
using namespace Demo;


void TerrainGame::InitGui()
{
	if (mPlatform)
		return;
	LogO(">> InitGui ***");
	//  Gui
	mPlatform = new Ogre2Platform();

	mPlatform->initialise(
		mGraphicsSystem->getRenderWindow(), mGraphicsSystem->getSceneManager(),
		"Essential",
		// PATHMANAGER::UserConfigDir() + "/MyGUI.log");
		"MyGUI.log");

	// mGraphicsSystem->mWorkspace = setupCompositor();


	mGui = new Gui();
	// pApp->mGui = mGui;  //no
	// pApp->hud->gui = mGui;
	
	mGui->initialise("core.xml");

	MyGUI::LanguageManager::getInstance().setCurrentLanguage("en");
		
	// mPlatform->getRenderManagerPtr()->setSceneManager(mGraphicsSystem->getSceneManager());
	// mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);


	//  Fps  test
	/*ImageBox* bckFps = mGui->createWidget<ImageBox>("ImageBox",
		0,350, 220,50, Align::Default, "Pointer", "Main");
	bckFps->setImageTexture("back_fps.png");
	bckFps->setAlpha(0.2f);
	bckFps->setVisible(true);

	TextBox* txFps = mGui->createWidget<TextBox>("TextBox",
		0,350, 1220,50, Align::Default, "Main");
	txFps->setFontName("DigGear");
	// txFps->setFontName("hud.text");  // fixme  ttf fails ?
	txFps->setCaption("0:12:34.56 -7.8");//\nasdf WERG sxv");
	txFps->setVisible(true);*/
	
	PointerManager::getInstance().setVisible(false);

	// lay
	// LayoutManager::getInstance().loadLayout("Game.layout");
	// LayoutManager::getInstance().loadLayout("Game_Main.layout");  //+
	// LayoutManager::getInstance().loadLayout("Game_Options.layout");
	/*Window* wnd = mGui->findWidget<Window>("MainMenuWnd");
	wnd->setVisible(true);/**/

	/**
	FactoryManager::getInstance().registerFactory<ResourceImageSetPointer>("Resource", "ResourceImageSetPointer");
	ResourceManager::getInstance().load("core.xml");

	PointerManager::getInstance().eventChangeMousePointer += newDelegate(this, &BaseApp::onCursorChange);
	PointerManager::getInstance().setVisible(false);/**/

}
