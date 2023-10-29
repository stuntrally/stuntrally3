#include "pch.h"
#include "enums.h"
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
	
		Vector3 vInpC(0,0,0);
		Real fSmooth = (powf(1.0f - pSet->cam_inert, 2.2f) * 40.f + 0.1f) * ivDT;

		const Real sens = 0.13;
		if (bCam())
			vInpC = Vector3(mx, my, 0)*sens;
		Vector3 vInp = Vector3(mx, my, 0)*sens;  mx = 0;  my = 0;
		vNew += (vInp-vNew) * fSmooth;

		if (mbMiddle){	mTrans.z += vInpC.y * 1.6f;  }  // zoom
		if (mbRight){	mTrans.x += vInpC.x;  mTrans.y -= vInpC.y;  }  // pan
		if (mbLeft){	mRotX -= vInpC.x;  mRotY -= vInpC.y;  }  // rot

		Real cs = pSet->cam_speed;  Degree cr(pSet->cam_speed);
		auto& tds = scn->sc->tds;
		Real fMove = !tds.empty()
			? tds[scn->terCur].fTerWorldSize * 0.15f * cs  // par dynamic
			: 100*cs;  // gui par..
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

//  Cam pos etc
//------------------------------------------------------------------------
void App::UpdCamPos()
{
	if (!pSet->camPos || !txCamPos)  return;

	const auto& pos = mCamera->getDerivedPosition();
	auto dir = mCamera->getDerivedDirection();
		
	String s = TR("#{Camera}: ")+fToStr(pos.x,1,5)+" " + fToStr(pos.y,1,5) + " " + fToStr(pos.z,1,5);
		//+ TR("\n#C0E0F0 #{Obj_Rot}: ")+fToStr(dir.x,2) + " "+fToStr(dir.y,2)+" "+fToStr(dir.z,2);

	if (scn->road && scn->road->bHitTer)
	{
		const auto& hit = scn->road->posHit;
		s += TR("\n#C0FFC0#{Terrain}: ")+fToStr(hit.x,1,5)+" " + fToStr(hit.y,1,5) + " " + fToStr(hit.z,1,5);
		auto dist = pos.distance(hit);
		s += TR("\n#D0FFD0#{Distance}: ")+fToStr(dist,1,5);
	}
	txCamPos->setCaption(s);
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
			gui->vSubTabsEdit.size() > TAB_Terrain &&
			gui->vSubTabsEdit[TAB_Terrain]->getIndexSelected() == 5;  // tab
		ndTerGen->setVisible(vis);
	}

	//  track events
	if (eTrkEvent != TE_None)
	{
		if (iUpdEvent == 0)
			iUpdEvent++;  // skip frame to show status
		else
		{	switch (eTrkEvent)
			{
				case TE_Load:	LoadTrackEv();  break;
				case TE_Save:	SaveTrackEv();  break;
				case TE_Update: UpdateTrackEv();  break;
				default:  break;
			}
			eTrkEvent = TE_None;
	}	}
	
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
		road->Pick(mCamera, mx, my,  setpos, edMode == ED_Road, hide, 
			scn->sc->tds[scn->terCur].fTerWorldSize);
	}

	EditMouse();  // edit


	///<>  Ter upd	- - -
	static int tu = 0, bu = 0;
	if (tu >= pSet->ter_skip)  //; restore mini rtt skip
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
	if (scn->ter && road && bEdit() && road->bHitTer)
	{
		Real s = shift ? 0.25 : ctrl ? 4.0 :1.0;
		switch (edMode)
		{
		case ED_Deform:
			if (mbLeft) {  deformed = true;  Deform(road->posHit, dt, s);  }else
			if (mbRight){  deformed = true;  Deform(road->posHit, dt,-s);  }
			break;
		case ED_Filter:
			if (mbLeft) {  deformed = true;  Filter(road->posHit, dt, s);  }
			break;
		case ED_Smooth:
			if (mbLeft) {  deformed = true;  Smooth(road->posHit, dt);  }
			break;
		case ED_Height:
			if (mbLeft) {  deformed = true;  Height(road->posHit, dt, s);  }
			break;
		default:  break;
		}
	}

	///  ⛰️ Terrain  ----
	if (mGraphicsSystem->getRenderWindow()->isVisible())
	{
		// Force update the shadow map every frame
		const float lightEpsilon = 0.0001f;  //** 0.0f slow
		for (auto ter : scn->ters)
		if (ter)
			ter->update( !scn->sun ? -Vector3::UNIT_Y :
				scn->sun->getDerivedDirectionUpdated(), 0, lightEpsilon );
	}


	//  🌿 upd grass after terrain deform
	if (0 && pSet->bTrees)  // todo ..
	{
		///  upd grass
		if (grsDens != gdOld)
		{
			scn->grass->Destroy();
			scn->grass->Create(this);
		}

		if (!deformed && defOld)
		{
			// scn->UpdGrassDens();  // upd..?
			//if (grd.rnd)
			//	grd.rnd->update();

			scn->grass->Destroy();
			scn->grass->Create(this);
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
			scn->grass->Create(this);  // 🌿
	}
	
	
	//  🛣️ roads  upd 🔁
	if (!scn->roads.empty())
	{
		SplineRoad* road1 = scn->roads[0];

		for (SplineRoad* road : scn->roads)
		{
			road->bCastShadow = pSet->g.shadow_type >= Sh_Depth;
			bool fu = road->RebuildRoadInt();
			if (fu)
				scn->grid.Create();
			
			bool full = road == road1 && fu;
			if (full && scn->pace[0])  // pace, only for 1st road
			{
				scn->pace[0]->SetupTer(scn->ters[0]);  // 1st ter only-
				road->RebuildRoadPace();
				scn->pace[0]->Rebuild(road, scn->sc, pSet->trk_reverse);
			}
		}
	}

	prvScene.Update(dt);  // 🧊 rotate


	UpdCubeRefl();  // 🔮💫

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
	//  fixes white texture flashes
	{
		auto* texMgr = mRoot->getRenderSystem()->getTextureGpuManager();
		texMgr->waitForStreamingCompletion();
	}

	UpdateKey(dt);  // key edits etc
	
	UpdFpsText(dt);
	UpdCamPos();


	scn->UpdSun(dt);  // ⛅

	for (int i=0; i < CScene::SK_ALL; ++i)
	if (scn->ndSky[i])
	{	scn->ndSky[i]->setPosition(mCamera->getPosition());
		scn->ndSky[i]->_getFullTransformUpdated();
	}

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

	//  speed multipliers
	moveMul = 1;  rotMul = 1;
	if(shift){  moveMul *= 0.2;  rotMul *= 0.4;  }  // 16 8, 4 3, 0.5 0.5
	if(ctrl) {  moveMul *= 4;    rotMul *= 2.0;  }
	if(alt)  {  moveMul *= 16;   rotMul *= 6.0;  }
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
		
		scn->refl.DestroyFluids();
		scn->refl.CreateFluids();
	}

	if (bRecreateFluidsRTT)
	{	bRecreateFluidsRTT = false;
		
		scn->refl.DestroyFluids();
		scn->refl.DestroyRTT();  // fixme, crash from listener?
		scn->refl.CreateRTT();
		// AddListenerRnd2Tex();
		scn->refl.CreateFluids();
	}

	//  🔥 emitters  upd 🔁
	if (bRecreateEmitters)
	{	bRecreateEmitters = false;

		scn->DestroyEmitters(false);
		scn->CreateEmitters(edMode == ED_PrvCam);
		UpdEmtBox();
	}

	
	//--  3d view upd  (is global in window)
	static bool oldVis = false;
	int tab = mWndTabsEdit->getIndexSelected(),
		st5 = gui->vSubTabsEdit[TAB_Veget]->getIndexSelected();
	bool vis = mWndEdit && mWndEdit->getVisible() &&
		(tab == TAB_Objects || tab == TAB_Veget && st5 == 1);

	if (oldVis != vis)
	{	oldVis = vis;
		rt[RT_PreView3D].nd->setVisible(vis);
		// gui->viewCanvas->setVisible(vis);
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
	scn->UpdateWeather(mCamera, 1.f/dt, !pSet->bWeather ? 0.f : 1.f);

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
	if (scn->pace[0])
		scn->pace[0]->UpdVis(Vector3::ZERO, edMode == ED_PrvCam);

	
	//  upd terrain generator preview
	if (bUpdTerPrv)
	{	bUpdTerPrv = false;
		UpdTerGenPrv();
	}

	
	///  simulate objects
	if (edMode == ED_Objects && objSim /*&& bEdit()*/)
		BltUpdate(dt);
	
	UpdObjNewNode();


	bFirstRenderFrame = false;


	UpdateEnd(dt);  //^

	UpdRnd2Tex();  // * *

}
