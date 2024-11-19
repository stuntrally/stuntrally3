#pragma once
#include "Gui_Def.h"
#include "SliderValue.h"

#include <MyGUI_Prerequest.h>
//#include <MyGUI_Types.h>
#include <MyGUI_WidgetToolTip.h>
#include <MyGUI_Enumerator.h>
#include <OgreString.h>
#include "settings_com.h"


namespace Ogre {  class Root;  class SceneManager;  class Window;  class Viewport;
	class SceneNode;  class Light;  class HlmsPbsDatablock;  }
class Scene;  class CData;  class SplineRoad;
class App;  class SETTINGS;
class TrackInfo;  class UserTrkInfo;
class HlmsPbsDb2;


//  tracks,cars list items - with info for sorting
struct TrkL
{
	std::string name;
	const class TrackInfo* ti;  // 0 means user trk
	const class UserTrkInfo* ui;  // 0 means user trk
	static int idSort;
	TrkL();
};


class CGuiCom : public BGui
{
public:
	App* app =0;
	SETTINGS* pSet =0;
	Scene* sc =0;
	MyGUI::Gui* mGui =0;
	
	CGuiCom(App* app1);

	typedef std::list <std::string> strlist;


	///  🎛️ Gui Common   --------------------------

	//  🗜️ resize
	void SizeGUI(), doSizeGUI(MyGUI::EnumeratorWidgetPtr), doTestGUI(MyGUI::EnumeratorWidgetPtr);
	float GetGuiMargin(int wy);
	void setOrigPos(WP wp, const char* relToWnd);
	void CreateFonts();  // 🔠

	//  ❔ tooltip
	WP mToolTip =0;  Ed mToolTipTxt =0;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(WP sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(WP moving, const MyGUI::IntPoint& point);

	//  🗺️ language
	bool bGuiReinit = 0;  // lang change
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>

	void UnfocusLists();
	Btn bnQuit =0;  void btnQuit(WP);

	//  🆕 init
	void GuiCenterMouse();
	void GuiInitAll(), GuiInitTooltip(), GuiInitLang();
	//  init tabs
	void GuiInitGraphics(), GuiInitTrack(), initMiniPos(int i);

	MyGUI::TabPtr FindSubTab(WP tab);  // util
	

	//  📈 Fps
	SlV(Fps);  SV svFpsMul;
	Txt txFpsInfo =0;  void nextFps();
	constexpr static int FPS_Modes = 4;

	///  📊 Graphics  --------------------------
	// Cmb cbAntiAlias =0;  void cmbAntiAlias(CMB);

	// Ck ckLimitFps;
	// SV svLimitFps,svLimitSleep;

	//----  same as SETcom struct Detail
	//{
		//  🖼️ textures
		SlV(Anisotropy);
		Cmb cbTexFilter =0;  void cmbTexFilter(CMB);
		
		//  geom detail 📦🪨 🛣️
		SlV(ViewDist);
		SlV(LodBias);  SV svRoadDist;

		//  ⛰️ terrain
		SV svTerTripl, svTriplHoriz;
		SV svHorizons;
		SlV(TerLod);  SV svHorizLod;

		//  🌳🪨🌿 veget
		SV svTrees, svTreesDist, svHorTrees, svHorTreesDist;
		SV svBush, svBushDist, svHorBush, svHorBushDist;
		SV svGrass, svGrassDist, svVegetLimit;
		void btnVegetReset(WP), btnVegetApply(WP);

		//  🌒 shadow
		SV svShadowSize, svShadowCount, svShadowType;
		SV svShadowDist;  SV svShadowFilter;  void btnShadowsApply(WP);
		//  💡 lights
		void slBright(WP);
	#ifndef SR_EDITOR
		Ck ckLiFront, ckLiFrontShadows;
		Ck ckLiBrake, ckLiRevese, ckLiUnder;
		Ck ckLiBoost, ckLiThrust;
	#endif
		Ck ckLiCollect, ckLiColRndClr, ckLiTrack;  // 💎 gems

		//  🌊 water
		SlV(WaterSize);  //SlV(WaterSkip);
		Ck ckWaterReflect, ckWaterRefract;  void chkWater(Ck*);
		SlV(WaterDist);  SlV(WaterLod);	 void btnWaterApply(WP);

		//  🔮 reflection
		SV svReflSize, svReflSkip, svReflFaces;
		SlV(ReflDist);  SlV(ReflLod);  //SlV(ReflMode);
		SlV(ReflIbl);  void btnReflApply(WP);
	//}
	//----

	//  track path 
	Ogre::String pathTrk[2];    // 0 read only  1 //U user paths for save
	std::string TrkDir();  // path to track dir (from pSet settings)

	Ogre::String PathListTrk(int user=-1);
	Ogre::String PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	bool TrackExists(Ogre::String name);
	Ogre::String GetSceneryColor(Ogre::String name, Ogre::String* scenery=0);
	std::map<Ogre::String, Ogre::String> scnClr, scnN;


	///  🏞️ Track list/gallery
	void listTrackChng(Mli2, size_t);
	void imgTrkGal(WP img);

	void SortTrkList();
	void TrackListUpd(bool resetNotFound=false);

	Mli2 trkList =0;  Scv scvTracks =0;
	int xTrk = 0, yTrk = 0;
	std::vector<Img> imgGal;
	std::vector<Txt> txtGal;


	///  🏞️ Track  ----------------------------------------------------

	//  selected track name, user
	Ogre::String sListTrack;
	bool bListTrackU = 0;
	
	WP panTrkDesc[2] ={0,0};
	Ed trkDesc[2] ={0,0}, trkAdvice[2] ={0,0};  // description, advice
	bool needSort(Mli2 li);

	Img imgPrv[3], imgMini[3], imgTer[3];  // view,  mini: road, terrain, [2] is fullscr
	Img imgMiniPos[3];  MyGUI::RotatingSkin* imgMiniRot[3];
	Img imgTrkIco1 =0, imgTrkIco2 =0;
	void ImgPrvClk(WP), ImgTerClk(WP), ImgPrvClose(WP), ImgPrvInit();


	//  📝 st- road stats, dim  inf- tracks.ini ratings
	const static int InfTrk = 13, ImStTrk = 4,
	#ifdef SR_EDITOR
		StTrk = 9;
	#else
		StTrk = 14;
	#endif
	Txt stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs
	Img imStTrk[2][ImStTrk], imInfTrk[2][InfTrk];
	//  for user tracks.xml
	SV svUserRate;  void slUserRate(SV*);  int iUserRate = 3;
	Ck ckUserBookm;  void chkUserBookm(Ck*);  bool bUserBookm = false;

	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack,
		float timeCur, bool reverse=false, int champ=0);
	void ReadTrkStats();

	//  📰 track views
	Cmb cmbTrkView =0;  void comboTrkView(CMB);	
	void btnTrkFilter(WP);
	void ChangeTrackView(), updTrkListDim();


	//  🔻 filter  ---
	Txt txtTracksFAll =0, txtTracksFCur =0;
	Ck ckTrkFilter;  void chkTrkFilter(Ck*);
	SV svTrkFilMin[COL_FIL], svTrkFilMax[COL_FIL];  void slTrkFil(SV*);
	
	//  🏛️ columns
	Ck ckTrkColVis[COL_VIS];  void chkTrkColVis(Ck*);
	void ChkUpd_Col();
	//  const list column widths  and 🌈 value colors
	const static int colTrk[35],
		iClrsVal = 22, iClrsDiff = 9, iClrsRating = 7, iClrsLong = 11, iClrsSum = 10;
	const static Ogre::String
		clrsVal[iClrsVal], clrsDiff[iClrsDiff], clrsRating[iClrsRating], clrsLong[iClrsLong], clrsSum[iClrsSum],
		getClrVal(int), getClrDiff(int), getClrRating(int), getClrLong(int), getClrSum(int);


	//  🔍 Find track
	Ogre::String sTrkFind;
	Ed edTrkFind =0;  void editTrkFind(Ed); 

	//  📃 List fill
	strlist liTracks,liTracksUser;
	std::list<TrkL> liTrk;
	void FillTrackLists();

	void AddTrkL(std::string name, int user, const TrackInfo* ti, const UserTrkInfo* ui);
	void trkListNext(int rel);


	///  🖥️ Screen  --------------------------
	Ck ckVRmode;  // unused
	Cmb resList =0;
	void InitGuiScreenRes(), ReInitGuiRSrest(), ResizeOptWnd();
	//  util
	std::vector<WP> vwGuiRS;  // RS rest widgets to destroy
	void setRScfg(const Ogre::String& cfg, const Ogre::String& val);
	Ogre::String getScreenRes(int* x=0, int* y=0);

	CK(VidFullscr);  CK(VidVSync);
	void btnResChng(WP);
	void cmbGraphicsPreset(CMB), cmbScreenRes(CMB), cmbRenderSystem(CMB), cmbAntiAliasing(CMB);
#ifndef SR_EDITOR
	void UpdSimQuality();
#endif

	//  💡 Light  --------------------------
	SV svBright, svContrast, svLightsGrid;
	SlV(CarLightBright);

	//  🪄 Effects  --------------------------
	Ck ckHDR;

	Ck ckSSAO;
	SV svSsaoRadius, svSsaoScale;
	
	Ck ckGI;
	Txt txGIinfo =0;
	void btnGInext(WP), btnGIhq(WP), btnGIvis(WP), btnGIvis2(WP);
	void btnGIbncInc(WP), btnGIbncDec(WP), btnGIiso(WP);

	// CK(AllEffects);
	// Ck ckSoftPar, ckHDR, ckBloom, ckMotBlur, ckGodRays;
	// void chkEffUpd(Ck*), chkEffUpdShd(Ck*);

	// SV svBloomInt, svBloomOrig;
	// void slEffUpd(SV*);
	// SV svHDRBloomInt, svHDRBloomOrig;


	//  ⚙️ Settings  --------------------------
	//  startup common	
	Ck ckStartInMain, ckAutoStart, ckEscQuits;
	Ck ckOgreDialog, ckMouseCapture;
	Ck ckCacheHlms, ckCacheShaders, ckDebugShaders, ckDebugProperties;


	//  🔗 util  --------------------------
	void OpenBrowserUrl(std::string url);


	std::vector<Ogre::String> vsMaterials, vsTweakMtrs;
	void GetMaterials(    Ogre::String filename, bool clear=true, Ogre::String type="hlms");  // ogre resource
	void GetMaterialsMat( Ogre::String filename, bool clear=true, Ogre::String type="hlms");  // direct path+file
	void GetMaterialsJson(Ogre::String filename, bool clear=true, Ogre::String type="pbs", Ogre::String matStart="\t\t\"");

    
   	//  🔧 Tweak Mtr - Material Editor  --------------------
	void InitGuiTweakMtr(), InitClrTweakMtr();
	void GetTweakMtr(), FillTweakMtr();
	Li liTweakMtr =0;  void listTweakMtr(Li, size_t);
	
	std::vector<std::pair<Ogre::String,Ogre::String>> clrTweakMtr;
	Ogre::String ClrName(Ogre::String s);

	void slTweakMtr(SV*), updTweakMtr();
	Ed edMtrFind =0;  void editMtrFind(Ed);
	
	void btnMatEditor(WP);
	void btnMtrLoad(WP), btnMtrSave(WP), btnMtrSaveAll(WP);

	struct TwkMtr  // adjust sld
	{
		// HlmsPbsDbCar *db =0;
		HlmsPbsDb2* db =0;
	#define SVf(n)  SV sv##n;  float f##n =0.f;
		SVf(DiffR)  SVf(DiffG)  SVf(DiffB)
		SVf(SpecR)  SVf(SpecG)  SVf(SpecB)
		SVf(FresR)  SVf(FresG)  SVf(FresB)  //SVf(Fresnel)
		SVf(Rough)  //SVf(Metal)
		SVf(ClearCoat)  SVf(ClearRough)
		SVf(BumpScale)  SVf(Transp)

  		SV svUser[3][4];  float fUser[3][4] = {0.f,};
  		SV svDet[4][4];  float fDet[4][4] = {0.f,};

		Ed edInfo =0;
	#undef SVf
	} twk;

};
