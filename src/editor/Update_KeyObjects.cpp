#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "Gui_Def.h"
#include "CData.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "SceneClasses.h"
#include <OgreVector3.h>
#include <OgreSceneNode.h>
using namespace Ogre;


//---------------------------------------------------------------------------------------------------------------
//  Key Press
//---------------------------------------------------------------------------------------------------------------

void App::keyPressObjects(SDL_Scancode skey)
{
	#define key(a)  SDL_SCANCODE_##a
	bool edit = bEdit();
	SplineRoad* road = scn->road;

	
	//  💧 Fluids  ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	if (edMode == ED_Fluids && edit)
	{	int fls = scn->sc->fluids.size();
		const auto& dfl = scn->data->fluids->fls;
		switch (skey)
		{
			//  ins
			case key(INSERT):  case key(KP_0):
			if (road && road->bHitTer)
			{
				FluidBox fb;  fb.name = newFluidName;
				fb.pos = road->posHit;	fb.rot = Vector3(0.f, 0.f, 0.f);
				fb.size = Vector3(50.f, 20.f, 50.f);  fb.tile = Vector2(0.01f, 0.01f);
				scn->sc->fluids.push_back(fb);
				scn->sc->UpdateFluidsId();
				iFlCur = scn->sc->fluids.size()-1;
				bRecreateFluids = true;
			}	break;
			default:  break;
		}
		if (fls > 0)
		{
			FluidBox& fb = scn->sc->fluids[iFlCur];
			switch (skey)
			{
				//  first, last
				case key(HOME):  case key(KP_7):
					iFlCur = 0;  UpdFluidBox();  break;
				case key(END):  case key(KP_1):
					iFlCur = fls-1;  UpdFluidBox();  break;

				//  prev,next
				case key(PAGEUP):  case key(KP_9):
					iFlCur = (iFlCur-1+fls)%fls;  UpdFluidBox();  break;
				case key(PAGEDOWN):	case key(KP_3):
					iFlCur = (iFlCur+1)%fls;	  UpdFluidBox();  break;

				//  del
				case key(DELETE):  case key(KP_PERIOD):
				case key(KP_5):
					if (fls == 1)	scn->sc->fluids.clear();
					else			scn->sc->fluids.erase(scn->sc->fluids.begin() + iFlCur);
					iFlCur = std::max(0, std::min(iFlCur, (int)scn->sc->fluids.size()-1));
					bRecreateFluids = true;
					break;

				//  prev,next type
				case key(9):  case key(MINUS):
				{
					fb.id = (fb.id-1 + dfl.size()) % dfl.size();
					fb.name = dfl[fb.id].name;  bRecreateFluids = true;
				}	break;
				case key(0):  case key(EQUALS):
				{
					fb.id = (fb.id+1) % dfl.size();
					fb.name = dfl[fb.id].name;  bRecreateFluids = true;
				}	break;

				//  quality
				case key(1):
					if (fb.hq > 0)  --fb.hq;  bRecreateFluids = true;  break;
				case key(2):
					if (fb.hq < 2)  ++fb.hq;  bRecreateFluids = true;  break;
				default:  break;
		}	}
	}

	//  💎 Collects  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	if (edMode == ED_Collects && edit)
	{	int cols = scn->sc->collects.size(), colAll = vColNames.size();
		SCollect* o = iColCur == -1 ? &colNew :
					((iColCur >= 0 && cols > 0 && iColCur < cols) ? &scn->sc->collects[iColCur] : 0);
		switch (skey)
		{
			case key(SPACE):
				iColCur = -1;  /*PickObject();  UpdObjPick();*/  break;
			
			//  ins
			case key(INSERT):	case key(KP_0):
			if (scn->road && scn->road->bHitTer)  // insert new
			{
				AddNewCol();
				iColCur = scn->sc->collects.size()-1;  //par? auto select inserted-
				UpdColPick();
			}	break;

			//  first, last
			case key(HOME):  case key(KP_7):
				iColCur = 0;  UpdColPick();  break;
			case key(END):  case key(KP_1):
				if (cols > 0)  iColCur = cols-1;  UpdColPick();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (cols > 0) {  iColCur = (iColCur-1+cols)%cols;  }  UpdColPick();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (cols > 0) {  iColCur = (iColCur+1)%cols;       }  UpdColPick();  break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (iColCur >= 0 && cols > 0)
				{	SCollect& o = scn->sc->collects[iColCur];
					mSceneMgr->destroyItem(o.it);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (cols == 1)	scn->sc->collects.clear();
					else			scn->sc->collects.erase(scn->sc->collects.begin() + iColCur);
					iColCur = std::min(iColCur, (int)scn->sc->collects.size()-1);
					UpdColPick();
				}	break;

			//  move,scale
			case key(1):
				if (!shift)  colEd = EO_Move;
				else if (o)
				{
					if (iColCur == -1)  // reset h
					{
						o->pos[2] = 0.f;
					}
					else if (road)  // move to ter
					{
						const Vector3& v = road->posHit;
						o->pos[0] = v.x;  o->pos[1] =-v.z;  o->pos[2] = v.y + colNew.pos[2];
						o->nd->setPosition(o->pos);  UpdColPick();
					}
				}	break;

			case key(3):
				if (!shift)  colEd = EO_Scale;
				else if (o)  // reset scale
				{
					o->scale = 1.f;
					o->nd->setScale(o->scale * Vector3::UNIT_SCALE);
				}	break;
			default:  break;
		}
	}

	//  📦 Objects  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
	if (edMode == ED_Objects && edit)
	{	int objs = scn->sc->objects.size(), objAll = vObjNames.size();
		switch (skey)
		{
			case key(SPACE):
				iObjCur = -1;  PickObject();  UpdObjPick();  break;
			
			//  ins
			case key(INSERT):	case key(KP_0):
			if (ctrl)  // copy selected
			{
				if (!vObjSel.empty())
				{
					vObjCopy.clear();
					for (auto it : vObjSel)
					{
						vObjCopy.push_back(scn->sc->objects[it]);
					}
					gui->Status("#{Copy}", 0.6,0.8,1.0);
			}	}
			else if (shift)  // paste copied
			{
				if (!vObjCopy.empty())
				{
					vObjSel.clear();  // unsel
					Object o = objNew;
					for (auto i : vObjCopy)
					{
						objNew = i;
						AddNewObj(false);
						vObjSel.insert(scn->sc->objects.size()-1);  // add it to sel
					}
					objNew = o;
					UpdObjSel();  UpdObjPick();
			}	}
			else
			if (scn->road && scn->road->bHitTer)  // insert new
			{
				AddNewObj();
				//iObjCur = scn->sc->objects.size()-1;  //par? auto select inserted-
				UpdObjPick();
			}	break;

			
			//  sel
			case key(BACKSPACE):
				if (alt)  // select all
					for (int i=0; i < objs; ++i)
						vObjSel.insert(i);
				else
				if (ctrl)  vObjSel.clear();  // unsel all
				else
				if (iObjCur > -1)
					if (vObjSel.find(iObjCur) == vObjSel.end())
						vObjSel.insert(iObjCur);  // add to sel
					else
						vObjSel.erase(iObjCur);  // unselect				

				UpdObjSel();
				break;

			//  prev,next type
			case key(9):  case key(MINUS):   SetObjNewType((iObjTNew-1 + objAll) % objAll);  break;
			case key(0):  case key(EQUALS):  SetObjNewType((iObjTNew+1) % objAll);  break;

			//  prev,next material
			case key(O):  case key(LEFTBRACKET):   NextObjMat(-1);  break;
			case key(P):  case key(RIGHTBRACKET):  NextObjMat( 1);  break;

			//  static
			case key(KP_ENTER):  case key(RETURN):  TogObjStatic();  break;
			
			default:  break;
		}
		::Object* o = iObjCur == -1 ? &objNew :
					((iObjCur >= 0 && objs > 0 && iObjCur < objs) ? &scn->sc->objects[iObjCur] : 0);
		switch (skey)
		{
			//  first, last
			case key(HOME):  case key(KP_7):
				iObjCur = 0;  UpdObjPick();  break;
			case key(END):  case key(KP_1):
				if (objs > 0)  iObjCur = objs-1;  UpdObjPick();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (objs > 0) {  iObjCur = (iObjCur-1+objs)%objs;  }  UpdObjPick();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (objs > 0) {  iObjCur = (iObjCur+1)%objs;       }  UpdObjPick();  break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (iObjCur >= 0 && objs > 0)
				{	::Object& o = scn->sc->objects[iObjCur];
					mSceneMgr->destroyItem(o.it);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (objs == 1)	scn->sc->objects.clear();
					else			scn->sc->objects.erase(scn->sc->objects.begin() + iObjCur);
					iObjCur = std::min(iObjCur, (int)scn->sc->objects.size()-1);
					UpdObjPick();
				}	break;

			//  move,rot,scale
			case key(1):
				if (!shift)  objEd = EO_Move;
				else if (o)
				{
					if (iObjCur == -1)  // reset h
					{
						o->pos[2] = 0.f;  o->SetFromBlt();  UpdObjPick();
					}
					else if (road)  // move to ter
					{
						const Vector3& v = road->posHit;
						o->pos[0] = v.x;  o->pos[1] =-v.z;  o->pos[2] = v.y + objNew.pos[2];
						o->SetFromBlt();  UpdObjPick();
					}
				}	break;

			case key(2):
				if (!shift)  objEd = EO_Rotate;
				else if (o)  // reset rot
				{
					o->rot = QUATERNION<float>(0,1,0,0);
					o->SetFromBlt();  UpdObjPick();
				}	break;

			case key(3):
				if (!shift)  objEd = EO_Scale;
				else if (o)  // reset scale
				{
					o->scale = Vector3::UNIT_SCALE * (ctrl ? 0.5f : 1.f);
					o->nd->setScale(o->scale);  UpdObjPick();
				}	break;
			default:  break;
		}
	}

	//  🔥 Emitters  : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : :
	if (edMode == ED_Particles && edit)
	{	int emts = scn->sc->emitters.size();
		switch (skey)
		{
			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (emts > 0) {  iEmtCur = (iEmtCur-1+emts)%emts;  }  UpdEmtBox();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (emts > 0) {  iEmtCur = (iEmtCur+1)%emts;       }  UpdEmtBox();  break;

			case key(1):  emtEd = EO_Move;    break;
			case key(2):  emtEd = EO_Rotate;  break;
			case key(3):  emtEd = EO_Scale;   break;

			//  prev,next type
			case key(9):  case key(MINUS):   SetEmtType(-1);  bRecreateEmitters = true;  break;
			case key(0):  case key(EQUALS):  SetEmtType( 1);  bRecreateEmitters = true;  break;

			//  static
			case key(KP_ENTER):  case key(RETURN):
				scn->sc->emitters[iEmtCur].stat = !scn->sc->emitters[iEmtCur].stat;
				bRecreateEmitters = true;  break;

			//  ins
			case key(INSERT):  case key(KP_0):
			if (road && road->bHitTer)
			{
				SEmitter em;  em.name = newEmtName;
				em.pos = road->posHit;  em.size = Vector3(2.f, 1.f, 2.f);  em.rate = 10.f;
				scn->sc->emitters.push_back(em);  iEmtCur = scn->sc->emitters.size()-1;
				bRecreateEmitters = true;
			}	break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (emts == 0)  break;
				scn->DestroyEmitters(false);
				if (emts == 1)	scn->sc->emitters.clear();
				else			scn->sc->emitters.erase(scn->sc->emitters.begin() + iEmtCur);
				iEmtCur = std::max(0, std::min(iEmtCur, (int)scn->sc->emitters.size()-1));
				bRecreateEmitters = true;
				break;
			default:  break;
		}
	}
}
