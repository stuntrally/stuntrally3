#pragma once
#include "SColor.h"
#include "mathvector.h"
#include "quaternion.h"
// #include <Ogre.h>
#include <OgreCommon.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreQuaternion.h>

namespace Ogre {  class SceneNode;  class Item;  class ParticleSystem;  }
namespace Forests {  class GrassLayer;  }


/*struct RoadSurface  // todo ..
{
	float dust, mud, dustS, smoke;  // particles intensities, S size
	SColor tclr;  Ogre::Vector4 tcl;  // trail color, rgba copy

	//  surface params bind
	std::string surfName;  int surfId;
	RoadSurface();
};*/

struct TerLayer		//. 🏔️ terrain texture layer
{
	bool on = true, triplanar = false;  // for highest slopes
	float tiling = 8.f;   // scale, texture repeat
	Ogre::String texFile, texNorm;  // textures _d, _n

	// particles intensities, S size  // todo create, auto from presets..
	float dust = 0.f, dustS = 0.2f, mud = 0.f, smoke = 0.f;
	SColor tclr{0.16f,0.5f,0.2f,0.7f};  Ogre::Vector4 tcl;  // trail color, rgba copy
	
	///  🏔️ blendmap
	//  min,max range and smooth range for terrain angle and height
	float angMin = 0.f, angMax = 90.f, angSm = 20.f;
	float hMin = -300.f, hMax = 300.f, hSm = 20.f;
	//  noise
	bool nOnly = false;  // ignores ranges above
	float noise = 1.f, nprev = 0.f, nnext2 = 0.f;   //  factors to blend layer +1,-1,+2
	float nFreq[2] = {25.f, 30.f}, nPers[2] = {0.3f, 0.4f};
	float nPow[2] = {1.5f, 1.2f};  int nOct[2] = {3,3};
	float fDamage = 0.f;  // car damage per sec
	
	//  surface params bind
	std::string surfName{"Default"};  int surfId = 0;  //!
};

//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class TerData		//  ⛰️ Terrain
{
public:	
	//  height field
	std::vector<float> hfHeight;
	int getFileSize(std::string filename);  // Inits iVertsX etc
	
	//  🗜️ size
	int iVertsX = 512,  // always 2^n  1024 etc
		iVertsXold = 512;  // 2^n + 1  1025 old SR 2.7, before convert
	float fTriangleSize = 1.f, fTerWorldSize = 512.f;  // scale size
	void UpdVals();

	//  pos
	float ofsZ = 0.f;  // not 0, after convert
	float posX = 0.f, posZ = 0.f;  // move, not 1st ter, etc

	//  🏔️ layers	
	const static int ciNumLay = 6;  // all, for edit
	TerLayer layersAll[ciNumLay];
	std::vector<int> layers;  // active only (on)
	void UpdLayers();  // tripl

	//  📊 graphics options
	//  which should have triplanar most (e.g. high slope mountains)
	int triplanarLayer1 = 8, triplanarLayer2 = 8, triplCnt = 0;  // off
	// float normScale;  // scale terrain normals -not used-

	bool emissive = false;  // emissive light from specular
	// float specularPow;  // -not used-
	float specularPowEm = 2.f;  // specular power (exponent)

	int iHorizon = 0;  // 0 off, 1st, 2nd
	//  🎳 physics
	bool collis = true;  // game, physics
	bool bL=1,bR=1,bF=1,bB=1;  // border planes < > ^ v
	
	//  methods
	TerData();	void Default();
	float getHeight(const float& fi, const float& fj);
};


class PagedLayer	//  🌳🪨 Vegetation model
{
public:
	bool on = false;
	Ogre::String name;  float dens = 0.1f;
	float windFx = 0.f, windFy = 0.f;
	int addRdist = 0, maxRdist = 20;  // add,max dist to road
	float minScale = 0.1f, maxScale = 0.25f, ofsY = 0.f;
	float maxTerAng = 50.f, minTerH = -100.f, maxTerH = 100.f;  // terrain
	float maxDepth = 5.f;  // in fluid
	int cnt = 0;  // count on track, for stats
};

//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class SGrassLayer	//  🌿 Grass layer
{
public:
	bool on = false;
	float dens = 0.1f;
	float minSx = 1.2f, minSy = 1.2f, maxSx = 1.6f, maxSy = 1.6f;  // x,y size range
	float swayDistr = 4.f, swayLen = 0.2f, swaySpeed = 0.5f;  // sway
	int iChan = 0;  // which channel to use
	Ogre::String material{"grassForest"},
		colorMap{"grClrForest.png"}; // not used, drop-
};

class SGrassChannel  //. 🌿⛰️ grass channel
{
public:
	//  min,max range and smooth range for terrain
	float angMin = 0.f, angMax = 30.f, angSm = 20.f;  // angle
	float hMin = -100.f, hMax = 100.f, hSm = 20.f;  // and height
	float noise = 0.0f, nFreq = 25.f, nPers = 0.3f, nPow = 1.2f;  int nOct = 3;  // noise params
	float rdPow = 0.f;  // road border adjust
};

//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class FluidBox		//  💧 fluid box shape - water, mud, etc.
{
public:
	Ogre::Vector3 pos{0,0,0}, rot{0,0,0}, size{0,0,0};
	Ogre::Vector2 tile{0.01,0.01};

	int id = -1;  // auto set, index to FluidParams, -1 doesnt exist
	std::string name;

	class btCollisionObject* cobj = 0;
	int idParticles = 0;   // auto set  index for wheel particles  -1 none
	bool solid = false, deep = false;  // auto set, from FluidParams
};


class Object		//  📦 object - mesh (static🪨) or prop (dynamic)
{
public:
	MATHVECTOR<float,3> pos{0,0,0};
	QUATERNION<float> rot{0,-1,0,0};
	Ogre::Vector3 scale{1,1,1};
	std::string name;  // mesh file name

	Ogre::SceneNode* nd = 0;  // ogre
	Ogre::Item* it = 0;
	class btDefaultMotionState* ms = 0;  // 🎳 bullet
	class btCollisionObject* co = 0;
	class btRigidBody* rb = 0;
	bool dyn = false;
	class btTransform* tr1 = 0;  // 1st pos after load, for reset

	void SetFromBlt();
};


class SEmitter		//  🔥 particles ⛅☢️
{
public:
	std::string name;  // particle_system
	Ogre::Vector3 pos{0,0,0}, size{1,1,1};
	Ogre::Vector3 up{0,1,0};  float rot = 0.f;  // dir
	float rate = 0.f;  // emit

	float upd = 2.f;  // update time for static
	bool stat = false;  // static for, e.g. clouds

	Ogre::SceneNode* nd = 0;  // ogre
	Ogre::ParticleSystem* ps = 0;
	
	void UpdEmitter();
};
