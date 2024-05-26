#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "settings.h"
#include "CHud.h"
#include "ViewDim.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "Road.h"
#include "Cam.h"

#include "HlmsPbs2.h"
#include <OgreVector3.h>
#include <OgreItem.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
using namespace Ogre;
using namespace std;


///  🆕 create HUD Arrow
///---------------------------------------------------------------------------------------------------------------
void CHud::Hud3d::Create(SceneManager* mgr, SETTINGS* pSet, int plr,
	float scale, Ogre::String mesh, Ogre::String mtr)
{
	if (it)  return;
	if (!node)
		node = mgr->getRootSceneNode()->createChildSceneNode();
	player = plr;

	if (mesh.empty())
	{	mesh = "arrow.mesh";
		if (mtr.empty())  mtr = "Arrow"+toStr(player);
	}
	it = mgr->createItem(mesh);  it->setCastShadows(false);
	it->setRenderQueueGroup(RQG_Hud3);
	if (!mtr.empty())
		it->setDatablockOrMaterialName(mtr);

	pDb = dynamic_cast<HlmsPbsDb2*>( it->getSubItem(0)->getDatablock() );
	// LogO(pDb ? "arrow db2 cast ok" : "arrow db2 cast fail");
	
	nodeRot = node->createChildSceneNode();
	nodeRot->attachObject(it);
	nodeRot->setScale(
		scale > 0.f ? Vector3::UNIT_SCALE :  // cup
		pSet->size_arrow/2.f * Vector3(0.5,1.f,2.f));  // arrows
	it->setVisibilityFlags(RV_Hud3D[player]);
	nodeRot->setVisible(scale > 0.f ? 0 : pSet->check_arrow);  // cups hidden
}

//  💥 destroy
void CHud::Hud3d::Destroy(SceneManager* mgr)
{
	if (nodeRot)  mgr->destroySceneNode(nodeRot);  nodeRot = 0;
	if (node)  mgr->destroySceneNode(node);  node = 0;
	if (it)  mgr->destroyItem(it);  it = 0;
	pDb = 0;
}


//  💫 Update
void CHud::Hud3d::UpdateChk(SplineRoad* road, CarModel* carM, const Vector3& pos)
{
	//  set animation start to old orientation
	qStart = qCur;
	
	//  game start no animation
	bool noAnim = carM->iNumChks == 0;
	
	//  get vector from camera to checkpoint
	int id = std::max(0, std::min( (int)road->mChks.size()-1, carM->iNextChk ));
	Vector3 chkPos = road->mChks[id].pos;

	//  last checkpoint, point to start pos
	if (carM->iNumChks == road->mChks.size())
		chkPos = carM->vStartPos;
	
	const Vector3& playerPos = pos;
	Vector3 dir = chkPos - playerPos;  dir[1] = 0;  // only x and z rotation
	Quaternion quat = Vector3::UNIT_Z.getRotationTo(-dir);

	const bool valid = !quat.isNaN();
	if (valid)
	{	if (noAnim)  qStart = quat;
		qEnd = quat;
	
		//  calc angle towards cam
		Real angle = (qCur.zAxis().dotProduct(
			carM->fCam->cam->cam->getOrientation().zAxis()) + 1.f) / 2.f;

		//  set color in material (red for wrong dir)
		Vector3 col1 = angle * Vector3(0.0, 0.8, 0.0) + (1.f-angle) * Vector3(0.8, 0.0, 0.0);
		Vector3 col2 = angle * Vector3(0.0, 0.3, 0.0) + (1.f-angle) * Vector3(0.3, 0.0, 0.0);
		if (pDb)
		{	pDb->setDiffuse(col1);
			pDb->setSpecular(col2);
	}	}
}

//  💫 Update 3d
void CHud::Hud3d::Update(CarModel* carM, float time)
{
	//  align checkpoint arrow,  move in front of camera
	if (!node)  return;
	Camera* cam = carM->fCam->cam->cam;

	Vector3 pos = cam->getPosition();
	Vector3 z = cam->getDirection();
	Vector3 y = cam->getUp();
	Vector3 arrowPos = pos + 10.f * z + 3.5f * y;
	node->setPosition(arrowPos);  // in 3d, camera space
	
	//  animate
	bool bFirstFrame = carM->bGetStart;
	if (bFirstFrame) // 1st frame: dont animate
		qCur = qEnd;
	else
		qCur = Quaternion::Slerp(time*5, qStart, qEnd, true);
	nodeRot->setOrientation(qCur);
	
	//  look down -y a bit so we can see the arrow better
	nodeRot->pitch(Degree(-20), SceneNode::TS_LOCAL); 
	nodeRot->_getFullTransformUpdated();
}


//  💫 Update 💎
void CHud::Hud3d::UpdateCol(CarModel* carM, float sc)
{
	if (!node)  return;
	Vector3 pos = carM->ndMain->getPosition();
	bool car = carM->fCam->TypeCar();  // incar
	if (!car)
		pos.y += 1.f;  // up 3d
	
	Vector3 dir = pos - posTo;  // to gem
	if (car)
	{	auto z = carM->ndMain->getOrientation().xAxis(), y = carM->ndMain->getOrientation().yAxis();
		pos -= z * 9.f + y * 0.3f;  // in front
	}

	Real dist = dir.length();
	dist = 1.5f - max(0.2f, min(1.0f, dist/30.f));
	dir.normalise();
	Quaternion quat = Vector3::UNIT_Z.getRotationTo(dir);

	Vector3 arrowPos = pos - dir * 1.f;  // par  car size?
	nodeRot->setPosition(arrowPos);
	nodeRot->setOrientation(quat);
	nodeRot->setScale(sc * 1.f * Vector3(0.5,1.f,2.f + 0.6f*dist) * dist);  // par
	nodeRot->_getFullTransformUpdated();
}


//  💫 Update cup 🏆
void CHud::Hud3d::UpdateCup(CarModel* carM, float time)
{
	if (!node)  return;
	if (!node)  return;
	Camera* cam = carM->fCam->cam->cam;

	Vector3 pos = cam->getPosition();
	Vector3 z = cam->getDirection();
	Vector3 y = cam->getUp();
	
	Vector3 arrowPos = pos + (1.6f + dist) * z + -0.2f * y;
	node->setPosition(arrowPos);  // in 3d, camera space

	if (dist > 0.f)
	{	dist -= time * 1.f + dist * time * 4.f;
		if (dist < 0.f)  dist = 0.f;
	}
	
	qCur.FromAngleAxis(yaw, Vector3(0,1,0));
	auto q = cam->getOrientation();
	yaw += Degree(time * 25.f);  // anim rotate
	qEnd.FromAngleAxis(Degree(20.f), y);  // pitch
	
	nodeRot->setOrientation(qEnd *q * qCur);
	// it->setLightMask(3); //..?
	nodeRot->_getFullTransformUpdated();
}

void CHud::Hud3d::ShowCup()
{
	if (!nodeRot)  return;
	dist = 10.f;
	nodeRot->setVisible(1);
}

void CHud::Hud3d::Hide()
{
	if (!nodeRot)  return;
	nodeRot->setVisible(0);
}
