#include "AppGui.h"
#include "Gui_Def.h"
#include "paths.h"
#include "settings.h"
#include "MainEntryPoints.h"

#include <MyGUI_Gui.h>
#include <MyGUI_Ogre2Platform.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>

#include <iterator>
#include <string>
#include <filesystem>
#include <locale.h>
#include <cctype>

#include <OgrePlatform.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#include <Winnls.h>  //<windows.h>
#endif
using namespace MyGUI;
using namespace std;


//  detect language
std::string AppGui::getSystemLanguage()
{
	const std::string default_lang = "en";

	setlocale(LC_ALL, "");

	char *loc = setlocale(LC_ALL, NULL);
	if (!loc)
		return default_lang;
	if (memcmp(loc, "C", 2) == 0)
		return "en";

	//  windows only
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		char buf[256];  // loc has same result?       // English name of language
		int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, buf, sizeof(buf));

			if (!strcmp(buf,"English"))     loc = "en";
		else if (!strcmp(buf,"German"))     loc = "de";
		else if (!strcmp(buf,"Finnish"))    loc = "fi";
		else if (!strcmp(buf,"Romanian"))   loc = "ro";
		else if (!strcmp(buf,"French"))     loc = "fr";
		else if (!strcmp(buf,"Russian"))    loc = "ru";
		else if (!strcmp(buf,"Portuguese")) loc = "pt";
		else if (!strcmp(buf,"Italian"))    loc = "it";
		else if (!strcmp(buf,"Polish"))     loc = "pl";
		else if (!strcmp(buf,"Slovakian"))  loc = "sk";
		else if (!strcmp(buf,"Spanish"))    loc = "es";
		else if (!strcmp(buf,"Czech"))      loc = "cs";
		else if (!strcmp(buf,"Japanese"))   loc = "ja";
	#endif

	//  We parse here only the first part of two part codes (e.g.fi_FI).
	//  We can revisit this if we get regional translations.
	std::string s(loc);
	if (s.size() > 2)
	{
		s = s.substr(0, 2);
		transform(s.begin(), s.end(), s.begin(), 
			[](unsigned char c){  return tolower(c);  } );
	}
	return s;
}


//  🌟🆕 init Gui
//----------------------------------------------------------------
void AppGui::InitAppGui()
{
	if (mPlatform)
		return;
	LogO("C::# Init MyGui");
	SetupKeysForGUI();

	//  Gui
	mPlatform = new Ogre2Platform();
	mPlatform->initialise(
		mWindow, mSceneMgr,
		"Essential",
	#ifdef SR_EDITOR
		PATHS::UserConfigDir() + "/MyGUI_ed.log");
	#else
		PATHS::UserConfigDir() + "/MyGUI.log");
	#endif

	mGui = new Gui();
	mGui->initialise("core.xml");

	// FactoryManager::getInstance().registerFactory<ResourceImageSetPointerFix>("Resource", "ResourceImageSetPointer");
	// MyGUI::ResourceManager::getInstance().load("core.xml");

	// PointerManager::getInstance().eventChangeMousePointer += newDelegate(this, &BaseApp::onCursorChange);
	//; PointerManager::getInstance().setVisible(false);
	
	//------------------------ language
	if (pSet->language == "")  // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }
	
	if (!PATHS::FileExists(PATHS::Data() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";  // use en if not found
	
	LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	//------------------------

	
	// mPlatform->getRenderManagerPtr()->setSceneManager(mSplitMgr->mGuiSceneMgr);
	// mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);

	// CreateFpsBar();  // after fonts
}

//  💥 destroy
void AppGui::DestroyGui()
{
	//LogO("D::# Destroy MyGui");  // cant log
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
	settings->Load(PATHS::GameConfigDir() + "/editor-default.cfg");
	settings->Save(setFile);
#else
	settings->Load(PATHS::GameConfigDir() + "/game-default.cfg");
	settings->Save(setFile);

	//  game delete old keys.xml too
	string sKeys = PATHS::UserConfigDir() + "/keys.xml";
	if (filesystem::exists(sKeys))
		filesystem::rename(sKeys, PATHS::UserConfigDir() + "/keys_old.xml");
#endif
}

///  📄 Load Settings .cfg
//----------------------------------------------------------------
void AppGui::LoadSettings()
{
	setlocale(LC_NUMERIC, "C");

	PATHS::Init();  // Paths
	
	pSet = new SETTINGS();  // set

#ifdef SR_EDITOR
	string setFile = PATHS::SettingsFile(1), oldFile = PATHS::SettingsFile(1, 1);
#else
	string setFile = PATHS::SettingsFile(0), oldFile = PATHS::SettingsFile(0, 1);
#endif
	
	if (!PATHS::FileExists(setFile))
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

	cout << PATHS::info.str();
	// LogO(PATHS::info.str());  // no log yet



#ifndef SR_EDITOR
	//  📡 Helper for testing networked games on 1 PC
	//  use number > 0 in command parameter,  adds it to nick, port and own ogre.log
	auto& args = MainEntryPoints::args;

	int num = -1;
	
	if (args.all.size() > 1)
	{
		num = Ogre::StringConverter::parseInt(args.all[1]);
	}
	if (num > 0)
	{
		pSet->net_local_plr = num;
		pSet->local_port += num;
		pSet->nickname += Ogre::StringConverter::toString(num);
	}
#endif
}
