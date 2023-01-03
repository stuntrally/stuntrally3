#include "pch.h"
#include "RenderConst.h"
#include "settings.h"
#include "CHud.h"
#include "CarModel.h"
#include "FollowCamera.h"
#include "Road.h"
#include "Cam.h"

#include <OgreItem.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
using namespace Ogre;


///  HUD Arrow
///---------------------------------------------------------------------------------------------------------------

void CHud::Arrow::Create(SceneManager* mgr, SETTINGS* pSet)
{
	if (!node)
		node = mgr->getRootSceneNode()->createChildSceneNode();
	if (it)  return;
	it = mgr->createItem("arrow.mesh");
	it->setRenderQueueGroup(RQG_Hud3);
	it->setCastShadows(false);
	
	nodeRot = node->createChildSceneNode();
	nodeRot->attachObject(it);
	nodeRot->setScale(pSet->size_arrow/2.f * Vector3::UNIT_SCALE);
	it->setVisibilityFlags(RV_Hud);
	nodeRot->setVisible(pSet->check_arrow);
}


void CHud::Arrow::UpdateChk(SplineRoad* road, CarModel* carM, const Vector3& pos)
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
		// Vector3 col1 = angle * Vector3(0.0, 1.0, 0.0) + (1-angle) * Vector3(1.0, 0.0, 0.0);
		// Vector3 col2 = angle * Vector3(0.0, 0.4, 0.0) + (1-angle) * Vector3(0.4, 0.0, 0.0);
		// get datablock, set diffuse..
	}
}

void CHud::Arrow::Update(CarModel* carM, float time)
{
	// align checkpoint arrow,  move in front of camera
	if (!node)  return;

	Camera* cam = carM->fCam->cam->cam;

	Vector3 pos = cam->getPosition();
	Vector3 dir = cam->getDirection();  dir.normalise();
	Vector3 up = cam->getUp();  up.normalise();
	Vector3 arrowPos = pos + 10.0f * dir + 3.5f*up;
	node->setPosition(arrowPos);
	
	// animate
	bool bFirstFrame = carM->bGetStart;
	if (bFirstFrame) // 1st frame: dont animate
		qCur = qEnd;
	else
		qCur = Quaternion::Slerp(time*5, qStart, qEnd, true);
	nodeRot->setOrientation(qCur);
	
	// look down -y a bit so we can see the arrow better
	nodeRot->pitch(Degree(-20), SceneNode::TS_LOCAL); 
}

void CHud::Arrow::Destroy(SceneManager* mgr)
{
	if (nodeRot)  mgr->destroySceneNode(nodeRot);  nodeRot = 0;
	if (node)  mgr->destroySceneNode(node);  node = 0;
	if (it)  mgr->destroyItem(it);  it = 0;
}
