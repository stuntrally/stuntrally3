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
	{
		auto& fld = scn->sc->fluids;
		int flds = fld.size();
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
				fld.push_back(fb);
				scn->sc->UpdateFluidsId();
				iFlCur = fld.size()-1;
				bRecreateFluids = true;
			}	break;
			default:  break;
		}
		if (flds > 0)
		{
			FluidBox& fb = fld[iFlCur];
			switch (skey)
			{
				//  first, last
				case key(HOME):  case key(KP_7):
					iFlCur = 0;  UpdFluidBox();  break;
				case key(END):  case key(KP_1):
					iFlCur = flds-1;  UpdFluidBox();  break;

				//  prev,next
				case key(PAGEUP):  case key(KP_9):
					iFlCur = (iFlCur-1+flds)%flds;  UpdFluidBox();  break;
				case key(PAGEDOWN):	case key(KP_3):
					iFlCur = (iFlCur+1)%flds;	  UpdFluidBox();  break;

				//  del
				case key(DELETE):  case key(KP_PERIOD):
				case key(KP_5):
					if (flds == 1)	fld.clear();
					else			fld.erase(fld.begin() + iFlCur);
					iFlCur = std::max(0, std::min(iFlCur, (int)fld.size()-1));
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

	//  📦 Objects  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
	if (edMode == ED_Objects && edit)
	{
		auto& obj = scn->sc->objects;
		int objs = obj.size(), objAll = vObjNames.size();
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
						vObjCopy.push_back(obj[it]);
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
						vObjSel.insert(obj.size()-1);  // add it to sel
					}
					objNew = o;
					UpdObjSel();  UpdObjPick();
			}	}
			else
			if (scn->road && scn->road->bHitTer)  // insert new
			{
				AddNewObj();
				//iObjCur = obj.size()-1;  //par? auto select inserted-
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
					((iObjCur >= 0 && objs > 0 && iObjCur < objs) ? &obj[iObjCur] : 0);
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
				{	::Object& o = obj[iObjCur];
					mSceneMgr->destroyItem(o.it);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (objs == 1)	obj.clear();
					else			obj.erase(obj.begin() + iObjCur);
					iObjCur = std::min(iObjCur, (int)obj.size()-1);
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
	{
		auto emt = scn->sc->emitters;
		int emts = emt.size();
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
				emt[iEmtCur].stat = !emt[iEmtCur].stat;
				bRecreateEmitters = true;  break;

			//  ins
			case key(INSERT):  case key(KP_0):
			if (road && road->bHitTer)
			{
				SEmitter em;  em.name = newEmtName;
				em.pos = road->posHit;  em.size = Vector3(2.f, 1.f, 2.f);  em.rate = 10.f;
				emt.push_back(em);  iEmtCur = emt.size()-1;
				bRecreateEmitters = true;
			}	break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (emts == 0)  break;
				scn->DestroyEmitters(false);
				if (emts == 1)	emt.clear();
				else			emt.erase(emt.begin() + iEmtCur);
				iEmtCur = std::max(0, std::min(iEmtCur, (int)emt.size()-1));
				bRecreateEmitters = true;
				break;
			default:  break;
		}
	}

	//  💎 Collects  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	if (edMode == ED_Collects && edit)
	{
		auto& col = scn->sc->collects;
		int cols = col.size();

		SCollect* c = iColCur == -1 ? &colNew :
					((iColCur >= 0 && cols > 0 && iColCur < cols) ? &col[iColCur] : 0);
		switch (skey)
		{
			case key(SPACE):
				iColCur = -1;  PickCollect();  UpdColPick();  break;
			
			//  ins
			case key(INSERT):	case key(KP_0):
			if (scn->road && scn->road->bHitTer)  // insert new
			{
				AddNewCol();
				iColCur = col.size()-1;  //par? auto select inserted-
				UpdColPick();
			}	break;

			//  prev,next type
			case key(9):  case key(MINUS):   SetColType(-1);  break;
			case key(0):  case key(EQUALS):  SetColType( 1);  break;
			
			case key(O):  case key(LEFTBRACKET):   SetColGroup(-1);  break;
			case key(P):  case key(RIGHTBRACKET):  SetColGroup( 1);  break;
			

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
				{	SCollect& c = col[iColCur];
					DestroyCollect(iColCur);
					
					if (cols == 1)	col.clear();
					else			col.erase(col.begin() + iColCur);
					iColCur = std::min(iColCur, (int)col.size()-1);
					UpdColPick();
				}	break;

			//  move,scale
			case key(1):
				if (!shift)  colEd = EO_Move;
				else if (c)
				{
					if (iColCur == -1)  // reset h
					{
						c->pos[2] = 0.f;
					}
					else if (road)  // move to ter
					{
						const Vector3& v = road->posHit;
						c->pos[0] = v.x;  c->pos[1] =-v.z;  c->pos[2] = v.y + colNew.pos[2];
						c->nd->setPosition(c->pos);  UpdColPick();
					}
				}	break;

			case key(3):
				if (!shift)  colEd = EO_Scale;
				else if (c)  // reset scale
				{
					c->scale = 1.f;
					c->nd->setScale(c->scale * Vector3::UNIT_SCALE);
				}	break;
			default:  break;
		}
	}
}
