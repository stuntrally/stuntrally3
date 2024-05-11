#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "paths.h"
#include "settings.h"
#include "SceneClasses.h"
#include "App.h"
#include "CGui.h"
#ifndef SR_EDITOR
	#include "game.h"
#endif
#include "SceneXml.h"
#include "CScene.h"
#include "CData.h"
#include "PresetsXml.h"

#include "SoundMgr.h"
#include "ShapeData.h"
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreItem.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
using namespace Ogre;
using namespace std;


//  💎🆕 Create Collectibles
//-------------------------------------------------------------------------------------------------------
void App::CreateCollects()
{
	iCollected = 0;  oldCollected = 0;  // game, hud
#ifndef SR_EDITOR
	if (pSet->game.collect_num < 0)
		return;

	pSet->game.collect_all = 0;
#endif
	for (int i=0; i < scn->sc->collects.size(); ++i)
		CreateCollect(i);
}

void App::CreateCollect(int i)
{
	SCollect& c = scn->sc->collects[i];
	String s = toStr(i);  // counter for names
	const PCollect* col = scn->data->pre->GetCollect(c.name);
	if (!col)
	{
		LogO(String("Collect not in presets name: ") + c.name);
		col = scn->data->pre->GetCollect("gem1b");  // use default 1st
	}

#ifndef SR_EDITOR
	int& ic = pSet->game.collect_num;
	const Collection& colx = data->collect->all[ic];
	//  game xml groups filter
	bool no = ((1u << c.group) & colx.groups) == 0;
	if (no)  return;
#endif

	//  add to ogre 🟢
	try 
	{	c.it = mSceneMgr->createItem(col->mesh+".mesh");
		if (!col->material.empty())
			c.it->setDatablockOrMaterialName(col->material);

		c.itBeam = mSceneMgr->createItem("check.mesh");  // todo: own?
		c.itBeam->setRenderQueueGroup(RQG_Ghost);  c.itBeam->setCastShadows(false);
		c.itBeam->setDatablockOrMaterialName(col->beamMtr);
		
		c.it->setName("cE"+s);
		SetTexWrap(c.it);
		// o.it->setCastShadows(o.shadow);
	}
	catch (Exception& e)
	{
		LogO(String("Create collect fail: ") + e.what());
		return;;
	}
#ifndef SR_EDITOR
	++pSet->game.collect_all;
#endif

	//  pos, scale  ----
	c.nd = mSceneMgr->getRootSceneNode(SCENE_DYNAMIC)->createChildSceneNode();
	c.nd->setPosition(c.pos);
	c.nd->setScale(c.scale * col->scale * Vector3::UNIT_SCALE);
	c.nd->attachObject(c.it);  c.it->setVisibilityFlags(RV_Objects);  //?

	c.ndBeam = c.nd->createChildSceneNode(SCENE_DYNAMIC);
	c.ndBeam->setPosition(c.scale * Vector3(0.f, -5.f, 0.f));  // below
	c.ndBeam->setScale(c.scale * Vector3(0.1f, 15.f, 0.1f));  // par
	c.ndBeam->attachObject(c.itBeam);
	c.itBeam->setVisibilityFlags(RV_Hud3D[0]);

	if (pSet->li.collect)  // 💡 light
	{
		c.light = mSceneMgr->createLight();
		c.light->setType(Light::LT_POINT);
		c.light->setCastShadows(0);
		
		ColourValue cl;
		if (pSet->collectRandomClr)
			cl = ColourValue(Math::UnitRandom(), Math::UnitRandom(), Math::UnitRandom());
		else  cl = col->clr;
		
		float bright = 1.2f * pSet->bright, contrast = pSet->contrast;
		c.light->setDiffuseColour(  cl * bright * contrast );
		c.light->setSpecularColour( cl * bright * contrast );

		float lightPower = 12.f * Math::PI; //par  bright
		c.light->setPowerScale( lightPower ); // * pSet->car_light_bright);

		c.light->setAttenuationBasedOnRadius( 2.0f, 0.1f );
		c.nd->attachObject( c.light );
	}

	//  alpha from presets.xml
	// auto* veg = scn->data->pre->GetVeget(name);
	// if (veg)
	// 	o.it->setRenderQueueGroup( veg->alpha ? RQG_AlphaVegObj : RQG_Road );
	c.it->setRenderQueueGroup( RQG_AlphaVegObj );
	c.nd->_getFullTransformUpdated();  //?
#ifdef SR_EDITOR  // ed hide
	c.nd->setVisible(0);
#endif


	//  add to bullet world (in game)
	#ifndef SR_EDITOR
	{
		///  🏢 static  . . . . . . . . . . . . 
		btCollisionShape* bshp = 0;
		bshp = new btSphereShape(c.scale * 0.3f);  //par

		size_t id = SU_Collect + i;
		bshp->setUserPointer((void*)id);
		bshp->setMargin(0.1f); //

		btCollisionObject* bco = new btCollisionObject();
		btTransform tr;  tr.setIdentity();  tr.setOrigin(btVector3(c.pos.x,-c.pos.z,c.pos.y));
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);

		bco->setCollisionFlags(bco->getCollisionFlags() |
			btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);

		bco->setUserPointer(new ShapeData(ST_Collect, 0, 0, 0, &c));  /// *
		pGame->collision.world->addCollisionObject(bco);
		// pGame->collision.shapes.push_back(bshp);  //?
		c.co = bco;
	}
	#endif
}


///  💎💥 Destroy
//-------------------------------------------------------------------------------------------------------
void App::DestroyCollects(bool clear)
{
	for (int i=0; i < scn->sc->collects.size(); ++i)
		DestroyCollect(i);

	if (clear)
		scn->sc->collects.clear();
}

void App::DestroyCollect(int i)
{
	SCollect& c = scn->sc->collects[i];
	//  ogre
	if (c.ndBeam)  mSceneMgr->destroySceneNode(c.ndBeam);  c.ndBeam = 0;
	if (c.itBeam)  mSceneMgr->destroyItem(c.itBeam);  c.itBeam = 0;
	if (c.light)  mSceneMgr->destroyLight(c.light);
	if (c.nd)  mSceneMgr->destroySceneNode(c.nd);  c.nd = 0;
	if (c.it)  mSceneMgr->destroyItem(c.it);  c.it = 0;

	//  bullet
	if (c.co)
	{	delete c.co->getCollisionShape();
		#ifdef SR_EDITOR
		world->removeCollisionObject(c.co);
		#else
		pGame->collision.world->removeCollisionObject(c.co);
		#endif
		delete c.co;  c.co = 0;
	}
}

//  reset  F4 game
void App::ResetCollects()
{
	#ifndef SR_EDITOR
	int& ic = pSet->game.collect_num;
	const Collection& colx = data->collect->all[ic];

	iCollected = 0;  oldCollected = 0;
	for (SCollect& c : scn->sc->collects)
		if (c.nd)
		{
			c.collected = 0;
			//  game xml groups filter
			bool no = ((1u << c.group) & colx.groups) == 0;
			c.nd->setVisible(!no);
		}
	gui->chkArrow(0);  // show col arr
	#endif
}

#ifndef SR_EDITOR  // game

//-------------------------------------------------------------------------------------------------------
//  💫💎  Update collection game frame
//-------------------------------------------------------------------------------------------------------
void App::UpdCollects()
{
	if (sc->collects.empty())
		return;
	int all = pSet->game.collect_all;

	//  count collected
	int iCol = 0;
	for (auto& c : sc->collects)
	{
		// c.ndBeam->setScale(Vector3(0.1f, 10.f, 0.1f));  // par
		if (c.collected)
		{	++iCol;
			c.nd->setVisible(0);
		}
	}
	oldCollected = iCollected;
	iCollected = iCol;
	
	if (oldCollected != iCollected)
	{
		// todo: show wnd, check best time, pass etc-

		int id = pSet->game.collect_num;
		const Collection& col = data->collect->all[id];

		//  upd progress xml
		bool best = 0;
		auto* pro = gui->progressC.Get(col.name);
		if (pro)
		{
			int i = 0;
			for (auto& c : sc->collects)
			{
				if (c.collected)
					pro->gems[i] = true;
				++i;
			}
		
			if (iCollected == all)  // end, prize
			{
				float time = pGame->timer.GetPlayerTime(0);
				if (time < pro->bestTime)
				{	best = 1;
					pro->bestTime = time;
				}
				pro->fin = 1;
				// todo: prize pro->
			}
		}else
			LogO("|! collect not found in progress, wont save! "+col.name);

		if (iCollected == all)  // 🔉 end snd
		{	if (best)
				pGame->snd_lapbest->start();
			else
				pGame->snd_lap->start();
		}else
			pGame->snd_chk->start();  // 🔉 one

		gui->ProgressSaveCollect(1);  // save
	}
}
#endif


#ifdef SR_EDITOR

///  🆕 add new Collectible
void App::AddNewCol(bool getName)
{
	SCollect o = colNew;
	// if (getName)
	// 	o.name = vColNames[iColTNew];

	//  pos, rot
	if (getName)
	{	// one new
		const Vector3& v = scn->road->posHit;
		// o.pos[0] = v.x;  o.pos[1] =-v.z;  o.pos[2] = v.y + colNew.pos[2];
		o.pos[0] = v.x;  o.pos[1] = v.y + colNew.pos[2];  o.pos[2] = v.z;
	}

	auto& c = scn->sc->collects;
	c.push_back(o);
	CreateCollect(c.size()-1);
}


void App::UpdColPick()
{
	bool st = edMode == ED_Collects && !bMoveCam;
	if (boxCol.nd)  boxCol.nd->setVisible(st);
	if (boxCol.nd)  boxCol.nd->setVisible(st && !scn->road->isLooped);  // end separate


	int cols = scn->sc->collects.size();
	bool bCols = edMode == ED_Collects && !bMoveCam && cols > 0 && iColCur >= 0;
	if (cols > 0)
		iColCur = std::min(iColCur, cols-1);

	if (!boxCol.nd)  return;
	boxCol.nd->setVisible(bCols);
	if (!bCols)  return;
	
	const SCollect& o = scn->sc->collects[iColCur];
	const Aabb& ab = o.nd->getAttachedObject(0)->getLocalAabb();
	Vector3 s = o.nd->getScale();  //o.scale * ab.getSize();  // * sel col's node aabb

	boxCol.nd->setPosition(o.pos);
	boxCol.nd->setScale(s);
	boxCol.nd->_getFullTransformUpdated();
}


//  👆 Pick
//-------------------------------------------------------------------------------------------------------
void App::PickCollect()
{
	const auto& col = scn->sc->collects;
	if (col.empty())  return;

	iObjCur = -1;
	const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
	Real mx = Real(mp.left)/mWindow->getWidth(), my = Real(mp.top)/mWindow->getHeight();
	Ray ray = mCamera->getCameraToViewportRay(mx,my);  // 0..1
	const Vector3& pos = mCamera->getDerivedPosition(), dir = ray.getDirection();

	//  query scene (aabbs are enough)
	RaySceneQuery* rq = mSceneMgr->createRayQuery(ray);
	rq->setSortByDistance(true);
	RaySceneQueryResult& res = rq->execute();

	Real distC = 100000.f;
	int io = -1;
	for (auto it = res.begin(); it != res.end(); ++it)
	{
		const String& s = (*it).movable->getName();
		if (StringUtil::startsWith(s,"cE",false))
		{
			// LogO("RAY "+s+" "+fToStr((*it).distance,2,4));
			int i = -1;
			//  find col with same name
			for (int o=0; o < col.size(); ++o)
				if (col[o].it && s == col[o].it->getName())
				{	i = o;  break;  }
			
			//  pick
			if (i != -1)
			{
				//Aabb ab = col[i].it->getLocalAabb();
				//ab.getCenter();  ab.getSize();

				//  closest to col center  // fixme fails..
				const Vector3 posSph = col[i].nd->getPosition();
				const Vector3 ps = pos - posSph;
				Vector3 crs = ps.crossProduct(dir);
				Real dC = crs.length() / dir.length();

				//if ((*it).distance < dist)  // closest aabb
				if (dC < distC)  // closest center
				{
					io = i;
					//dist = (*it).distance;
					distC = dC;
			}	}
	}	}
	
	if (io != -1)  //  if none picked
	if (iColCur == -1)
		iColCur = io;
	
	//rq->clearResults();
	mSceneMgr->destroyQuery(rq);
}

#if 0
//  change collect to insert
#define ITEM_NONE -1  //?
void CGui::listColsChng(MyGUI::List* l, size_t t)
{
	if (t == ITEM_NONE)  //  sel default
	{	t = 0;  l->setIndexSelected(t);  }
	
	std::string s = l->getItemNameAt(t).substr(7);
	for (int i=0; i < app->vColNames.size(); ++i)
		if (s == app->vColNames[i])
		{
			app->SetColNewType(i);
			Upd3DView(s+".mesh");
			return;
		}
}

void CGui::listObjsNext(int rel)
{
	Li li = listCols;
	if (listCols->getIndexSelected()!= ITEM_NONE)
	{
		size_t cnt = li->getItemCount();
		if (cnt == 0)  return;
		int i = std::max(0, std::min((int)cnt-1, (int)li->getIndexSelected()+rel ));
		li->setIndexSelected(i);
		li->beginToItemAt(std::max(0, i-11));  // center
		listObjsChng(li, li->getIndexSelected());
	}
}
#endif


//  collect type add +-1
void App::SetColType(int add)
{
	auto& col = scn->sc->collects;
	if (iColCur < 0 && iColCur >= col.size())
		return;
	auto& name = col[iColCur].name;

	auto* p = scn->data->pre;
	int id = 0;
	auto f = p->icol.find(name);
	if (f != p->icol.end())
		id = f->second -1;
	
	int si = p->col.size();
	id = (id +si +add) % si;

	DestroyCollect(iColCur);
	name = p->col[id].name;

	CreateCollect(iColCur);
	UpdColPick();
}

//  group  add +-1
void App::SetColGroup(int add)
{
	auto& col = scn->sc->collects;
	if (iColCur < 0 && iColCur >= col.size())
		return;
	auto& g = col[iColCur].group;
	if (add < 0 && g > 0)  g += add;
	else if (add > 0 && g < 9)  g += add;
}


#if 0
void App::UpdColNewNode()
{
	if (!scn->road || !objNew.nd)  return;

	bool vis = scn->road->bHitTer && bEdit() && iObjCur == -1 && edMode == ED_Objects;
	objNew.nd->setVisible(vis);
	if (!vis)  return;
	
	Vector3 p = scn->road->posHit;  p.y += objNew.pos[2];
	objNew.SetFromBlt();
	objNew.nd->setPosition(p);
	objNew.nd->setScale(objNew.scale);
	objNew.nd->_getFullTransformUpdated();
}
#endif

#endif
