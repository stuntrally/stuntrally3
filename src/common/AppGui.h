#pragma once
#include "Gui_Def.h"
#include <string>
#include <OgreString.h>
#include <OgreHlmsCommon.h>

namespace MyGUI{  class Gui;  class Ogre2Platform;  }
namespace Ogre {  class Root;  class SceneManager;  class Window;  class Camera;
	class TextureGpu;  class CompositorWorkspace;  class HlmsSamplerblock;  }
class GraphicsSystem;  class SETTINGS;

//  cube reflections, car etc
enum IblQuality
{
	MipmapsLowest, IblLow, IblMedium, IblHigh
};

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
	Ogre::Root* mRoot =0;
	Ogre::SceneManager* mSceneMgr =0;
	Ogre::Window* mWindow =0;
	Ogre::Camera* mCamera =0;

	GraphicsSystem* mGraphicsSystem =0;

	//  🎛️ Gui  ----------------
	MyGUI::Gui* mGui =0;
	MyGUI::Ogre2Platform* mPlatform =0;


	//  📈 Fps bar  ----------------
	Img bckFps =0;
	Txt txFps =0;
	void UpdFpsText(), CreateFpsBar();
	float GetGPUmem();


	//  ⛓️ utils  wireframe
	bool bWireframe = 0;
	void SetWireframe();
	void SetWireframe(Ogre::HlmsTypes type, bool wire);
	//  tex wrap
	void SetTexWrap(Ogre::HlmsTypes type, Ogre::String name, bool wrap = true);
	void SetTexWrap(Ogre::Item* it, bool wrap = true);
	void InitTexFilters(Ogre::HlmsSamplerblock* sb, bool wrap = true);


	//  🆕 Init  ----------------
	void InitAppGui();
	void DestroyGui();


	//  🔮 Reflection Cubemap  ----------------
	Ogre::Camera* mCubeCamera = 0;
	Ogre::TextureGpu* mCubeReflTex = 0;

	IblQuality mIblQuality = IblLow;  // par in ctor-
	Ogre::CompositorWorkspace* mWorkspace = 0;
	Ogre::CompositorWorkspace* SetupCompositor();

	
};
