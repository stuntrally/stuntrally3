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
//  📄📦 Objects  save  .tobj
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportObjects()
{
	std::vector<string> dirs{"objects", "objects2", "objectsC", "objects0", "obstacles"};
	
	string objFile = path + name + "-obj.tobj";
	ofstream obj;
	obj.open(objFile.c_str(), std::ios_base::out);

	int iodef = 0, iObjMesh = 0;
	std::map<string, int> once;
	hasObjects = sc->objects.empty();

	for (const auto& o : sc->objects)
	{
		Vector3 p = Axes::toOgre(o.pos);
		auto q = Axes::toOgreW(o.rot);

		obj << fToStr(half - p.z)+", "+fToStr(p.y - hmin)+", "+fToStr(p.x + half)+", ";
		// todo  fix all rot ?
		// obj << fToStr(q.getPitch().valueDegrees()+90.f,0,3)+", "+fToStr(q.getYaw().valueDegrees(),0,3)+", "+fToStr(q.getRoll().valueDegrees(),0,3)+", ";
		obj << fToStr(90.f,0,3)+", "+fToStr(0.f,0,3)+", "+fToStr(q.getYaw().valueDegrees(),0,3)+", ";
		// todo  no scale ??
		obj << o.name +"\n";
		
	// #define COPY_OBJS 1  // NO, once for all tracks
	#ifdef COPY_OBJS

		//  object  save  .odef
		//------------------------------------------------------------
		const string mesh = o.name + ".mesh";
		string odefFile = path + o.name + ".odef";
		// if (!fs::exists(odefFile))
		if (once.find(o.name) == once.end())
		{
			once[o.name] = 1;
			ofstream odef;
			odef.open(odefFile.c_str(), std::ios_base::out);
			
			odef << mesh + "\n";
			odef << "1, 1, 1\n";
			odef << "\n";
			odef << "beginmesh\n";
			odef << "mesh " + o.name + ".mesh\n";
			odef << "stdfriction concrete\n";
			odef << "endmesh\n";
			odef << "\n";
			odef << "end\n";
			
			odef.close();  ++iodef;


			//------------------------------------------------------------
			//  Find mesh  in old SR dirs
			//------------------------------------------------------------
			bool exists = 0;
			string from, to;
			for (auto& d : dirs)
			{
				string file = pSet->pathExportOldSR + d +"/"+ mesh;
				if (PATHS::FileExists(file))
				{	exists = 1;  from = file;  }
				// gui->Exp(CGui::INFO, String("obj: ")+file+ "  ex: "+(exists?"y":"n"));
			}
			if (exists)
			{
				//  copy
				to = path + mesh;
				CopyFile(from, to);
				if (CopyFile(from, to))
					++iObjMesh;
				else
					continue;

				//  get mtr?  read .mat,  copy textures,  write .material ...
			}
		}
	#endif
	}
	obj.close();

	gui->Exp(CGui::TXT, "Objects: "+toStr(sc->objects.size())+"  odef: "+toStr(iodef)+"  meshes: "+toStr(iObjMesh) + "\n");
}
