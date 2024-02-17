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


//  Tools other
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ConvertTerrainTex()
{
	Ogre::Timer ti;

	//  Gui status
	gui->Status("Create .odef", 0.7,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Create .odef for all .mesh ..");

	SetupPath();

	std::vector<string> dirs{
		"objects", "objects2", "objectsC", "objects0", "obstacles",
		"trees", "trees2", "trees-old", "rocks", "rockshex"};

	int iodef = 0;
	for (auto& dir : dirs)
	{
		string path1 = pSet->pathExportRoR + dir;
		gui->Exp(CGui::NOTE, "Listing: " + path1);

		strlist files;
		PATHS::DirList(path1, files);

		for (auto mesh : files)
		if (StringUtil::endsWith(mesh, ".mesh"))
		{
			// gui->Exp(CGui::TXT, mesh);

			//  object  save  .odef
			//------------------------------------------------------------
			String odefFile = path1 + "/" +
				StringUtil::replaceAll(mesh, ".mesh", ".odef");

			ofstream odef;
			odef.open(odefFile.c_str(), std::ios_base::out);
			
			odef << mesh + "\n";
			odef << "1, 1, 1\n";
			odef << "\n";
			odef << "beginmesh\n";
			odef << "mesh " + mesh +"\n";
			odef << "stdfriction concrete\n";
			odef << "endmesh\n";
			odef << "\n";
			odef << "end\n";
			
			odef.close();  ++iodef;
		}
	}

	gui->Exp(CGui::INFO, "Count all: "+toStr(iodef));
	gui->Exp(CGui::INFO, "Ended Create .odef");
	gui->Exp(CGui::INFO, "Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}


//------------------------------------------------------------------------------------------------------------------------
//  Convert .mat
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ConvertMat()
{
	Ogre::Timer ti;
	
	//  Gui status
	gui->Status("Convert materials", 0.7,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Started Convert materials..");

	SetupPath();

	string pathMat = pSet->pathExportOldSR + "/materials/scene/";
	std::vector<string> matFiles{
		"objects_dynamic.mat", "objects_static2.mat", "objects_static.mat",
		"rocks.mat", "grass.mat",
		"trees_ch.mat",	"trees.mat", "trees_old.mat"};


	int all = 0;
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
			/**gui->Exp(CGui::TXT, material +"  diff: "+diffTex+"  norm: "+normTex+
				"  alpha: "+alphaReject+"  cull: "+cull_hardware );/**/
			/**gui->Exp(CGui::TXT, material +"  diff: "+diffTex+"  norm: "+normTex+
				"  alpha: "+alphaReject+"  cull: "+cull_hardware );/**/
			++all;

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
				
				//  fix special cases
				if (material.find("parent") == string::npos &&
					material.find("grass") != string::npos && !first)
					material += " : grass";

				material = StringUtil::replaceAll(material, "{  parent", ":");  // grass
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

	gui->Exp(CGui::INFO, "\nCount: "+toStr(all));
	gui->Exp(CGui::INFO, "Ended Convert materials");
	gui->Exp(CGui::INFO, "Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}
