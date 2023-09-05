#include "pch.h"
#include "settings.h"
#include "CScene.h"
#include "CData.h"
#include "SceneXml.h"
// #include "WaterRTT.h"
#include "Road.h"
#include "PaceNotes.h"
#include "Grass.h"
#include "App.h"

#include <OgreItem.h>
#include <OgreHlmsPbsPrerequisites.h>
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsSamplerblock.h>
using namespace Ogre;


CScene::CScene(App* app1)
	:app(app1)
{
	data = new CData();
	sc = new Scene();
	// mWaterRTT = new WaterRTT();
	refl.app = app1;

	//  Grass
	grass = new Grass();
	grass->mSceneMgr = app->mSceneMgr;
	// grass->terrain = app->mTerra;
	grass->scn = this;
	grass->pSet = app->pSet;
}

CScene::~CScene()
{
	delete grass;
	//?DestroyRoad();
	// delete pace;

	delete sc;
	delete data;
}

void CScene::DestroyRoads()
{
	for (auto r : roads)
		r->Destroy();
	roads.clear();
	rdCur = 0;
	road = 0;
	
	grid.Destroy();
}

void CScene::DestroyPace()
{
	if (pace)
	{	pace->Destroy();
		delete pace;  pace = 0;
	}
}
void CScene::DestroyTrail()
{
	if (trail)
	{	trail->Destroy();
		delete trail;  trail = 0;
	}
}

void CScene::destroyScene()
{
	// mWaterRTT->destroy();

	// DestroyRoads();  DestroyPace();  DestroyTrail();
	// DestroyTrees();
	// DestroyWeather();
}
