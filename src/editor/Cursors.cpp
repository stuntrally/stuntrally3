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
void App::CreateBox(SceneNode*& nd, Item*& it,
	String sMat, String sMesh, int x, bool shadow)
{
	if (nd)  return;
	// LogO("---- create cursor: " + sMat +" "+ sMesh);
	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	it = mSceneMgr->createItem(sMesh);
	it->setVisibilityFlags(RV_Hud3Ded);
	it->setCastShadows(shadow);
	if (!sMat.empty())
		it->setDatablockOrMaterialName(sMat);
	if (!shadow)
		it->setRenderQueueGroup(RQG_Ghost);  // after road
	
	nd->setPosition(Vector3(x,0,0));
	nd->attachObject(it);
	nd->setVisible(false);
}

//  🧊🚧  Init 3d cursor meshes
void App::CreateCursors()
{
	LogO("C--- create Cursors");
	CreateBox(ndCar, itCar, "", "car.mesh", 0, 1);
	
	CreateBox(ndStartBox[0], itStartBox[0], "start_box", "cube.mesh", 20000);
	CreateBox(ndStartBox[1], itStartBox[1], "end_box", "cube.mesh", 20000);

	CreateBox(ndFluidBox, itFluidBox, "fluid_box", "box_fluids.mesh");
	CreateBox(ndObjBox, itObjBox, "object_box", "box_obj.mesh");
	CreateBox(ndEmtBox, itEmtBox, "emitter_box", "box_obj.mesh");
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
			ndCar->setPosition(p1);  ndCar->setOrientation(q1);
			ndCar->setVisible(vis && gui->bGI && scn->road);  // hide before load
			ndCar->_getFullTransformUpdated();
		}else
			ndCar->setVisible(vis);
		ndStartBox[i]->setPosition(p1);  ndStartBox[i]->setOrientation(q1);

		if (scn->road)
			ndStartBox[i]->setScale(Vector3(1, scn->road->vStartBoxDim.y, scn->road->vStartBoxDim.z));
	
		ndStartBox[i]->_getFullTransformUpdated();
		ndStartBox[i]->setVisible(vis && edMode == ED_Start && bEdit());
	}
}

//  🚧 ed cursor
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
	ndFluidBox->_getFullTransformUpdated();
}
