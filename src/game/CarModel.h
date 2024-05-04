#pragma once
#include "mathvector.h"
#include "quaternion.h"
#include "cardefs.h"
#include "CarPosInfo.h"

#include <OgreVector3.h>
#include <OgreMatrix4.h>
#include <OgrePrerequisites.h>
#include <OgreColourValue.h>
#include <vector>
#include <string>

namespace Ogre {
	class SceneNode;  class Camera;  class SceneManager;
	class ParticleSystem;  class Item;  //class Terra;
	namespace v1 {  class RibbonTrail;  class BillboardSet;  }  }
namespace MyGUI {  class TextBox;  }
class SETTINGS;  class GAME;  class CAR;
class Scene;  class App;  class FollowCamera;  struct Cam;
class HlmsPbsDb2;


//  CarModel is the "Ogre" part of a Vehicle.
//  It is used to put meshes together, particle emitters, game logic etc.
//------------------------------------------------------------------------------------------------
class CarModel
{
public:
	// 🚗 Car Type |  Source          | 🎥 Camera  | 🔨 Physics (VDrift CAR)
	//-------------+------------------+------------+------------
	// CT_LOCAL:   | 👥 Local player  | yes        | yes
	// CT_REMOTE:  | 📡 Network       | no         | yes
	// CT_REPLAY:  | 📽️ Replay file   | yes        | no 
	// CT_GHOST:   | 👻 Ghost file    | no         | no 

	// CT_GHOST2:  | 🚗👻 other car's ghost file .rpl
	// CT_TRACK:   | 🏞️👻 track's ghost file .gho

	enum eCarType
	{	CT_LOCAL=0, CT_REMOTE, CT_REPLAY,  CT_GHOST, CT_GHOST2, CT_TRACK };
	eCarType    cType = CT_LOCAL;
	VehicleType vType = V_Car;
	int numWheels = 4;
	int numLights = 0;  // front

	bool isGhost()    const {  return cType >= CT_GHOST;  }
	bool isGhostTrk() const {  return cType == CT_TRACK;  }
	void SetNumWheels(int n);
	bool hasRpm()     const {  return vType == V_Car;  }
	bool isHover()    const
	{	return vType == V_Hovercar || vType == V_Hovercraft || vType == V_Drone;  }
	

	//  🌟 ctor  ----
	CarModel(int index, int colorId,
		eCarType type, const std::string& name,
		Cam* cam1, App* app);
	~CarModel();
	
	App* pApp =0;
	GAME* pGame =0;
	SETTINGS* pSet =0;
	Scene* sc =0;

	//  🟢 Ogre
	Ogre::SceneManager* mSceneMgr =0;


	Ogre::String sDispName;  // diplay name in opponents list (nick for CT_REMOTE)
	MyGUI::TextBox* pNickTxt =0;  // multiplayer 3d nick above car

	bool updTimes = true, updLap = true;
	float fLapAlpha = 1.f;
	
	
	///  📄 Config  ------------------------------------------------
	void LoadConfig(const std::string & pathCar), Defaults();
	void Load(int startId, bool loop);  // create game CAR
	Ogre::Vector3 posFix(float pos[3]);

	//  🚗 model params  from .car
	float interiorOfs[3];
	
	//  🎥 mounted cameras
	float driver_view[3], hood_view[3], ground_view[3];


	//  🔴 brake, front light flares  --------
	struct FlareSet
	{
		std::vector<Ogre::Vector3> pos;
		std::vector<float> lit;  // same size, light power, 0 off
		float size = 0.2f;
		Ogre::ColourValue clr{1,1,1,1};
	}
	fsBrakes, fsFlares, fsReverse, fsUnder;
	
	//  💨🔥 boost, thrusters  --------
	int boostCnt =0;  Ogre::Vector3 boostPos[PAR_BOOST];
	float boostClr[3], thrustClr[3];  // light clr
	std::string sBoostParName;

	struct Thruster  // 🚀 spaceships
	{
		std::string sPar;  // particles
		float ofs[3] = {0.f,0.f,0.f}, sizeZ = 0.f;
		int lit = 0;  // light
	}
	thruster[PAR_THRUST];


	// ⚫💭 for tire trails
	std::vector<float> whRadius, whWidth;
	std::vector<MATHVECTOR<float,3>> whPos;
	QUATERNION<float> qFixWh[2];

	float maxangle = 26.f;  // steer
	bool bRotFix =0;

	//  💨 exhaust position for boost particles-
	// bool manualExhaustPos;  // if true, use values below, if false, guess from bounding box
	// bool has2exhausts;  // car has 2nd exhaust, if true, mirror exhaust 1 for position
	// float exhaustPos[3];  // position of first exhaust
	

	///  🆕 Create  ------------------------
	void Create(), Destroy();  // CreateReflection()

	void CreatePart(Ogre::SceneNode* ndCar, Ogre::Vector3 vPofs,
		Ogre::String sCar2, Ogre::String sCarI, Ogre::String sMesh, Ogre::String sMat,
		bool ghost, Ogre::uint32 visFlags,
		Ogre::Aabb* bbox=0, bool bLogInfo=true, bool body = false);

	void LogMeshInfo(const Ogre::Item* ent, const Ogre::String& name, int mul=1);
	int all_subs = 0, all_tris = 0;  //stats
	
	
	//  💫 Update  --------
	void Update(PosInfo& posInfo, PosInfo& posInfoCam, float time);
	void UpdKeysCam();  // for camera X,C, last chk 0


	//  resource  --------
	int iIndex = 0;  // car id for  timer, camview
	std::string sDirname;  // dir name of car (e.g. ES)
	Ogre::String resGrpId, mtrId;  // resource group name, material suffix
	std::string resCar;  // path to car textures
	//  🎨 paint
	int iColor = 0;  // color id
	Ogre::ColourValue color;  // for minimap pos tri color
	HlmsPbsDb2 *db =0;
	void SetPaint();  //  Apply new color
	
	
	///  🎥 Camera, can be null  ----
	Cam* cam =0;
	FollowCamera* fCam =0;
	int iCamFluid = -1;   // id to fluids[], -1 none
	float fCamFl = 0.6f;  // factor, close to surface
	float camDist;  // mul from .car

	int iFirst = 0;
	void First();  // reset camera after pos change etc	


	//  🚗 Main node  --------
	Ogre::SceneNode* ndMain =0, *ndSph =0;
	Ogre::Vector3 posSph[2] = {{0,0,0},{0,0,0}};  // 🟢🌿 grass sphere

	//  hide/show
	void setVisible(bool visible);
	bool bVisible = true;  float hideTime = 1.f;
		
	//  🚗 VDrift car
	CAR* pCar =0;  // all need this set (even ghost, has it from 1st car)


	//  💡 Lights  --------
	enum LiType
	{	LI_Front, LI_Brake, LI_Revese, LI_Boost, LI_Thrust, LI_Under, LI_All  };
	const std::string strLiType[LI_All] =
	{	"Front", "Brake", "Revese", "Boost", "Thrust", "Under" };
	
	struct CarLight
	{
		Ogre::Light* li =0;
		LiType type = LI_Front;
		float power = 1.f;
	};
	std::vector<CarLight> lights;
	std::vector<int> liCnt;  // each type counter, size LI_All
	void updLightsBright();
	
	void CreateLight(Ogre::SceneNode* ndCar, LiType type,
		Ogre::Vector3 pos, Ogre::ColourValue c, float power);
	
	
	///  🔵 Checkpoints  --------
	int iInChk = -1, iCurChk = -1,
		iNextChk = 0, iNumChks = 0;  // cur checkpoint -1 at start
	int iWonPlace = 0, iWonPlaceOld = 0;  // 👥🥇
	bool bWrongChk = 0;  int iInWrongChk = -1;  // ❌
	float fChkTime = 0.f, timeAtCurChk = 0.f;
	
	//  🏁 start/finish pos
	bool bInStart = 0, bGetStart = true;
	Ogre::Matrix4 matStart;  Ogre::Vector4 vStartDist{0,0,0,0};
	float iWonMsgTime = 0.f;
	Ogre::Vector3 vStartPos{0,0,0};

	void ResetChecks(bool bDist=false), UpdNextCheck(), ShowNextChk(bool visible);
	Ogre::String sChkMtr;  bool bChkUpd = true;


	//  ➰🎥 for loop camera change
	int iLoopChk = -1, iLoopLastCam = -1;
	//  cam,chk old states
	int iCamNextOld = 0;  bool bLastChkOld = 0;

	///  Logic vars  ----
	float angCarY = 0.f;  // car yaw angle for minimap
	float distFirst = 1.f, distLast = 1.f, distTotal = 10.f;  // checks const distances set at start

	//  % of track driven
	float trackPercent = 0.f;
	void UpdTrackPercent();

	
	//  ✨ Particle systems  --------
	enum EParTypes {  PAR_Smoke=0, PAR_Mud, PAR_Dust, PAR_Water, PAR_MudHard, PAR_MudSoft, PAR_ALL };
	//  par-wheels, boost-car rear, spaceship thruster, sparks-world hit
	Ogre::ParticleSystem* par[PAR_ALL][MAX_WHEELS];
	Ogre::ParticleSystem* parBoost[PAR_BOOST], *parThrust[PAR_THRUST*2], *parHit;
	std::vector<Ogre::v1::RibbonTrail*> whTrail;  // tire trail
	std::vector<Ogre::Real> whTemp;  // spin time, approx tire temp.

	void UpdParsTrails(bool visible=true);


	//  ⚫ Wheels, Nodes  --------
	std::vector<Ogre::SceneNode*> ndWh, ndWhE, ndBrake;
	
	//  ⛰️ track surface for wheels
	void UpdWhTerMtr();
	Ogre::String txtDbgSurf;

	//  🥛 next chk beam
	Ogre::SceneNode* ndNextChk =0;
	Ogre::Item* itNextChk =0;


	//  💥 to destroy
	std::vector<Ogre::SceneNode*> vDelNd;		void ToDel(Ogre::SceneNode* nd);
	std::vector<Ogre::Item*> vDelIt;			void ToDel(Ogre::Item* it);
	std::vector<Ogre::ParticleSystem*> vDelPar;	void ToDel(Ogre::ParticleSystem* par);

	//  🔴 brakes, front flares
	Ogre::v1::BillboardSet* bsBrakes =0, *bsFlares =0, *bsReverse =0;

	//  💡 Lights
	bool bBraking = 1, bLightsOld =0, bLights =0, bReverseOld =0, bReverse = 0;
};
