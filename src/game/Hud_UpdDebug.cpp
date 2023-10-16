#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
#include "TracksXml.h"
#include "CScene.h"

#include "game.h"
#include "quickprof.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
// #include "CGui.h"
// #include "SplitScreen.h"
#include "FollowCamera.h"
#include "CarModel.h"
// #include "MultiList2.h"
// #include "GraphView.h"

// #include <OgreWindow.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreManualObject2.h>
#include <OgreSceneManager.h>
#include <OgreColourValue.h>
using namespace Ogre;
using namespace MyGUI;


//-------------------------------------------------------------------------------------------------------------------
///  Debug infos
//-------------------------------------------------------------------------------------------------------------------
void CHud::UpdDebug(CAR* pCar, CarModel* pCarM)
{
	//  text clr upd  ===
	int clr = pSet->car_dbgtxtclr;
	static int oldTxtClr = -2;
	if (oldTxtClr != clr)
	{	oldTxtClr = clr;

		for (int i=0; i < 4; ++i)
		{	txDbgCar[i]->setTextColour(clr ? Colour(1,1,1) : Colour(0,0,0));
			txDbgCar[i]->setTextShadowColour(!clr ? Colour(0.5,0.5,0.5) : Colour(0,0,0));
	}	}


	//  car debug text  === === === ===
	static bool oldCarTxt = false;
	if (pCar && txDbgCar[0])
	{
		if (pSet->car_dbgtxt)
		{
			for (int i=0; i < 4; ++i)
			{
				std::stringstream ss;
				pCar->DebugPrint(ss, i==0, i==1, i==2, i==3);
				txDbgCar[i]->setCaption(ss.str());
			}
		}else
		if (pSet->car_dbgtxt != oldCarTxt)
		{
			for (int i=0; i < 4; ++i)
				txDbgCar[i]->setCaption("");
		}
	}
	oldCarTxt = pSet->car_dbgtxt;

	
	//  profiling times --------
	if (pSet->profilerTxt && txDbgProfTim)
	{
		PROFILER.endCycle();
		static int frame=0;  ++frame;

		if (frame > 10)  //par
		{	frame = 0;
			std::string sProf = PROFILER.getAvgSummary(quickprof::MILLISECONDS);
			//sProf = "PROF "+fToStr(1000.f/PROFILER.getAvgDuration(" frameSt",quickprof::MILLISECONDS), 2,4);
			txDbgProfTim->setCaption(sProf);
		}
		//if (newPosInfos.size() > 0)
		//ov[3].oU->setCaption("carm: " + toStr(ap->carModels.size()) + " newp: " + toStr((*newPosInfos.begin()).pos));
	}


#ifndef BT_NO_PROFILE
	//  bullet profiling text  --------
	static bool oldBltTxt = false;
	if (txDbgProfBlt)
	{
		if (pSet->bltProfilerTxt)
		{
			static int cc = 0;  ++cc;
			if (cc > 40)
			{	cc = 0;
				std::stringstream os;
				bltDumpAll(os);
				os << "aaaa";
				txDbgProfBlt->setCaption(os.str());
			}
		}
		else
		if (pSet->bltProfilerTxt != oldBltTxt)
			txDbgProfBlt->setCaption("");
	}
	oldBltTxt = pSet->bltProfilerTxt;
#endif // BT_NO_PROFILE


#if 0
	//  wheels slide, susp bars  --------
	if (pSet->car_dbgbars && pCar)
	{
		const Real xp = 80, yp = -530, ln = 20, y4 = 104;
		//const static char swh[4][6] = {"F^L<","F^R>","RvL<","RvR>"};
		for (int w=0; w < 4; ++w)
		if (ov[3-w].oL && ov[3-w].oR && ov[3-w].oS)
		{	
			float slide = /*-1.f*/0.f, sLong = 0.f, sLat = 0.f;
			float squeal = pCar->GetTireSquealAmount((WHEEL_POSITION)w, &slide, &sLong, &sLat);

			//MATHVECTOR<float,3> vwhVel = pCar->dynamics.GetWheelVelocity((WHEEL_POSITION)w);
			//float whVel = vwhVel.Magnitude() * 3.6f;

			/**  //  info
			static char ss[256];
			sprintf(ss, "%s %6.3f %6.3f  %6.3f %6.3f\n", swh[w],
				sLong/4.f, sLat/3.f, slide, squeal);
			ColourValue clr;  clr.setHSB( slide/20.f, 0.8f, 1.f );  //clr.a = min(0.f, slide/2.f);
			ovL[3-w]->setCaption(String(ss));
			ovL[3-w]->setColour(clr);
			//ovL[3-w]->setPosition(0.f, 230 + w*22);
			/**/

			//  bar meters |
			float susp = pCar->dynamics.GetSuspension(WHEEL_POSITION(w)).GetDisplacementPercent();
			sLong = fabs(sLong);
			float slng = sLong / sLong * powf(sLong, 0.3f);  // slide*20.f

			ov[3-w].oR->setPosition(slng * 14.f +xp, yp + w*ln);
			ov[3-w].oL->setPosition(sLat * 14.f +xp, yp + w*ln +y4);
			ov[3-w].oS->setPosition(susp * 70.f +xp, yp + w*ln -y4);
		}
		if (ov[4].oL)  ov[4].oL->setPosition(xp, yp + -20 +y4+3);
		if (ov[4].oS)  ov[4].oS->setPosition(xp + 70, yp + -20 -104-3);

		//ov[3-w].oR->setCaption("|");  ov[3-w].oR->setColour(ColourValue(0.6,1.0,0.7));
	}
#endif

	//  input values
	/*if (pCar && ap->pGame && ap->pGame->profilingmode)
	{	const std::vector<float>& inp = pCar->dynamics.inputsCopy;
	if (ov[2].oU && inp.size() == CARINPUT::ALL)
	{	sprintf(s, 
		" Throttle %5.2f\n Brake %5.2f\n Steer %5.2f\n"
		" Handbrake %5.2f\n Boost %5.2f\n Flip %5.2f\n"
		,inp[CARINPUT::THROTTLE], inp[CARINPUT::BRAKE], -inp[CARINPUT::STEER_LEFT]+inp[CARINPUT::STEER_RIGHT]
		,inp[CARINPUT::HANDBRAKE],inp[CARINPUT::BOOST], inp[CARINPUT::FLIP] );
		ov[2].oU->setCaption(String(s));
	}	}/**/


	//  wheels ter mtr, surfaces info  ---------
	if (pSet->car_dbgsurf && pCar)
	{
		String ss = pCarM->txtDbgSurf;

		/*ss += "\n";
		for (int i=0; i < ap->pGame->track.tracksurfaces.size(); ++i)
			ss += String(ap->pGame->track.tracksurfaces[i].name.c_str()) + "\n";/**/

		if (txDbgSurf)
			txDbgSurf->setCaption(ss);
	}
	

	///  tire vis circles  + + + +
	if (pCar && hrTireVis[0] && pSet->car_tirevis)
	{
		const Real z = 6000.f / pSet->tc_r, zy = pSet->tc_xr,
			m_z = 2.f * z;  // scale, max factor
		const int na = 32;  // circle quality
		const Real ad = 2.f*PI_d/na, u = 0.02f;  // u line thickness
		const ColourValue cb(0.8,0.8,0.8),cl(0.2,1,0.2),cr(0.9,0.4,0),cc(1,1,0);
		
		const CARDYNAMICS& cd = pCar->dynamics;
		const int numWh = std::min(cd.numWheels, MAX_TireVis);
		for (int i=0; i < numWh; ++i)
		{
			const CARWHEEL::SlideSlip& t = cd.wheel[i].slips;
			float d = cd.wheel_contact[i].GetDepth() - 2*cd.wheel[i].GetRadius();
			bool off = !(d > -0.1f && d <= -0.01f);  // not in air
			//bool on = cd.wheel_contact[i].GetColObj();
			
			auto* m = hrTireVis[i];
			m->begin();  // counts need updating in Create too
			//  back +  2
			m->position(-1,0,0);  m->color(cb);
			m->position( 1,0,0);  m->color(cb);
			m->position(0,-1,0);  m->color(cb);
			m->position(0, 1,0);  m->color(cb);
			
			//  tire, before combine  3*3
			Real lx = off ? 0.f : -t.preFy/z*zy,  ly = off ? 0.f : t.preFx/z;
			for (int y=-1; y<=1; ++y)
			for (int x=-1; x<=1; ++x)  {
				m->position(0  +x*u, 0  +y*u, 0);  m->color(cr);
				m->position(lx +x*u, ly +y*u, 0);  m->color(cr);  }

			//  tire line /  5*5
			lx = off ? 0.f : -t.Fy/z*zy;  ly = off ? 0.f : t.Fx/z;
			for (int y=-2; y<=2; ++y)
			for (int x=-2; x<=2; ++x)  {
				m->position(0  +x*u, 0  +y*u, 0);  m->color(cl);
				m->position(lx +x*u, ly +y*u, 0);  m->color(cl);  }

			//  max circle o  33
			Real rx = off || t.Fym > m_z ? 0.f : t.Fym/z,
			     ry = off || t.Fxm > m_z ? 0.f : t.Fxm/z, a = 0.f;
			Real ox=0, oy=0, px=0, py=0;
			
			for (int n=0; n <= na; ++n)
			{
				px = rx*cosf(a)*zy;  py =-ry*sinf(a);
				if (n > 0)  {
					m->position(ox, oy, 0);  m->color(cc);
					m->position(px, py, 0);    m->color(cc);  }
				a += ad;  ox = px;  oy = py;
			}
			m->end();
		}
	}
}
