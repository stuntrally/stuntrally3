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

#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreException.h>

#include <exception>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <sstream>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//  gui events
//------------------------------------------------------------
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

void CGui::btnConvertTerrain(WP)
{
	app->ror->ConvertTerrainTex();
}
void CGui::btnConvertMat(WP)
{
	app->ror->ConvertMat();
}


//  ctor
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

//  utils
//------------------------------------------------------------

//  util convert SR pos to RoR pos
Ogre::String ExportRoR::strPos(Ogre::Vector3 pos)
{
	stringstream ss;
	// todo?  add -?terZofs
	ss << half - pos.z << ", " << pos.y - hmin << ", " << pos.x + half << ", ";
	return ss.str();
}

//  util copy file
bool ExportRoR::CopyFile(std::string from, std::string to)
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
		String s = "Copied file: " + from + "\n  to: " + to + "\n  ";
		gui->Exp(CGui::TXT, s);
	}
	catch (exception ex)
	{
		String s = "Error copying file: " + from + "\n  to: " + to + "\n  " + ex.what();
		gui->Exp(CGui::WARN, s);
		return false;
	}
	return true;
}


//  setup path, name, create dir
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
	dirName = "^" + name;  // ^ for on top

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


	//  🖼️ copy Preview  mini
	//------------------------------------------------------------
	String pathPrv = PATHS::Tracks() + "/" + name + "/preview/";
	string from = pathPrv + "view.jpg", to = path + name + "-mini.jpg";
	CopyFile(from, to);


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
	//  ror = sr
	//  0, y, 0        = -470, y, 460
	//  959, 340 y, 950 = 487, y, -472
	Vector3 st = Axes::toOgre(sc->startPos[0]);
	trn << "StartPosition = " << strPos(st) + "\n";
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


//  other
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ConvertTerrainTex()
{

}

//  Convert .mat
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ConvertMat()
{
	Ogre::Timer ti;
	
	//  Gui status
	gui->Status("Convert materials", 0.7,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Started Convert materials..");

	string pathMat = pSet->pathExportOldSR + "/materials/scene/";
	std::vector<string> matFiles{
		"objects_dynamic.mat", "objects_static2.mat", "objects_static.mat",
		"rocks.mat",
		"trees_ch.mat",	"trees.mat", "trees_old.mat"};


	SetupPath();

	// auto& fmat = mats[2];  // test one
	for (auto& fmat : matFiles)  // all
	{
		// string mtrFile = path + fmat + "erial";
		string mtrFile = pSet->pathExportRoR + "materials/" + fmat + "erial";
		ofstream mat;
		mat.open(mtrFile.c_str(), std::ios_base::out);

		string matFile = pathMat + fmat;
		ifstream fi(matFile.c_str());
		bool first = 1;

		//  mat params to get
		string material, diffTex, normTex,
			alphaReject, cull_hardware,
			ambient, diffuse, specular;

		gui->Exp(CGui::NOTE, String("\nread .mat: " + matFile));
		gui->Exp(CGui::NOTE, String("save to: " + mtrFile));
		
		//  material in .material
		//------------------------------------------------------------
		auto Write = [&]()
		{
			/**/gui->Exp(CGui::TXT, material +"  diff: "+diffTex+"  norm: "+normTex+
				"  alpha: "+alphaReject+"  cull: "+cull_hardware );/**/

			mat << "material " << material << "\n";
			mat << "{\n";
			mat << "	technique\n";
			mat << "	{\n";
			mat << "		pass\n";
			mat << "		{\n";

			if (!ambient.empty())
				mat << "			ambient "<< ambient <<"\n";
			if (!diffuse.empty())
				mat << "			diffuse "<< diffuse <<"\n";
			if (!specular.empty())
				mat << "			specular "<< specular <<"\n";

			if (!cull_hardware.empty())
			{	mat << "			cull_hardware "<< cull_hardware <<"\n";
				mat << "			cull_software "<< cull_hardware <<"\n";  //?
			}
			bool alpha = !alphaReject.empty();
			if (alpha)
				mat << "			alpha_rejection "<< alphaReject << "\n";

			if (!diffTex.empty())
			{
				mat << "			texture_unit\n";
				mat << "			{\n";
				mat << "				texture	" << diffTex << "\n";
				if (alpha)
					mat << "				tex_address_mode clamp\n";  // for grass, only?
				mat << "			}\n";
			}
			mat << "		}\n";
			mat << "	}\n";
			mat << "}\n";
			mat << "\n";
		};

		//  read .mat file
		//  todo won't work for inherited materials ..?
		//------------------------------------------------------------
		string s;
		while (getline(fi,s))
		{
			string w = "material";
			auto it = s.find(w);
			if (it != string::npos)
			{
				if (!first)  // write mtr
					Write();

				//  set name and reset rest  -----
				material = s.substr(it + w.length() + 1);
				material = StringUtil::replaceAll(material, "{	parent", ":");  // trees_ch

				diffTex.clear();  normTex.clear();
				alphaReject.clear();  cull_hardware.clear();
				ambient.clear();  diffuse.clear();  specular.clear();

				first = 0;
			}

			//  textures
			w = "diffuseMap";  it = s.find(w);  bool dtex = it != string::npos;
			if (dtex){  diffTex = s.substr(it + w.length() + 1);  /* gui->Exp(CGui::TXT, material +" d:"+diffuse);*/  }
			
			w = "normalMap";  it = s.find(w);
			if (it != string::npos)  normTex = s.substr(it + w.length() + 1);
			
			//  alpha
			w = "alpha_rejection";  it = s.find(w);
			if (it != string::npos)  alphaReject = s.substr(it + w.length() + 1);

			w = "cull_hardware";  it = s.find(w);
			if (it != string::npos)  cull_hardware = s.substr(it + w.length() + 1);

			//  colors
			w = "ambient";  it = s.find(w);
			if (it != string::npos)  ambient = s.substr(it + w.length() + 1);
			if (!dtex)
			{
			w = "diffuse";  it = s.find(w);
			if (it != string::npos)  diffuse = s.substr(it + w.length() + 1);
			}
			w = "specular";  it = s.find(w);
			if (it != string::npos)  specular = s.substr(it + w.length() + 1);

			// todo?
			// tree_wind true
			// receives_shadows false
			// transparent true
		}
		fi.close();

		if (!first)  // write last
			Write();

		mat.close();
	}

	gui->Exp(CGui::INFO, "Ended Convert materials");
	gui->Exp(CGui::INFO, "Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}
