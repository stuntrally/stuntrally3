#include "AppGui.h"
#include "Gui_Def.h"
#include "pathmanager.h"
#include "settings.h"

#include <MyGUI_Gui.h>
#include <MyGUI_Ogre2Platform.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>

#include <string>
#include <filesystem>
using namespace MyGUI;
using namespace std;


//  🌟🆕 init Gui
//----------------------------------------------------------------
void AppGui::InitGuiCom()
{
	if (mPlatform)
		return;
	LogO("C::# Init MyGui");

	//  Gui
	mPlatform = new Ogre2Platform();
	mPlatform->initialise(
		mWindow, mSceneMgr,
		"Essential",
	#ifdef SR_EDITOR
		PATHMANAGER::UserConfigDir() + "/MyGUI_ed.log");
	#else
		PATHMANAGER::UserConfigDir() + "/MyGUI.log");
	#endif

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

	
	// mPlatform->getRenderManagerPtr()->setSceneManager(mSplitMgr->mGuiSceneMgr);
	// mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);

	CreateFpsBar();
}

//  💥 destroy
void AppGui::DestroyGui()
{
	LogO("D::# Destroy MyGui");
	if (mGui)
	{	mGui->shutdown();  delete mGui;  mGui = 0;  }
	if (mPlatform)
	{	mPlatform->shutdown();  delete mPlatform;  mPlatform = 0;  }
}


//  📄 load settings from default file
//----------------------------------------------------------------
void AppGui::LoadDefaultSet(SETTINGS* settings, string setFile)
{
#ifdef SR_EDITOR
	settings->Load(PATHMANAGER::GameConfigDir() + "/editor-default.cfg");
	settings->Save(setFile);
#else
	settings->Load(PATHMANAGER::GameConfigDir() + "/game-default.cfg");
	settings->Save(setFile);

	//  game delete old keys.xml too
	string sKeys = PATHMANAGER::UserConfigDir() + "/keys.xml";
	if (filesystem::exists(sKeys))
		filesystem::rename(sKeys, PATHMANAGER::UserConfigDir() + "/keys_old.xml");
#endif
}

///  📄 Load Settings .cfg
//----------------------------------------------------------------
void AppGui::LoadSettings()
{
	setlocale(LC_NUMERIC, "C");

	PATHMANAGER::Init();  // Paths
	
	pSet = new SETTINGS();  // set

#ifdef SR_EDITOR
	string setFile = PATHMANAGER::SettingsFile(1), oldFile = PATHMANAGER::SettingsFile(1, 1);
#else
	string setFile = PATHMANAGER::SettingsFile(0), oldFile = PATHMANAGER::SettingsFile(0, 1);
#endif
	
	if (!PATHMANAGER::FileExists(setFile))
	{
		cerr << "Settings not found - loading defaults." << endl;
		LoadDefaultSet(pSet, setFile);
	}
	pSet->Load(setFile);  // Load

	if (pSet->version != SET_VER)  // loaded older, use default
	{
		cerr << "Settings found, but older version - loading defaults." << endl;
		filesystem::rename(setFile, oldFile);  // rename _old
		LoadDefaultSet(pSet, setFile);
		
		pSet->Load(setFile);  // Load
	}

	LogO(PATHMANAGER::info.str());  // log info
}
