#pragma once
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreString.h>
#include <vector>


enum CamTypes
{
	CAM_Follow = 0,	//  0 Follow - car rotY & pos from behind car, smooth
	CAM_Free,		//  1 Free   - free rot, pos from car
	CAM_Arena,		//  2 Arena  - free pos & rot, fly
	CAM_Car,		//  3 Car    - car pos & rot, full
	CAM_ExtAng,		//  4 Extended, angles - car rotY & pos, smooth, const distance
	CAM_ALL
};

const char CAM_Str[CAM_ALL][10] =
	{"Follow", "Free", "Arena", "Car", "ExtAng" };


class CameraView
{
public:
	CamTypes  mType = CAM_Follow;
	Ogre::String  mName{"Follow Default"};
	Ogre::Real  mDist = 7.f, mSpeed = 10.f, mSpeedRot = 10.f, mOfsMul = 1.f;
	Ogre::Radian  mYaw{0.f}, mPitch{7.f};
	Ogre::Vector3 mOffset{0, 1.2f, 0.f};
	int mMain = 0, mHideGlass = 0;
};


namespace Ogre {  class TerrainGroup;  class Camera;  class OverlayElement;  class SceneNode;  }
struct PosInfo;  class SETTINGS;  struct Cam;
class COLLISION_WORLD;  class btRigidBody;

//#define CAM_TILT_DBG  // show wheels in ray hit poses


class FollowCamera
{
public:
	FollowCamera(Cam* cam1, SETTINGS* pSet1);
	~FollowCamera();

	SETTINGS* pSet =0;
	Cam* cam =0;  // has camera and node

	//  🟢 Ogre
	Ogre::TerrainGroup* mTerrain =0;
	class btRigidBody* chassis =0;

	///  state vars  ----
	Ogre::Vector3 mLook, mPosNodeOld;

	Ogre::Quaternion qq;  // for ext cam
	Ogre::Radian mAPitch, mAYaw, mATilt;  // angles for arena cam, smoothing

	Ogre::Vector3 camPosFinal;  // final for mCamera
	Ogre::Quaternion camRotFinal;

	Ogre::Real mVel =0.f;
	Ogre::Real mDistReduce =0.f;  //float dbgLen;

	#ifdef CAM_TILT_DBG
		Ogre::Vector3 posHit[4];
	#endif

	///  💫 update, simulates camera  ----
	void update(Ogre::Real time, const PosInfo& posInPrev, PosInfo* posOut, COLLISION_WORLD* world, bool bounce, bool sphere);
	
	//  apply, sets mCamera's pos and rot
	void Apply(const PosInfo& posIn);

	//  🖱️ mouse move
	void Move( bool mbLeft, bool mbRight, bool mbMiddle, bool shift, Ogre::Real mx, Ogre::Real my, Ogre::Real mz );
	Ogre::Real fMoveTime = 0.f;


	//  ❔ move info string with values  ----
	char ss[512] = {0,};
	bool updInfo(Ogre::Real time = 0);
	Ogre::String sName;  bool updName = 0;

	//  info text formats
	Ogre::String sFmt_Follow, sFmt_Free, sFmt_ExtAng, sFmt_Arena, sFmt_Car;
	void updFmtTxt();


	//  🎥🎥 Cameras, Views  --------
	CameraView* ca =0;
	int miCount = 0, miCurrent = 0;
	std::vector<CameraView*> mViews;
	
	//  first (after change, reset etc)
	bool first = 1;  int iFirst = 0;  void First();


	bool loadCameras();  void saveCamera(), Destroy();
	void updView(), incCur(int dir);
	void Next(bool bPrev = false, bool bMainOnly = false);
	void setCamera(int ang);
	
	bool TypeCar() {  return ca && ca->mType == CAM_Car;  }
};
