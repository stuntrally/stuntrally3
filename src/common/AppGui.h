#pragma once
#include "Gui_Def.h"
#include <string>
#include <OgreString.h>
#include <OgreHlmsCommon.h>

#include <SDL_keycode.h>
#include <MyGUI_KeyCode.h>
#include <MyGUI_MouseButton.h>

namespace MyGUI{  class Gui;  class Ogre2Platform;  }
namespace Ogre {  class Root;  class SceneManager;  class Window;  class Camera;
	class TextureGpu;  class CompositorWorkspace;  class HlmsSamplerblock;
	class Terra;  class HlmsPbsTerraShadows;  }
class GraphicsSystem;  class SETTINGS;  class CScene;  class CGui;  class CGuiCom;

//  cube reflections, car etc
enum IblQuality
{
	MipmapsLowest, IblLow, IblMedium, IblHigh
};

//--------------------------------------------------------------------------------
//  Base application with:
//    Ogre, MyGui, settings, Fps bar, utils
//    Reflection cube, Terrain
//--------------------------------------------------------------------------------
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

	//  🏞️ Scene
	CScene* scn =0;


	//  🎛️ Gui  ----------------
	MyGUI::Gui* mGui =0;
	MyGUI::Ogre2Platform* mPlatform =0;

	CGui* gui =0;
	CGuiCom* gcom =0;

	//  Gui input utils  ----
	MyGUI::MouseButton sdlButtonToMyGUI(int button);
	std::vector<unsigned long> utf8ToUnicode(const std::string& utf8);

	MyGUI::KeyCode SDL2toGUIKey(SDL_Keycode code);
	void SetupKeysForGUI();
	std::unordered_map<SDL_Keycode, MyGUI::KeyCode> mKeyMap;


	//  📈 Fps bar  ----------------
	Img bckFps =0;
	Txt txFps =0;
	void UpdFpsText(), CreateFpsBar();
	float GetGPUmem();


	//  ⛓️ Utils  wireframe  ----
	bool bWireframe = 0;
	void SetWireframe();
	void SetWireframe(Ogre::HlmsTypes type, bool wire);
	//  tex wrap  ----
	void SetTexWrap(Ogre::HlmsTypes type, Ogre::String name, bool wrap = true);
	void SetTexWrap(Ogre::Item* it, bool wrap = true);
	void InitTexFilters(Ogre::HlmsSamplerblock* sb, bool wrap = true);

	//  unload tex, gpu mem  ----
	template <typename T, size_t MaxNumTextures>
	void unloadTexturesFromUnusedMaterials(
		Ogre::HlmsDatablock* datablock,
		std::set<Ogre::TextureGpu*> &usedTex,
		std::set<Ogre::TextureGpu*> &unusedTex );
	void unloadTexturesFromUnusedMaterials();
	void unloadUnusedTextures();

	void MinimizeMemory();  // mtr,tex, reduce mem, each track load
	// void setTightMemoryBudget(), setRelaxedMemoryBudget();


	//  🆕 Init  ----------------
	void InitAppGui();
	void DestroyGui();
	void Quit();


	//  🔮 Reflection Cubemap  ----------------
	Ogre::Camera* mCubeCamera = 0;
	Ogre::TextureGpu* mCubeReflTex = 0;

	IblQuality mIblQuality = IblLow;  // par in ctor-
	std::vector<Ogre::CompositorWorkspace*> mWorkspaces;
	Ogre::CompositorWorkspace* SetupCompositor();


	//  ⛰️ Terrain  ----------------
	Ogre::Terra* mTerra = 0;
	void CreateTerrain(), DestroyTerrain();

	Ogre::String mtrName;
	Ogre::SceneNode* nodeTerrain = 0;
	//  listener to make PBS objects also be affected by terrain's shadows
	Ogre::HlmsPbsTerraShadows* mHlmsPbsTerraShadows = 0;
	
};
