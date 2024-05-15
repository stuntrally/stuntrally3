#pragma once
#include "BaseApp.h"
#include "SceneClasses.h"
#include "enums.h"
#include "mathvector.h"
#include "quaternion.h"
#include "tracksurface.h"
#include "SceneXml.h"  //Object-
#include "brushes.h"
#include "PreviewTex.h"
#include "PreviewScene.h"

#include <SDL_keycode.h>
#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>

#define BrushMaxSize  1024

//  Gui
const int ciAngSnapsNum = 7;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,5,15,30,45,90,180};

namespace Ogre  {  class Rectangle2D;  class SceneNode;  class RenderTexture;  class Item;
	class Terra;  class HlmsPbsTerraShadows;  class CompositorWorkspace;  }
class CScene;  class CGui;  class CGuiCom;
class GraphicsSystem;  class HudRenderable;  class ExportRoR;

enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  };


class App : public BaseApp
{
public:
	App();
	virtual ~App();
	void Load(), LoadData();


	PreviewTex prvView,prvRoad,prvTer;  // track tab

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
	void SaveHmaps(), DelNewHmaps();
	enum TrkEvent {  TE_None=0, TE_Load, TE_Save, TE_Update  }
	eTrkEvent = TE_None;
	int iUpdEvent = 0;

	void createScene01() override;
	void destroyScene() override;


	//  💫 Update
	void update(float dt) override;
	void UpdCamPos();
	void UpdateKey(float dt);
	void UpdateEnd(float dt);

	void processCamera(double dt), UpdKeyBar(Ogre::Real dt);
	Ogre::Vector3 vNew{0,0,0};
	
	//  🛠️ Edit all  :
	void EditMouse(),  MouseRoad(), MouseStart();
	void MouseFluids(), MouseObjects(), MouseEmitters();
	void MouseCollects(), MouseFields();

	void KeyTxtRoad(Ogre::Real q), KeyTxtTerrain(Ogre::Real q), KeyTxtStart(Ogre::Real q);
	void KeyTxtFluids(Ogre::Real q), KeyTxtObjects(), KeyTxtEmitters(Ogre::Real q);
	void KeyTxtCollects(), KeyTxtFields();
	

	//  🆕 Create  . . . . . . . . . . . . . . . . . . . . . . . . 
	bool bVegetGrsUpd =0;
	Ogre::String resTrk;  void NewCommon(bool onlyTerVeget);

	//  📦🏢 Objects
	void CreateObjects(), DestroyObjects(bool clear), ResetObjects();
	void UpdObjPick(), PickObject(), ToggleObjSim(), FocusCam();

	//  💎 Collectibles
	void CreateCollects(), DestroyCollects(bool clear), ResetCollects();
	void CreateCollect(int i), DestroyCollect(int i);

	int iCollected = 0, oldCollected = 0;  //-
	void UpdColPick(), PickCollect();

	//  🎆 Fields
	void CreateFields(), DestroyFields(bool clear);
	void CreateField(int i), DestroyField(int i);


	//  🌍 minimap  ----
	HudRenderable* hrPos =0;
	Ogre::SceneNode* ndPos =0;  // dims
	Ogre::Real asp = 4.f/3.f, xm1 = -1.f, ym1 = 1.f, xm2 = 1.f, ym2 = -1.f;

	//  🖼️ rtt  render to texture  --------
	enum ERnd2Tex
	{	RT_RoadDens,  // roadDensity.png   road (on terrain) map to exclude grass,trees
		RT_RoadPrv,   // road.png     minimap road preview, colors
		RT_Terrain,   // terrain.png  minimap terrain
		RT_View3D,    // view.jpg     preview camera, track 3d view
		RT_PreView3D, // 3d preview, object
		RT_ALL
	};
	struct SRndTrg
	{
		Ogre::Camera* cam =0;
		Ogre::TextureGpu* tex =0, *rtt =0;
		Ogre::CompositorWorkspace* ws =0;
		//  mini rect
		HudRenderable* hr =0;
		Ogre::SceneNode* nd =0;
	} rt[RT_ALL];
	HudRenderable* hrBck =0;  // bck cover prv cam-
	Ogre::SceneNode* ndBck =0;

	void CreateRnd2Tex(), DestroyRnd2Tex(), UpdRnd2Tex(), AddListenerRnd2Tex();
	void UpdMiniSize(), UpdMiniVis(), UpdMiniPos(), DestroyEdHud();
	

	//  ⛰️📍 terrain cursor, circle mesh  o  ----
	HudRenderable*   hrTerC[ED_TerEditMax] ={0,0,0,0};
	Ogre::SceneNode* ndTerC[ED_TerEditMax] ={0,0,0,0};
	void TerCircleInit(), TerCircleUpd(float dt);

	//  🖼️ brush, ter gen  preview tex  ----
	HudRenderable* hrBrush =0, *hrTerGen =0;
	Ogre::SceneNode* ndBrush =0, *ndTerGen =0;

	PreviewTex prvBrushes, prvBrush;
	void CreatePreviews();

	void updBrushData(Ogre::uint8* data, int brId, bool all);
	void UpdBrushPrv(bool first=false), UpdTerGenPrv(bool first=false);
	void GenBrushesPrv();

	bool bUpdTerPrv =0;
	Ogre::TextureGpu *terPrvTex =0;
	const static int BrPrvSize = 256, BrIcoSize = 128, TerPrvSize = 512;


	///<>  🖌️⛰️ terrain edit, brush
	BrushesIni brSets;
	const static float brClr[ED_TerEditMax][3];

	void SetBrushPreset(int id, bool upd =1), SetBrushRandom(int n);
	void updBrush(), UpdBr();


	//  🖌️ brush vars
	bool bTerUpd =0, bTerUpdBlend =0;  char sBrushTest[512] ={0,};
	float* pBrFmask =0, *mBrushData =0;
	bool brLockPos =0;

	//  🖌️ params
	float mBrFiltOld = 1.f;
	BrushSet br[ED_TerEditMax];
	int iCurBr = 0;
	BrushSet& curBr() {  return br[iCurBr];  }


	//  ⛰️ Terrain edit  ----
	bool GetEditRect(Ogre::Vector3& pos, Ogre::Rect& brushrect, Ogre::Rect& maprect, int size, int& cx, int& cy);
	void Deform(Ogre::Vector3 &pos, float dtime, float brMul);
	void Height(Ogre::Vector3 &pos, float dtime, float brMul);

	void Smooth(Ogre::Vector3 &pos, float dtime);
	void SmoothTer(Ogre::Vector3 &pos, float avg, float dtime);
	void CalcSmoothFactor(Ogre::Vector3 &pos, float& avg, int& sample_count);

	void Filter(Ogre::Vector3 &pos, float dtime, float brMul);


	///  🎳 bullet world, simulate
	class btDiscreteDynamicsWorld* world =0;
	class btDefaultCollisionConfiguration* config =0;
	class btCollisionDispatcher* dispatcher =0;
	class bt32BitAxisSweep3* broadphase =0;
	class btSequentialImpulseConstraintSolver* solver =0;

	void BltWorldInit(), BltWorldDestroy(), BltClear(), BltUpdate(float dt);


	//  🛠️ Tools, road  -in base
	void SaveGrassDens();
	void AlignTerToRoad(), AlignHorizonToTer();
	int iSnap = 0;  Ogre::Real angSnap = 0.f;
	int iEnd = 0;  // edit: 0 scn->start 1 end
	
	ExportRoR* ror =0;


	//  🚧 box cursors  for each edit mode
	void UpdStartPos(bool vis = true), CreateCursors();

	struct BoxCur  // cursors
	{	Ogre::SceneNode* nd =0;
		Ogre::Item*      it =0;
	}
	boxCar,boxStart[2], boxFluid, boxObj, boxEmit, boxCol, boxField;

	void CreateBox(BoxCur& box, Ogre::String sMat, Ogre::String sMesh, int x = 0, bool shadow =false);

	void togPrvCam();


	//  💧 Fluids  ~~~~
	bool bRecreateFluids =0;  // todo: only one
	bool bRecreateFluidsRTT =0;  // gui opt apply
	int iFlCur =0;
	void UpdFluidBox();
	std::string newFluidName = "water blue";
	

	//  📦 Objects  ----
	ED_OBJ objEd = EO_Move;  // edit mode

	int iObjCur = -1;  // picked id
	int iObjTNew = 0;  // new object's type, id for vObjNames
	std::vector<std::string> vObjNames, vBuildings;
	
	void SetObjNewType(int tnew), UpdObjNewNode();
	void NextObjMat(int add), NextObjMat(int add, Object& o);
	void TogObjStatic();  // toggle static

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
	std::string newEmtName = "SmokeBrown";


	//  💎 Collectibles  ----
	ED_OBJ colEd = EO_Move;  // edit mode
	int iColCur = -1;  // picked id
	
	SCollect colNew;
	std::vector<std::string> vColNames;

	void SetColType(int add);//, UpdColNewNode();
	void SetColGroup(int add);
	void AddNewCol();


	//  🎆 Fields  ----
	ED_OBJ fldEd = EO_Move;  // edit mode
	int iFldCur = -1;  // picked id
	
	void SetFldNewType(int tnew), UpdFldNewNode();
	void AddNewFld();
	void UpdFldPick();

	SField fldNew;
	void SetFldType(int add);


	//  ⚫ Surfaces  ----
	std::vector <TRACKSURFACE> surfaces;  // all
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();

	//  🛣️ Roads  ----
	void RoadsNext(int add = 1), RoadsAdd(), RoadsDel();


	//  🧊 3D Preview  for objects, vegetation
	PreviewScene prvScene;
};
