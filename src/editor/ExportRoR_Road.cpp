#include "pch.h"
#include "ExportRoR.h"

#include "enums.h"
#include "Def_Str.h"
#include "BaseApp.h"
#include "settings.h"
#include "paths.h"

#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "CData.h"
#include "TracksXml.h"
#include "PresetsXml.h"
#include "Axes.h"
#include "Road.h"

#include <Terra.h>
#include <OgreString.h>
#include <OgreImage2.h>
#include <OgreVector3.h>
#include <OgreException.h>

#include <exception>
#include <string>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//------------------------------------------------------------------------------------------------------------------------
//  🛣️ Road  points
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportRoad()
{
	Ogre::Timer ti;

	std::vector<string> chks;  int chk1st = 0;
	const bool roadtxt = !scn->roads.empty();
	const bool road = roadtxt && scn->roads[0]->getNumPoints() > 2;
	const float fLenDim = cfg->roadStepDist;  // points len density
	const float fAngDiv = cfg->roadAngDiv;  // points angle density

	hasRoad = 0;  hasRoadChks = 0;
	if (!road)
	{	gui->Exp(CGui::WARN, "No Road\n");
		return;
	}

	const auto& rd = scn->roads[0];

	//  if all points on ter, skip road, is on blendmap
	const int num = rd->getNumPoints();
	bool roadOnTer = 1;
	for (int i=0; i < num; ++i)
		if (!rd->getPoint(i).onTer)
			roadOnTer = 0;

	//  has some bridges  ------------------------
	if (roadOnTer)
		gui->Exp(CGui::NOTE, "Whole road on terrain");
	else
	{	hasRoad = 1;
		string roadFile = path + name + "-road.tobj";
		ofstream trd;
		trd.open(roadFile.c_str(), std::ios_base::out);

		trd << "//  position x,y,z   rotation rx,ry,rz,   width,   border width, border height,  type\n";
		bool begin = 0;  int iroads = 0;

		//  prepass
		SplineRoad::DataRoad dr(0, 1);
		dr.segs = num;
		dr.sMin = 0;  dr.sMax = dr.segs;
		rd->PrepassAngles(dr);

		int iila = 0;  // stat cnt
		const int num1 = rd->isLooped ? num + 1 : num;  // loop it
		for (int i=0; i < num1; ++i)
		{
			//  i0,1
			const int i0 = rd->getAdd(i,0), i1 = rd->getNext(i);
			const auto& p = rd->getPoint(i0), p1 = rd->getPoint(i1);
			bool bridge = !p.onTer || !p1.onTer;
			if (p.idMtr < 0)  bridge = 0;  // hidden

			// gui->Exp(CGui::TXT, "Road i0: "+toStr(i0+1)+
			// 	"  ter: "+toStr(p.onTer?1:0) + "  ter1: "+toStr(p1.onTer?1:0) + "  brd: "+toStr(bridge?1:0)  );

			if ( (i==0 || !begin) && bridge)
			{	begin = 1;  ++iroads;
				trd << "begin_procedural_roads\n";
				// gui->Exp(CGui::TXT, "Road begin");
			}

			if (begin && !bridge)
			{	begin = 0;
				trd << "end_procedural_roads\n";
				// gui->Exp(CGui::TXT, "Road end");
			}

			//  add points on bridge  ----------
			if (begin)
			{
				//  length steps  |
				// const int il = 2;  // const LQ
				float len = rd->GetSegLen(i0);
				//  more with bigger angle changes?
				float ayaw = fabs(p1.aYaw  - p.aYaw) / fAngDiv;  // par quality subdiv
				float arol = fabs(p1.aRoll - p.aRoll) / fAngDiv;
				int ilen = min(10, int(len / fLenDim));  // par max, len subdiv-

				string s = "Road seg: "+toStr(i)+"  len "+fToStr(len)+"  ilen "+toStr(ilen)+"\n";
				s += "Road roll "+fToStr(p.aRoll)+"  ayaw "+fToStr(ayaw) +" aroll "+fToStr(arol);
				
				//  road segment, divisions
				ayaw = min(ayaw, 4.f);  arol = min(arol, 3.f);  // par max
				const int il = 1 + ilen + ayaw + arol;  // var by length and angles
				const int ila = il + (p1.onTer ? 1 : 0);  // bridge will end, use all length points
				iila += ila;
				
				s += " div: "+toStr(il);
				// gui->Exp(CGui::TXT, s);  // test, gui chk?

				for (int l = 0; l < ila; ++l)
				{	//  pos
					float fl = l, fil = fl / il;
					Vector3 vP = rd->interpolate(i0, fil);
					Vector3 vP1 = rd->interpolate(i0, (fl+1.f) / il);
					Vector3 dir = vP1 - vP;  // along length

					const float width = rd->interpWidth(i0, fil);

					// if (p.onTer)  // ?
					// 	vP.y = scn->getTerH(vP.x, vP.z);
					vP.y += cfg->roadHadd;  // lower for entry  // -0.9f + rd->g_Height;

					//  rot y
					float yaw = TerUtil::GetAngle(dir.x, dir.z) *180.f/PI_d - 90.f;  // ok
					//  rot x
					float roll = p1.aRoll * fil + p.aRoll * (1.f - fil);  // interpolate
					// gui->Exp(CGui::TXT, "Road l: "+toStr(l)+" r "+fToStr(roll));

					//  write  ------
					//  pos
					// trd << half - vP.z << ", " << vP.y - hmin << ", " << vP.x + half << ",   ";
					trd << strPos(vP) << "  ";
					//  rot
					// trd << "0, " << yaw << ", 0,  ";  // only
					trd << roll << ", " << yaw << ", 0,  ";  // todo PR
					trd << width << ",   ";
					
					//  bridge
					//if (p.onTer)  trd << "0.5,  0.2,  flat\n";  else
					//  trd << "0.6,  1.0,  bridge\n";
					// bool low = p.onTer;
					bool low = p.onTer && l == 0 || p1.onTer && l == ila-1;
					trd << cfg->wallX << ",  " << (low ? 0.f : cfg->wallY) << 
						(cfg->roadCols ? ",  bridge\n" : ",  bridge_no_pillars\n");
					// 	flat - none   left, right, both - borders   bridge, bridge_no_pillars
			}	}
		}
		if (begin)
			trd << "end_procedural_roads\n";

		trd.close();
		gui->Exp(CGui::NOTE, "Roads: "+toStr(iroads)+"  total divs: "+toStr(iila));
	}


	//  get checkpoints
	//------------------------------------------------------------
	for (int i=0; i < num; ++i)
	{
		const int i0 = rd->getAdd(i,0);
		const auto& p = rd->getPoint(i0);
		if (p.chkR > 0.1f)
		{
			//Vector3 vP = p.pos;
			Vector3 vP = rd->interpolate(i, 0.f);
			Vector3 vP1 = rd->interpolate(i, 0.2f);  // par-
			Vector3 dir = vP1 - vP;  // along length
			float yaw = TerUtil::GetAngle(dir.x, dir.z) *180.f/PI_d;  // ok
			//  pos 
			string s = strPos(vP) + "  ";
			//  rot
			s += "0.0,  " + fToStr(yaw) + ",  0.0";
			
			if (p.chk1st)  chk1st = chks.size();
			chks.push_back(s);
	}	}


	//  Road script  checks
	//------------------------------------------------------------------------------------------------------------------------
	const int nchk = chks.size();
	gui->Exp(CGui::NOTE, "Road checks: " + toStr(nchk)+"  first: "+toStr(chk1st));
	if (nchk > 1)
	{
		hasRoadChks = 1;
		string asFile = path + name + ".as";
		ofstream as;
		as.open(asFile.c_str(), std::ios_base::out);

		as << "#include \"base.as\"\n";
		as << "#include \"races.as\"\n";
		as << "\n";
		as << "racesManager races();\n";
		as << "\n";
		as << "void createRaces()\n";
		as << "{\n";
		as << "    races.showCheckPointInfoWhenNotInRace = true;\n";
		as << "\n";
		as << "    races.setCallback(\"RaceFinish\", on_raceFinish);\n";
		as << "    \n";
		as << "    double[][] race_sr = \n";
		as << "    {\n";
	#if 0
		int i = 0;
		for (auto ch : chks)
		{
			bool last = i == nchk-1;  // no ,
			as << "			{ " << ch << (last ? " }\n" : " },\n");
			++i;
		}
	#else  // right order
		int i = chk1st, n = 0, nc = nchk-1;
		for (n=0; n < nchk; ++n)
		{
			bool last = n == nc;  // no ,
			as << "			{ " << chks[i] << (last ? " }\n" : " },\n");
			i += rd->iDir;
			if (i < 0)  i = nc;
			if (i > nc)  i = 0;
		}
	#endif
		as << "		};\n";
		as << "	int raceID = races.addRace(\"SR\", race_sr, " <<
			"races.LAPS_Unlimited, \"sr-checkpoint\", \"sr-start\");\n";
			// "races.LAPS_Unlimited, \"sr-checkpoint\");\n";  // default start/finish
			// "races.LAPS_Unlimited, \"sr-checkpoint\", \"sr-start\");\n";
			//checkpoint object     startline object        finishline object
			// "\"sr-checkpoint\", \"sr-checkpoint\", \"sr-checkpoint\", \"CryHam-1.0\");\n";
		// as << "	races.setVersion(raceID, \"CryHam-1.0\");\n";
		as << "}\n";
		as << "\n";
		as << "void main()\n";
		as << "{\n";
		as << "    createRaces();\n";
		as << "    //races.submitScore(false);\n";  // ?
		as << "}\n";
		as << "\n";
		as << "void eventCallback(int eventnum, int value)\n";
		as << "{\n";
		as << "    races.eventCallback(eventnum, value);\n";
		as << "}\n";
		as << "\n";
		as << "//  finishing a race\n";
		as << "void on_raceFinish(dictionary@ info)\n";
		as << "{\n";
		as << "    int raceID;\n";
		as << "    info.get(\"raceID\", raceID);\n";
		as << "    \n";
		as << "    float lapTime = game.getTime() - races.lapStartTime;\n";
		as << "    float raceTime = game.getTime() - races.raceStartTime;\n";
		as << "    \n";
    	//as << "    game.flashMessage(\"YES\", 3, -1);\n";  // nothing, meh
		// as << "    races.message(\"FINISH\", \"lightning.png\");\n";  // nothing
		//as << "    game.message(\"----------------------\", \"chart_bar.png\", 30000, true);\n";
		as << "    game.message(\"Lap time ++: \" + lapTime , \"chart_bar.png\", 30000, true);\n";
		as << "    \n";
		as << "    // Race specific\n";
		as << "    if (raceID == races.getRaceIDbyName(\"SR\"))\n";
		as << "    {\n";
		as << "        // Calculate the average speeds (0.25 mile = 0.40... kilometers)\n";
		as << "        float avg_mph = 0.25 / lapTime * 3600;\n";  // todo road length * 0.6
		as << "        float avg_kph = 0.402336 / lapTime * 3600;\n";
		as << "        \n";
		as << "        game.message(\"Average speed: \" + avg_mph + \" mph (\" + avg_kph + \" km/h).\", \"chart_bar.png\", 30000, true);\n";
		as << "    }\n";
		as << "}\n";

		as.close();
	}

	gui->Exp(CGui::INFO, "Road Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s\n");
}
