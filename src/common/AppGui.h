#pragma once
#include "Gui_Def.h"
#include "Cam.h"
#include "ViewDim.h"
#include "par.h"
#include <string>
#include <OgreString.h>
#include <OgreHlmsCommon.h>

#include <SDL_keycode.h>
#include <MyGUI_KeyCode.h>
#include <MyGUI_MouseButton.h>

namespace Ogre
{	class Root;  class SceneManager;  class Window;  class Camera;
	class TextureGpu;  class HlmsSamplerblock;  class HlmsPbs;
	
	class CompositorWorkspace;  class CompositorWorkspaceDef;  class RenderTargetViewDef;
	class CompositorNodeDef;  class CompositorTargetDef;  class CompositorPassSceneDef;
	class CompositorPassQuadDef;
	// namespace v1 {  class Overlay;  }
	class Terra;  class HlmsPbsTerraShadows;  class PlanarReflections;
    class VctVoxelizer;  class VctLighting;  class IrradianceField;
}
namespace MyGUI
{	class Gui;  class Ogre2Platform;
}
class GraphicsSystem;  class HlmsPbsDb2;
class SETTINGS;  class CScene;  class CGui;  class CGuiCom;

//  cube Reflections, car etc
enum IblQuality
{
	MipmapsLowest, IblLow, IblMedium, IblHigh
};

//  Global Illumination
enum GiMode
{
	NoGI, Ifd, Vct, IfdVct, NumGiModes
};


//--------------------------------------------------------------------------------
//  Base application with:
//    Ogre, MyGui, settings, Fps bar, utils
//    Shadows, Reflection cube
//--------------------------------------------------------------------------------
class AppGui : public BaseGui
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
	Txt txFps =0, txFpsInfo =0;
	void UpdFpsText(float dt), CreateFpsBar();
	float timFps = 0.f;
	float GetGPUmem();


	//  ⛓️ Utils  --------------------------------
	//  🌐 Wireframe
	bool bWireframe = 0;
	void SetWireframe();
	void SetWireframe(Ogre::HlmsTypes type, bool wire);
	//  Texture filtering, wrap
	void SetTexWrap(Ogre::HlmsTypes type, Ogre::String name, bool wrap = true);
	void SetTexWrap(Ogre::Item* it, bool wrap = true);
	void InitTexFiltUV(Ogre::HlmsSamplerblock* sb, bool wrap = true);
	void InitTexFilt(Ogre::HlmsSamplerblock* sb);
	void SetAnisotropy();
	void SetAnisotropy(Ogre::Item* it);
	//  select
	void UpdSelectGlow(Ogre::Renderable *rend, bool selected);
	void BarrierResolve(Ogre::TextureGpu* tex);  // fix rtt for vulkan

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
	std::map<std::string, int> mapCommonTex;  // preload and don't unload
	void InitCommonTex(), LoadCommonTex();
	void LoadTex(Ogre::String fname, bool wait=true), WaitForTex();


	//  🆕 Init  ----------------
	std::string getSystemLanguage();
	void InitAppGui();
	void DestroyGui();
	void Quit();


	//  ⛰️ Terrain  ----------------
	// Ogre::String mtrName;
	// Ogre::SceneNode* nodeTerrain = 0;


	//  🌒 Shadows ----------------
	void createPcfShadowNode();
	void createEsmShadowNodes();
	void setupESM();
	Ogre::CompositorWorkspace *setupShadowCompositor();
	void updShadowFilter();

	const char *chooseEsmShadowNode();  // fixme ESM..
	// void setupShadowNode( bool forEsm );


	//  🔮 Reflection Cubemap  ----------------
	Ogre::Camera* mCubeCamera = 0;
	Ogre::TextureGpu* mCubeReflTex = 0;
	Ogre::CompositorWorkspace* wsCubeRefl = 0;
	int iReflSkip =0, iReflStart =0;
	void CreateCubeReflect(), UpdCubeRefl();

	void ApplyReflect();  // to all Db2s, after CubeRefl created
	std::set<HlmsPbsDb2*> vDbRefl;


	//  🪄 Compositor Setup  --------------------------------
	std::vector<Ogre::CompositorNodeDef*> vNodes;  // stuff we create and destroy
	std::vector<Ogre::CompositorWorkspace*> vWorkspaces;
	std::vector<Ogre::TextureGpu*> vRtt, vTex;
	std::vector<Ogre::CompositorWorkspaceDef*> vWorkDefs;
	constexpr static const char* csShadow = "ShadowMapFromCodeShadowNode";
	
	//  compositor Add, utils
	Ogre::CompositorNodeDef*      AddNode(Ogre::String name);
	Ogre::CompositorWorkspaceDef* AddWork(Ogre::String name);
	Ogre::CompositorPassQuadDef*  AddQuad(Ogre::CompositorTargetDef* td);
	Ogre::CompositorPassSceneDef* AddScene(Ogre::CompositorTargetDef* td);
	
	Ogre::RenderTargetViewDef*    AddRtv(Ogre::CompositorNodeDef* nd,
		Ogre::String name, Ogre::String colour, Ogre::String depth="",
		Ogre::String colour2="", Ogre::String colour3="");

	Ogre::TextureGpu* AddSplitRTT(Ogre::String id, float width, float height);
	void AddHudGui(Ogre::CompositorTargetDef* td);

	void AddShadows(Ogre::CompositorPassSceneDef* ps);
	Ogre::String getSplitMtr(int splits);
	Ogre::String getWsInfo();  // log

	//  Compositor Create
	//  One  returns rtt if made, when not final wnd  // cur view num / all
	Ogre::TextureGpu* CreateCompositor(int edRtt, int view, int splits, float width, float height);
	//  All  Full
	void SetupCompositors(), DestroyCompositors();


	//  👥 Split screen  ----------------
	std::vector<Cam> mCamsAll, mCams;  // for each player
	Cam* findCam(Ogre::String name);

	Cam* CreateCamera(Ogre::String name,
		Ogre::SceneNode* node,  // creates for 0, attaches if not 0
		Ogre::Vector3 pos, Ogre::Vector3 lookAt);
	void DestroyCameras();
	
	//  viewport dimensions, for each player
	ViewDim mDims[MAX_Players];

	//  workspace, Ogre render setup   players + 1 for Refl cube, more for fluids etc
	// std::vector<Ogre::CompositorWorkspace*> mWorkspaces;


	//  Effects post
	void InitEffects(), UpdateEffects(Ogre::Camera *camera);

	//  🕳️ SSAO  ----------------
	Ogre::Pass *mSSAOPass =0;
	Ogre::Pass *mApplyPass =0;
	Ogre::TextureGpu *noiseTexture =0;
	void InitSSAO(), UpdSSAO(Ogre::Camera *camera);
	bool edPrvCam =0;

	//  🔆 Lens flare
	Ogre::Vector4 uvSunPos_Fade;  // on screen
	Ogre::Vector4 efxClrSun, efxClrRays;  // params for post effects
	void UpdSunPos(Ogre::Camera *camera);

	Ogre::Pass *mLensPass =0;
	void InitLens(), UpdLens();

	//  🌄 Sunbeams
	Ogre::Pass *mSunbeamsPass =0;
	void InitSunbeams(), UpdSunbeams();


	//  🌇 GI  ----------------
	void InitGI(), UpdateGI(), DestroyGI();
	void GIVoxelizeScene();

	//  IR  Low quality
	// Ogre::InstantRadiosity *mIR;
	//  local cubemaps refl-
	// Ogre::PccPerPixelGridPlacement 

	//  GI var
	Ogre::FastArray<Ogre::Item*> mItems;
	//  VCT  High quality  spec refl
	Ogre::VctLighting *mVCT =0;
	Ogre::VctVoxelizer *mVoxelizer =0;
	//  VCT par
	float fVctThinWall = 1.f;
	int iVctBounces = 3; //6

	//  IFD  Med quality  diffuse
	Ogre::IrradianceField *mIFD =0;
	bool bIfdRaster = false;  // slow
	//  IFD par
	int iIfdRaysPpx = 1;
	int iIfdResolution = 6;
	int iIfdDepthProbeRes = 12;

	//  ⛓️ GI util
	GiMode GIgetMode() const;
	Ogre::String GIstrMode() const;
	bool GIneedsVoxels() const;
	Ogre::HlmsPbs *getHlmsPbs();

	void GItoggletVctQuality(), GInextMethod(int add);

	//  🛠️ visualize
	// TestUtils *mTestUtils =0;
	int iVctDebugVis, iIfdDebugVis;
	void GIupdText();  // gui info

	void GInextVctVis(int add), GInextIfdProbeVis(int add);
	int iIfdVisSphereVert = 4;  // quality, vertex tessellation
	void GIIfdVisUpd();
};
