#include "pch.h"
#include "BaseApp.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "Road.h"
#include "paths.h"
// #include <OgreRenderTexture.h>
#include <MyGUI.h>
#include <SDL_events.h>
using namespace MyGUI;
using namespace Ogre;


//---------------------------------------------------------------------------------------------------------------
//  Key Press
//---------------------------------------------------------------------------------------------------------------

void App::keyPressed(const SDL_KeyboardEvent &arg)
{
	SDL_Scancode skey = arg.keysym.scancode;
	#define key(a)  SDL_SCANCODE_##a

	BaseKeyPressed(arg);

	///  Preview camera  ---------------------
	if (edMode == ED_PrvCam)
	{
		switch (skey)
		{
			case key(ESCAPE):  case key(F7):  togPrvCam();  break;  // exit

			case key(KP_ENTER):  case key(RETURN):  // save screen
			{	int u = pSet->allow_save ? pSet->gui.track_user : 1;
				// rt[RT_View].tex->writeContentsToFile(gcom->pathTrk[u] + pSet->gui.track + "/preview/view.jpg");
				gcom->listTrackChng(gcom->trkList,0);  // upd gui img
				gui->Status("#{Saved}", 1,1,0);
			}	break;

			case key(F12):  // screenshot
				// mWindow->writeContentsToTimestampedFile(PATHS::Screenshots() + "/", ".jpg");
				return;
			default:  break;
		}
		return;
	}

	//  main menu keys
	Widget* wf = InputManager::getInstance().getKeyFocusWidget();
	bool editFocus = wf && wf->getTypeName() == "EditBox";

	if (pSet->bMain && bGuiFocus)
	{
		switch (skey)
		{
		case key(UP):  case key(KP_8):
			pSet->inMenu = (pSet->inMenu-1+WND_ALL)%WND_ALL;
			gui->toggleGui(false);  return;

		case key(DOWN):  case key(KP_2):
			pSet->inMenu = (pSet->inMenu+1)%WND_ALL;
			gui->toggleGui(false);  return;

		case key(KP_ENTER):  case key(RETURN):
			pSet->bMain = false;
			gui->toggleGui(false);  return;
		default:  break;
		}
	}
	if (!pSet->bMain && bGuiFocus)
	{
		switch (skey)
		{
		case key(BACKSPACE):
			if (pSet->bMain)  break;
			if (bGuiFocus)
			{	if (editFocus)  break;
				pSet->bMain = true;  gui->toggleGui(false);  }
			return;
		default:  break;
		}
	}

	//  change gui tabs
	TabPtr tab = 0;  TabControl* sub = 0;  int iTab1 = 1;
	if (bGuiFocus && !pSet->bMain)
	switch (pSet->inMenu)
	{
		case WND_Track:   tab = mWndTabsTrack;  sub = gui->vSubTabsTrack[tab->getIndexSelected()];  break;
		case WND_Edit:    tab = mWndTabsEdit;   sub = gui->vSubTabsEdit[tab->getIndexSelected()];  break;
		case WND_Help:    tab = sub = gui->vSubTabsHelp[1];  iTab1 = 0;  break;
		case WND_Options: tab = mWndTabsOpts;   sub = gui->vSubTabsOpts[tab->getIndexSelected()];  break;
		default:  break;
	}
	bool edit = bEdit();
	SplineRoad* road = scn->road;
	bool bRoad = edMode == ED_Road && road && edit;


	///  Pick open  ---------------------
	bool editGui = bGuiFocus && !pSet->bMain && pSet->inMenu==WND_Edit;
	if (skey==key(TAB) && editGui)
	{
		switch (tab->getIndexSelected())
		{	case TAB_Sun:  /*if (sub->getIndexSelected()==0)*/ {  gui->btnPickSky(0);  return;  }  break;
			case TAB_Layers:  gui->btnPickTex(0);  return;
			case TAB_Grass:  if (sub->getIndexSelected()==1) {  gui->btnPickGrass(0);  return;  }  break;
			case TAB_Veget:  if (sub->getIndexSelected()==1) {  gui->btnPickVeget(0);  return;  }  break;
			case TAB_Road:
				switch (sub->getIndexSelected())
				{
				case 0:  gui->btnPickRoad(0);   return;
				case 1:  gui->btnPickPipe(0);   return;
				case 2:  gui->btnPickRoadCol(0);   return;
				}	break;
			default:  break;
		}
		mWndPick->setVisible(false);
	}

	//  Global keys
	//------------------------------------------------------------------------------------------------------------------------------
	switch (skey)
	{
		case key(ESCAPE): //  quit
			if (pSet->escquit)
				Quit();
			return;

		case key(F1):  case key(GRAVE):
			if (ctrl)  // context help (show for cur mode)
			{
				if (bMoveCam)         gui->GuiShortcut(WND_Help, 1, 1);
				else switch (edMode)
				{	case ED_Smooth: case ED_Height: case ED_Filter:
					case ED_Deform:   gui->GuiShortcut(WND_Help, 1, 3);  break;
					case ED_Road:     gui->GuiShortcut(WND_Help, 1, 5);  break;
					case ED_Start:    gui->GuiShortcut(WND_Help, 1, 8);  break;
					case ED_Fluids:   gui->GuiShortcut(WND_Help, 1, 9);  break;
					case ED_Objects:  gui->GuiShortcut(WND_Help, 1, 10);  break;
					case ED_Particles:gui->GuiShortcut(WND_Help, 1, 11);  break;
					case ED_PrvCam:
					default:		  gui->GuiShortcut(WND_Help, 1, 0);  break;
			}	}
			else	//  Gui mode, Options
				gui->toggleGui(true);
			return;

		case key(F12):  //  screenshot
			// mWindow->writeContentsToTimestampedFile(PATHS::Screenshots() + "/",
			// 	pSet->screen_png ? ".png" : ".jpg");
			return;

		//  save, reload
		case key(F4):  if (!alt)  SaveTrack();  return;
		case key(F5):  LoadTrack();  return;
		
		case key(F8):  // update
			if (editGui)
			switch (tab->getIndexSelected())
			{	case TAB_Layers:  gui->btnUpdateLayers(0);  return;
				case TAB_Grass:  gui->btnUpdateGrass(0);  return;
				case TAB_Veget:  gui->btnUpdateVeget(0);  return;
			}
			UpdateTrack();  return;  // default full

		case key(F9):  // blendmap
			gui->ckDebugBlend.Invert();
			bTerUpdBlend = true;
			return;


		//  prev num tab (layers,grasses,models)
		case key(1):  if (alt && !bRoad)  {  gui->NumTabNext(-1);  return;  }  break;
		//  next num tab
		case key(2):  if (alt && !bRoad)  {  gui->NumTabNext( 1);  return;  }  break;

		case key(F2):  // +-rt num
   			if (shift)
   			{	pSet->num_mini = (pSet->num_mini - 1 + RT_ALL) % RT_ALL;  UpdMiniVis();  }
   			else
   			if (bGuiFocus && tab && !pSet->bMain)
   				if (alt)  // prev gui subtab
   				{
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() - 1 + num) % num );  }
	   			}
   				else	// prev gui tab
   				{	int num = tab->getItemCount()-1, i = tab->getIndexSelected();
					if (i==iTab1)  i = num;  else  --i;
					tab->setIndexSelected(i);  if (iTab1==1)  gui->tabMainMenu(tab,i);
	   			}
   			break;

		case key(F3):  // tabs,sub
   			if (shift)
   			{	pSet->num_mini = (pSet->num_mini + 1) % RT_ALL;  UpdMiniVis();  }
   			else
   			if (bGuiFocus && tab && !pSet->bMain)
   				if (alt)  // next gui subtab
   				{
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() + 1) % num );  }
	   			}
	   			else	// next gui tab
	   			{	int num = tab->getItemCount()-1, i = tab->getIndexSelected();
					if (i==num)  i = iTab1;  else  ++i;
					tab->setIndexSelected(i);  if (iTab1==1)  gui->tabMainMenu(tab,i);
				}
   			break;
   			
   		case key(SPACE):  // subtabs
   			if (bGuiFocus && !editFocus && tab && !pSet->bMain)
				if (sub)  {  int num = sub->getItemCount();
					sub->setIndexSelected( (sub->getIndexSelected() + (shift ? -1 : 1) + num) % num );  }
			break;
   			
		case key(KP_ENTER):  case key(RETURN):  // load track
			if (bGuiFocus)
			if (mWndTabsTrack->getIndexSelected() == 1 && !pSet->bMain && pSet->inMenu == WND_Track)
				gui->btnNewGame(0);
   			break;


		//  Fps, WireFrame  F11, F10
		case key(F11):
			gui->ckFps.Invert();  return;

		case key(F10):
			gui->ckWireframe.Invert();  return;


		//  Show Stats  ctrl-I
		case key(I):
   			if (ctrl) {  gui->ckInputBar.Invert();  return;  }
			break;

		//  Top view  alt-Z
		case key(Z):
			if (alt)  {  gui->toggleTopView();  return;  }
			break;

		//  load next track  F6
		case key(F6):
			if (pSet->check_load)
			{	gui->iLoadNext = shift ? -1 : 1;  return;  }
			break;
		default:  break;
	}

	//  GUI  keys in edits  ---------------------
	if (bGuiFocus && mGui && !alt && !ctrl)
	{
		MyGUI::KeyCode kc = SDL2toGUIKey(arg.keysym.sym);
		MyGUI::InputManager::getInstance().injectKeyPress(kc);
		return;
	}


	///  Road keys  * * * * * * * * * * * * * * *
	if (bRoad)
		keyPressRoad(skey);


	//  ter brush shape
	if (edMode < ED_Road && !alt && edit)
	switch (skey)
	{
		case key(K):    if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]-1 + BRS_ALL) % BRS_ALL);  updBrush();  }  break;
		case key(L):    if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]+1) % BRS_ALL);            updBrush();  }  break;
		case key(N): case key(COMMA):   mBrOct[curBr] = std::max(1, mBrOct[curBr]-1);  updBrush();  break;
		case key(M): case key(PERIOD):  mBrOct[curBr] = std::min(7, mBrOct[curBr]+1);  updBrush();  break;

		case key(KP_ENTER):
		case key(RETURN):  brLockPos = !brLockPos;  break;
		case key(SPACE):  //  set brush height from terrain
			if (edMode == ED_Height)
			{	terSetH = road->posHit.y;  }
			break;
		default:  break;
	}

	//  ter brush presets  ----
	if (edMode < ED_Road && alt && skey >= key(1) && skey <= key(0) && edit)
	{
		int id = skey - key(1);
		if (shift)  id += 10;
		SetBrushPreset(id);
	}


	//  Fluids, Objects, Emitters  * * * * * * * * * * *
	keyPressObjects(skey);


	///  Common Keys
	//************************************************************************************************************
	if (alt)
	switch (skey)    //  alt-  Shortcuts  ???????
	{
		case key(Q):  gui->GuiShortcut(WND_Track, 1);  return;  // Q Track  ???????
		case key(O):  gui->GuiShortcut(WND_Track, 2);  return;  // O Tools  ???????

		case key(W):  gui->GuiShortcut(WND_Track, 3);  return;  // W Game  ????
		case key(P):  gui->GuiShortcut(WND_Track, 4);  return;  // P Pacenotes  ????
		case key(J):  gui->GuiShortcut(WND_Track, 5);  return;  // J Warnings  ???

		case key(S):  gui->GuiShortcut(WND_Edit, TAB_Sun);       return;  // S Sun  ???????
		case key(H):  gui->GuiShortcut(WND_Edit, TAB_Terrain);   return;  // H Heightmap  ??????
		 case key(D): gui->GuiShortcut(WND_Edit, TAB_Terrain,1); return;  //  D -Brushes

		case key(T):  gui->GuiShortcut(WND_Edit, TAB_Layers);    return;  // T Layers (Terrain)  ???????
		 case key(B): gui->GuiShortcut(WND_Edit, TAB_Layers,0);  return;  //  B -Blendmap

		case key(G):  gui->GuiShortcut(WND_Edit, TAB_Grass);     return;  // G Grasses  ????
		 case key(F): gui->GuiShortcut(WND_Edit, TAB_Grass,2);   return;  //  F -Channels

		case key(V):  gui->GuiShortcut(WND_Edit, TAB_Veget);     return;  // V Vegetation  ????????
		 case key(M): gui->GuiShortcut(WND_Edit, TAB_Veget,1);   return;  //  M -Models

		case key(U):  gui->GuiShortcut(WND_Edit, TAB_Surface);   return;  // U Surfaces  ???????
		case key(R):  gui->GuiShortcut(WND_Edit, TAB_Road);      return;  // R Road  ???????
		case key(X):  gui->GuiShortcut(WND_Edit, TAB_Objects);   return;  // X Objects  ????

		case key(C):  gui->GuiShortcut(WND_Options, 1);	  return;  // C Screen  ???????
		case key(A):  gui->GuiShortcut(WND_Options, 2);   return;  // A Graphics  ????

		case key(E):  gui->GuiShortcut(WND_Options, 3);   return;  // E View /Settings  ??????
		case key(K):  gui->GuiShortcut(WND_Options, 4);   return;  // K Tweak  ????
		
		case key(I):  gui->GuiShortcut(WND_Help, 1);  return;  // I Input/help  ????
		default:  break;
	}
	//............................................................................................................
	else switch (skey)
	{
		case key(TAB):	//  ???? Camera / Edit mode
		if (!bGuiFocus && !alt)  {
			bMoveCam = !bMoveCam;
			UpdVisGui();  UpdFluidBox();  UpdObjPick();
		}	break;

		//  ??? toggle fog, veget, weather, particles
		case key(V):  bVegetGrsUpd = true;  break;
		case key(G):  gui->ckFog.Invert();  break;
		case key(I):  gui->ckWeather.Invert();  break;
		case key(P):  bParticles = !bParticles;  bRecreateEmitters = true;  break;

		//  ?????? terrain
		case key(D):  if (bEdit()){  SetEdMode(ED_Deform);  curBr = 0;  updBrush();  UpdEditWnds();  }	break;
		case key(S):  if (bEdit()){  SetEdMode(ED_Smooth);  curBr = 1;  updBrush();  UpdEditWnds();  }	break;
		case key(E):  if (bEdit()){  SetEdMode(ED_Height);  curBr = 2;  updBrush();  UpdEditWnds();  }	break;
		case key(F):  if (bEdit()){  SetEdMode(ED_Filter);  curBr = 3;  updBrush();  UpdEditWnds();  }
			else  //  focus on find edit  (global)
			if (ctrl && gcom->edTrkFind /*&& bGuiFocus &&
				!pSet->isMain && pSet->inMenu == WND_Edit && mWndTabsEdit->getIndexSelected() == 1*/)
			{
				if (wf == gcom->edTrkFind)  // ctrl-F  twice to toggle filtering
				{	gcom->ckTrkFilter.Invert();  return;  }
				gui->GuiShortcut(WND_Track, 1);  // Track tab
				InputManager::getInstance().resetKeyFocusWidget();
				InputManager::getInstance().setKeyFocusWidget(gcom->edTrkFind);
				return;
			}	break;

		//  ??????? road
		case key(R):  if (bEdit()){  SetEdMode(ED_Road);	UpdEditWnds();  }	break;
		case key(B):  if (road)  {  road->UpdPointsH();  road->Rebuild(true);  }  break;
		case key(T):  if (edMode == ED_Road && mWndRoadStats)
						mWndRoadStats->setVisible(!mWndRoadStats->getVisible());  break;
		case key(M):  if (edMode == ED_Road && road)  road->ToggleMerge();  break;

		//  ???? start pos
		case key(Q):  if (bEdit()){  SetEdMode(ED_Start);  UpdEditWnds();  }   break;
		case key(SPACE):
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
		case key(KP_ENTER):  case key(RETURN):
			if (edMode == ED_Start)  iEnd = 1 - iEnd;  UpdStartPos();  break;


		//  ??????????? prv cam
		case key(F7):  togPrvCam();  break;


		//  ???? fluids
		case key(W):  if (bEdit()){  SetEdMode(ED_Fluids);  UpdEditWnds();  }   break;
		//case key(F10):  SaveWaterDepth();   break;

		//  ???? objects
		case key(C):  if (edMode == ED_Objects)  {  objSim = !objSim;  ToggleObjSim();  }  break;
		case key(X):  if (bEdit()){  SetEdMode(ED_Objects);  UpdEditWnds();  }   break;

		//  ???? particles
		case key(A):  if (bEdit()){  SetEdMode(ED_Particles);  UpdEditWnds();  }   break;
		default:  break;
	}
}
