#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CData.h"
#include "ShapeData.h"
#include "CScene.h"
#ifdef SR_EDITOR
	#include "CApp.h"
#else
	#include "CGame.h"
#endif
#include "settings.h"
#include <OgreCommon.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
using namespace Ogre;



///  🔥 create particle Emitters  : : : : : : : : 
//----------------------------------------------------------------------------------------------------------------------
void CScene::CreateEmitters(bool force)
{
#ifdef SR_EDITOR
	if (!app->pSet->bEmitters && !force)
		return;
#else	// game
	if (!app->pSet->particles)
		return;
#endif
	LogO("C--- create Emitters");
	SceneNode* rt = app->mSceneMgr->getRootSceneNode(SCENE_DYNAMIC);
	for (int i=0; i < sc->emitters.size(); ++i)
	{
		String n = "PE_" +toStr(i);
		SEmitter& em = sc->emitters[i];
		if (em.name.empty())  continue;

		ParticleSystem* ps;
		try
		{	ps = app->mSceneMgr->createParticleSystem(em.name);
			// LogO("C--- create Emitter" + em.name);
		}
		catch (Exception& ex)
		{
			LogO("Warning: emitter " + toStr(i) + " particle system: " + em.name + " doesn't exist");
			continue;
		}
		ps->setVisibilityFlags(RV_Particles);
		ps->setRenderQueueGroup(RQG_Weather); //RQG_CarParticles);

		SceneNode* nd = rt->createChildSceneNode(SCENE_DYNAMIC, em.pos);
		nd->attachObject(ps);
		em.nd = nd;  em.ps = ps;

		em.par.x = s2r(ps->getParameter("particle_width"));  // store orig
		em.par.y = s2r(ps->getParameter("particle_height"));
		em.UpdEmitter();
		ps->_update(em.upd);  //  started already 2 sec ago
		ps->setSpeedFactor(em.stat ? 0.f : 1.f);  // static
	}
}

void SEmitter::UpdEmitter()
{
	if (!ps)  return;
	ps->setParameter("particle_width",  toStr(parScale * par.x));
	ps->setParameter("particle_height", toStr(parScale * par.y));
	ps->getEmitter(0)->setParameter("width",  toStr(size.x));
	ps->getEmitter(0)->setParameter("height", toStr(size.z));
	ps->getEmitter(0)->setParameter("depth",  toStr(size.y));  // h
	ps->getEmitter(0)->setEmissionRate(rate);

	Vector3 dir = SplineRoad::GetRot(yaw, pitch);
	ps->getEmitter(0)->setDirection(dir);
	// LogO("Emit dir "+toStr(dir));
	//ps->getEmitter(0)->setAngle(roll);
	//ps->getEmitter(0)->setParticleVelocity(minVel,maxVel);  // todo?
	//ps->getEmitter(0)->setMinTimeToLive(minTime);
	//ps->getEmitter(0)->setMaxTimeToLive(maxTime);
}

void SField::UpdEmitter()
{
	if (!ps)  return;
	Vector3 v = SplineRoad::GetRot(yaw, pitch);
	ps->getEmitter(0)->setDirection(v);
	dir.setX(v[0]);  dir.setY(-v[2]);  dir.setZ(v[1]);
	LogO("Field dir "+toStr(v));
}

void CScene::DestroyEmitters(bool clear)
{
	LogO("D--- destroy Emitters");
	for (int i=0; i < sc->emitters.size(); ++i)
	{
		SEmitter& em = sc->emitters[i];
		if (em.nd) {  app->mSceneMgr->destroySceneNode(em.nd);  em.nd = 0;  }
		if (em.ps) {  app->mSceneMgr->destroyParticleSystem(em.ps);  em.ps = 0;  }
	}
	if (clear)
		sc->emitters.clear();
}


//-------------------------------------------------------------------------------------------------------
#ifdef SR_EDITOR
void App::PickEmitters()
{
	const auto& emts = scn->sc->emitters;
	bool vis = edMode == ED_Particles && !emts.empty() && !bMoveCam && pSet->bEmitters;
	if (!vis)  return;

	iEmtCur = -1;
	const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
	Real mx = Real(mp.left)/mWindow->getWidth(), my = Real(mp.top)/mWindow->getHeight();
	Ray ray = mCamera->getCameraToViewportRay(mx,my);  // 0..1
	const Vector3& pos = mCamera->getDerivedPosition(), dir = ray.getDirection();

	Real distC = 100000.f;
	int ie = -1;
	for (int i=0; i < emts.size(); ++i)
	{
		//  closest to emt center  // fixme fails..
		const Vector3 posSph = emts[i].nd->getPosition();
		const Vector3 ps = pos - posSph;
		Vector3 crs = ps.crossProduct(dir);
		Real dC = crs.length() / dir.length();

		//  same side of z plane?
		// box emts[i].size ?..
		//if ((*it).distance < dist)  // closest aabb
		if (dC < distC)  // closest center
		{
			ie = i;
			//dist = (*it).distance;
			distC = dC;
	}	}
	
	if (ie != -1)  //  if none picked
	if (iEmtCur == -1)
		iEmtCur = ie;
}

void App::UpdEmtBox()
{
	int emts = scn->sc->emitters.size();
	bool vis = edMode == ED_Particles && emts > 0 && !bMoveCam && pSet->bEmitters;
	if (emts > 0)
		iEmtCur = std::max(0, std::min(iEmtCur, emts-1));

	if (!boxEmit.nd)  return;
	boxEmit.nd->setVisible(vis);
	if (!vis)  return;
	
	const SEmitter& em = scn->sc->emitters[iEmtCur];
	boxEmit.nd->setPosition(em.pos);
	//boxEmit.nd->setOrientation(Quaternion(Degree(em.rot), em.up));
	boxEmit.nd->setScale(em.size);
	boxEmit.nd->_getFullTransformUpdated();
}

void App::SetEmtType(int rel)
{
	int emtAll = vEmtNames.size();
	if (emtAll <= 0)  return;
	iEmtNew = (iEmtNew + rel + emtAll) % emtAll;
	
	int emts = scn->sc->emitters.size();
	if (emts <= 0)  return;
	SEmitter& em = scn->sc->emitters[iEmtCur];
	em.name = vEmtNames[iEmtNew];
}
#endif
