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
#include "Axes.h"
#include "GuiCom.h"

#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreException.h>

#include <exception>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <time.h>
#include <filesystem>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//  gui events
//------------------------------------------------------------
void CGui::btnExport(WP)
{
	app->ror->ExportTrack();
}

void CGui::btnConvertTerrain(WP)
{
	app->ror->ConvertTerrainTex();
}
void CGui::btnConvertMat(WP)
{
	app->ror->ConvertMat();
}
void CGui::btnCreateOdef(WP)
{
	app->ror->CreateOdef();
}

void CGui::btnSaveSceneXml(WP)
{
	String dir = app->gcom->TrkDir();
	scn->sc->SaveXml(dir + "scene.xml");
}

//  settings
void CGui::editRoRPath(Ed ed)
{
	pSet->pathExportRoR = ed->getCaption();
}
void CGui::editOldSRPath(Ed ed)
{
	pSet->pathExportOldSR =  ed->getCaption();
}

//  road lay tex name
void CGui::slRoR_RoadLay(SV* sv)
{
	int i = app->scn->sc->rorCfg.roadTerTexLayer;
	auto& td = scn->sc->tds[0];
	txRoR_RoadLay->setCaption(
		i >= td.layers.size() ? "--" :
		td.layersAll[i].texFile);
}


//  🌟 ctor
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

	version = 1;  // increase..

	copyTerTex =0; // 1 data inside track
	copyVeget =0;  // 0 data in packs
	copyGrass =0;
	copyObjs =0;
}

//  ⛓️ utils
//------------------------------------------------------------

//  util convert SR pos to RoR pos
Ogre::String ExportRoR::strPos(const Ogre::Vector3& pos)
{
	stringstream ss;
	// todo?  add -?terZofs
	ss << half - pos.z << ", " << pos.y - hmin << ", " << pos.x + half << ", ";
	return ss.str();
}

//  util copy file
bool ExportRoR::CopyFile(const std::string& from, const std::string& to)
{
	try
	{
	#if 0  // leave
		if (!fs::exists(to.c_str()))
			fs::copy_file(from.c_str(), to.c_str());
	#else  // replace
		if (fs::exists(to.c_str()))
			fs::remove(to.c_str());
		fs::copy_file(from.c_str(), to.c_str());
	#endif
		String s = "Copied: " + from + "\n        to: " + to /*+ "\n  "*/;
		gui->Exp(CGui::DBG, s);
	}
	catch (exception ex)
	{
		String s = "Error copying file: " + from + "\n  to: " + to + "\n  " + ex.what();
		gui->Exp(CGui::WARN, s);
		return false;
	}
	return true;
}


//  ⚙️ Setup path, name, create dir
//------------------------------------------------------------
void ExportRoR::SetupPath()
{
	string& dirRoR = pSet->pathExportRoR;
	if (dirRoR.empty())
	{	gui->Exp(CGui::ERR, "Export path empty. Need to set export RoR path first.");
		return;
	}
	//  end with /
	if (!StringUtil::endsWith(dirRoR, "\\") &&
		!StringUtil::endsWith(dirRoR, "/"))
		dirRoR += "/";

	//  dir  track name
	name = pSet->gui.track;
	dirName = "+" + name;  // ^ for on top

	const string dirTrk = dirRoR + dirName;
	if (!PATHS::CreateDir(dirTrk))
	{	gui->Exp(CGui::ERR, "Can't create track dir: "+dirTrk);
		return;
	}
	path = dirTrk + "/";
}


//  Export current track for Rigs Of Rods
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportTrack()  // whole, full
{
	Ogre::Timer ti;
	
	//  Gui status
	gui->Status("RoR Export..", 1,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Export to RoR started..");


	SetupPath();

	ExportTerrain();

	ExportWaterSky();

	ExportObjects();

	ExportVeget();

	ExportRoad();

	
	//  just copy common .material to track dir  (done by convert in materials/)
	if (0)
	{
	string pathMtr = pSet->pathExportRoR + "materials/";
	std::vector<string> files{
		"objects_static.material",
		"rocks.material",
		"trees_ch.material",
		"trees.material",
		"trees_old.material"};
	for (auto& mtr : files)
		CopyFile(pathMtr + mtr, path + mtr);
	}
	// todo  and their textures? for used mesh materials ?


	//  🖼️ copy Preview  mini
	//------------------------------------------------------------
	// String pathPrv = PATHS::Tracks() + "/" + name + "/preview/";  // new SR
	string pathPrv = pSet->pathExportOldSR + "tracks/" + name + "/preview/";  // old SR
	string from = pathPrv + "view.jpg", to = path + name + "-mini.jpg";
	CopyFile(from, to);


	//  get Authors etc from tracks.ini
	//------------------------------------------------------------
	string authors = "CryHam", scenery;
	int difficulty = -1;
	int trkId = 0;  // N from ini  // todo Test* same-

	int id = scn->data->tracks->trkmap[pSet->gui.track];
	if (id > 0)
	{	const TrackInfo& ti = scn->data->tracks->trks[id-1];

		difficulty = ti.diff;
		scenery = ti.scenery;
		authors = ti.author=="CH" ? "CryHam" : ti.author;
		trkId = ti.n;
	}else
		gui->Exp(CGui::ERR, "Track not in tracks.ini, no guid id or authors set.");
	

	//------------------------------------------------------------------------------------------------------------------------
	//  🏞️ Track/map setup  save  .terrn2
	//------------------------------------------------------------------------------------------------------------------------
	string terrn2File = path + name + ".terrn2";
	ofstream trn;
	trn.open(terrn2File.c_str(), std::ios_base::out);

	trn << "[General]\n";
	trn << "Name = " + dirName + "\n";
	trn << "GeometryConfig = " + name + ".otc\n";
	trn << "\n";
	trn << "Water=" << water << "\n";
	trn << "WaterLine=" << Ywater << "\n";
	trn << "\n";
	trn << "AmbientColor = 1.0, 1.0, 1.0\n";  // unused-

	Vector3 st = Axes::toOgre(sc->startPos[0]);
	trn << "StartPosition = " << strPos(st) + "\n";
	//StartRotation // ? todo yaw
	trn << "\n";

	trn << "CaelumConfigFile = " + name + ".os\n";
	trn << "CaelumFogStart = " << sc->fogStart << "\n";  // fog
	trn << "CaelumFogEnd = " << sc->fogEnd << "\n";

	trn << "SandStormCubeMap = tracks/skyboxcol\n";  // sky meh-
	trn << "Gravity = " << -sc->gravity << "\n";
	trn << "\n";

	trn << "CategoryID = 129\n";
	trn << "Version = " << version << "\n";
	
	//  Guid
	//------------------------------------------------------------
	//  hash from tacrk name
	size_t hsh = std::hash<std::string>()(name);
	hsh &= 0xFFFFFFFFFFFFu;  // max 12 chars
	char hex[32];  sprintf(hex, "%012zX", hsh);
	string shex = hex;  //if (shex.length() > 12)  shex = shex.substr(0,12);
	gui->Exp(CGui::TXT, "Track id: " + toStr(trkId) + "  Name hash: " + shex);
	
	trn << "GUID = 11223344-5566-7788-" << fToStr(trkId,0,4,'0') <<"-"<< shex <<"\n";
	trn << "\n";

	//  if has groundmodel, define landuse file
	trn << "#TractionMap = landuse.cfg\n";  // todo  surfaces.cfg
	trn << "\n";


	//  Info text
	//------------------------------------------------------------
	trn << "[Authors]\n";
	trn << "Authors = " + authors + "  .\n";
	trn << "Conversion = Exported from Stunt Rally 3 Track Editor, version: " << SET_VER << "  .\n";

	time_t now = time(0);
	tm tn;  tn = *localtime(&now);
	char dtm[80];  strftime(dtm, sizeof(dtm), "%Y-%m-%d.%X", &tn);
	trn << "Date = " << dtm << "   .\n";

	//  extra info from SR3 track
	if (!scenery.empty())
		trn << "stat0 = " << "Scenery:  " << scenery << "   .\n";
	if (difficulty >= 0)
		trn << "stat1 = " << "Difficulty:  " << TR("#{Diff"+toStr(difficulty)+"}") << "   .\n";  // no TR? _en

	const bool roadtxt = !scn->roads.empty();
	if (roadtxt)
	{	auto& rd = scn->roads[0];
		auto len = rd->st.Length;  // road stats

		trn << "stat2 = " << "Length:  " <<  fToStr(len * 0.001f,2,4) << " km  /  " << fToStr(len * 0.000621371f,2,4) << " mi  .\n";
		trn << "stat3 = " << "Width average:  " << fToStr(rd->st.WidthAvg,1,3) << " m  .\n";
		trn << "stat4 = " << "Height range:  " << fToStr(rd->st.HeightDiff,0,2) << " m  .\n";
		trn << "stat5 = " << "Bridges:  " << fToStr(rd->st.OnTer,0,2) << " %  .\n";
		// trn << "stat7 = " << "bank angle avg: " << fToStr(rd->st.bankAvg,0,2) << "\n";
		trn << "stat6 = " << "Max banking angle:  " << fToStr(rd->st.bankMax,0,1) << "°  .\n";

		trn << "Description = "+rd->sTxtDescr+"   .\n";  // text
		trn << "drive_Advice = "+rd->sTxtAdvice+"   .\n";
	}
	trn << " \n";


	//------------------------------------------------------------
	trn << "[AssetPacks]\n";  // todo
	trn << "sr-checkpoint-v1.assetpack=\n";
	trn << "sr-materials-v1.assetpack=\n";

	trn << "sr-objects0ad-v1.assetpack=\n";  // todo check if needed
	trn << "sr-objects-v1.assetpack=\n";
	trn << "sr-objects2-v1.assetpack=\n";
	trn << "sr-objectsC-v1.assetpack=\n";
	// trn << "sr-obstacles-v1.assetpack=\n";

	trn << "sr-grass-v1.assetpack=\n";
	trn << "sr-rocks-v1.assetpack=\n";

	// trn << "sr-terrain-ext-v1.assetpack=\n";  // aln lava uni sur
	trn << "sr-terrain-v1.assetpack=\n";
	
	trn << "sr-trees-v1.assetpack=\n";
	trn << "sr-trees2-v1.assetpack=\n";
	trn << "sr-trees-old-v1.assetpack=\n";


	//------------------------------------------------------------
	trn << "[Objects]\n";
	if (hasRoad)
		trn << name+"-road.tobj=\n";
	if (hasVeget)
		trn << name+"-veget.tobj=\n";
	if (hasObjects)
		trn << name+"-obj.tobj=\n";
	trn << "\n";

	if (hasRoadChks)
	{	trn << "[Scripts]\n";
		trn << name + ".as=\n";	 //".terrn.as=\n";
	}

	trn.close();


	gui->Exp(CGui::INFO, "Export to RoR end.");
	gui->Exp(CGui::INFO, "Time Total: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}
