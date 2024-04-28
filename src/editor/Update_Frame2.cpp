#include "pch.h"
#include "enums.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "HudRenderable.h"

#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
using namespace Ogre;
using namespace MyGUI;


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
void App::UpdateKey(float dt)
{
	UpdMiniPos();
	
	//  status
	if (fStFade > 0.f)
	{	fStFade -= dt;
		
		gui->panStatus->setAlpha(std::min(1.f, fStFade / 1.5f));
		if (fStFade <= 0.f)
			gui->panStatus->setVisible(false);
	}

	//  key,mb info  ==================
	if (pSet->inputBar)
		UpdKeyBar(dt);


	//  📃 keys up/dn - for trk list
	//------------------------------------------------------------------------
	#define isKey(a)  IsKey(SDL_SCANCODE_##a)
	bool up   = isKey(UP)  ||isKey(KP_8);
	bool down = isKey(DOWN)||isKey(KP_2);
	bool pgup   = isKey(PAGEUP)  ||isKey(KP_9);
	bool pgdown = isKey(PAGEDOWN)||isKey(KP_3);
	#undef isKey

	static float fUp = 0.f, fDn = 0.f, fPgUp = 0.f, fPgDn = 0.f;
	const float rpt = -0.15f;  // -0.15 s delay
	const int d = alt ? 16 : ctrl ? 4  : 1,
	         pg = alt ? 64 : ctrl ? 32 : 8;

	WP wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	if (bGuiFocus && wf != (WP)gcom->trkDesc[0] && wf != (WP)gcom->trkAdvice[0])
	{
		if (down)  fDn += dt;  else  if (pgdown)  fPgDn += dt;  else
		if (up)    fUp += dt;  else  if (pgup)    fPgUp += dt;  else
		{	fUp = 0.f;  fDn = 0.f;  fPgUp = 0.f;  fPgDn = 0.f;  }
		if (fUp   > 0.f) {  gcom->trkListNext(-d);  fUp = rpt;  }
		if (fDn   > 0.f) {  gcom->trkListNext( d);  fDn = rpt;  }
		if (fPgUp > 0.f) {  gcom->trkListNext(-pg);  fPgUp = rpt;  }
		if (fPgDn > 0.f) {  gcom->trkListNext( pg);  fPgDn = rpt;  }
	}

	
	///  Update Info texts  and edit by key continuous 
	///. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	const Real q = (shift ? 0.05 : ctrl ? 4.0 :1.0) * 20 * dt;

	SplineRoad* road = scn->road;
	if (edMode == ED_Road && bEdit() && scn->road)
		KeyTxtRoad(q);

	else if (edMode < ED_Road)
		KeyTxtTerrain(q);  // Brush

	else if (edMode == ED_Start && road)
		KeyTxtStart(q);  // Start box, road dir

	else if (edMode == ED_Fluids)
		KeyTxtFluids(q);

	else if (edMode == ED_Objects)
		KeyTxtObjects();

	else if (edMode == ED_Particles)
		KeyTxtEmitters(q);

	else if (edMode == ED_Collects)
		KeyTxtCollects();


	mz = 0;  // mouse wheel

	
	//  rebuild road after end of selection change
	static bool bSelChngOld = false;
	if (road)
	{
		road->fLodBias = pSet->g.road_dist;  // after rebuild

		if (bSelChngOld && !road->bSelChng)
			road->Rebuild(true);

		bSelChngOld = road->bSelChng;
		road->bSelChng = false;
	}

	///  upd road lods
	static int dti = 5, ti = dti-1;  ++ti;
	if (road && ti >= dti)
	{	ti = 0;

		Real dist = pSet->g.road_dist, dist2 = 600*600; //par
		bool prv = edMode == ED_PrvCam, ed = edMode == ED_Road && !bMoveCam;

		int i = 0;
		for (auto* r : scn->roads)
		if (!r->IsTrail())
		{
			r->UpdLodVis(dist, prv);
			r->UpdLodVisMarks(dist2, ed && i == scn->rdCur);
			++i;
		}
	}
}
