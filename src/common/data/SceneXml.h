#pragma once
#include "SColor.h"
#include "SceneClasses.h"

#include <OgreString.h>
#include <OgreVector3.h>
#include "mathvector.h"
#include "quaternion.h"

class GAME;  class ReverbsXml;  class FluidsXml;
namespace tinyxml2 {  class XMLElement;  }


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
	int  rainEmit,rain2Emit;  Ogre::String rainName,rain2Name;
	//  🌞 Sun, Light
	float ldPitch, ldYaw;  // dir angles
	SColor lAmb,lDiff,lSpec;
	
	bool needLights =0;  // dark, for vehicle

	//  🌫️ Fog
	float fogStart, fogEnd;  // lin range
	SColor fogClr,fogClr2;  // 2colors sun-away  .a = intensity

	SColor fogClrH;  // height fog color
	float fogHeight, fogHDensity, fogHStart, fogHEnd;
	float fHDamage;  // damage from low height fog


	//  game
	bool asphalt;  // use asphalt tires
	bool denyReversed;  // track (road) dir
	bool noWrongChks;  // dont show "wrong checkpoint" messages

	float windAmt;  //, windDirYaw, windTurbulFreq,windTurbulAmp;
	float damageMul;  // reduce car damage in loops
	float gravity;  // 9.81
	bool road1mtr = 1;  // if true (default) road has only 1 surface type
	

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

	//  🛣️ Road surfaces
	TerLayer layerRoad[4];

	
	///  🌳🪨  Vegetation params  --------
	float densTrees, densGrass;  int grDensSmooth;
	float grPage, grDist;
	float trPage, trDist, trDistImp;
	int trRdDist;  // dist from road to trees

	//  🌿 grass layers  paged
	const static int ciNumGrLay = 9;  // all, for edit
	SGrassLayer grLayersAll[ciNumGrLay];
	SGrassChannel grChan[4];

	//  model layers  (trees,rocks,etc)  not paged
	const static int ciNumPgLay = 14;  // all, for edit
	PagedLayer pgLayersAll[ciNumPgLay];
	std::vector<int> pgLayers;    // active only (on)
	void UpdPgLayers();

	
	//  💧 Fluids  ~~~
	std::vector<FluidBox> fluids;
	FluidsXml* pFluidsXml =0;  //! set this after Load
	float GetDepthInFluids(Ogre::Vector3 pos);
	
	//  📦 Objects  []o
	std::vector<Object> objects;

	//  🔥 Particles  Emitters  ::
	std::vector<SEmitter> emitters;


	//  base track (new from) for info
	std::string baseTrk;
	int secEdited;  // time in seconds of track editing

	
	//  🌟 Main methods  ----
	Scene();
	void Default();
	void UpdateFluidsId(), UpdateSurfId();

	GAME* pGame =0;  // for all surfaces by name
	bool LoadXml(Ogre::String file, bool bTer = true), SaveXml(Ogre::String file);
	bool LoadTerData(TerData& td, tinyxml2::XMLElement* e);//, SaveTerData(const TerData& td, XMLElement*& e);
};
