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
void App::CreateBox(SceneNode*& nd, Item*& it, String sMat, String sMesh, int x)
{
	if (nd)  return;
	// LogO("---- create cursor: " + sMat +" "+ sMesh);
	MaterialPtr mtr;
	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	it = mSceneMgr->createItem(sMesh);
	it->setVisibilityFlags(RV_Hud3D);
	it->setCastShadows(false);
	if (!sMat.empty())
		it->setDatablockOrMaterialName(sMat);
	it->setRenderQueueGroup(RQG_Ghost);  // after road
	
	nd->setPosition(Vector3(x,0,0));
	nd->attachObject(it);
	nd->setVisible(false);
}

//  🧊🚧  Init 3d cursor meshes
void App::CreateCursors()
{
	LogO("C--- create Cursors");
	CreateBox(ndCar, itCar, "", "car.mesh");
	
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
		}else
			ndCar->setVisible(vis);
		ndStartBox[i]->setPosition(p1);  ndStartBox[i]->setOrientation(q1);

		if (scn->road)
			ndStartBox[i]->setScale(Vector3(1, scn->road->vStartBoxDim.y, scn->road->vStartBoxDim.z));
	
		ndStartBox[i]->setVisible(vis && edMode == ED_Start && bEdit());
	}
}
