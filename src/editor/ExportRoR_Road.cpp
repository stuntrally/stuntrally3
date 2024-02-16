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

	std::vector<string> chks;
	const bool roadtxt = !scn->roads.empty();
	const bool road = roadtxt && scn->roads[0]->getNumPoints() > 2;
	hasRoad = 0;
	if (road)
	{	const auto& rd = scn->roads[0];

		//  if all points on ter, skip road, is on blendmap
		bool roadOnTer = 1;
		for (int i=0; i < rd->getNumPoints(); ++i)
			if (!rd->getPoint(i).onTer)
				roadOnTer = 0;

		//  has some bridges  ------------------------
		if (!roadOnTer)
		{	hasRoad = 1;
			string roadFile = path + name + "-road.tobj";
			ofstream trd;
			trd.open(roadFile.c_str(), std::ios_base::out);

			trd << "//  position x,y,z   rotation rx,ry,rz,   width,   border width, border height,  type\n";
			bool begin = 0;  int iroads = 0;

			for (int i=0; i < rd->getNumPoints() + 1; ++i)  // loop it
			{
				//  i0,1
				const int i0 = rd->getAdd(i,0), i1 = rd->getNext(i);
				const auto& p = rd->getPoint(i0), p1 = rd->getPoint(i1);
				bool bridge = !p.onTer || !p1.onTer;

				if ( (i==0 || !begin) && bridge)
				{	begin = 1;  ++iroads;
					trd << "begin_procedural_roads\n";
					// gui->Exp(CGui::TXT, "Road begin");
				}

				//  length steps  |
				Real len = rd->GetSegLen(i0);
				const float fLenDim = cfg->roadStepDist;  // par ! quality, points density

				// gui->Exp(CGui::TXT, "Road i0: "+toStr(i0)+"  ter: "+toStr(p.onTer?1:0) +"  l "+fToStr(len) );

				//  add points on bridge  ----------
				if (begin)
				{
					//  len steps
					// const int il = 2;  // const LQ
					const int il = 1 + (len / fLenDim);  // var, by dist
					const int ila = il + (!bridge ? 1 : 0);  // will end, all len
					// gui->Exp(CGui::TXT, "Road il: "+toStr(il) );
					
					for (int l = 0; l < ila; ++l)
					{	//  pos
						float fl = l;
						Vector3 vP = rd->interpolate(i0, fl / il);
						Vector3 vP1 = rd->interpolate(i0, (fl+1.f) / il);
						Vector3 dir = vP1 - vP;  // along length

						// float width = p.width + 2.f;  //-
						const float width = rd->interpWidth(i0, fl / il);

						// if (p.onTer)  // ?
						// 	vP.y = scn->getTerH(vP.x, vP.z);
						vP.y += cfg->roadHadd;  // lower for entry  // -0.9f + rd->g_Height;

						//  rot y
						float yaw = TerUtil::GetAngle(dir.x, dir.z) *180.f/PI_d - 45.f - 15.f;  // par ? 45

						//  write  ------
						//  pos
						// trd << half - vP.z << ", " << vP.y - hmin << ", " << vP.x + half << ",   ";
						trd << strPos(vP) << "  ";
						//  rot
						trd << "0, " << yaw << ", 0,  ";  // todo  p.aRoll
						trd << width << ",   ";
						
						//  bridge
						/*if (p.onTer)
							trd << "0.5,  0.2,  flat\n";
						else*/
						/*if (p.cols)  // too much, bad
							trd << "0.6,  1.0,  bridge\n";
						else*/
							// trd << "0.6,  1.0,  bridge_no_pillars\n";
							trd << cfg->wallX << ",  " << (p.onTer ? 0.f : cfg->wallY) << 
								(cfg->roadCols ? ",  bridge\n" : ",  bridge_no_pillars\n");
				}	}

	
				if (begin && !bridge)
				{	begin = 0;
					trd << "end_procedural_roads\n";
					// gui->Exp(CGui::TXT, "Road end");
				}
			}
			if (begin)
				trd << "end_procedural_roads\n";

			/*	0,0,0,         0,0,0,    10.0,  0,            0,             flat
				0,0,0,         0,0,0,    10.0,  0.25,         1.0,           both
				flat - none
				left, right, both - borders
				bridge, bridge_no_pillars
			*/
			trd.close();
			gui->Exp(CGui::TXT, "Roads: "+toStr(iroads));
		}

		//  get checkpoints
		for (int i=0; i < rd->getNumPoints() + 1; ++i)  // loop it
		{
			const auto& p = rd->getPoint(i);
			if (p.chkR > 0.1f)
			{
				// Vector3 vP = p.pos;
				Vector3 vP = rd->interpolate(i, 0.f);
				Vector3 vP1 = rd->interpolate(i, 0.2f);  // par-
				Vector3 dir = vP1 - vP;  // along length
				float yaw = TerUtil::GetAngle(dir.x, dir.z) *180.f/PI_d - 45.f - 15.f;  // par ? 45
				// pos 
				string s = strPos(vP) + "  ";
				// rot
				s += "0.0,  " + fToStr(yaw) + ",  0.0";
				chks.push_back(s);
		}	}
	}

	//  Road script  checks
	//------------------------------------------------------------
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
	as << "	//races.showCheckPointInfoWhenNotInRace = true;\n";
	as << "\n";
	as << "	races.setCallback(\"RaceFinish\", on_raceFinish);\n";
	as << "	\n";
	as << "	double[][] race_autocross = \n";
	as << "		{\n";
#if 1
	int i = 0, n = chks.size();
	for (auto ch : chks)
	{
		bool last = i == n-1;  // no ,
		as << "			{ " << ch << (last ? " }\n" : " },\n");
		++i;
	}
#else
	as << "			{916.0,     9.0,   454.0,     0.0,     0,  0.0},\n";
	as << "			{871.0,     9.0,   475.0,     0.0,   180,  0.0},\n";
	as << "			{897.0,     9.0,   524.0,     0.0,  -120,  0.0}\n";
#endif
	as << "		};\n";
	as << "	int raceID = races.addRace(\"Autocross\", race_autocross, races.LAPS_Unlimited, \"31-checkpoint\", \"31-checkpoint\", \"31-checkpoint\", \"CryHam-1.0\");\n";
	as << "	\n";
	as << "	races.setVersion(raceID, \"CryHam-1.0\");\n";
	as << "	races.finalize(raceID);\n";
	as << "}\n";
	as << "\n";
	as << "void main()\n";
	as << "{\n";
	as << "	createRaces();\n";
	as << "	//races.submitScore(false);\n";
	as << "}\n";
	as << "\n";
	as << "void eventCallback(int eventnum, int value)\n";
	as << "{\n";
	as << "	// required for the races!\n";
	as << "	races.eventCallback(eventnum, value);\n";
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
	as << "    // Race specific stuff\n";
	as << "    if(raceID == races.getRaceIDbyName(\"Autocross\"))\n";
	as << "    {\n";
	as << "        // Calculate the average speeds (0.25 mile = 0.40... kilometers)\n";
	as << "        float averageSpeed_mph = 0.25 / lapTime * 3600;\n";  // todo road length * 0.6
	as << "        float averageSpeed_kph = 0.402336 / lapTime * 3600;\n";
	as << "        \n";
	as << "        game.message(\"Average speed: \" + averageSpeed_mph + \" mph (\" + averageSpeed_kph + \" km/h).\", \"chart_bar.png\", 30000, true);\n";
	as << "    }\n";
	as << "}  \n";
	as << "\n";

	as.close();

	gui->Exp(CGui::INFO, "Road Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}
