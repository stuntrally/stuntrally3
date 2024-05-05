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
	#endif

	for (int i=0; i < scn->sc->collects.size(); ++i)
		CreateCollect(i);
}

void App::CreateCollect(int i)
{
	SCollect& c = scn->sc->collects[i];
	// String s = toStr(i);  // counter for names
	const PCollect* col = scn->data->pre->GetCollect(c.name);
	if (!col)
	{
		LogO(String("Collect not in presets name: ") + c.name);
		col = scn->data->pre->GetCollect("gem1b");  // use default 1st
	}

	//  add to ogre 🟢
	try 
	{	c.it = mSceneMgr->createItem(col->mesh+".mesh");
		if (!col->material.empty())
			c.it->setDatablockOrMaterialName(col->material);

		c.itBeam = mSceneMgr->createItem("check.mesh");  // todo: own?
		c.itBeam->setRenderQueueGroup(RQG_Ghost);  c.itBeam->setCastShadows(false);
		c.itBeam->setDatablockOrMaterialName(col->beamMtr);
		
		// c.it->setName("Ce"+s);
		SetTexWrap(c.it);
		// o.it->setCastShadows(o.shadow);
	}
	catch (Exception& e)
	{
		LogO(String("Create collect fail: ") + e.what());
		return;;
	}
	//  pos, scale  ----
	c.nd = mSceneMgr->getRootSceneNode(SCENE_DYNAMIC)->createChildSceneNode();
	c.nd->setPosition(c.pos);
	c.nd->setScale(c.scale * Vector3::UNIT_SCALE);
	c.nd->attachObject(c.it);  //o.it->setVisibilityFlags(RV_Objects);
	// todo: collect in splitscreen

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
		
		// ColourValue c(0.0f, 0.5f, 1.f);  //par
		ColourValue cl(Math::UnitRandom(), Math::UnitRandom(), Math::UnitRandom());  // rnd
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
	#ifndef SR_EDITOR
		pGame->collision.world->addCollisionObject(bco);
		// pGame->collision.shapes.push_back(bshp);  //?
		c.co = bco;
	#else
		world->addCollisionObject(bco);
	#endif
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
	iCollected = 0;  oldCollected = 0;
	for (SCollect& o : scn->sc->collects)
		if (o.nd)
		{
			o.collected = 0;
			o.nd->setVisible(1);
		}
}

#ifndef SR_EDITOR  // game

//  Update collection game frame
void App::UpdCollects()
{
	int all = sc->collects.size();
	if (!all)
		return;

	//  count collected
	int cols = 0;
	for (auto& c : sc->collects)
	{
		// c.ndBeam->setScale(Vector3(0.1f, 10.f, 0.1f));  // par
		if (c.collected)
		{	++cols;
			c.nd->setVisible(0);
		}
	}
	oldCollected = iCollected;
	iCollected = cols;
	
	if (oldCollected != iCollected)
	{
		if (iCollected == all)
			pGame->snd_lap->start();  // 🔉 end
		else
			pGame->snd_chk->start();  // 🔉 one
		
		// todo: show wnd, check best time, pass etc

		int id = pSet->game.collect_num;
		const Collect& col = data->collect->all[id];

		auto* pro = gui->progressC.Get(col.name);
		if (pro)
		{
		// pro->
		}
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
	Vector3 s = o.scale * ab.getSize();  // * sel col's node aabb

	boxCol.nd->setPosition(o.pos);
	boxCol.nd->setScale(s);
	boxCol.nd->_getFullTransformUpdated();
}

#if 0  /// TODO.. ed

//  👆 Pick
//-------------------------------------------------------------------------------------------------------
void App::PickObject()
{
	const auto& objs = scn->sc->objects;
	if (objs.empty())  return;

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
		if (StringUtil::startsWith(s,"oE",false))
		{
			// LogO("RAY "+s+" "+fToStr((*it).distance,2,4));
			int i = -1;
			//  find obj with same name
			for (int o=0; o < objs.size(); ++o)
				if (objs[o].it && s == objs[o].it->getName())
				{	i = o;  break;  }
			
			//  pick
			if (i != -1)
			{
				//Aabb ab = objs[i].it->getLocalAabb();
				//ab.getCenter();  ab.getSize();

				//  closest to obj center  // fixme fails..
				const Vector3 posSph = objs[i].nd->getPosition();
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
	if (iObjCur == -1)
		iObjCur = io;
	
	//rq->clearResults();
	mSceneMgr->destroyQuery(rq);
}
#endif

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
	Li li = 0;
	if (objListDyn->getIndexSelected()!= ITEM_NONE)  li = objListDyn; //else
	if (objListSt->getIndexSelected() != ITEM_NONE)  li = objListSt;
	if (objListBld->getIndexSelected()!= ITEM_NONE)  li = objListBld;
	if (li)
	{
		size_t cnt = li->getItemCount();
		if (cnt == 0)  return;
		int i = std::max(0, std::min((int)cnt-1, (int)li->getIndexSelected()+rel ));
		li->setIndexSelected(i);
		li->beginToItemAt(std::max(0, i-11));  // center
		listObjsChng(li, li->getIndexSelected());
	}
}


//  🔮 cycle object materials
void App::NextObjMat(int add)
{
	if (!vObjSel.empty())
	{	for (int i : vObjSel)
			NextObjMat(add, scn->sc->objects[i]);
		return;
	}
	bool bNew = iObjCur == -1;
	Object& o = bNew || scn->sc->objects.empty() ? objNew : scn->sc->objects[iObjCur];
	NextObjMat(add, o);
}

void App::NextObjMat(int add, Object& o)
{
	const PObject* obj = scn->data->pre->GetObject(o.name);
	if (!obj || !obj->pMatSet)  return;
	
	auto& m = obj->pMatSet->mats;
	int j = -1, si = m.size();
	if (si < 2)  return;
	
	for (int i=0; i < si; ++i)  // find cur
		if (m[i] == o.material)
		{	j = i;  break;  }
	if (j == -1)
		j = 0;
	else
		j = (j + add + si) % si;  // inc
	o.material = m[j];  // set

	if (!o.material.empty())
		o.it->setDatablockOrMaterialName(o.material);
}
#endif

#if 0
//  preview model for insert
void App::SetColNewType(int tnew)
{
	iObjTNew = tnew;
	if (objNew.nd)	{	mSceneMgr->destroySceneNode(objNew.nd);  objNew.nd = 0;  }
	if (objNew.it)	{	mSceneMgr->destroyItem(objNew.it);  objNew.it = 0;  }
	
	String name = vObjNames[iObjTNew];
	objNew.dyn = PATHS::FileExists(PATHS::Data()+"/objects/"+ name + ".bullet");
	if (objNew.dyn)  objNew.scale = Vector3::UNIT_SCALE;  // dyn no scale
	try
	{	objNew.it = mSceneMgr->createItem(name + ".mesh");
		objNew.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		objNew.nd->attachObject(objNew.it);  objNew.it->setVisibilityFlags(RV_Vegetation);
		UpdObjNewNode();
	}
	catch (Exception ex)
	{
		LogO("no object! " + ex.getFullDescription());
	}
}

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
