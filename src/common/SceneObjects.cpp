#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
#include "Axes.h"
#include "paths.h"
#include "settings.h"
#include "BtOgreGP.h"
#include "ShapeData.h"
#include "App.h"
#ifdef SR_EDITOR
	#include "CGui.h"
	#include "Road.h"
#else
	#include "game.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <LinearMath/btSerializer.h>
#include <BulletFileLoader/btBulletFile.h>
#include <BulletWorldImporter/btBulletWorldImporter.h>

#include <OgreCommon.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreItem.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
// #include <MyGUI.h>
// #include <MyGUI_InputManager.h>
using namespace Ogre;
// using namespace MyGUI;
using namespace std;



///  🎳📦 Objects  ... .. . . .
//----------------------------------------------------------------------------------------------------------------------
class BulletWorldOffset : public btBulletWorldImporter
{
public:
	btTransform mTrOfs;  // in offset
	btDefaultMotionState* ms;  // out
	btRigidBody* rb;  // out
	
	BulletWorldOffset(btDynamicsWorld* world=0)
		: btBulletWorldImporter(world), ms(0), rb(0)
	{
		mTrOfs.setIdentity();
	}
	
	//todo: shape->setUserPointer((void*)SU_ObjectDynamic);  // mark shapes..
	
	btCollisionObject* createCollisionObject(const btTransform& startTransform,btCollisionShape* shape, const char* bodyName)
	{
		return createRigidBody(false,0,startTransform,shape,bodyName);
	}

	btRigidBody* createRigidBody(bool isDynamic, btScalar mass, const btTransform& startTransform,
								 btCollisionShape* shape, const char* bodyName)
	{
		btVector3 localInertia;
		localInertia.setZero();

		if (mass)
			shape->calculateLocalInertia(mass,localInertia);
		
		ms = new btDefaultMotionState();
		ms->setWorldTransform(mTrOfs);
		btRigidBody* body = new btRigidBody(mass,ms,shape,localInertia);	
		body->setDamping(0.1f, 0.3f);
		//body->setFriction(0.5f);
		rb = body;

		#ifdef SR_EDITOR
		//body->setActivationState(DISABLE_DEACTIVATION);
		#else
		body->setActivationState(WANTS_DEACTIVATION);  // game creates deactivated (sleeping)
		#endif

		if (m_dynamicsWorld)
			m_dynamicsWorld->addRigidBody(body);
		
		/*if (bodyName)
		{
			char* newname = duplicateName(bodyName);
			m_objectNameMap.insert(body,newname);
			m_nameBodyMap.insert(newname,body);
		}*/
		m_allocatedRigidBodies.push_back(body);
		return body;
	}
};


//  🆕 Create
//-------------------------------------------------------------------------------------------------------
void App::CreateObjects()
{
	//  maps for file exist (optimized)
	using std::map;  using std::string;
	map<string,bool> objExists, objHasBlt;
	
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		const string& s = scn->sc->objects[i].name;
		objExists[s] = false;  objHasBlt[s] = false;
	}
	const static int dirCnt = 5;
	const static char* dirs[dirCnt] = {"objects", "rocks", "objects2", "objects0", "objectsC"};
	for (auto& o : objExists)
	{
		o.second = false;
		for (int d=0; d < dirCnt; ++d)
			if (PATHS::FileExists(PATHS::Data() +"/"+ dirs[d] +"/"+ o.first + ".mesh"))
			{	o.second = true;  break;  }

		if (!o.second)
			LogO("Warning: CreateObjects mesh doesn't exist: " + o.first + ".mesh");
	}
	for (auto& ob : objHasBlt)
		ob.second = PATHS::FileExists(PATHS::Data() +"/"+ dirs[0] +"/"+ ob.first + ".bullet");


	//  loader
	#ifndef SR_EDITOR
	btDiscreteDynamicsWorld* world = pGame->collision.world;
	#endif
	BulletWorldOffset* fileLoader = new BulletWorldOffset(world);


	///  create  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		Object& o = scn->sc->objects[i];
		String s = toStr(i);  // counter for names
		o.dyn = objHasBlt[o.name];
		#ifndef SR_EDITOR
		if (o.dyn && !pSet->game.dyn_objects)  continue;
		#endif

		//  add to ogre
		bool no = !objExists[o.name];
		if (no)  continue;  //- no spheres
		try 
		{	o.it = mSceneMgr->createItem((no ? "sphere" : o.name) + ".mesh");
			o.it->setName("oE"+s);
			SetTexWrap(o.it);
		}
		catch( Ogre::Exception& e )
		{
			LogO(String("Create obj fail: ") + e.what());
			continue;
		}
		o.nd = mSceneMgr->getRootSceneNode(SCENE_DYNAMIC)->createChildSceneNode();
		o.SetFromBlt();
		o.nd->attachObject(o.it);  o.it->setVisibilityFlags(RV_Objects);
		o.nd->setScale(o.scale);
		if (no)  continue;

		//  add to bullet world (in game)
		if (!o.dyn)
		{
			///  🏢 static  . . . . . . . . . . . . 
			Vector3 posO = Axes::toOgre(o.pos);
			Quaternion rotO = Axes::toOgreW(o.rot);

			Matrix4 tre;  tre.makeTransform(posO, o.scale, rotO);
			BtOgre::StaticMeshToShapeConverter converter(o.it, tre);
			btCollisionShape* shape = converter.createTrimesh();  //=new x2 todo:del?...
			shape->setUserPointer((void*)SU_ObjectStatic);  // mark

			btCollisionObject* bco = new btCollisionObject();
			btTransform tr;  tr.setIdentity();  //tr.setOrigin(btVector3(pos.x,-pos.z,pos.y));
			bco->setActivationState(DISABLE_SIMULATION);  // WANTS_DEACTIVATION
			bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
			bco->setFriction(0.7f);   //+
			bco->setRestitution(0.f);
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
			world->addCollisionObject(bco);
			#ifndef SR_EDITOR
			o.co = bco;  o.ms = 0;  o.rb = 0;
			pGame->collision.shapes.push_back(shape);
			#endif
		}
		else  ///  📦 dynamic  . . . . . . . . . . . . 
		{
			// .bullet load
			fileLoader->mTrOfs.setOrigin(btVector3(o.pos[0],o.pos[1],o.pos[2]));
			fileLoader->mTrOfs.setRotation(btQuaternion(o.rot[0],o.rot[1],o.rot[2],o.rot[3]));
			//fileLoader->setVerboseMode(true);//
			std::string file = PATHS::Data()+"/objects/"+o.name+".bullet";

			if (fileLoader->loadFile(file.c_str()))
			{
				o.ms = fileLoader->ms;  // 1 only
				o.rb = fileLoader->rb;  // 1 only

				/*int nshp = fileLoader->getNumCollisionShapes();
				for (int i=0; i < nshp; ++i)
					pGame->collision.shapes.push_back(
						fileLoader->getCollisionShapeByIndex(i));/**/

				#ifndef SR_EDITOR
				btTransform t1;  // save 1st pos for reset
				o.ms->getWorldTransform(t1);
				o.tr1 = new btTransform(t1);
				#endif
				#if 0
				LogO(".bullet: "+o.name+
					"  shapes:"+toStr(fileLoader->getNumCollisionShapes())+
					"  bodies:"+toStr(fileLoader->getNumRigidBodies())+
					"  constr:"+toStr(fileLoader->getNumConstraints())); /**/
				#endif
			}else
				LogO(".bullet: Load Error: "+o.name);
		}
	}
	delete fileLoader;

	#ifdef SR_EDITOR
	iObjLast = scn->sc->objects.size();
	#endif
}


///  💥 destroy
void App::DestroyObjects(bool clear)
{
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		Object& o = scn->sc->objects[i];
		delete o.tr1;  o.tr1 = 0;

		// ogre
		if (o.nd)  mSceneMgr->destroySceneNode(o.nd);  o.nd = 0;
		if (o.it)  mSceneMgr->destroyItem(o.it);  o.it = 0;

		#ifdef SR_EDITOR
		// bullet
		if (o.co)
		{	delete o.co->getCollisionShape();
			#ifdef SR_EDITOR
			world->removeCollisionObject(o.co);
			#else
			pGame->collision.world->removeCollisionObject(o.co);
			#endif
			delete o.co;  o.co = 0;
		}
		if (o.rb)
		{	delete o.rb->getCollisionShape();
			delete o.ms;  o.ms = 0;
			#ifdef SR_EDITOR
			world->removeRigidBody(o.rb);
			#else
			pGame->collision.world->removeCollisionObject(o.rb);
			#endif
			delete o.rb;  o.rb = 0;
		}
		#endif
	}
	if (clear)
		scn->sc->objects.clear();
}

void App::ResetObjects()
{
	for (Object& o : scn->sc->objects)
		if (o.dyn && o.ms && o.tr1)
		{
			o.rb->clearForces();
			o.rb->setHitFraction(0.f);
			o.rb->setLinearVelocity(btVector3(0,0,0));
			o.rb->setAngularVelocity(btVector3(0,0,0));
			o.rb->setActivationState(WANTS_DEACTIVATION);
			o.rb->setWorldTransform(*o.tr1);
			o.ms->setWorldTransform(*o.tr1);
			o.SetFromBlt();
	}
}


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

				//  closest to obj center
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


///  🎳 toggle objects simulation (bullet world)
//-------------------------------------------------------------------------------------------------------
void App::ToggleObjSim()
{
	if (gui->objPan)  gui->objPan->setVisible(objSim);
	
	DestroyObjects(false);

	if (!objSim)  // off sim
	{
		//  Destroy blt world
		for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = world->getCollisionObjectArray()[i];
			delete obj->getCollisionShape();
			
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body)
				delete body->getMotionState();

			if (obj->getUserPointer() != (void*)111)
			{
				ShapeData* sd = static_cast<ShapeData*>(obj->getUserPointer());
				delete sd;
			}
			world->removeCollisionObject(obj);
			delete obj;
		}
	}
	else  // on sim
	{
		//  Create blt world
		scn->CreateBltTerrains();
		scn->road->RebuildRoadInt(false,true);
	}
	CreateObjects();
	UpdObjPick();
}


///  🆕 add new object
void App::AddNewObj(bool getName)  //App..
{
	::Object o = objNew;
	if (getName)
		o.name = vObjNames[iObjTNew];
	++iObjLast;
	String s = toStr(iObjLast);  // counter for names
	///TODO: ?dyn objs size, !?get center,size, rmb height..

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
	o.it = mSceneMgr->createItem(o.name + ".mesh");
	o.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	o.SetFromBlt();
	o.nd->setScale(o.scale);
	o.nd->attachObject(o.it);  o.it->setVisibilityFlags(RV_Vegetation);

	o.dyn = PATHS::FileExists(PATHS::Data()+"/objects/"+ o.name + ".bullet");
	scn->sc->objects.push_back(o);
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


//  preview model for insert
void App::SetObjNewType(int tnew)
{
	iObjTNew = tnew;
	if (objNew.nd)	{	mSceneMgr->destroySceneNode(objNew.nd);  objNew.nd = 0;  }
	if (objNew.it)	{	mSceneMgr->destroyItem(objNew.it);  objNew.it = 0;  }
	
	String name = vObjNames[iObjTNew];
	objNew.dyn = PATHS::FileExists(PATHS::Data()+"/objects/"+ name + ".bullet");
	if (objNew.dyn)  objNew.scale = Vector3::UNIT_SCALE;  // dyn no scale
	objNew.it = mSceneMgr->createItem(name + ".mesh");
	objNew.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	objNew.nd->attachObject(objNew.it);  objNew.it->setVisibilityFlags(RV_Vegetation);
	UpdObjNewNode();
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
}

#endif
