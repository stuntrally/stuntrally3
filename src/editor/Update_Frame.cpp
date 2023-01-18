#include "pch.h"
#include "Def_Str.h"
#include "CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "PaceNotes.h"
#include "Grass.h"

#include "MultiList2.h"
// #include "RenderBoxScene.h"
#include "GraphicsSystem.h"
#include "Terra.h"
#include <OgreSceneNode.h>
#include <MyGUI.h>
using namespace Ogre;


///  🖱️ Mouse
//---------------------------------------------------------------------------------------------------------------
void App::processMouse(double fDT)
{
	//  static vars are smoothed
	static Radian sYaw(0), sPth(0);
	static Vector3 sMove(0,0,0);
	static double time = 0.0;
	time += fDT;
	
	const double ivDT = 0.004;  // const interval
	while (time > ivDT)
	{	time -= ivDT;
	
		Vector3 vInpC(0,0,0),vInp;
		Real fSmooth = (powf(1.0f - pSet->cam_inert, 2.2f) * 40.f + 0.1f) * ivDT;

		const Real sens = 0.13;
		if (bCam())
			vInpC = Vector3(mx, my, 0)*sens;
		vInp = Vector3(mx, my, 0)*sens;  mx = 0;  my = 0;
		vNew += (vInp-vNew) * fSmooth;

		if (mbMiddle){	mTrans.z += vInpC.y * 1.6f;  }  //zoom
		if (mbRight){	mTrans.x += vInpC.x;  mTrans.y -= vInpC.y;  }  //pan
		if (mbLeft){	mRotX -= vInpC.x;  mRotY -= vInpC.y;  }  //rot

		Real cs = pSet->cam_speed;  Degree cr(pSet->cam_speed);
		Real fMove = 100*cs;  //par speed
		Degree fRot = 300*cr, fkRot = 160*cr;

		Radian inYaw = rotMul * ivDT * (fRot* mRotX + fkRot* mRotKX);
		Radian inPth = rotMul * ivDT * (fRot* mRotY + fkRot* mRotKY);
		Vector3 inMove = moveMul * ivDT * (fMove * mTrans);

		sYaw += (inYaw - sYaw) * fSmooth;
		sPth += (inPth - sPth) * fSmooth;
		sMove += (inMove - sMove) * fSmooth;

		mCamera->yaw( sYaw );
		mCamera->pitch( sPth );
		mCamera->moveRelative( sMove );
	}
}


//---------------------------------------------------------------------------------------------------------------
//  💫 frame events
//---------------------------------------------------------------------------------------------------------------
void App::UpdateEnd(float dt)
{
	//  show when in gui on generator subtab
	if (ndTerGen)
	{	bool vis = bGuiFocus && mWndEdit &&
			mWndEdit->getVisible() && mWndTabsEdit->getIndexSelected() == TAB_Terrain &&
			gui->vSubTabsEdit.size() > TAB_Terrain && gui->vSubTabsEdit[TAB_Terrain]->getIndexSelected() == 2;
		ndTerGen->setVisible(vis);
	}

	//  track events
	if (eTrkEvent != TE_None)
	{	switch (eTrkEvent)
		{
			case TE_Load:	LoadTrackEv();  break;
			case TE_Save:	SaveTrackEv();  break;
			case TE_Update: UpdateTrackEv();  break;
			default:  break;
		}
		eTrkEvent = TE_None;
	}
	
	///  input
	// mInputWrapper->capture(false);

	//  🛣️📍 Road Pick  --------
	SplineRoad* road = scn->road;
	if (road)
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		Real mx = Real(mp.left) / mWindow->getWidth(),
			 my = Real(mp.top) / mWindow->getHeight();
		bool setpos = edMode >= ED_Road || !brLockPos,
			hide = !(edMode == ED_Road && bEdit());
		road->Pick(mCamera, mx, my,  setpos, edMode == ED_Road, hide);
	}

	EditMouse();  // edit


	///<>  Ter upd	- - -
	static int tu = 0, bu = 0;
	if (tu >= pSet->ter_skip)
	if (bTerUpd)
	{	bTerUpd = false;  tu = 0;
		// if (scn->mTerrainGroup)
		// 	scn->mTerrainGroup->update();
	}	tu++;

	if (bu >= pSet->ter_skip)
	if (bTerUpdBlend)
	{	bTerUpdBlend = false;  bu = 0;
		//if (terrain)
			scn->UpdBlendmap();
	}	bu++;


	TerCircleUpd(dt);
	
	bool deformed = false;
	static bool defOld = false;
	
	float grsDens = scn->sc->densGrass;  // grass dens changed
	static float gdOld = scn->sc->densGrass;

	///<>  ⛰️ Edit Terrain, brushes
	if (scn->terrain && road && bEdit() && road->bHitTer)
	{
		Real s = shift ? 0.25 : ctrl ? 4.0 :1.0;
		switch (edMode)
		{
		case ED_Deform:
			if (mbLeft) {  deformed = true;  deform(road->posHit, dt, s);  }else
			if (mbRight){  deformed = true;  deform(road->posHit, dt,-s);  }
			break;
		case ED_Filter:
			if (mbLeft) {  deformed = true;  filter(road->posHit, dt, s);  }
			break;
		case ED_Smooth:
			if (mbLeft) {  deformed = true;  smooth(road->posHit, dt);  }
			break;
		case ED_Height:
			if (mbLeft) {  deformed = true;  height(road->posHit, dt, s);  }
			break;
		default:  break;
		}
	}

	///  ⛰️ Terrain  ----
	if (mTerra && mGraphicsSystem->getRenderWindow()->isVisible() )
	{
		// Force update the shadow map every frame to avoid the feeling we're "cheating" the
		// user in this sample with higher framerates than what he may encounter in many of
		// his possible uses.
		const float lightEpsilon = 0.0001f;  //** 0.0f slow
		mTerra->update( !scn->sun ? -Vector3::UNIT_Y :
			scn->sun->getDerivedDirectionUpdated(), lightEpsilon );
	}


	//  🌿 upd grass after terrain deform
	if (0 && pSet->bTrees)  // todo ..
	{
		///  upd grass
		if (grsDens != gdOld)
		{
			scn->grass->Destroy();
			scn->grass->Create();
		}

		if (!deformed && defOld)
		{
			// scn->UpdGrassDens();  // upd..?
			//if (grd.rnd)
			//	grd.rnd->update();

			scn->grass->Destroy();
			scn->grass->Create();
		}
		defOld = deformed;
		gdOld = grsDens;
	}


	///  🌳🪨 vegetation toggle  upd 🔁 * * *
	if (bVegetGrsUpd)
	{	bVegetGrsUpd = false;
		pSet->bTrees = !pSet->bTrees;

		scn->LoadRoadDens();
		scn->RecreateTrees();
		scn->grass->Destroy();
		if (pSet->bTrees)
			scn->grass->Create();  // 🌿
	}
	
	
	//  🛣️ roads  upd 🔁
	if (!scn->roads.empty())
	{
		SplineRoad* road1 = scn->roads[0];

		for (SplineRoad* road : scn->roads)
		{
			road->bCastShadow = pSet->shadow_type >= Sh_Depth;
			bool fu = road->RebuildRoadInt();
			
			bool full = road == road1 && fu;
			if (full && scn->pace)  // pace, only for 1st
			{
				scn->pace->SetupTer(scn->terrain);
				road->RebuildRoadPace();
				scn->pace->Rebuild(road, scn->sc, pSet->trk_reverse);
			}
		}
	}


	///**  Render Targets update  //; fixme
	if (edMode == ED_PrvCam)
	{
		scn->sc->camPos = mCamera->getPosition();
		scn->sc->camDir = mCamera->getDirection();
		// if (rt[RT_View].tex)
		// 	rt[RT_View].tex->update();
	}else{
		static int ri = 0;
		if (ri >= pSet->mini_skip)
		{	ri = 0;
			// for (int i=0; i < RT_View; ++i)
			// 	if (rt[i].tex)
			// 		rt[i].tex->update();
		}	ri++;
	}
	//LogO(toStr(dt));
}


//-----------------------------------------------------------------------------------------------------------------------------
//  💫 Update  frame
//-----------------------------------------------------------------------------------------------------------------------------
void App::update( float dt )
{
	UpdateKey(dt);  // key edits etc
	
	UpdFpsText();


	scn->UpdSun();  // ⛅

	if (scn->ndSky)
		scn->ndSky->setPosition(mCamera->getPosition());

	if (mCubeCamera)  // refl
		mCubeCamera->setPosition(mCamera->getPosition());


	static Real time1 = 0.;
	mDTime = dt;
	
	//  inc edit time
	time1 += mDTime;
	if (time1 > 1.)
	{	time1 -= 1.;  ++scn->sc->secEdited;

		if (bGuiFocus)	//  upd ed info txt
			gui->UpdEdInfo();
	}
	
	if (mDTime > 0.1f)  mDTime = 0.1f;  //min 5fps


	//  update input  ----
	//  camera Move,Rot
	#define isKey(a)  IsKey(SDL_SCANCODE_##a)
	mRotX = 0; mRotY = 0;  mRotKX = 0; mRotKY = 0;  mTrans = Vector3::ZERO;
	if (bCam())
	{
		mTrans.x = isKey(D) - isKey(A);  // cam move
		mTrans.z = isKey(S) - isKey(W);
		mTrans.y = isKey(E) - isKey(Q);
		
		mRotKY = isKey(UP)   - isKey(DOWN);  // cam rot
		mRotKX = isKey(LEFT) - isKey(RIGHT);
	}
	#undef isKey

	//  speed multiplers
	moveMul = 1;  rotMul = 1;
	if(shift){	moveMul *= 0.2;	 rotMul *= 0.4;	}  // 16 8, 4 3, 0.5 0.5
	if(ctrl){	moveMul *= 4;	 rotMul *= 2.0;	}
	//if(alt)  {	moveMul *= 0.5;	 rotMul *= 0.5;	}
	//const Real s = (shift ? 0.05 : ctrl ? 4.0 :1.0)

	if (imgCur)  //-
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		imgCur->setPosition(mp);
		imgCur->setVisible(bGuiFocus || !bMoveCam);
	}

	processMouse(mDTime);

	
	///  gui
	gui->GuiUpdate();
	
	
	//  💧 fluids  upd 🔁
	if (bRecreateFluids)
	{	bRecreateFluids = false;

		scn->DestroyFluids();
		scn->CreateFluids();
		UpdFluidBox();
	}

	//  🔥 emitters  upd 🔁
	if (bRecreateEmitters)
	{	bRecreateEmitters = false;

		scn->DestroyEmitters(false);
		if (bParticles)
			scn->CreateEmitters();
		UpdEmtBox();
	}

	
	//--  3d view upd  (is global in window)
	static bool oldVis = false;
	int tab = mWndTabsEdit->getIndexSelected(), st5 = gui->vSubTabsEdit[TAB_Veget]->getIndexSelected();
	bool vis = mWndEdit && mWndEdit->getVisible() && (tab == TAB_Objects || tab == TAB_Veget && st5 == 1);

	if (oldVis != vis)
	{	oldVis = vis;
		gui->viewCanvas->setVisible(vis);
	}
	if (gui->tiViewUpd >= 0.f)
		gui->tiViewUpd += dt;

	if (gui->tiViewUpd > 0.0f)  //par delay 0.1
	{	gui->tiViewUpd = -1.f;

		/*gui->viewBox->clearScene();
		if (!gui->viewMesh.empty() && ResourceGroupManager::getSingleton().resourceExistsInAnyGroup(gui->viewMesh))
		{	gui->viewSc = gui->viewBox->injectObject(gui->viewMesh);
			gui->updVegetInfo();
	}*/	}
	
	
	//  🌧️ Update rain/snow - depends on camera
	scn->UpdateWeather(mCamera, 1.f/dt, pSet->bWeather ? 0.f : 1.f);

	// update shader time
	// mTimer += evt.timeSinceLastFrame;
	// mFactory->setSharedParameter("windTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(mTimer)));
	// mFactory->setSharedParameter("waterTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(mTimer)));
	
	/*if (ndCar && road)  ///()  grass sphere test
	{
		const Vector3& p = ndCar->getPosition();  Real r = road->vStBoxDim.z/2;  r *= r;
		mFactory->setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(p.x,p.y,p.z,r)));
		mFactory->setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(p.x,p.y,p.z,r)));
	}/**/
	
	
	//  pace vis
	if (scn->pace)
		scn->pace->UpdVis(Vector3::ZERO, edMode == ED_PrvCam);

	
	//  upd terrain generator preview
	if (bUpdTerPrv)
	{	bUpdTerPrv = false;
		updateTerGenPrv();
	}

	
	///  simulate objects
	if (edMode == ED_Objects && objSim /*&& bEdit()*/)
		BltUpdate(dt);
	
	UpdObjNewNode();


	bFirstRenderFrame = false;


	UpdRnd2Tex();  // * *

	UpdateEnd(dt);  //^
}
