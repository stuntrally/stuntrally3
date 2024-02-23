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
//  combine terrain textures to _ds, _nh
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ConvertTerrainTex()
{
	Ogre::Timer ti;
	const String pathTer = PATHS::Data() + "/terrain/";

	//  Gui status
	gui->Status("Convert Terrain", 0.7,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Started Convert Terrain Textures ..");
	
	gui->Exp(CGui::NOTE, "All from presets.xml, in: " + pathTer);

	const string pathTo = pSet->pathExportRoR + "terrain/";
	if (!PATHS::CreateDir(pathTo))
	{	gui->Exp(CGui::ERR, "Can't create dir: "+pathTo);
		return;
	}

	// SetupPath();

	int all = 0, errors = 0;
	for (auto& l : pre->ter)
	if (l.texFile[0] != '-')
	{
		++all;
		//  diffuse _d, normal _n, specular _s
		String d_d, d_s, d_r, n_n, n_s;
		d_d = l.texFile +".jpg";  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");  // _s
		n_n = l.texNorm +".jpg";  // _n
		n_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");  // _s
		
		string ext = 1 ? "png" : "dds";  // todo as dds fails..
		String layTexDS = StringUtil::replaceAll(d_d,"_d.jpg","_ds."+ext);
		String layTexNH = StringUtil::replaceAll(n_n,"_n.jpg","_nh."+ext);
		// amntn_ds.pngark_ds.png  ?

		gui->Exp(CGui::TXT, " diff, norm:  " + d_d + "  " + n_n);
		
		String diff = d_d, norm = n_n;
		string from, to;
		{	//  copy _d _n
			from = pathTer + diff;  to = pathTo + diff;
			CopyFile(from, to);

			from = pathTer + norm;  to = pathTo + norm;
			CopyFile(from, to);
		}

		//  find _s  for specular
		String spec = d_s;
		if (!PATHS::FileExists(pathTer + spec))
		{	spec = n_s;
			if (!PATHS::FileExists(pathTer + spec))
			{	spec = d_d;
				gui->Exp(CGui::TXT, " spec:  " + spec + "  " + fToStr(l.tiling));
		}	}

		//  combine RGB+A  diff + spec
		//------------------------------------------------------------
        Image2 imD, imS, imDS;
        imD.load(diff, "General");  imS.load(spec, "General");
		const int xx = imD.getWidth(), yy = imD.getHeight();
		
		auto pfA = PFG_RGBA8_UNORM;
		imDS.createEmptyImage(xx, yy, 1, TextureTypes::Type2D, pfA);
		try
		{
			TextureBox tbD = imD.getData(0), tbS = imS.getData(0), tbDS = imDS.getData(0);
			auto pfD = imD.getPixelFormat(), pfS = imD.getPixelFormat();
			for (int y = 0; y < yy; ++y)
			for (int x = 0; x < xx; ++x)
			{
				ColourValue rgb = tbD.getColourAt(x, y, 0, pfD);
				// ColourValue a = tbS.getColourAt(x, y, 0, pfS) * 0.1f;  // par spec mul-
				// ColourValue ds(rgb.r, rgb.g, rgb.b, a.r);
				ColourValue ds(rgb.r, rgb.g, rgb.b, 0.f);  //! have to 0.f, white spots bug-
				tbDS.setColourAt(ds, x, y, 0, pfA);
			}
			imDS.save(pathTo + layTexDS, 0, 0);
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in combine DS, save to: ") + pathTo + layTexDS);
			++errors;
		}

		//  combine Norm+H
		//------------------------------------------------------------
        Image2 imN, imH, imNH;
        imN.load(norm, "General");  //imS.load(spec, "General");
		const int xn = imN.getWidth(), yn = imD.getHeight();
		
		imNH.createEmptyImage(xx, yy, 1, TextureTypes::Type2D, pfA);
		try
		{
			TextureBox tbN = imN.getData(0), tbNH = imNH.getData(0);
			auto pfN = imN.getPixelFormat();
			for (int y = 0; y < yn; ++y)
			for (int x = 0; x < xn; ++x)
			{
				ColourValue c = tbN.getColourAt(x, y, 0, pfN);
				// const float a = 0.f;  // off
				const float a = max(0.f, 1.f - 0.2f * (c.r + c.g));  // par  side ?..
				ColourValue nh(c.r, c.g, c.b, a);
				tbNH.setColourAt(nh, x, y, 0, pfA);
			}
			imNH.save(pathTo + layTexNH, 0, 0);
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in combine NH, save to: ") + pathTo + layTexNH);
			++errors;
		}	
	}

	gui->Exp(CGui::INFO, "Count all: "+toStr(all));
	if (errors)
		gui->Exp(CGui::WARN, "Errors: "+toStr(errors));
	gui->Exp(CGui::INFO, "Ended Convert Terrain Textures");
	gui->Exp(CGui::INFO, "Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}


//------------------------------------------------------------------------------------------------------------------------
//  Create .odef for all .mesh
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::CreateOdef()
{
	Ogre::Timer ti;

	//  Gui status
	gui->Status("Create .odef", 0.7,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Create .odef for all .mesh ..");

	// SetupPath();

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
//  Convert .mat to .material
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ConvertMat()
{
	Ogre::Timer ti;
	
	//  Gui status
	gui->Status("Convert materials", 0.7,0.5,1);
	gui->edExportLog->setCaption("");
	gui->Exp(CGui::INFO, "Started Convert materials..");

	// SetupPath();
	string pathTo = pSet->pathExportRoR + "materials/";
	if (!PATHS::CreateDir(pathTo))
	{	gui->Exp(CGui::ERR, "Can't create dir: "+pathTo);
		return;
	}

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
		string mtrFile = pathTo + fmat + "erial";
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
		gui->Exp(CGui::NOTE, String("     save to: " + mtrFile));
		
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
			auto found = [](auto& it) -> bool {  return it != string::npos;  };
			
			string w = "material";
			auto it = s.find(w), cmt = s.find("//");
			if (found(it) && (!found(cmt) || cmt > it))  // ignore //material
			{
				if (!first)  // write mtr
					Write();

				//  set name and reset rest  -----
				material = s.substr(it + w.length() + 1);
				
				//  fix special cases
				if (material.find("parent") == string::npos &&
					material.find("grass") != string::npos && !first)
					material += " : grass";

				//  cleanup
				material = StringUtil::replaceAll(material, "parent", ":");  // grass, trees_ch
				material = StringUtil::replaceAll(material, "{", "");
				StringUtil::trim(material);

				diffTex.clear();  normTex.clear();
				alphaReject.clear();  cull_hardware.clear();
				ambient.clear();  diffuse.clear();  specular.clear();

				first = 0;
			}

			//  textures
			w = "diffuseMap";  it = s.find(w);  bool dtex = found(it);
			cmt = s.find("//");
			if (dtex && (!found(cmt) || cmt > it))  // ignore //diffTex
			{
				//diffTex = s.substr(it + w.length() + 1);  // no-
				diffTex = s;  //  cleanup
				diffTex = StringUtil::replaceAll(diffTex, w, "");  // trees_ch
				diffTex = StringUtil::replaceAll(diffTex, "}", "");
				StringUtil::trim(diffTex);
				
				// w = "}";  it = s.find(w);  //  clean same line },  todo after .png .jpg?
				// if (found(it))  diffTex = s.substr(0, it);
				gui->Exp(CGui::TXT, material +"  diff: "+ diffTex);
			}		
			w = "normalMap";  it = s.find(w);
			if (found(it))  normTex = s.substr(it + w.length() + 1);
			
			//  alpha
			w = "alpha_rejection";  it = s.find(w);
			if (found(it))  alphaReject = s.substr(it + w.length() + 1);

			w = "cull_hardware";  it = s.find(w);
			if (found(it))  cull_hardware = s.substr(it + w.length() + 1);

			//  colors
			w = "ambient";  it = s.find(w);
			if (found(it))  ambient = s.substr(it + w.length() + 1);
			if (!dtex)
			{
			w = "diffuse";  it = s.find(w);
			if (found(it)){  cmt = s.find("true");  // ignore, bad
				if (!found(cmt))
					diffuse = s.substr(it + w.length() + 1);
			}	}
			w = "specular";  it = s.find(w);
			if (found(it))  specular = s.substr(it + w.length() + 1);

			// tree_wind true
			// receives_shadows false
			// transparent true
			// mirror  clamp  ..
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


//------------------------------------------------------------------------------------------------------------------------
//  RoR packs
//------------------------------------------------------------------------------------------------------------------------

//  check in which mesh2pack, and to packs
bool ExportRoR::AddPackFor(std::string mesh)
{
	auto it = mesh2pack.find(mesh);
	if (it != mesh2pack.end())
	{
		packs.emplace(it->second);
		return 1;
	}
	gui->Exp(CGui::WARN, "Mesh not found in packs: "+mesh);
	return 0;
}

//  check in which ter2pack, and to packs
bool ExportRoR::AddPackForTer(std::string tex)
{
	auto it = ter2pack.find(tex);
	if (it != ter2pack.end())
	{
		packs.emplace(it->second);
		return 1;
	}
	gui->Exp(CGui::WARN, "Terrain tex not found in packs: "+tex);
	return 0;
}

void ExportRoR::ListPacks()
{
	Ogre::Timer ti;
	gui->Exp(CGui::INFO, "Started List RoR packs..");

	mesh2pack.clear();
	ter2pack.clear();
	
	std::set<string> pks;  // for info
	int imesh = 0, iter = 0;

	strlist all_mods;  // dirs and zip files-
	PATHS::DirList(pSet->pathExportRoR, all_mods);
	for (auto& dir : all_mods)
	{
		if (StringUtil::startsWith(dir, "sr-"))
		{
			strlist all_files;  // content
			PATHS::DirList(pSet->pathExportRoR+"/"+dir, all_files);
			
			bool ok = 0;
			for (auto& fil : all_files)
			if (StringUtil::endsWith(fil, ".assetpack"))  // has to be inside
				ok = 1;
			
			if (!ok)
				continue;

			pks.emplace(dir);
			bool ter = dir.find("terrain") != string::npos;
			
			for (auto& fil : all_files)
			{
				if (StringUtil::endsWith(fil, ".mesh"))
				{
					mesh2pack[fil] = dir;  // add mesh
					++imesh;
					// gui->Exp(CGui::TXT, "Added "+fil+" in pack "+dir);
				}
				else if (ter)
				if (StringUtil::endsWith(fil, ".png"))
				{
					ter2pack[fil] = dir;  // add ter tex
					++iter;
					// gui->Exp(CGui::TXT, "Added "+fil+" in pack "+dir);
				}
			}
		}
	}
	gui->Exp(CGui::NOTE, "Packs: "+toStr(pks.size())+"  meshes: "+toStr(imesh)+"  terrain tex: "+toStr(iter));

	gui->Exp(CGui::INFO, "Ended List RoR packs.");
	gui->Exp(CGui::INFO, "Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s\n");
}
