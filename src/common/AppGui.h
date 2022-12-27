#pragma once
#include "Gui_Def.h"
#include <string>

namespace MyGUI{  class Gui;  class Ogre2Platform;  }
namespace Ogre {  class Root;  class SceneManager;  class Window;  class Camera;  }
class SETTINGS;


//  Base application with:
//  Ogre, MyGui, settings, Fps bar
class AppGui : public BGui
{
public:
	//  .cfg set  ----------------
	SETTINGS* pSet =0;
	void LoadDefaultSet(SETTINGS* settings, std::string setFile);
	void LoadSettings();

	//  🟢 Ogre  ----------------
	Ogre::Root *mRoot =0;
	Ogre::SceneManager* mSceneMgr =0;
	Ogre::Window* mWindow =0;
	Ogre::Camera* mCamera =0;

	//  🎛️ Gui  ----------------
	MyGUI::Gui* mGui =0;
	MyGUI::Ogre2Platform* mPlatform =0;

	//  📈 Fps bar  ----------------
	Img bckFps =0;
	Txt txFps =0;
	void UpdFpsText(), CreateFpsBar();
	float GetGPUmem();

	//  🆕 Init  ----------------
	void InitGuiCom();
	void DestroyGui();
};
