#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "CData.h"
#include "ShapeData.h"

// #include "WaterRTT.h"
#include "CScene.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
	#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#else
	#include "CGame.h"
	#include "game.h"
	//#include "settings.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <OgreCommon.h>
// #include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreTimer.h>

#include <OgreItem.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreManualObject2.h>
using namespace Ogre;


//  🚧 ed cursor
#ifdef SR_EDITOR
void App::UpdFluidBox()
{
	int fls = scn->sc->fluids.size();
	bool bFluids = edMode == ED_Fluids && fls > 0 && !bMoveCam;
	if (fls > 0)
		iFlCur = std::max(0, std::min(iFlCur, fls-1));

	if (!ndFluidBox)  return;
	ndFluidBox->setVisible(bFluids);
	if (!bFluids)  return;
	
	FluidBox& fb = scn->sc->fluids[iFlCur];
	ndFluidBox->setPosition(fb.pos);
	ndFluidBox->setScale(fb.size);
}

void App::UpdMtrWaterDepth()
{
	float fl = edMode == ED_Fluids ? 0.f : 1.f;
	// sh::Factory::getInstance().setSharedParameter("waterDepth", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(fl)));
}
#endif


//;  Water rtt, setup and recreate
/*void CScene::UpdateWaterRTT(Camera* cam)
{
	mWaterRTT->setRTTSize(ciShadowSizesA[app->pSet->water_rttsize]);
	mWaterRTT->setReflect(app->pSet->water_reflect);
	mWaterRTT->setRefract(app->pSet->water_refract);

	mWaterRTT->setViewerCamera(cam);
	mWaterRTT->mSceneMgr = app->mSceneMgr;

	if (!sc->fluids.empty())  // first fluid's plane
		mWaterRTT->setPlane(Plane(Vector3::UNIT_Y, sc->fluids.front().pos.y));
	mWaterRTT->recreate();
	mWaterRTT->setActive(!sc->fluids.empty());
}
*/
