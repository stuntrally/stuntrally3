#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CScene.h"
#include "Axes.h"
#include "CGui.h"
#include "CApp.h"
#include "Road.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreItem.h>
using namespace Ogre;


//  🆕🚧 Create cursor
void App::CreateBox(BoxCur& box, String sMat, String sMesh, int x, bool shadow)
{
	if (box.nd)  return;
	// LogO("---- create cursor: " + sMat +" "+ sMesh);
	box.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	box.it = mSceneMgr->createItem(sMesh);
	box.it->setVisibilityFlags(RV_Hud3Ded);
	box.it->setCastShadows(shadow);
	if (!sMat.empty())
		box.it->setDatablockOrMaterialName(sMat);
	if (!shadow)
		box.it->setRenderQueueGroup(RQG_Ghost);  // after road
	
	box.nd->setPosition(Vector3(x,0,0));
	box.nd->attachObject(box.it);
	box.nd->setVisible(false);
}

//  🧊🚧  Init 3d cursor meshes
void App::CreateCursors()
{
	LogO("C--- create Cursors");
	CreateBox(boxCar, "", "car.mesh", 0, 1);
	
	CreateBox(boxStart[0], "start_box", "cube.mesh", 20000);
	CreateBox(boxStart[1], "end_box", "cube.mesh", 20000);

	CreateBox(boxFluid, "fluid_box", "box_fluids.mesh");
	CreateBox(boxObj, "object_box", "box_obj.mesh");
	CreateBox(boxEmit, "emitter_box", "box_obj.mesh");
	CreateBox(boxCol, "collect_box", "box_obj.mesh");
}


//  🔁🏁 upd start, end boxes
void App::UpdStartPos(bool vis)
{
	for (int i=0; i < 2; ++i)
	{
		Vector3 p1 = Axes::toOgre(scn->sc->startPos[i]);
		Quaternion q1 = Axes::toOgre(scn->sc->startRot[i]);
		if (i == iEnd)
		{
			boxCar.nd->setPosition(p1);  boxCar.nd->setOrientation(q1);
			boxCar.nd->setVisible(vis && gui->bGI && scn->road);  // hide before load
			boxCar.nd->_getFullTransformUpdated();
		}else
			boxCar.nd->setVisible(vis);
		boxStart[i].nd->setPosition(p1);  boxStart[i].nd->setOrientation(q1);

		if (scn->road)
			boxStart[i].nd->setScale(Vector3(1, scn->road->vStartBoxDim.y, scn->road->vStartBoxDim.z));
	
		boxStart[i].nd->_getFullTransformUpdated();
		boxStart[i].nd->setVisible(vis && edMode == ED_Start && bEdit());
	}
}

//  🚧 ed cursor
void App::UpdFluidBox()
{
	int fls = scn->sc->fluids.size();
	bool bFluids = edMode == ED_Fluids && fls > 0 && !bMoveCam;
	if (fls > 0)
		iFlCur = std::max(0, std::min(iFlCur, fls-1));

	if (!boxFluid.nd)  return;
	boxFluid.nd->setVisible(bFluids);
	if (!bFluids)  return;
	
	FluidBox& fb = scn->sc->fluids[iFlCur];
	boxFluid.nd->setPosition(fb.pos);
	boxFluid.nd->setScale(fb.size);
	boxFluid.nd->_getFullTransformUpdated();
}
