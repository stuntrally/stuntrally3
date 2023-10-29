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
#include <OgreItem.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
using namespace Ogre;


///  HUD Arrow
///---------------------------------------------------------------------------------------------------------------

void CHud::Arrow::Create(SceneManager* mgr, SETTINGS* pSet, int plr)
{
	if (!node)
		node = mgr->getRootSceneNode()->createChildSceneNode();
	if (it)  return;
	player = plr;

	it = mgr->createItem("arrow.mesh");  it->setCastShadows(false);
	it->setRenderQueueGroup(RQG_Hud3);
	it->setDatablockOrMaterialName("Arrow"+toStr(player));

	pDb = dynamic_cast<HlmsPbsDb2*>( it->getSubItem(0)->getDatablock() );
	// LogO(pDb ? "arrow db2 cast ok" : "arrow db2 cast fail");
	
	nodeRot = node->createChildSceneNode();
	nodeRot->attachObject(it);
	nodeRot->setScale(pSet->size_arrow/2.f * Vector3::UNIT_SCALE);
	it->setVisibilityFlags(RV_Hud3D[player]);
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
		Vector3 col1 = angle * Vector3(0.0, 0.8, 0.0) + (1.f-angle) * Vector3(0.8, 0.0, 0.0);
		Vector3 col2 = angle * Vector3(0.0, 0.3, 0.0) + (1.f-angle) * Vector3(0.3, 0.0, 0.0);
		if (pDb)
		{	pDb->setDiffuse(col1);
			pDb->setSpecular(col2);
	}	}
}

void CHud::Arrow::Update(CarModel* carM, float time)
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

void CHud::Arrow::Destroy(SceneManager* mgr)
{
	if (nodeRot)  mgr->destroySceneNode(nodeRot);  nodeRot = 0;
	if (node)  mgr->destroySceneNode(node);  node = 0;
	if (it)  mgr->destroyItem(it);  it = 0;
	pDb = 0;
}
