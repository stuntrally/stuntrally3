#include "pch.h"
#include "Game.h"
#include "CGame.h"
#include "GuiCom.h"
#include "GraphicsSystem.h"
#include "OgreWindow.h"

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
#include "MultiList2.h"
#include "Slider.h"

using namespace MyGUI;
using namespace Ogre;


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


	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");

	CGuiCom gcom(pApp);
	gcom.CreateFonts();

	//  Fps  test
	/*ImageBox* bckFps = mGui->createWidget<ImageBox>("ImageBox",
		0,350, 220,50, Align::Default, "Pointer", "Main");
	bckFps->setImageTexture("back_fps.png");
	bckFps->setAlpha(0.2f);
	bckFps->setVisible(true);

	TextBox* txFps = mGui->createWidget<TextBox>("TextBox",
		0,350, 1220,50, Align::Default, "Main");
	// txFps->setFontName("DigGear");
	txFps->setFontName("hud.text");
	txFps->setCaption("0:12:34.56 +7.8\nasdf WERG sxv");
	txFps->setVisible(true);/**/
	
	// PointerManager::getInstance().setVisible(false);  //+

	//  load
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

void TerrainGame::DestroyGui()
{
	if (mGui)
	{	mGui->shutdown();  delete mGui;  mGui = 0;  }
	if (mPlatform)
	{	mPlatform->shutdown();  delete mPlatform;  mPlatform = 0;  }
}
