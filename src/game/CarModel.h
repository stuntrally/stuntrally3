#pragma once
#include <Ogre.h>
// #include <OgreVector2.h>
// #include <OgreVector3.h>
// #include <OgreQuaternion.h>
// #include <OgreVector4.h>
// #include <OgreMatrix4.h>
// #include <OgreColourValue.h>
#include "OgrePrerequisites.h"
#include "mathvector.h"
#include "quaternion.h"
#include "cardefs.h"
#include "CarPosInfo.h"

/*namespace Ogre {  class SceneNode;  class Terrain;  class Camera;  class SceneManager;
	class ParticleSystem;  class Item;  class ManualObject;  class AxisAlignedBox;  
	namespace v1 {	class RibbonTrail;  class BillboardSet;  }  }*/
// namespace MyGUI {  class TextBox;  }
class SETTINGS;  class GAME;  class CAR;
class Scene;  class App;  class FollowCamera;  class CarReflection;


//  CarModel is the "Ogre" part of a vehicle.
//  It is used to put meshes together, particle emitters, etc.

class CarModel
{
public:
	/// -------------------- Car Types ---------------------------
	//              Source          Physics (VDrift car)    Camera
	// CT_LOCAL:    Local player    yes	                    yes
	// CT_REMOTE:   Network	        yes	                    no
	// CT_REPLAY:   Replay file     no                      yes
	// CT_GHOST:	Ghost file		no						no
	// CT_GHOST2:	other car's ghost file
	// CT_TRACK:	track's ghost file

	enum eCarType {  CT_LOCAL=0, CT_REMOTE, CT_REPLAY,  CT_GHOST, CT_GHOST2, CT_TRACK };
	eCarType    cType = CT_LOCAL;
	VehicleType vType = V_Car;
	int numWheels = 4;

	bool isGhost()    const {  return cType >= CT_GHOST;  }
	bool isGhostTrk() const {  return cType == CT_TRACK;  }
	void SetNumWheels(int n);
	
	//  ctor  ----
	CarModel(int index, int colorId, eCarType type, const std::string& name,
		Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* sc,
		Ogre::Camera* cam, App* app);
	~CarModel();
	
	App* pApp =0;
	GAME* pGame =0;
	SETTINGS* pSet =0;
	Scene* sc =0;
	Ogre::Camera* mCamera =0;
	Ogre::SceneManager* mSceneMgr =0;


	Ogre::String sDispName;  // diplay name in opponents list (nick for CT_REMOTE)
	// MyGUI::TextBox* pNickTxt =0;  // multiplayer nick above car

	bool updTimes = true, updLap = true;
	float fLapAlpha = 1.f;
	
	
	///----  model params  from .car
	float driver_view[3], hood_view[3], ground_view[3];  // mounted cameras
	float interiorOffset[3], boostOffset[3], boostSizeZ;
	float thrusterOfs[PAR_THRUST][3], thrusterSizeZ[PAR_THRUST];

	std::vector<Ogre::Vector3> brakePos;  // brake flares
	float brakeSize = 0.2f;
	Ogre::ColourValue brakeClr;
	
	std::string sBoostParName, sThrusterPar[PAR_THRUST];
	bool bRotFix;

	std::vector<float> whRadius, whWidth;  // for tire trails
	std::vector<MATHVECTOR<float,3> > whPos;
	QUATERNION<float> qFixWh[2];
	float maxangle = 26.f;  //steer

	//  exhaust position for boost particles
	bool manualExhaustPos;  // if true, use values below, if false, guess from bounding box
	bool has2exhausts;  // car has 2nd exhaust, if true, mirror exhaust 1 for position
	float exhaustPos[3];  // position of first exhaust
	
	void LoadConfig(const std::string & pathCar), Defaults();


	///--------  Create
	void Load(int startId, bool loop), Create(), /*CreateReflection(),*/ Destroy();
	void CreatePart(Ogre::SceneNode* ndCar, Ogre::Vector3 vPofs,
		Ogre::String sCar2, Ogre::String sCarI, Ogre::String sMesh, Ogre::String sEnt,
		bool ghost, Ogre::uint32 visFlags,
		Ogre::Aabb* bbox=0, bool bLogInfo=true, bool body = false);

	void LogMeshInfo(const Ogre::Item* ent, const Ogre::String& name, int mul=1);
	int all_subs = 0, all_tris = 0;  //stats
	
	
	//--------  Update
	void Update(PosInfo& posInfo, PosInfo& posInfoCam, float time);
	void UpdateKeys();  // for camera X,C, last chk F12


	//  reset camera after pos change etc	
	void First();
	int iFirst = 0;
	
	//  color
	Ogre::ColourValue color;  // for minimap pos tri color  //float hue, sat, val;
	void ChangeClr();  //  Apply new color
		
	//  track surface for wheels
	void UpdWhTerMtr();
	Ogre::String txtDbgSurf;
	
	void UpdParsTrails(bool visible=true);
	
	
	///----  Camera, can be null
	FollowCamera* fCam =0;
	int iCamFluid = -1;   // id to fluids[], -1 none
	float fCamFl = 0.6f;  // factor, close to surface
	float camDist;  // mul from .car

	//  Main node
	Ogre::SceneNode* pMainNode =0, *ndSph =0;
	Ogre::Vector3 posSph[2];
	Ogre::v1::BillboardSet* brakes =0;
	std::vector<Ogre::Light*> lights;
	
	void setVisible(bool visible);  // hide/show
	bool mbVisible = true;  float hideTime = 1.f;
		
	//  VDrift car
	CAR* pCar =0;  // all need this set (even ghost, has it from 1st car)
	
	
	///----  Logic vars
	float angCarY = 0.f;  // car yaw angle for minimap
	float distFirst = 1.f, distLast = 1.f, distTotal = 10.f;  // checks const distances set at start
	float trackPercent = 0.f;  // % of track driven
	void UpdTrackPercent();

	///  Checkpoint vars,  start pos, lap
	bool bGetStPos = true;  Ogre::Matrix4 matStPos;  Ogre::Vector4 vStDist;
	int iInChk = -1, iCurChk = -1,
		iNextChk = 0, iNumChks = 0,  // cur checkpoint -1 at start
		iInWrChk = -1,
		iWonPlace = 0, iWonPlaceOld = 0;
	float iWonMsgTime = 0.f;
	bool bInSt = 0, bWrongChk = 0;  float fChkTime = 0.f;
	float timeAtCurChk = 0.f;

	//bool Checkpoint(const PosInfo& posInfo, class SplineRoad* road);  // update
	Ogre::Vector3 vStartPos;
	void ResetChecks(bool bDist=false), UpdNextCheck(), ShowNextChk(bool visible);
	Ogre::String sChkMtr;  bool bChkUpd = true;
	//  for loop camera change
	int iLoopChk = -1, iLoopLastCam = -1;
	//  cam,chk old states
	int iCamNextOld = 0;  bool bLastChkOld = 0;
	
	
	//----  resource
	int iIndex = 0, iColor = 0;  // car id, color id
	std::string sDirname;  // dir name of car (e.g. ES)
	Ogre::String resGrpId, mtrId;  // resource group name, material suffix
	std::string resCar;  // path to car textures


	//--------  Particle systems
	enum EParTypes {  PAR_Smoke=0, PAR_Mud, PAR_Dust, PAR_Water, PAR_MudHard, PAR_MudSoft, PAR_ALL };
	//  par-wheels, boost-car rear, spaceship thruster, sparks-world hit
	Ogre::ParticleSystem* par[PAR_ALL][MAX_WHEELS];
	Ogre::ParticleSystem* parBoost[PAR_BOOST], *parThrust[PAR_THRUST*2], *parHit;
	std::vector<Ogre::v1::RibbonTrail*> whTrail;  // tire trail
	std::vector<Ogre::Real> whTemp;  // spin time, approx tire temp.
	
	//  Wheels, Nodes
	std::vector<Ogre::SceneNode*> ndWh, ndWhE, ndBrake;
	Ogre::SceneNode* ndNextChk =0;  // beam
	Ogre::Item* itNextChk =0;

	//  to destroy
	std::vector<Ogre::SceneNode*> vDelNd;		void ToDel(Ogre::SceneNode* nd);
	std::vector<Ogre::Item*> vDelIt;			void ToDel(Ogre::Item* it);
	std::vector<Ogre::ParticleSystem*> vDelPar;	void ToDel(Ogre::ParticleSystem* par);

	//  brake state
	bool bBraking = true;
	void UpdateBraking();
};
