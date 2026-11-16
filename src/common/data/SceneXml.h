#pragma once
#include "CScene.h"
#include "par.h"
#include "SColor.h"
#include "SceneClasses.h"

#include <OgreString.h>
#include <OgreVector3.h>
#include "mathvector.h"
#include "quaternion.h"

class CGuiCom;  class GAME;  class CScene;
class ReverbsXml;  class FluidsXml;
namespace tinyxml2 {  class XMLElement;  }


//  RoR only  for export,  not needed in SR
//------------------------
struct RoRconfig
{
	float lAmbAdd;  // 🌞 sun
	float lAmb, lDiff, lSpec;
	float fogMul;

	int water;        // 🌊 -1 off  0 auto  1 custom
	float yWaterOfs;  // add offset or height for custom
	
	float treesMul;   // 🌳🪨  veget
	float grassMul;   // 🌿

	int roadTerTexLayer;  // 🛣️  terrain layer num, used as texture for road
	bool roadCols;
	float roadStepDist, roadAngDiv;
	float roadHadd, wallX, wallY;
	int roadVegetDist;  // same for all veget models
	
	float tileMul;  // terrain & road layer tex tiling, scale
	float yObjOfs;  // 📦🏢 add objects pos y

	void Default();
};


///  Scene setup xml
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class Scene
{
public:

	//  🏁 car start pos
	MATHVECTOR <float,3> startPos[2];  // [1] is end for not looped, and start for reversed
	QUATERNION <float>   startRot[2];
	std::pair <MATHVECTOR<float,3>, QUATERNION<float>> GetStart(int index, bool looped);

	//  preview cam
	Ogre::Vector3 camPos, camDir;  // init all in ctor


	//  ⛅ Sky
	Ogre::String skyMtr;  float skyYaw;
	
	//  🌧️ weather
	int  rainEmit[NumWeather];
	Ogre::String rainName[NumWeather];

	//  🌞 Sun, Light
	float ldPitch, ldYaw;  // dir angles
	SColor lAmb,lDiff,lSpec;
	
	bool needLights =0;  // dark, for vehicle

	//  🌫️ Fog
	float fogStart, fogEnd;  // fake-linear range
	SColor fogClr,fogClr2;  // 2colors sun-away  .a = intensity

	SColor fogClrH;  // height fog color
	float fogHeight, fogHDensity, fogHStart, fogHEnd;
	float fHDamage;  // damage from low height fog


	//  game
	bool asphalt;  // use asphalt tires
	bool denyReversed;  // track (road) dir
	bool noWrongChks;  // dont show "wrong checkpoint" messages

	float damageMul;  // reduce car damage in loops
	float gravity;  // 9.81
	bool road1mtr = 1;  // if true (default) road has only 1 surface type

	//  🍃 wind
	float windForce, windOfs, windAmpl, windFreq, windYaw;


	//  🔉 sound
	std::string ambientSnd, sReverbs;  void UpdRevSet();
	float ambientVol = 1.f;
	struct RevSet  // copy from ReverbSet, name = sReverbs, from base if ""
	{	std::string descr,
			normal, cave, cavebig, pipe, pipebig, influid;
	} revSet;
	ReverbsXml* pReverbsXml =0;  //! set this after Load
	

	//  ⚫💭  wheel particle types
	Ogre::String  sParDust, sParMud, sParSmoke,  // todo: auto create from presets.xml
		sFluidWater, sFluidMud, sFluidMudSoft;  // fluids  todo: auto from fluids.xml
	
	///  ⛰️ Terrains  ----
	std::vector<TerData> tds;
	//  world size
	bool bAutoTerSize = 1;  // for more ters, not size from 1st
	float fTerSize = -1.f;  // custom size
	
	float GetTerSize() const
	{	return !bAutoTerSize && fTerSize > 0.f ?
			fTerSize : tds[0].fTerWorldSize;	}

	//  🛣️ Road surfaces
	TerLayer layerRoad[4];  // todo: per road, move there..

	
	///  🌳🪨  Vegetation params  --------
	float densTrees, densGrass;  int grDensSmooth;
	int trRdDist;  // dist from road to trees

	//  🌿 grass layers  paged
	const static int ciNumGrLay = 9;  // all, for edit
	SGrassLayer grLayersAll[ciNumGrLay];
	SGrassChannel grChan[4];

	//  model layers  (trees,rocks,etc)  not paged
	const static int ciNumVegLay = 25;  // max all, for edit
	VegetLayer vegLayersAll[ciNumVegLay];
	std::vector<int> vegLayers;    // active only (on)
	void UpdVegLayers();

	
	//  💧 Fluids  ~~~
	std::vector<FluidBox> fluids;
	FluidsXml* pFluidsXml =0;  //! set this after Load
	float GetDepthInFluids(Ogre::Vector3 pos, bool skipDeep =0) const;
	
	//  📦 Objects  []o
	std::vector<Object> objects;

	//  🔥 Particles  Emitters  ::
	std::vector<SEmitter> emitters;


	//  💎 Collectible gems  *
	std::vector<SCollect> collects;
	std::vector<QCollect> qcols;  // from LoadXmlCollects only

	//  🎆 Fields  ^
	std::vector<SField> fields;


	//  base track (new from) for info
	std::string baseTrk;
	int secEdited;  // time in seconds of track editing


	//  RoR only for export
	RoRconfig rorCfg;
	

	//  🌟 Main methods  ----
	Scene();
	void Default();
	void UpdateFluidsId(), UpdateSurfId();

	// CGuiCom* gcom =0;  // for TrkDir to hmap file
	// GAME* pGame =0;  // for all surfaces by name, in game not ed
	std::map <std::string, int>* surf_map =0;  // name to surface id
	// ! set surf_map before full LoadXml
	bool LoadXml(Ogre::String file, bool bTer = true), SaveXml(Ogre::String file);
	
	bool LoadXmlCollects(Ogre::String file);
	bool LoadTerData(Ogre::String file, TerData& td, tinyxml2::XMLElement* e);//, SaveTerData(const TerData& td, XMLElement*& e);
};
