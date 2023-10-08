#pragma once
// #include "PreviewTex.h"
//#include "SceneClasses.h"
#include "Road.h"
#include "Reflect.h"
#include "Grid.h"
#include <OgreVector3.h>
#include <OgreString.h>

namespace Ogre  {
	class Terra;  class Atmosphere2Npr;  class HlmsPbsTerraShadows;
	class Light;  class SceneNode;  class Camera;  class SceneManager;
	class Viewport;  class Root;  class ParticleSystem;  }
class App;  class Scene;  class CData;  class TerData;
class SplineRoad;  class PaceNotes;  class Grass;


///  🟢 Ogre parts for a track/map
//--------------------------------------------------------------------------------
class CScene
{
public:
	//  🌟 ctor  ----
	App* app = 0;
	CScene(App* app1);
	~CScene();

	void destroyScene();


	//  Shadows-
	// void changeShadows(), UpdShaderParams(), UpdPaceParams();
	

	///  📄 Setup  scene.xml
	Scene* sc = 0;

	//  const, xmls
	CData* data = 0;

	
	//  All:  sun, atmo/fog, weather
	//-----------------------------------
	void CreateAllAtmo(), DestroyAllAtmo();

	//  ⛅ Sky ~
	enum ESkies { SK_Main=0,  SK_CubeRefl,  SK_PlanarRefl, SK_ALL };
	Ogre::String sMeshSky[SK_ALL];
	Ogre::Item* itSky[SK_ALL] = {0, 0, 0};
	Ogre::SceneNode* ndSky[SK_ALL] = {0, 0, 0};

	void CreateSkyDome(Ogre::String sMater, float yaw), UpdSkyScale();
	void DestroySkyDome(), UpdSky();

	//  🌞 Sun *
	Ogre::Light* sun = 0;
	Ogre::SceneNode *ndSun = 0;
	void CreateSun(), DestroySun(), UpdSun(float dt=0.f);

	//  🌫️ Fog / Atmo
	Ogre::Atmosphere2Npr* atmo = 0;
	void CreateFog(), DestroyFog(), UpdFog(bool force_on =false, bool force_off =false);
	
	//  🌧️ Weather :  rain, snow
	Ogre::ParticleSystem *pr = 0, *pr2 = 0;
	void CreateWeather(), DestroyWeather();
	void UpdateWeather(Ogre::Camera* cam, float lastFPS = 60.f, float emitMul = 1.f);


	//  🔥 Emitters
	void CreateEmitters(bool force=false), DestroyEmitters(bool clear);


	//  💧 Fluids  water, mud
	//-----------------------------------
	FluidsReflect refl;  // with RTT


	//  🛣️ Road
	//-----------------------------------
	int rdCur = 0;  // cur
	std::vector<SplineRoad*> roads;
	SplineRoad* road = 0;  // main
	
	GridMtrs grid;

	//  driving aids, game only
	SplineRoad* trail = 0;
	PaceNotes* pace = 0;
	void DestroyRoads(), DestroyPace(), DestroyTrail();

	
	//  🌳🪨 Vegetation
	//-----------------------------------
	Ogre::Image2* imgRoad = 0;  int imgRoadSize = 0;
	void LoadRoadDens(), DelRoadDens();

	void CreateTrees(), DestroyTrees(), RecreateTrees(); //, updGrsTer(), UpdCamera();
	int iVegetAll = 0;  // for info
	std::vector<Ogre::Item*> vegetItems;
	std::vector<Ogre::SceneNode*> vegetNodes;
	
	//  🌿 Grass
	Grass* grass = 0;


	///  ⛰️ Terrain
	//-----------------------------------
	int terCur = 0, iNewHmap = -1;  // cur, ed -new
	std::vector<Ogre::Terra*> ters;  // all terrains

	Ogre::Terra* ter = 0;  // \ cur for edit
	TerData* td = 0;       // / set in TerNext
	void TerNext(int add);
	Ogre::String getHmap(int n, bool bNew);  // file name

	Ogre::Real getTerH(Ogre::Real x, Ogre::Real z);
	bool getTerH(Ogre::Vector3& pos);  // sets y, all ters
	bool getTerH(int id, Ogre::Vector3& pos);  // sets y, 1 ter

	//  HlmsListener  to make PBS objects also be affected by terrain's shadows
	Ogre::HlmsPbsTerraShadows* mHlmsPbsTerraShadows = 0;  // 1st ter only-

	void CreateTerrain1(int n);
	void CreateTerrains(bool terLoad/*=true*/);
	void CreateTerrain(int n, bool terLoad);
	void DestroyTerrain1(int n), DestroyTerrains();
	void CreateBltTerrains(), copyTerHmap(), updTerLod();

	
	//  🏔️ Blendmap, rtt
	//-----------------------------------
	void UpdBlendmap();
	// void UpdGrassDens(), UpdGrassPars();
	
	//  tex, mtr names
	const static Ogre::String sHmap, sAng,sAngMat,
		sBlend,sBlendMat, sGrassDens,sGrassDensMat;
	
	// todo: grassDensRTT;


	//  🌀 noise  -------
	static float Noise(float x, float zoom, int octaves, float persistence);
	static float Noise(float x, float y, float zoom, int octaves, float persistance);
	//     xa  xb
	//1    .___.
	//0__./     \.___
	//   xa-s    xb+s    // val, min, max, smooth range
	inline static float linRange(const float& x, const float& xa, const float& xb, const float& s)
	{
		if (x <= xa-s || x >= xb+s)  return 0.f;
		if (x >= xa && x <= xb)  return 1.f;
		if (x < xa)  return (x-xa)/s+1;
		if (x > xb)  return (xb-x)/s+1;
		return 0.f;
	}
};
