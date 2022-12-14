#pragma once
#include "Gui_Def.h"
#include "SliderValue.h"

#include <MyGUI_Prerequest.h>
//#include <MyGUI_Types.h>
#include <MyGUI_WidgetToolTip.h>
#include <MyGUI_Enumerator.h>
#include <OgreString.h>
#include "settings_com.h"


namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class Window;  class Viewport;  class Light;  }
class Scene;  class CData;  class SplineRoad;
class App;  class SETTINGS;


//  tracks,cars list items - with info for sorting
struct TrkL
{
	std::string name;
	const class TrackInfo* ti;  // 0 means user trk
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


	///  ποΈ Gui Common   --------------------------

	//  ποΈ resize
	void SizeGUI(), doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	float GetGuiMargin(int wy);
	void setOrigPos(WP wp, const char* relToWnd);
	void CreateFonts();  // π 

	//  β tooltip
	WP mToolTip =0;  Ed mToolTipTxt =0;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(WP sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(WP moving, const MyGUI::IntPoint& point);

	//  πΊοΈ language
	bool bGuiReinit = 0;  // lang change
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>

	void UnfocusLists();
	Btn bnQuit =0;  void btnQuit(WP);

	//  π init
	void GuiCenterMouse();
	void GuiInitTooltip(), GuiInitLang();
	//  init tabs
	void GuiInitGraphics(), GuiInitTrack(), initMiniPos(int i);

	MyGUI::TabPtr FindSubTab(WP tab);  // util
	

	///  π Graphics  --------------------------

	//  textures
	SlV(Anisotropy);
	void comboTexFilter(CMB), cmbAntiAliasing(CMB);
	// SV svTerTripl;
	void btnShadersApply(WP);

	//  geom detail
	SlV(ViewDist);  SlV(LodBias);
	SlV(TerDetail);  SV svRoadDist;

	//  π³πͺ¨πΏ veget
	SV svTrees, svGrass, svTreesDist, svGrassDist;
	void btnVegetReset(WP), btnVegetApply(WP);

	//  π shadow
	SV svShadowType, svShadowCount, svShadowSize, svShadowDist;
	void btnShadowsApply(WP);

	//  π‘ lights
	Ck ckCarLights, ckCarLightsShadows;

	//  π? reflection
	SV svReflSkip, svReflFaces, svReflSize;
	SlV(ReflDist);  SlV(ReflMode);

	//  π water
	SlV(WaterSize);
	Ck ckWaterReflect, ckWaterRefract;  void chkWater(Ck*);

	// Ck ckLimitFps;
	// SV svLimitFps,svLimitSleep;
	

	//  track path 
	Ogre::String pathTrk[2];    // 0 read only  1 //U user paths for save
	std::string TrkDir();  // path to track dir (from pSet settings)

	Ogre::String PathListTrk(int user=-1);
	Ogre::String PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	bool TrackExists(Ogre::String name);
	Ogre::String GetSceneryColor(Ogre::String name, Ogre::String* scenery=0);
	std::map<Ogre::String, Ogre::String> scnClr, scnN;


	///  ποΈ Track  ----------------------------------------------------

	//  selected track name, user
	Ogre::String sListTrack;
	bool bListTrackU = 0;
	
	void listTrackChng(Mli2, size_t);
	void SortTrkList();
	void TrackListUpd(bool resetNotFound=false);

	Mli2 trkList =0;
	Ed trkDesc[2], trkAdvice[2];  // description, advice
	bool needSort(Mli2 li);

	Img imgPrv[3], imgMini[3], imgTer[3];  // view,  mini: road, terrain, [2] is fullscr
	Img imgMiniPos[3];  MyGUI::RotatingSkin* imgMiniRot[3];
	Img imgTrkIco1 =0, imgTrkIco2 =0;
	

	//  π st- road stats, dim  inf- tracks.ini ratings
	const static int InfTrk = 13, ImStTrk = 4,
	#ifdef SR_EDITOR
		StTrk = 9;
	#else
		StTrk = 14;
	#endif
	Txt stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs
	Img imStTrk[2][ImStTrk], imInfTrk[2][InfTrk];

	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack,
		float timeCur, bool reverse=false, int champ=0);
	void ReadTrkStats();

	//  π° track views
	void btnTrkView1(WP), btnTrkView2(WP), btnTrkFilter(WP);
	void ChangeTrackView();
	void updTrkListDim();


	//  π» filter  ---
	Txt txtTracksFAll =0, txtTracksFCur =0;
	Ck ckTrkFilter;  void chkTrkFilter(Ck*);
	SV svTrkFilMin[COL_FIL], svTrkFilMax[COL_FIL];  void slTrkFil(SV*);
	
	//  ποΈ columns
	Ck ckTrkColVis[COL_VIS];  void chkTrkColVis(Ck*);
	void ChkUpd_Col();
	//  const list column widths
	const static int colTrk[33], iClrsDiff = 9, iClrsRating = 7, iClrsLong = 11, iClrsSum = 10;
	const static Ogre::String clrsDiff[iClrsDiff], clrsRating[iClrsRating], clrsLong[iClrsLong], clrsSum[iClrsSum];
	const static Ogre::String getClrDiff(int), getClrRating(int), getClrLong(int), getClrSum(int);


	//  π Find track
	Ogre::String sTrkFind;
	Ed edTrkFind =0;  void editTrkFind(Ed); 

	//  π List fill
	strlist liTracks,liTracksUser;
	std::list<TrkL> liTrk;
	void FillTrackLists();

	void AddTrkL(std::string name, int user, const class TrackInfo* ti);
	void trkListNext(int rel);


	///  π₯οΈ Screen  --------------------------
	Ck ckVRmode;
	Cmb resList =0;
	void InitGuiScreenRes(), ResizeOptWnd();

	CK(VidFullscr);  CK(VidVSync);
	void btnResChng(WP);
	void comboGraphicsAll(CMB), comboRenderSystem(CMB);

	//  π‘ Light  --------------------------
	SV svBright, svContrast;


	//  π util
	void OpenBrowserUrl(std::string url);
};
