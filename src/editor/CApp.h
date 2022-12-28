#pragma once
#include "BaseApp.h"
#include "mathvector.h"
#include "quaternion.h"
#include "tracksurface.h"
#include "SceneXml.h"  //Object-
// #include "PreviewTex.h"

#include <SDL_keycode.h>
// #include <Ogre.h>
#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>
// #include <OgreRenderTargetListener.h>
// #include <OgreShadowCameraSetup.h>
// #include <OgreTexture.h>

#define BrushMaxSize  512

//  Gui
const int ciAngSnapsNum = 7;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,5,15,30,45,90,180};

namespace Ogre  {  class Rectangle2D;  class SceneNode;  class RenderTexture;  class Item;
	class Terra;  class HlmsPbsTerraShadows;  }
class CScene;  class CGui;  class CGuiCom;
class GraphicsSystem;  class HudRenderable;

enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  };


class App : public BaseApp
			// public Ogre::RenderTargetListener
{
public:
	App();
	virtual ~App();
	void Load(), LoadData();

	CScene* scn =0;


	///  Gui
	CGui* gui =0;
	CGuiCom* gcom =0;

	// PreviewTex prvView,prvRoad,prvTer;  // track tab

	float mTimer =0.f;

	
	///  main
	void LoadTrack(), SaveTrack(), UpdateTrack(), CreateRoads();
	
	void SetEdMode(ED_MODE newMode);
	void UpdVisGui(), UpdEditWnds();
	void UpdWndTitle(), SaveCam();

	//  🕹️ input
	void keyPressed(const SDL_KeyboardEvent &arg) override;
	void keyPressRoad(SDL_Scancode skey);
	void keyPressObjects(SDL_Scancode skey);

	//  ⏳ Loading
	void LoadTrackEv(), SaveTrackEv(), UpdateTrackEv();
	enum TrkEvent {  TE_None=0, TE_Load, TE_Save, TE_Update  }
	eTrkEvent = TE_None;

	void createScene01() override;
	void destroyScene() override;


	//  💫 Update
	void update(float dt) override;
	void UpdateKey(float dt);
	void UpdateEnd(float dt);

	void processMouse(double dt), UpdKeyBar(Ogre::Real dt);
	Ogre::Vector3 vNew;
	
	//  🛠️ Edit all  :
	void EditMouse(),  MouseRoad(), MouseStart(), MouseFluids(), MouseEmitters(), MouseObjects();
	void KeyTxtRoad(Ogre::Real q), KeyTxtTerrain(Ogre::Real q), KeyTxtStart(Ogre::Real q);
	void KeyTxtFluids(Ogre::Real q), KeyTxtObjects(), KeyTxtEmitters(Ogre::Real q);
	

	//  🆕 Create  . . . . . . . . . . . . . . . . . . . . . . . . 
	bool bNewHmap =0, bTrGrUpd =0, bParticles =1;
	Ogre::String resTrk;  void NewCommon(bool onlyTerVeget);

	void CreateObjects(), DestroyObjects(bool clear), ResetObjects();
	void UpdObjPick(), PickObject(), ToggleObjSim();


	///  🖼️ rnd to tex  minimap  * * * * * * * * *	
	HudRenderable* mpos =0;
	Ogre::SceneNode *ndPos =0;  // dims
	Ogre::Real asp = 4.f/3.f, xm1 = -1.f, ym1 = 1.f, xm2 = 1.f, ym2 = -1.f;

	enum ERnd2Tex
	{	RT_Road=0, RT_Grass, RT_Terrain, RT_View, RT_Last, RT_Brush, RT_ALL  };
	struct SRndTrg
	{
		Ogre::Camera* cam = 0;
		Ogre::RenderTexture* tex = 0;
		Ogre::Rectangle2D* mini = 0;
		Ogre::SceneNode* ndMini = 0;
	} rt[RT_ALL];

	void Rnd2TexSetup(), UpdMiniVis();
	// virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	// virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	

	//  📍 terrain cursor, circle mesh
	HudRenderable* moTerC =0;
	Ogre::SceneNode* ndTerC =0;
	void TerCircleInit(), TerCircleUpd();

	//  🖼️ brush preview tex
	void createBrushPrv();
	void updateBrushPrv(bool first=false), updateTerPrv(bool first=false);

	bool bUpdTerPrv =0;
	Ogre::TextureGpu* brushPrvTex =0, *terPrvTex =0;
	const static int BrPrvSize = 128, TerPrvSize = 256;


	///<>  ⛰️ terrain edit, brush
	enum EBrShape
	{   BRS_Triangle=0, BRS_Sinus, BRS_Noise, BRS_Noise2, BRS_Ngon, BRS_ALL
	} mBrShape[ED_ALL];
	const static Ogre::String csBrShape[BRS_ALL];

	struct BrushSet  // brush preset ----
	{
		ED_MODE edMode;  int curBr;
		float Size,Intens,Pow,Fq,NOf;
		int Oct;  EBrShape shape;
		float Filter,HSet;
		signed char newLine;
		Ogre::String name;
	};

	const static int brSetsNum = 87;
	const static BrushSet brSets[brSetsNum];
	const static float brClr[4][3];

	void SetBrushPreset(int id);
	void updBrush();


	//  brush vars
	int curBr = 0;
	bool bTerUpd =0, bTerUpdBlend =0;  char sBrushTest[512] ={0,};
	float* pBrFmask =0, *mBrushData =0;
	bool brLockPos =0;

	//  params
	float terSetH = 10.f,  mBrFilt = 2.f, mBrFiltOld = 1.f;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL], mBrPow[ED_ALL];
	float mBrFq[ED_ALL],mBrNOf[ED_ALL];  int mBrOct[ED_ALL];

	//  brush deform
	bool getEditRect(Ogre::Vector3& pos, Ogre::Rect& brushrect, Ogre::Rect& maprect, int size, int& cx, int& cy);


	//  ⛰️ Terrain  ----
	Ogre::String mtrName;
	Ogre::Terra *mTerra = 0;
	void CreateTerrain(), DestroyTerrain();

	Ogre::SceneNode *nodeTerrain = 0;
	//  listener to make PBS objects also be affected by terrain's shadows
	Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows = 0;

	//  ⛰️ terrain edit
	void deform(Ogre::Vector3 &pos, float dtime, float brMul);
	void height(Ogre::Vector3 &pos, float dtime, float brMul);

	void smooth(Ogre::Vector3 &pos, float dtime);
	void smoothTer(Ogre::Vector3 &pos, float avg, float dtime);
	void calcSmoothFactor(Ogre::Vector3 &pos, float& avg, int& sample_count);

	void filter(Ogre::Vector3 &pos, float dtime, float brMul);


	///  📦 bullet world, simulate
	class btDiscreteDynamicsWorld* world =0;
	class btDefaultCollisionConfiguration* config =0;
	class btCollisionDispatcher* dispatcher =0;
	class bt32BitAxisSweep3* broadphase =0;
	class btSequentialImpulseConstraintSolver* solver =0;

	void BltWorldInit(), BltWorldDestroy(), BltClear(), BltUpdate(float dt);


	//  🛠️ Tools, road  -in base
	void SaveGrassDens(), SaveWaterDepth();
	void AlignTerToRoad();
	int iSnap = 0;  Ogre::Real angSnap = 0.f;
	int iEnd = 0;  // edit: 0 scn->start 1 end


	//  🚧 box cursors  car start,end,  fluids, objects, emitters
	void UpdStartPos();
	void CreateBox(Ogre::SceneNode*& nd, Ogre::Item*& ent, Ogre::String sMat, Ogre::String sMesh, int x=0);

	Ogre::SceneNode* ndCar =0, *ndStartBox[2]={0,0},  *ndFluidBox =0, *ndObjBox =0, *ndEmtBox =0;
	Ogre::Item*      itCar =0, *itStartBox[2]={0,0},  *itFluidBox =0, *itObjBox =0, *itEmtBox =0;
	void togPrvCam();


	//  💧 Fluids
	int iFlCur =0;  bool bRecreateFluids =0;
	void UpdFluidBox(), UpdMtrWaterDepth();
	

	//  📦 Objects  ----
	ED_OBJ objEd = EO_Move;  // edit mode

	int iObjCur = -1;  // picked id
	int iObjLast = 0;  // last counter, just for naming

	int iObjTNew = 0;  // new object's type, id for vObjNames
	std::vector<std::string> vObjNames, vBuildings;
	void SetObjNewType(int tnew), UpdObjNewNode();

	void AddNewObj(bool getName=true);

	std::set<int> vObjSel;  // selected ids for sc.objects[]
	void UpdObjSel();  // upd selected glow
	Ogre::Vector3 GetObjPos0();  // sel center

	bool objSim = 0;  // dynamic simulate on
	Object objNew;  //Object*..

	std::vector<Object> vObjCopy;  // copied objects


	//  🔥 Emitters  ----
	ED_OBJ emtEd = EO_Move;  // edit mode
	int iEmtCur = -1;  // picked id
	SEmitter emtNew;
	
	int iEmtNew = 0;  // id for vEmtNames
	std::vector<std::string> vEmtNames;
	void SetEmtType(int rel);

	void UpdEmtBox();
	bool bRecreateEmitters = 0;


	//  ⚫ Surfaces  ----
	std::vector <TRACKSURFACE> surfaces;  // all
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();
};
