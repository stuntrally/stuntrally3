#include "pch.h"
#include "enums.h"
#include "Def_Str.h"
#include "CScene.h"
#include "Axes.h"
#include "CApp.h"
#include "Road.h"
#include "settings.h"
#include "SceneClasses.h"

#include <OgreVector3.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_Window.h>
using namespace Ogre;
using namespace MyGUI;


//---------------------------------------------------------------------------------------------------------------
///  🖱️ Mouse
//---------------------------------------------------------------------------------------------------------------
void App::EditMouse()
{
	if (!bEdit())  return;
	
	if (scn->road && edMode == ED_Road)
		MouseRoad();

	if (edMode == ED_Height)  ///  edit ter height val
	{
		if (mbRight)
		{	Real ym = -vNew.y * 4.f * moveMul;
			curBr().height += ym;
	}	}
	
	if (edMode == ED_Start /*&&	vStartPos.size() >= 4 && vStartRot.size() >= 4*/)
		MouseStart();

	if (edMode == ED_Particles && !scn->sc->emitters.empty() && pSet->bEmitters)
		MouseEmitters();
	
	if (edMode == ED_Fluids && !scn->sc->fluids.empty())
		MouseFluids();

	bool mbAny = mbLeft || mbMiddle || mbRight;
	if (edMode == ED_Objects && mbAny)
		MouseObjects();

	if (edMode == ED_Collects && !scn->sc->collects.empty())
		MouseCollects();

	if (edMode == ED_Fields && !scn->sc->fields.empty())
		MouseFields();
}


///  🛣️ edit Road  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseRoad()
{
	const Real fMove(0.2f), fRot(10.f);  //par speed
	SplineRoad* road = scn->road;
	const Real s = moveMul * pSet->move_speed;
	const Real d = road->iChosen == -1 ? 30.f * fMove:
		mCamera->getRealPosition().distance(road->getPos(road->iChosen)) * fMove;

	if (!alt)
	{
		if (mbLeft)    // move on xz
		{	Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
			road->Move((vNew.x * vx - vNew.y * vz) * d * s);
		}else
		if (mbRight)   // height
			road->Move(-vNew.y * Vector3::UNIT_Y * d * s);
		else
		if (mbMiddle)  // width
			road->AddWidth(vNew.x * 1.f * s);
	}else
	{	//  alt
		if (mbLeft)    // rot pitch
			road->AddYaw(   vNew.x * fRot * s, 0.f,false/*alt*/);
		if (mbRight)   // rot yaw
			road->AddRoll(  vNew.y *-fRot * s, 0.f,false/*alt*/);
	}
}


///  🏁 edit Start pos	 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseStart()
{
	if (MouseStart(scn->sc->startPos[iEnd], scn->sc->startRot[iEnd]))
		UpdStartPos();
}
bool App::MouseStart(MATHVECTOR <float,3>& pos, QUATERNION <float>& rot)
{
	const Real fMove(0.2f), fRot(2.f);  //par speed
	const Real d = mCamera->getPosition().distance(Axes::toOgre(pos)) * fMove;
	if (!alt)
	{
		if (mbLeft)
		{
			if (ctrl)  // set pos, from ter hit point
			{	if (scn->road && scn->road->bHitTer)
				{
					Vector3 v = scn->road->posHit;
					pos[0] = v.x;
					pos[1] =-v.z;
					pos[2] = v.y+0.6f;  //car h above
			}	}
			else  // move
			{
				Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
				Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
				Vector3 vm = (-vNew.y * vx - vNew.x * vz) * d * moveMul;
				pos[0] += vm.z;
				pos[1] += vm.x;
			}	return true;
		}
		else if (mbRight)
		{
			Real ym = -vNew.y * d * moveMul;
			pos[2] += ym;  return true;
		}
	}else  //  alt
	{
		typedef QUATERNION<float> Qf;  Qf qr;
		if (mbLeft)    // rot yaw
		{
			qr.Rotate(vNew.x * fRot, 0,0,1);
			Qf& q = rot;
			if (shift)  q = qr * q;  else  q = q * qr;
			return true;
		}
		else if (mbRight)   // rot pitch, roll
		{
			if (shift)  qr.Rotate(vNew.x * fRot, 1,0,0);
			else        qr.Rotate(vNew.y *-fRot, 0,1,0);
			Qf& q = rot;
			q = q * qr;  return true;
		}
		else if (mbMiddle)  // rot reset
		{
			qr.Rotate(0, 0,0,1);
			rot = qr;  return true;
		}
	}
	return false;
}


///  💧 edit Fluids . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseFluids()
{
	FluidBox& fb = scn->sc->fluids[iFlCur];
	const Real fMove(0.06f), scaleMul(0.3f);  //par speed
	const Real d = mCamera->getPosition().distance(fb.pos) * fMove * pSet->move_speed;
	if (!alt)
	{
		auto& fl = scn->refl.fluids[iFlCur];
		if (mbLeft)	// move on xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
			Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
			fb.pos += vm;

			for (int n=0; n < 2; ++n)
				if (fl.node[n]){  fl.node[n]->setPosition(fb.pos);  fl.node[n]->_getFullTransformUpdated();  }
			UpdFluidBox();
		}
		else if (mbRight)  // move y
		{
			Real ym = -vNew.y * d * moveMul;
			fb.pos.y += ym;
			
			for (int n=0; n < 2; ++n)
				if (fl.node[n]){  fl.node[n]->setPosition(fb.pos);  fl.node[n]->_getFullTransformUpdated();  }
			UpdFluidBox();
		}
		//  rot not supported  (bullet trigger isnt working, trees check  is a lot simpler)
		/*else if (mbMiddle)  // rot yaw
		{
			Real xm = vNew.x * fRot * moveMul;
			fb.rot.x += xm;
			vFlNd[iFlCur]->setOrientation(Quaternion(Degree(fb.rot.x),Vector3::UNIT_Y));
		}/**/
	}else
	{
		if (mbLeft)  // size xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();  vx.x = fabs(vx.x);  vx.z = fabs(vx.z);
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();  vz.x = fabs(vz.x);  vz.z = fabs(vz.z);
			Vector3 vm = (vNew.y * vz + vNew.x * vx) * d * moveMul * scaleMul;
			fb.size += vm;
			if (fb.size.x < 0.2f)  fb.size.x = 0.2f;
			if (fb.size.z < 0.2f)  fb.size.z = 0.2f;
			bRecreateFluids = true;  //
		}
		else if (mbRight)  // size y
		{
			float vm = -vNew.y * d * moveMul * scaleMul;
			fb.size.y += vm;
			if (fb.size.y < 0.2f)  fb.size.y = 0.2f;
			bRecreateFluids = true;  //
		}
	}
}


///  🔥 edit Emitters . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseEmitters()
{
	SEmitter& em = scn->sc->emitters[iEmtCur];
	const Real fMove(0.2f), fRot(20.f);  //par speed
	const Real s = pSet->move_speed;
	const Real d = mCamera->getPosition().distance(em.pos) * fMove * s;
	if (emtEd == EO_Move)
	{
		if (mbLeft)	// move on xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
			Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
			em.pos += vm;
			em.nd->setPosition(em.pos);  em.nd->_getFullTransformUpdated();  UpdEmtBox();
		}
		else if (mbRight)  // move y
		{
			Real ym = -vNew.y * d * moveMul;
			em.pos.y += ym;
			em.nd->setPosition(em.pos);  em.nd->_getFullTransformUpdated();  UpdEmtBox();
		}
	}else if (emtEd == EO_Rotate)
	{
		if (mbLeft)  // rot y
		{
			Real xm = vNew.x * fRot * moveMul;
			em.yaw += xm;
			em.UpdEmitter();
		}
		else if (mbRight)  // rot x
		{
			Real ym = vNew.x * fRot * moveMul;
			em.pitch += ym;
			em.UpdEmitter();
		}
	}else if(emtEd == EO_Scale)
	{
		if (mbLeft)  // size xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();  vx.x = fabs(vx.x);  vx.z = fabs(vx.z);
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();  vz.x = fabs(vz.x);  vz.z = fabs(vz.z);
			Vector3 vm = (vNew.y * vz + vNew.x * vx) * d * moveMul;
			em.size += vm;
			if (em.size.x < 0.f)  em.size.x = 0.f;
			if (em.size.z < 0.f)  em.size.z = 0.f;
			em.UpdEmitter();  UpdEmtBox();
		}
		else if (mbRight)  // size y
		{
			float vm = -vNew.y * d * moveMul;
			em.size.y += vm;
			if (em.size.y < 0.f)  em.size.y = 0.f;
			em.UpdEmitter();  UpdEmtBox();
		}
	}
	//else if(emtEd == EO_Rotate)
}


///  📦 edit Objects . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseObjects()
{
	const Real fMove(0.2f), fRot(20.f), fScale(0.01f);  //par speed
	bool upd = false, sel = !vObjSel.empty();
	const Real s = pSet->move_speed;

	//  rotate/scale selected
	Vector3 pos0{0,0,0};  Matrix3 m = Matrix3::IDENTITY;
	if (sel && objEd != EO_Move)
	{
		pos0 = GetObjPos0();
		if (objEd == EO_Rotate)
		{
			Real relA = -vNew.x * fRot * moveMul * s;
			Quaternion q;  q.FromAngleAxis(Degree(relA),
				mbLeft ? Vector3::UNIT_Y : (mbRight ? -Vector3::UNIT_Z : Vector3::UNIT_X));
			q.ToRotationMatrix(m);
	}	}

	//  selection, picked or new
	auto it = vObjSel.begin();
	int i = sel ? *it : iObjCur;
	while (i == -1 || (i >= 0 && i < scn->sc->objects.size()))
	{
		Object& o = i == -1 ? objNew : scn->sc->objects[i];
		bool upd1 = false;
		const Real d = mCamera->getPosition().distance(Axes::toOgre(o.pos)) * fMove * s;

		switch (objEd)
		{
			case EO_Move:
			{
				if (mbLeft && i != -1)  // move on xz
				{
					Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
					Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
					o.pos[0] += vm.x;  o.pos[1] -= vm.z;  // todo: for selection ..
					o.SetFromBlt();	 upd1 = true;
				}
				else if (mbRight)  // move y
				{
					Real ym = -vNew.y * d * moveMul;
					o.pos[2] += ym;
					o.SetFromBlt();	 upd1 = true;
				}
			}	break;

			case EO_Rotate:
			{
				Real xm = -vNew.x * fRot * moveMul *PI_d/180.f * s;
				Quaternion q(o.rot.w(),o.rot.x(),o.rot.y(),o.rot.z());
				Radian r = Radian(xm);  Quaternion qr;

				if (sel)  // rotate selected
				{
					Vector3 p(o.pos[0],o.pos[2],-o.pos[1]);  p = p-pos0;
					p = m * p;
					o.pos = MATHVECTOR<float,3>(p.x+pos0.x, -p.z-pos0.z, p.y+pos0.y);
				}

				qr.FromAngleAxis(r, mbLeft ? Vector3::UNIT_Z : (mbRight ? Vector3::UNIT_Y : Vector3::UNIT_X));
				if (sel || alt)  q = qr * q;  else  q = q * qr;
				o.rot = QUATERNION<float>(q.x,q.y,q.z,q.w);

				o.SetFromBlt();	 upd1 = true;
			}	break;

			case EO_Scale:
			{
				float vm = (vNew.y + vNew.x) * d * moveMul;
				float sc = 1.f + vm * fScale;  // todo: is fps dependent
		
				if (sel)  // scale selected
				{
					Vector3 p(o.pos[0],o.pos[2],-o.pos[1]);  p = p-pos0;
					p = p * sc + pos0;
					if (mbLeft)        o.pos = MATHVECTOR<float,3>(p.x, -p.z, p.y);
					else if (mbRight)  o.pos = MATHVECTOR<float,3>(o.pos[0], o.pos[1], p.y);
					else               o.pos = MATHVECTOR<float,3>(p.x, o.pos[1], o.pos[2]);  // todo: use rot for x,z ..
				}
				
				if (!o.dyn)  // static objs only
				{
					if (mbLeft)        o.scale *= sc;  // xyz
					else if (mbRight)  o.scale.y *= sc;  // y
					else               o.scale.z *= sc;  // z
					o.nd->setScale(o.scale);
				}
				o.SetFromBlt();	 upd1 = true;
			}	break;
		}
		if (upd1)
			upd = true;

		if (sel)
		{	++it;  // next sel
			if (it == vObjSel.end())  break;
			i = *it;
		}else  break;  // only 1
	}
	if (upd)
	{	UpdObjPick();
		UpdObjNewNode();
	}
}


///  💎 Collectibles . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseCollects()
{
	const Real fMove(0.2f), fRot(20.f), fScale(0.01f);  //par speed
	bool upd = false;
	const Real s = pSet->move_speed;

	int i = iColCur;  //  picked or new
	if (i < scn->sc->collects.size())
	{
		SCollect& c = i == -1 ? colNew : scn->sc->collects[i];
		const Real d = mCamera->getPosition().distance(c.pos) * fMove * s;

		switch (colEd)
		{
			case EO_Move:
			{
				if (mbLeft && i != -1)  // move on xz
				{
					Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
					Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
					c.pos.x += vm.x;  c.pos.z += vm.z;
					c.nd->setPosition(c.pos);  upd = true;
				}
				else if (mbRight)  // move y
				{
					Real ym = -vNew.y * d * moveMul;
					c.pos.y += ym;
					c.nd->setPosition(c.pos);  upd = true;
				}
			}	break;

			case EO_Scale:
			{
				float vm = (vNew.y + vNew.x) * d * moveMul;
				float sc = 1.f + vm * fScale;
	
				if (mbLeft)  c.scale *= sc;  // xyz
				c.nd->setScale(c.scale * Vector3::UNIT_SCALE);  upd = true;
			}	break;

			default:  break;
		}
	}
	if (upd)
	{	UpdColPick();
	// 	UpdObjNewNode();
	}
}


///  🎆 Fields . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseFields()
{
	const Real fMove(0.2f), fRot(20.f), fScale(0.01f);  //par speed
	bool upd = false;
	const Real s = pSet->move_speed;

	int i = iFldCur;  //  picked or new
	if (i < scn->sc->fields.size())
	{
		SField& f = i == -1 ? fldNew : scn->sc->fields[i];
		if (f.type == TF_Teleport && iEnd)
		{
			MouseStart(f.pos2, f.dir2);
			UpdTelepEnd();
			return;
		}

		const Real d = mCamera->getPosition().distance(f.pos) * fMove * s;

		switch (fldEd)
		{
			case EO_Move:
			{
				if (mbLeft && i != -1)  // move on xz
				{
					Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
					Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
					f.pos.x += vm.x;  f.pos.z += vm.z;
					f.nd->setPosition(f.pos);  upd = true;
				}
				else if (mbRight)  // move y
				{
					Real ym = -vNew.y * d * moveMul;
					f.pos.y += ym;
					f.nd->setPosition(f.pos);  upd = true;
				}
			}	break;

			case EO_Rotate:
			{
				if (mbLeft)  // rot y
				{
					Real xm = vNew.x * fRot * moveMul;
					f.yaw += xm;
					f.UpdEmitter();
				}
				else if (mbRight)  // rot x
				{
					Real ym = vNew.x * fRot * moveMul;
					f.pitch += ym;
					f.UpdEmitter();
				}
			}	break;

			case EO_Scale:
			{
				float vm = (vNew.y + vNew.x) * d * moveMul;
				float sc = 1.f + vm * fScale;
	
				if (mbLeft)        f.size *= sc;  // xyz
				else if (mbRight)  f.size.y *= sc;  // y
				else if (mbMiddle) f.size.z *= sc;  // z
				f.nd->setScale(f.size);  upd = true;
			}	break;

			default:  break;
		}
	}
	if (upd)
	{	UpdFldPick();
	// 	UpdObjNewNode();
	}
}
