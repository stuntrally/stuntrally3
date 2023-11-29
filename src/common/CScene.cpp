#include "pch.h"
#include "settings.h"
#include "CScene.h"
#include "CData.h"
#include "SceneXml.h"

#include "Road.h"
#include "PaceNotes.h"
#include "Grass.h"
#include "App.h"

using namespace Ogre;


CScene::CScene(App* app1)
	:app(app1)
{
	data = new CData();
	sc = new Scene();
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
	// delete pace;

	delete sc;
	delete data;
}

void CScene::DestroyRoads()
{
	for (auto& r : roads)
		r->Destroy();
	roads.clear();
	rdCur = 0;
	road = 0;
	
	grid.Destroy();
}

void CScene::DestroyPace()
{
	for (auto*& p : pace)
	if (p)
	{	p->Destroy();
		delete p;  p = 0;
	}
}

void CScene::DestroyTrail(int i)
{
	if (trail[i])
	{	trail[i]->Destroy();
		delete trail[i];  trail[i] = 0;
	}
}
void CScene::DestroyTrails()
{
	for (int i=0; i < MAX_Players; ++i)
		DestroyTrail(i);
}
