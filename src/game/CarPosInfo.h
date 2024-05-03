#pragma once
#include "mathvector.h"
#include "quaternion.h"
#include "cardefs.h"
#include <OgreVector3.h>
#include <OgreQuaternion.h>

struct ReplayFrame;  struct ReplayFrame2;
class CAR;


//  Stores all the needed information about car coming from vdrift
//  position,rotation of car and wheels
//  and all data needed to update particles emitting rates and sounds

struct PosInfo
{
	bool bNew;  //  new posinfo available for Update
	//  🚗 car
	Ogre::Vector3 pos, carY;  // init in ctor
	//  ⚫ wheel
	const static int W = MAX_WHEELS;
	Ogre::Vector3 whPos[W];
	Ogre::Quaternion rot, whRot[W];

	float whVel[W], whSlide[W], whSqueal[W];
	int whTerMtr[W],whRoadMtr[W];

	float fboost,steer, percent;
	char braking,reverse;
	float hov_roll/*= sph_yaw for O*/, hov_throttle;

	//  💧 fluids
	float whH[W],whAngVel[W], speed, whSteerAng[W];  int whP[W];
	
	//  ✨ hit sparks
	float fHitTime, /*?fHitForce,*/fParIntens,fParVel;
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	
	//  🎥 camera view
	Ogre::Vector3 camPos;  Ogre::Quaternion camRot;
	Ogre::Vector3 camOfs;  // hit bounce offset

	//  🌟 ctor
	PosInfo();
	
	//  copy
	void FromRpl2(const ReplayFrame2* rf, class CARDYNAMICS* cd);
	void FromCar(CAR* pCar);
};
