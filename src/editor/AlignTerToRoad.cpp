#include "pch.h"
#include "Def_Str.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "paths.h"
#include "HudRenderable.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include <OgrePrerequisites.h>
#include <OgreTimer.h>
#include "Terra.h"
#include <OgreSceneNode.h>
#include <OgreCommon.h>
using namespace Ogre;


///  align terrain to road selected segments
//-----------------------------------------------------------------------------------------------------------
struct RayResult : public btCollisionWorld::RayResultCallback
{
	RayResult(const btVector3& rayFromWorld, const btVector3& rayToWorld)
		: m_rayFromWorld(rayFromWorld), m_rayToWorld(rayToWorld)
	{	}

	btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
	btVector3	m_rayToWorld;

	btVector3	m_hitNormalWorld;
	btVector3	m_hitPointWorld;
		
	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		const btCollisionObject* obj = rayResult.m_collisionObject;
		if (obj->getUserPointer() != (void*)111)  // allow only road
			return 1.0;

		//caller already does the filter on the m_closestHitFraction
		btAssert(rayResult.m_hitFraction <= m_closestHitFraction);
		
		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = obj;
		
		if (normalInWorldSpace)
			m_hitNormalWorld = rayResult.m_hitNormalLocal;
		else  ///need to transform normal into worldspace
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;

		m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
		return rayResult.m_hitFraction;
	}
};

//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
//  ⛰️🛣️  Align
//-----------------------------------------------------------------------------------------------------------
void App::AlignTerToRoad()
{
	SplineRoad* road = scn->road;
	if (road->vSel.empty())  return;
	Ogre::Timer ti;

	///  create 🎳 bullet road for selected segments
	road->ed_Wmul = pSet->al_w_mul;
	road->ed_Wadd = pSet->al_w_add;
	road->RebuildRoadInt(true);

	//  terrain
	std::vector<float>& fHmap = mTerra->getHeightData();
	const int w = mTerra->getSize(), h = w;
	// const int w = scn->sc->td.iVertsX - 1 /*!*/, h = w;
	const float fh = h-1, fw = w-1;

	float *rd = new float[w*h];  // road depth
	bool  *rh = new bool[w*h];  // road hit

	const float ws = scn->sc->td.fTerWorldSize;
	const float Len = 400;  // max ray length
	int x,y,a,b;
	float v,k, fx,fz, wx,wz;
	
	///  🎯 ray casts  -----------
	for (y = 0; y < h; ++y) {  a = y*w;  b = (h-1-y)*w;
	for (x = 0; x < w; ++x, ++a,++b)
	{
		//  pos 0..1
		fx = float(x)/fh;  fz = float(y)/fw;
		//  pos on ter  -terSize..terSize
		wx = (fx-0.5f) * ws;  wz = (fz-0.5f) * ws;

		btVector3 from(wx,wz, Len*0.8f), to(wx,wz, -Len*1.1f);  // x -z y
		RayResult rayRes(from, to);
		world->rayTest(from, to, rayRes);

		//  terrain height if not hit
		bool hit = rayRes.hasHit();
		rh[b] = hit;
		rd[b] = hit ? rayRes.m_hitPointWorld.getZ() : fHmap[b];
	}	}

	//  smooth edges, road-terrain border
	const float fv = pSet->al_smooth;
	if (fv > 0.5f)
	{
		const int f = std::ceil(fv);
		const unsigned int fs = (f*2+1)*(f*2+1);
		float ff = 0.f;
		int i,j,m,d,b;

		//  gauss kernel for smoothing
		float *mask = new float[fs];  m = 0;
		for (j = -f; j <= f; ++j)
		for (i = -f; i <= f; ++i, ++m)
		{
			v = std::max(0.f, 1.f - sqrtf((float)(i*i+j*j)) / fv );
			mask[m] = v;  ff += v;
		}
		ff = 1.f / ff;  // smooth, outside (>1.f)
			
		//  sum kernel
		for (y = f; y < h-f; ++y) {  a = y*w +f;
		for (x = f; x < w-f; ++x, ++a)
		{		
			v = 0;  m = 0;  b = 0;
			for (j = -f; j <= f; ++j) {  d = a -f + j*w;
			for (i = -f; i <= f; ++i, ++d, ++m)
			{	k = mask[m];  //maskB ?
				v += rd[d] * k;
				if (rh[d] && k > 0.1f)  ++b;
			}	}
			if (b > 0 && b < fs*0.8f)  //par?
				rd[a] = v * ff;
		}	}
		delete[] mask;
	}

	//  set new hmap
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		fHmap[a] = rd[a];
	}	}

	delete[] rd;  delete[] rh;



	//  clear bullet world
	for (int i=0; i < road->vbtTriMesh.size(); ++i)
		delete road->vbtTriMesh[i];
	road->vbtTriMesh.clear();
	
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		if (obj->getUserPointer() == (void*)111)  // only road
		{
			delete obj->getCollisionShape();  //?
			
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
				delete body->getMotionState();

			world->removeCollisionObject(obj);
			delete obj;
	}	}


	//  update terrain
	scn->terrain->dirtyRect(Rect(0,0,1,1));
	scn->UpdBlendmap();
	bTerUpd = true;


	//  put sel segs on terrain
	for (auto i : scn->road->vSel)
		scn->road->mP[i].onTer = true;

	//  restore orig road width
	scn->road->Rebuild(true);
	
	// todo: ?restore road sel after load F5..

	LogO(String(":::* Time Ter Align: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
