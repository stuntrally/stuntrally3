#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "paths.h"
#include "settings.h"
#include "SceneClasses.h"
#include "App.h"
#ifdef SR_EDITOR
	#include "CGui.h"
	#include "Road.h"
#else
	#include "game.h"
#endif
#include "SceneXml.h"
#include "CScene.h"
#include "CData.h"
#include "PresetsXml.h"

#include "Axes.h"
#include "BtOgreGP.h"
#include "ShapeData.h"
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreItem.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreWindow.h>
using namespace Ogre;
using namespace std;


//  💎🆕 Create Collectibles
//-------------------------------------------------------------------------------------------------------
void App::CreateCollects()
{
	for (int i=0; i < scn->sc->collects.size(); ++i)
	{
		SCollect& o = scn->sc->collects[i];
		String s = toStr(i);  // counter for names
		auto name = "sphere.mesh";

		//  add to ogre
		try 
		{	o.it = mSceneMgr->createItem(name);
			// if (!o.material.empty())
			// 	o.it->setDatablockOrMaterialName(o.material);
			
			o.it->setName("cE"+s);
			SetTexWrap(o.it);
			// o.it->setCastShadows(o.shadow);
		}
		catch (Exception& e)
		{
			LogO(String("Create collect fail: ") + e.what());
			continue;
		}
		o.nd = mSceneMgr->getRootSceneNode(SCENE_DYNAMIC)->createChildSceneNode();
		// o.SetFromBlt();
		o.nd->setPosition(o.pos);
		o.nd->attachObject(o.it);  o.it->setVisibilityFlags(RV_Objects);  // todo: in splitscreen
		o.nd->setScale(o.scale * Vector3::UNIT_SCALE);

		//  alpha from presets.xml
		auto* veg = scn->data->pre->GetVeget(name);
		if (veg)
			o.it->setRenderQueueGroup( veg->alpha ? RQG_AlphaVegObj : RQG_Road );


		//  add to bullet world (in game)
		{
			///  🏢 static  . . . . . . . . . . . . 
			//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _

			btCollisionShape* bshp = 0;
			bshp = new btSphereShape(o.scale);

			size_t id = SU_Collect + i;
			bshp->setUserPointer((void*)id);
			bshp->setMargin(0.1f); //

			btCollisionObject* bco = new btCollisionObject();
			btTransform tr;  tr.setIdentity();  tr.setOrigin(btVector3(o.pos.x,-o.pos.z,o.pos.y));
			bco->setActivationState(DISABLE_SIMULATION);
			bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);

			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE/**/);

			// bco->setUserPointer(new ShapeData(ST_Fluid, 0, &fb));  ///~~
		#ifndef SR_EDITOR
			pGame->collision.world->addCollisionObject(bco);
			pGame->collision.shapes.push_back(bshp);
			o.co = bco;
		#else
			world->addCollisionObject(bco);
		#endif
		}
	}
	#ifdef SR_EDITOR
	// iObjLast = scn->sc->objects.size();
	#endif
}


///  💎💥 destroy
void App::DestroyCollects(bool clear)
{
	for (int i=0; i < scn->sc->collects.size(); ++i)
	{
		SCollect& o = scn->sc->collects[i];
		//  ogre
		if (o.nd)  mSceneMgr->destroySceneNode(o.nd);  o.nd = 0;
		if (o.it)  mSceneMgr->destroyItem(o.it);  o.it = 0;

		//  bullet
		if (o.co)
		{	delete o.co->getCollisionShape();
			#ifdef SR_EDITOR
			world->removeCollisionObject(o.co);
			#else
			pGame->collision.world->removeCollisionObject(o.co);
			#endif
			delete o.co;  o.co = 0;
		}
	}
	if (clear)
		scn->sc->collects.clear();
}

void App::ResetCollects()
{
	for (SCollect& o : scn->sc->collects)
		if (o.nd)
			o.nd->setVisible(1);
}

#if 0  /// TODO.. ed
//  👆 Pick
//-------------------------------------------------------------------------------------------------------

#ifdef SR_EDITOR
void App::UpdObjPick()
{
	bool st = edMode == ED_Start && !bMoveCam;
	if (ndStartBox[0])  ndStartBox[0]->setVisible(st);
	if (ndStartBox[1])  ndStartBox[1]->setVisible(st && !scn->road->isLooped);  // end separate


	int objs = scn->sc->objects.size();
	bool bObjects = edMode == ED_Objects && !bMoveCam && objs > 0 && iObjCur >= 0;
	if (objs > 0)
		iObjCur = std::min(iObjCur, objs-1);

	if (!ndObjBox)  return;
	ndObjBox->setVisible(bObjects);
	if (!bObjects)  return;
	
	const Object& o = scn->sc->objects[iObjCur];
	const Aabb& ab = o.nd->getAttachedObject(0)->getLocalAabb();
	Vector3 s = o.scale * ab.getSize();  // * sel obj's node aabb

	Vector3 posO = Axes::toOgre(o.pos);
	Quaternion rotO = Axes::toOgreW(o.rot);

	// Vector3 scaledCenter = ab.getCenter() * o.scale;
	// posO += (rotO * scaledCenter);

	ndObjBox->setPosition(posO);
	ndObjBox->setOrientation(rotO);
	ndObjBox->setScale(s);
	ndObjBox->_getFullTransformUpdated();
}

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

//  upd obj selected glow
void App::UpdObjSel()
{
	int objs = scn->sc->objects.size();
	for (int i=0; i < objs; ++i)
	{
		bool bSel = vObjSel.find(i) != vObjSel.end();
		auto* rend = scn->sc->objects[i].it->getSubItem(0);
		UpdSelectGlow(rend, bSel);
	}
}

//  selection center pos, or picked pos for multi rotate and scale
Vector3 App::GetObjPos0()
{
	Vector3 pos0{0,0,0};
	if (iObjCur>=0)
	{
		MATHVECTOR<float,3> p = scn->sc->objects[iObjCur].pos;
		pos0 = Vector3(p[0],p[2],-p[1]);
	}
	else if (!vObjSel.empty())
	{
		for (std::set<int>::iterator it = vObjSel.begin(); it != vObjSel.end(); ++it)
		{
			MATHVECTOR<float,3> p = scn->sc->objects[(*it)].pos;
			pos0 += Vector3(p[0],p[2],-p[1]);
		}
		pos0 /= Real(vObjSel.size());
	}
	return pos0;
}


///  🆕 add new object
void App::AddNewObj(bool getName)  //App..
{
	::Object o = objNew;
	if (getName)
		o.name = vObjNames[iObjTNew];
	++iObjLast;
	String s = toStr(iObjLast);  // counter for names

	//  pos, rot
	if (getName)
	{	// one new
		const Vector3& v = scn->road->posHit;
		o.pos[0] = v.x;  o.pos[1] =-v.z;  o.pos[2] = v.y + objNew.pos[2];
	}else  // many
	{	// offset for cursor pos..
		//o.pos[0] = v.x;  o.pos[1] =-v.z;  o.pos[2] = v.y + objNew.pos[2];
	}

	//  create object
	try
	{	o.it = mSceneMgr->createItem(o.name + ".mesh");
		//  alpha from presets.xml
		auto* veg = scn->data->pre->GetVeget(o.name);
		if (veg)
			o.it->setRenderQueueGroup( veg->alpha ? RQG_AlphaVegObj : RQG_Road );

		o.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		o.SetFromBlt();
		o.nd->setScale(o.scale);
		o.nd->attachObject(o.it);  o.it->setVisibilityFlags(RV_Vegetation);

		o.dyn = PATHS::FileExists(PATHS::Data()+"/objects/"+ o.name + ".bullet");
		scn->sc->objects.push_back(o);
	}
	catch (Exception ex)
	{
		LogO("no object! " + ex.getFullDescription());
	}
}


//  change obj to insert
#define ITEM_NONE -1  //?
void CGui::listObjsChng(MyGUI::List* l, size_t t)
{
	//  unselect other
	if (l != objListDyn)  objListDyn->setIndexSelected(ITEM_NONE);
	if (l != objListSt)	  objListSt->setIndexSelected(ITEM_NONE);
	if (l != objListBld)  objListBld->setIndexSelected(ITEM_NONE);

	if (t == ITEM_NONE)  //  sel default
	{	/*l = objListDyn;*/  t = 0;  l->setIndexSelected(t);  }
	
	std::string s = l->getItemNameAt(t).substr(7);
	for (int i=0; i < app->vObjNames.size(); ++i)
		if (s == app->vObjNames[i])
		{
			app->SetObjNewType(i);
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


//  change category, fill 🏢 buildings list
void CGui::listObjsCatChng(Li li, size_t id)
{
	if (id == ITEM_NONE || id >= li->getItemCount())  id = 0;
	if (li->getItemCount()==0)  return;
	string cat = li->getItemNameAt(id).substr(7);
	
	objListBld->removeAllItems();
	for (size_t i=0; i < app->vBuildings.size(); ++i)
	{
		const string& s = app->vBuildings[i];
		if (//id == 0 ||/*all*/
			s.length() > 4 && s.substr(0,4) == cat)
			objListBld->addItem("#E0E080"+s);
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

void App::TogObjStatic()  // toggle static
{
	if (!vObjSel.empty())
	{	for (int i : vObjSel)
			scn->sc->objects[i].stat = !scn->sc->objects[i].stat;
		return;
	}
	bool bNew = iObjCur == -1;
	Object& o = bNew || scn->sc->objects.empty() ? objNew : scn->sc->objects[iObjCur];
	o.stat = !o.stat;
}


//  preview model for insert
void App::SetObjNewType(int tnew)
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

void App::UpdObjNewNode()
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
