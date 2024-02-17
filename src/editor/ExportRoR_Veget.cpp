#include "pch.h"
#include "ExportRoR.h"
#include "Def_Str.h"
#include "paths.h"

#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "CData.h"
#include "PresetsXml.h"
#include "Axes.h"
#include "Road.h"
#include "TracksXml.h"

#include <Terra.h>
#include <OgreImage2.h>
#include <OgreVector3.h>

#include <exception>
#include <string>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//------------------------------------------------------------------------------------------------------------------------
//  🌳🪨 Vegetation setup  save  .tobj .png
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportVeget()
{
	Ogre::Timer ti;

	const Real tws = sc->tds[0].fTerWorldSize;
	const auto* terrain = scn->ters[0];

	hasVeget = 1;  // par.. not in FPS variant
	if (hasVeget)
	{
		string vegFile = path + name + "-veget.tobj";
		ofstream veg;
		veg.open(vegFile.c_str(), std::ios_base::out);

		// string matFile = path + name + "-veget.material";
		// ofstream mat;
		// mat.open(matFile.c_str(), std::ios_base::out);


		//------------------------------------------------------------------------------------------------------------------------
		//  🌿 Grass layers
		//------------------------------------------------------------------------------------------------------------------------
		veg << "// grass range,  SwaySpeed, SwayLength, SwayDistribution,   Density,  minx, miny, maxx, maxy,   fadetype, minY, maxY,   material colormap densitymap\n";

		const SGrassLayer* g0 = &sc->grLayersAll[0];
		int igrs = 0;
		for (int i=0; i < sc->ciNumGrLay; ++i)
		{
			const SGrassLayer* gr = &sc->grLayersAll[i];
			if (gr->on)
			{
				const SGrassChannel* ch = &scn->sc->grChan[gr->iChan];
				const string mapName = name + "-grass"+toStr(gr->iChan)+".png";
				++igrs;

				//grass 200,  0.5, 0.05, 10,  0.1, 0.2, 0.2, 1, 1,  1, 0, 9, seaweed none none
				//grass 600,  0.5, 0.15, 10,  0.3, 0.2, 0.2, 1, 1,  1, 10, 0, grass1 aspen.jpg aspen_grass_density.png

				//  write  ------
				//  range,
				veg << "grass " << "300,  ";  // par mul
				//  Sway: Speed, Length, Distribution,
				veg << "0.5, 0.1, 10,  ";  
				
				//  Density,  minx, miny, maxx, maxy,
				veg << gr->dens * sc->densGrass * cfg->grassMul << ",  ";
				veg << gr->minSx <<", "<< gr->minSy <<", "<< gr->maxSx <<", "<< gr->maxSy << ", ";
				
				//  fadetype, minY, maxY,
				veg << "1, 10, 0,  ";
				//  material  colormap  densitymap
				veg << gr->material << " none " << mapName << "\n";


				//  grass dens map
				//------------------------------------------------------------
				Image2 img, im2;
				try
				{
					img.load(String("roadDensity.png"), "General");
					im2.load(String("roadDensity.png"), "General");
					// im2.createEmptyImage(xx, yy, 1, TextureTypes::Type2D, pf);

					const int xx = img.getWidth(), yy = img.getHeight();
					TextureBox tb = img.getData(0), tb2 = im2.getData(0);
					auto pf = img.getPixelFormat();

					for (int y = 0; y < yy; ++y)
					for (int x = 0; x < xx; ++x)
					{
						float rd = tb.getColourAt(xx-1 - x, y, 0, pf).r;  // flip x
						
						float xw = (float(x) / (xx-1) - 0.5f) * tws,
							  zw = (float(yy-1 - y) / (yy-1) - 0.5f) * tws;

						Real a = terrain->getAngle(xw, zw, 1.f);
						Real h = terrain->getHeight(xw, zw);  // /2 par..
						float d = rd
								* scn->linRange(a, ch->angMin, ch->angMax, ch->angSm)
								* scn->linRange(h, ch->hMin, ch->hMax, ch->hSm);

						ColourValue cv(d,d,d);  // white
						tb2.setColourAt(cv, x, y, 0, pf);
					}
					im2.save(path + mapName, 0, 0);
				}
				catch (exception ex)
				{
					gui->Exp(CGui::WARN, string("Exception in grass dens map: ") + ex.what());
				}

			#if 0  // NO, once for all

				//  copy grass*.png
				//------------------------------------------------------------
				String pathGrs = PATHS::Data() + "/grass/";
				String grassPng = gr->material + ".png";  // same as mtr name
				 
				//  copy _d _n
				string from = pathGrs + grassPng, to = path + grassPng;
				CopyFile(from, to);
				
				//  create .material for it
				mat << "material " << gr->material << "\n";
				mat << "{\n";
				mat << "	technique\n";
				mat << "	{\n";
				mat << "		pass\n";
				mat << "		{\n";
				mat << "			cull_hardware none\n";
				mat << "			cull_software none\n";
				mat << "			alpha_rejection greater 128\n";
				mat << "			texture_unit\n";
				mat << "			{\n";
				mat << "				texture	" << grassPng << "\n";
				mat << "				tex_address_mode clamp\n";
				mat << "			}\n";
				mat << "		}\n";
				mat << "	}\n";
				mat << "}\n";
				mat << "\n";
			#endif
			}
		}
		// mat.close();

		gui->Exp(CGui::NOTE, "Grasses: " + toStr(igrs));



		//------------------------------------------------------------------------------------------------------------------------
		//  🌳🪨 Vegetation  layers
		//------------------------------------------------------------------------------------------------------------------------
		std::vector<string> dirs{"trees", "trees2", "trees-old", "rocks", "rockshex"};
		std::map<string, int> once;
		int iVegetMesh = 0;

		for (size_t l=0; l < sc->vegLayers.size(); ++l)
		{
			VegetLayer& vg = sc->vegLayersAll[sc->vegLayers[l]];
			const String mesh = vg.name;
			const string mapName = name + "-veget"+toStr(l)+".png";

			//  veget dens map
			//------------------------------------------------------------
			Image2 img, im2;
			try
			{
				img.load(String("roadDensity.png"), "General");
				im2.load(String("roadDensity.png"), "General");
				// im2.createEmptyImage(xx, yy, 1, TextureTypes::Type2D, pf);

				const int xx = img.getWidth(), yy = img.getHeight();
				TextureBox tb = img.getData(0), tb2 = im2.getData(0);
				auto pf = img.getPixelFormat();

				for (int y = 0; y < yy; ++y)
				for (int x = 0; x < xx; ++x)
				{
					float rd = tb.getColourAt(xx-1 - x, y, 0, pf).r;  // flip x

					float xw = (float(x) / (xx-1) - 0.5f) * tws,
						  zw = (float(yy-1 - y) / (yy-1) - 0.5f) * tws;

					Real a = terrain->getAngle(xw, zw, 1.f);  //td.fTriangleSize);
					Real h = terrain->getHeight(xw, zw);
					float d = rd;
					if (a > vg.maxTerAng || rd == 0.f ||
						h > vg.maxTerH || h < vg.minTerH)
						d = 0.f;
					else
					{	//  check if in fluids
						float fa = sc->GetDepthInFluids(Vector3(xw, 0.f, zw));
						if (fa > vg.maxDepth)
							d = 0.f;
						else
						{
						#if 1  //  slow ..
							int c = sc->trRdDist + vg.addRdist;
							int d = c;
							bool bMax = vg.maxRdist < 20; //100 slow-
							if (bMax)
								d = c + vg.maxRdist+1;  // not less than c

							//  find dist to road
							int ii,jj, rr, rmin = 3000;  //d
							for (jj = -d; jj <= d; ++jj)
							for (ii = -d; ii <= d; ++ii)
							{
								const int
									xi = std::max(0,std::min(xx-1, y+ii)),
									yj = std::max(0,std::min(yy-1, x+jj));

								const float cr = tb.getColourAt(
									xi, yj, 0, Ogre::PFG_RGBA8_UNORM_SRGB ).r;
								
								if (cr < 0.75f)  // par-
								{
									rr = abs(ii)+abs(jj);
									//rr = sqrt(float(ii*ii+jj*jj));  // much slower
									rmin = std::min(rmin, rr);
								}
							}
							if (rmin <= c)
								d = 0.f;

							if (bMax && /*d > 1 &&*/ rmin > d-1)  // max dist (optional)
								d = 0.f;
						#endif
					}	}

					ColourValue cv(d,d,d);  // white
					tb2.setColourAt(cv, x, y, 0, pf);
				}
				im2.save(path + mapName, 0, 0);
			}
			catch (exception ex)
			{
				gui->Exp(CGui::WARN, string("Exception in grass dens map: ") + ex.what());
			}

			
			//  presets.xml needed
			auto nomesh = mesh.substr(0, mesh.length()-5);
			const PVeget* pveg = scn->data->pre->GetVeget(nomesh);
			if (!pveg)  continue;
			bool add = 1;


		// #define COPY_VEGET 1  // NO, once for all tracks
		#ifdef COPY_VEGET

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
				// gui->Exp(CGui::NOTE, String("veget: ")+file+ "  ex: "+(exists?"y":"n"));
			}

			//  copy mesh from old SR  ..or slow convert v2 to v1-
			if (exists)
			{
				if (once.find(mesh) == once.end())
				{	once[mesh] = 1;

					to = path + mesh;
					if (CopyFile(from, to))
						++iVegetMesh;
					else
 						continue;
				}

				//  get mesh mtr
				//---------------------------------------------------------------------------------------
				try
				{
					String resGrp = "MeshV1";
					v1::MeshPtr v1Mesh;
					ResourceGroupManager::getSingleton().addResourceLocation(path, "FileSystem", resGrp);
					v1Mesh = v1::MeshManager::getSingleton().load( mesh, resGrp,
						v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );
					
					int si = v1Mesh->getNumSubMeshes();
					for (int i=0; i < si; ++i)
					{
						auto sm = v1Mesh->getSubMesh(i);
						String mtr = sm->getMaterialName();
						gui->Exp(CGui::TXT, mesh +" - "+ mtr);
					}
				}	
				catch (Exception ex)
				{
					String s = "Error: loading mesh: " + mesh + " \nfrom: " + path + "\n failed ! \n" + ex.what() + "\n";
					gui->Exp(CGui::WARN, s);
				}
		#endif

				//  write  ------
				if (l==0)
					veg << "\n// trees  yawFrom, yawTo,  scaleFrom, scaleTo,  highDensity,  distance1, distance2,  meshName colormap densitymap\n";
				if (add)
				{	//trees 0, 360, 0.1, 0.12, 2, 60, 3000, fir05_30.mesh aspen-test.dds aspen_grass_density2.png 
					veg << "trees 0, 360,  ";
					veg << vg.minScale << ", " << vg.minScale << ",  ";
					veg << vg.dens * sc->densTrees * 2.f << ",  ";

					// veg << ", 60, 1000, ";  // vis dist
					veg << pveg->visDist * 0.5f << ", " << pveg->farDist << ",  ";  // par .. todo
					veg << vg.name << " none " << mapName << "\n";
				}

		#ifdef COPY_OBJS
			}
		#else
				++iVegetMesh;
		#endif
		}
		
		veg.close();

		gui->Exp(CGui::NOTE, "Veget meshes: " + toStr(iVegetMesh));
	}

	gui->Exp(CGui::INFO, "Time Veget: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s\n");
}
