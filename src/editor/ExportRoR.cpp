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
#include "TracksXml.h"

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


//  gui events
void CGui::editRoRPath(Ed ed)
{
	pSet->pathExportRoR = ed->getCaption();
}
void CGui::editOldSRPath(Ed ed)
{
	pSet->pathExportOldSR =  ed->getCaption();
}
void CGui::btnExport(WP)
{
	app->ror->ExportTrack();
}

ExportRoR::ExportRoR(App* app1)
{
	app = app1;
	gui = app->gui;
	pSet = app->pSet;

	scn = app->scn;  // auto set
	sc = scn->sc;
	cfg = &sc->rorCfg;

	data = scn->data;
	pre = data->pre;
}


//  Export current track for Rigs Of Rods
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportTrack()  // whole, full
{
	Ogre::Timer ti;
	
	//  Gui status
	gui->Status("RoR Export..", 1,0.5,1);
	gui->edExport->setCaption("");
	gui->Exp(CGui::INFO, "Export to RoR started..");


	//  dir  export to RoR content or mods path
	string& dirRoR = pSet->pathExportRoR;
	if (dirRoR.empty())
	{	gui->Exp(CGui::ERR, "Export path empty. Need to set export RoR path first.");
		return;
	}
	//  end with /
	if (!StringUtil::endsWith(dirRoR, "\\") &&
		!StringUtil::endsWith(dirRoR, "/"))
		dirRoR += "/";

	//  subdir  track name
	const string trk = "^" + pSet->gui.track;  // ^ for on top
	name = pSet->gui.track;  // main

	const string dirTrk = dirRoR + trk;
	if (!PATHS::CreateDir(dirTrk))
	{	gui->Exp(CGui::ERR, "Can't create track dir: "+dirTrk);
		return;
	}
	path = dirTrk + "/";  // main


	//  Export all
	//------------------------------------------------------------

	ExportTerrain();


	//------------------------------------------------------------------------------------------------------------------------
	//  🖼️ copy Preview  mini
	//------------------------------------------------------------
	String pathPrv = PATHS::Tracks() + "/" + name + "/preview/";
	try
	{	string from = pathPrv + "view.jpg", to = path + name + "-mini.jpg";
		if (!fs::exists(to.c_str()))
			fs::copy_file(from.c_str(), to.c_str());
	}
	catch (exception ex)
	{
		gui->Exp(CGui::WARN, string("Exception copy preview: ") + ex.what());
	}

	//  get Authors from tracks.ini
	//------------------------------------------------------------
	string authors = "CryHam";
	int trkId = 0;  // N from ini  // todo Test* same-
	int id = scn->data->tracks->trkmap[pSet->gui.track];
	if (id > 0)
	{	const TrackInfo& ti = scn->data->tracks->trks[id-1];

		authors = ti.author=="CH" ? "CryHam" : ti.author;
		trkId = ti.n;
	}else
		gui->Exp(CGui::ERR, "Track not in tracks.ini, no guid id or authors set.");
	
	ExportWaterSky();

	ExportObjects();

	ExportVeget();

	ExportRoad();


	//------------------------------------------------------------------------------------------------------------------------
	//  🏞️ Track/map setup  save  .terrn2
	//------------------------------------------------------------------------------------------------------------------------
	string terrn2File = path + name + ".terrn2";
	ofstream trn;
	trn.open(terrn2File.c_str(), std::ios_base::out);

	trn << "[General]\n";
	trn << "Name = "+trk+"\n";
	trn << "GeometryConfig = " + name + ".otc\n";
	trn << "\n";
	trn << "Water=" << water << "\n";
	trn << "WaterLine=" << Ywater << "\n";
	trn << "\n";
	trn << "AmbientColor = 1.0, 1.0, 1.0\n";  // unused-
	//  ror = sr
	//  0, y, 0        = -470, y, 460
	//  959, 340 y, 950 = 487, y, -472
	Vector3 st = Axes::toOgre(sc->startPos[0]);
	trn << "StartPosition = " << fToStr(half - st.z)+", "+fToStr(st.y - hmin)+", "+fToStr(st.x + half)+"\n";
	trn << "\n";

	trn << "CaelumConfigFile = " + name + ".os\n";
	trn << "SandStormCubeMap = tracks/skyboxcol\n";  // sky meh-
	trn << "Gravity = " << -sc->gravity << "\n";
	trn << "\n";

	trn << "CategoryID = 129\n";
	trn << "Version = " << 1 << "\n";  // todo global manual +
	
	//  guid  ----
	//  hash from tacrk name
	size_t hsh = std::hash<std::string>()(name);
	hsh = max(0xFFFFFFFFFFFFu, hsh);  // max 12 chars
	char hex[32];
	sprintf(hex, "%012X", hsh);
	gui->Exp(CGui::INFO, "Name hash: " + string(hex));
	
	trn << "GUID = 11223344-5566-7788-" << fToStr(trkId,0,4,'0') <<"-"<< hex <<"\n";
	trn << "\n";

	//  if has groundmodel, define landuse file
	trn << "#TractionMap = landuse.cfg\n";  // todo  surfaces.cfg
	trn << "\n";

	trn << "[Authors]\n";
	trn << "Authors = " + authors + "  .\n";
	trn << "Conversion = Exported from Stunt Rally 3 Track Editor, version: " << SET_VER << "  .\n";

	const bool roadtxt = !scn->roads.empty();
	if (roadtxt)
	{	auto& rd = scn->roads[0];  // extra info from SR3 track
		auto len = rd->st.Length;  // road stats

		trn << "stat1 = " << "Length: " <<  fToStr(len * 0.001f,2,4) << " km  /  " << fToStr(len * 0.000621371f,2,4) << " mi  .\n";
		trn << "stat2 = " << "Width average: " << fToStr(rd->st.WidthAvg,1,3) << " m  .\n";
		trn << "stat3 = " << "Height range: " << fToStr(rd->st.HeightDiff,0,3) << " m  .\n";
		trn << "stat4 = " << "Bridges: " << fToStr(rd->st.OnTer,0,3) << " %  .\n";
		// trn << "stat5 = " << "bank angle avg: " << fToStr(rd->st.bankAvg,0,2) << "\n";
		trn << "stat5 = " << "Max banking angle: " << fToStr(rd->st.bankMax,0,2) << "'  .\n";

		trn << "Description = "+rd->sTxtDescr+"   .\n";  // text
		trn << "drive_Advice = "+rd->sTxtAdvice+"   .\n";
	}
	trn << " \n";

	trn << "[Objects]\n";
	if (hasRoad)
		trn << name+"-road.tobj=\n";
	if (hasVeget)
		trn << name+"-veget.tobj=\n";
	if (hasObjects)
		trn << name+"-obj.tobj=\n";
	trn << "\n";

	trn << "[Scripts]\n";
	trn << name + ".as=\n";
	//trn << name+".terrn.as=\n";

	trn.close();


	gui->Exp(CGui::INFO, "Export to RoR end.");
	gui->Exp(CGui::INFO, "Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}
