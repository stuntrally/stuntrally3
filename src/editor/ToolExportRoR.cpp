#include "pch.h"
#include "enums.h"
#include "BaseApp.h"
#include "settings.h"
#include "paths.h"

#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "CData.h"
#include "TracksXml.h"
#include "Axes.h"
// #include "Road.h"
#include <OgreImage2.h>

#include <exception>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//  gui events
void CGui::editRoRPath(Ed ed)
{
	pSet->pathExportRoR = ed->getCaption();
}
void CGui::btnExport(WP)
{
	app->ToolExportRoR();
}

//  Export current track for RigsOfRods
//------------------------------------------------------------------------------------------------------------------------
void App::ToolExportRoR()
{
	//  Gui status
	gui->Status("RoR Export..", 1,0.5,1);
	gui->edExport->setCaption("");
	gui->Exp(CGui::INFO, "Export to RoR started..");


	//  dir  export to RoR content path
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
	const string name = pSet->gui.track;
	const string dirTrk = dirRoR + trk;
	if (!PATHS::CreateDir(dirTrk))
	{	gui->Exp(CGui::ERR, "Can't create track dir: "+dirTrk);
		return;
	}
	const string path = dirTrk + "/";  // main
	const auto& sc = scn->sc;


	//  ⛰️ Heightmap convert to .raw
	//------------------------------------------------------------
	const auto& td = sc->tds[0];  // 1st ter only

	const int size  = td.iVertsX;  // org  e.g. 1024
	const int size1 = size + 1;  // e.g. 1025
	const int hsize = size1 * size1;

	ushort* hmap = new ushort[hsize];
	memset(hmap, 0, hsize * 2);

	//  find h min, max
	float hmin = 100000.f, hmax = -hmin;
	for (int y=0; y < size; ++y)
	{
		int a = y * size;
		for (int x=0; x < size; ++x,++a)
		{
			float h = td.hfHeight[a];
			if (h > hmax)  hmax = h;
			if (h < hmin)  hmin = h;
	}	}
	const float hsc = 1.f / (hmax - hmin);
	
	//  world size
	const float Ysize = hmax - hmin;
	const float XZsize = size *  td.fTriangleSize;

	gui->Exp(CGui::TXT, "Hmap size   " + toStr(size));
	gui->Exp(CGui::TXT, "Height min  " + toStr(hmin));
	gui->Exp(CGui::TXT, "Height max  " + toStr(hmax));
	gui->Exp(CGui::TXT, "World XZ size  " + toStr(XZsize));
	gui->Exp(CGui::TXT, "World Y size   " + toStr(Ysize));

	//  normalize  to 2B raw:  0..65535
	for (int y=0; y < size; ++y)
	{
		int a = y * size;
		for (int x=0; x < size; ++x,++a)
			hmap[x*size1 + y] =  // flip x-y
				(td.hfHeight[a] - hmin) * hsc * 32767.f + 32767.f;
	}

	//  todo works?
	//  add extra +1 col  |
	for (int y=0; y < size; ++y)
		hmap[y * size + size-1] = hmap[y * size + size-2];
	
	//  add extra +1 row __
	const int a = (size-1) * size, a1 = (size-2) * size;
	for (int x=0; x < size; ++x)
		hmap[x + a] = hmap[x + a1];


	//  save Hmap  .raw  16bit 2B
	//------------------------------------------------------------
	const string hmapFile = path + name + ".raw";
	
	ofstream of;
	of.open(hmapFile.c_str(), std::ios_base::binary);
	of.write((const char*)&hmap[0], hsize * 2);
	of.close();
	delete[] hmap;  hmap = 0;


	//  🏔️ Blendmap
	//------------------------------------------------------------
	gui->Exp(CGui::TXT, "Terrain layers  " + toStr(td.layers.size()));

	//  copy layer textures

	int layers = std::min(4, (int)td.layers.size());
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];
		// di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String pathTerTex = PATHS::Data() + "/terrain/";
		String diff = l.texFile;  // ends with _d
		String norm = l.texNorm;  // _n
		gui->Exp(CGui::TXT, "layer " + toStr(i+1) + " diff, norm:  " + diff + "  " + norm);
		
		string from, to;
		try
		{
			from = pathTerTex + diff;  to = path + diff;
			if (!fs::exists(to.c_str()))
				fs::copy_file(from.c_str(), to.c_str());

			from = pathTerTex + norm;  to = path + norm;
			if (!fs::exists(to.c_str()))
				fs::copy_file(from.c_str(), to.c_str());
		}
		catch (const fs::filesystem_error & ex)
		{
			String s = "Error: Copying file " + from + " to " + to + " failed ! \n" + ex.what();
			gui->Exp(CGui::WARN, s);
		}

		// todo combine RGB+A ..  diff+spec .. ?
		// todo convert blending to lerp(lerp( .. ??
	}

	
	//  📄🏔️ Terrain layers setup  save  page.otc
	//------------------------------------------------------------------------------------------------------------------------
	string opgFile = path + name + "-page-0-0.otc";
	ofstream lay;
	lay.open(opgFile.c_str(), std::ios_base::out);

	lay << name + ".raw\n";
	lay << "2\n";  // todo
	// lay << layers << "\n";
	lay << "; worldSize, diffusespecular, normalheight, blendmap, blendmapmode, alpha\n";

	// int layers = std::min(4, (int)td.layers.size());  // todo
	layers = 2;  // 0 ter, 1 road  only ...
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];
		// di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String pathTerTex = PATHS::Data() + "/terrain/";
		String diff = l.texFile;  // ends with _d
		String norm = l.texNorm;  // _n

		if (i==0)
			lay << l.tiling << " , " << diff+", "+norm+"\n";
		else
			lay << l.tiling << " , " << diff+", "+norm+", " + name + "-road.png, R, 0.99\n";

		//  todo all layers, blendmap R,G,B,A ..
		// "5    , desert_sand_d.jpg, desert_sand_n.jpg, roadDensity.png, R, 0.99\n";
	}
	lay.close();


	//  🛣️ Road image,  blend last
	//------------------------------------------------------------
	Image2 img, im2;  // fixme outside road
	try
	{
		img.load(String("roadDensity.png"), "General");
	#if 0  //  doesnt work-
		img.flipAroundX();
		// img.flipAroundY();
		img.save(path + name + "-road.png", 0, 0);
	#else
		im2.load(String("roadDensity.png"), "General");

		const int xx = img.getWidth(), yy = img.getHeight();
	  #if 0  //  slow way
		for (int y = 0; y < yy; ++y)
		for (int x = 0; x < xx; ++x)
		{
			ColourValue cv = img.getColourAt(xx-1 - x, y, 0);
			im2.setColourAt(cv, x, y, 0);
		}
	  #else
		TextureBox tb = img.getData(0), tb2 = im2.getData(0);
		auto pf = img.getPixelFormat();
		for (int y = 0; y < yy; ++y)
		for (int x = 0; x < xx; ++x)
		{
			ColourValue cv = tb.getColourAt(xx-1 - x, y, 0, pf);
			tb2.setColourAt(cv, x, y, 0, pf);
		}
	  #endif

		im2.save(path + name + "-road.png", 0, 0);  // road blend map
	#endif
	}
	catch (exception ex)
	{
		gui->Exp(CGui::WARN, string("Exception in road Dens flip: ") + ex.what());
	}
	int r = img.getWidth();
	gui->Exp(CGui::TXT, "Road image width: "+fToStr(r));


	//  📄⛰️ Terrain hmap, setup  save  .otc
	//------------------------------------------------------------
	string otcFile = path + name + ".otc";
	ofstream otc;
	otc.open(otcFile.c_str(), std::ios_base::out);

	otc << "# the amount of pages in this terrain\n";
	otc << "# 0, 0 means that you only have one page\n";
	otc << "PagesX=0\n";
	otc << "PagesZ=0\n";
	otc << "\n";
	otc << "PageFileFormat=" + name + "-page-0-0.otc\n";
	otc << "\n";
	otc << "Heightmap.0.0.raw.size=" << size1 << "\n";
	otc << "Heightmap.0.0.raw.bpp=2\n";
	otc << "Heightmap.0.0.flipX=0\n";
	otc << "\n";
	otc << "# The world size of the terrain\n";
	otc << "WorldSizeX=" << XZsize << "\n";
	otc << "WorldSizeZ=" << XZsize << "\n";
	otc << "\n";
	otc << "# the factor with what the heightmap values get multiplied with\n";
	otc << "WorldSizeY=" << Ysize * 2.f << "\n";
	otc << "\n";
	otc << "# Sets the default size of blend maps for a new terrain. This is the resolution of each blending layer for a new terrain. default: 1024\n";
	otc << "LayerBlendMapSize=2048\n";  // todo 1024 ?
	otc << "\n";
	otc << "# disableCaching=1 will always enforce regeneration of the terrain, useful if you want to change the terrain config (.otc) and test it. Does not cache the objects on it.\n";
	otc << "disableCaching=1\n";
	otc << "\n";
	otc << "#optimizations\n";
	otc << "\n";
	otc << "# Minimum batch size (along one edge) in vertices; must be 2^n+1. The terrain will be divided into tiles, and this is the minimum size of one tile in vertices (at any LOD). default: 17\n";
	otc << "minBatchSize=17\n";
	otc << "\n";
	otc << "# Maximum batch size (along one edge) in vertices; must be 2^n+1 and <= 65. The terrain will be divided into hierarchical tiles, and this is the maximum size of one tile in vertices (at any LOD). default: 65\n";
	otc << "maxBatchSize=65\n";
	otc << "\n";
	otc << "# Whether to support a light map over the terrain in the shader, if it's present (default true).\n";
	otc << "LightmapEnabled=0\n";
	otc << "\n";
	otc << "# Whether to support normal mapping per layer in the shader (default true). \n";
	otc << "NormalMappingEnabled=0\n";
	otc << "\n";
	otc << "# Whether to support specular mapping per layer in the shader (default true). \n";
	otc << "SpecularMappingEnabled=1\n";
	otc << "\n";
	otc << "# Whether to support parallax mapping per layer in the shader (default true). \n";
	otc << "ParallaxMappingEnabled=0\n";
	otc << "\n";
	otc << "# Whether to support a global colour map over the terrain in the shader, if it's present (default true). \n";
	otc << "GlobalColourMapEnabled=0\n";
	otc << "\n";
	otc << "# Whether to use depth shadows (default false). \n";
	otc << "ReceiveDynamicShadowsDepth=0\n";
	otc << "\n";
	otc << "# Sets the default size of composite maps for a new terrain, default: 1024\n";
	otc << "CompositeMapSize=1024\n";
	otc << "\n";
	otc << "# Set the distance at which to start using a composite map if present, default: 4000\n";
	otc << "CompositeMapDistance=5000\n";
	otc << "\n";
	otc << "# the default size of 'skirts' used to hide terrain cracks, default: 30\n";
	otc << "SkirtSize=30\n";
	otc << "\n";
	otc << "#  Sets the default size of lightmaps for a new terrain, default: 1024\n";
	otc << "LightMapSize=1024\n";
	otc << "\n";
	otc << "# Whether the terrain will be able to cast shadows, default: 0\n";
	otc << "CastsDynamicShadows=0\n";
	otc << "\n";
	otc << "# Set the maximum screen pixel error that should  be allowed when rendering, default:\n";
	otc << "MaxPixelError=3\n";  // todo >
	otc << "\n";
	// otc << "# dump the blend maps to files named blendmap_layer_X.png\n";
	// otc << "DebugBlendMaps=0\n";
	// otc << "NormalMappingEnabled=1\n";

	otc.close();


	//  get authors from tracks.ini
	string authors = "CryHam";
	int trkId = 0;  // N from ini
	int id = scn->data->tracks->trkmap[pSet->gui.track];
	if (id > 0)
	{	const TrackInfo& ti = scn->data->tracks->trks[id-1];

		authors = ti.author=="CH" ? "CryHam" : ti.author;
		trkId = ti.n;
	}else
		gui->Exp(CGui::ERR, "Track not in tracks.ini, no id or authors set.");
	

	//  🏞️⛅ Track/map setup  save  .terrn2
	//------------------------------------------------------------------------------------------------------------------------
	string terrn2File = path + name + ".terrn2";
	ofstream trn;
	trn.open(terrn2File.c_str(), std::ios_base::out);

	trn << "[General]\n";
	trn << "Name = "+trk+"\n";
	trn << "GeometryConfig = " + name + ".otc\n";
	trn << "Water=0\n";  // todo  find big water h-
	trn << "WaterLine=1\n";

	trn << "AmbientColor = 0.99, 0.98, 0.97\n";  // todo ? sc->lAmb
	//trn << "StartPosition = 0.0, 110.0, 0.0\n";  // todo  start
	Vector3 st = Axes::toOgre(sc->startPos[0]);
	st.x += XZsize * 0.5f;  // half ter size
	st.z += XZsize * 0.5f;
	trn << "StartPosition = " << fToStr(st.x)+", "+fToStr(st.y)+", "+fToStr(st.z)+"\n";

	trn << "#CaelumConfigFile = \n";  // todo  caelum.os
	trn << "SandStormCubeMap = tracks/skyboxcol\n";  // sky meh-

	trn << "Gravity = " << -sc->gravity << "\n";
	trn << "CategoryID = 129\n";
	trn << "Version = 1\n";
	// todo, random hash from trk name?
	trn << "GUID = 11223344-5566-7788-" << fToStr(trkId,0,4,'0') <<"-012345678901\n";
	trn << "\n";
	// trn << "//If your map has a groundmodel, define the landuse file in this format:\n";
	trn << "#TractionMap = landuse.cfg\n";  // todo  surfaces.cfg
	trn << "\n";
	trn << "[Authors]\n";
	trn << "track = " + authors + "\n";
	trn << "conversion = Exported from Stunt Rally 3 Track Editor\n";
	trn << " \n";
	trn << "#[Objects]\n";  // todo  objs
	trn << "#"+name+".tobj=\n";
	trn << "\n";
	trn << "#[Scripts]\n";  // todo  road, checks
	// trn << "//If your map has a race script define it here in this format:\n";
	// trn << "//All 0.3x races use .terrn.as as the extension, include the .terrn also:\n";
	trn << "#"+name+".terrn.as=\n";

	trn.close();


	//  📄📦 Objects  save  .tobj
	//------------------------------------------------------------
	string objFile = path + name + ".tobj";
	ofstream obj;
	obj.open(objFile.c_str(), std::ios_base::out);

	for (const auto& o : sc->objects)
	{
		Vector3 p = Axes::toOgre(o.pos);
		auto q = Axes::toOgreW(o.rot);
		// obj << "257.641, 1.19744, 225.319, -0, 0, 0, 8c07UID-baja_rock_01\n";
		obj << fToStr(p.x)+", "+fToStr(p.y)+", "+fToStr(p.z);
		// todo q.toEulerAngles ?
		obj << "-0, 0, 0, ";
		obj << o.name +"\n";
	}
	//  vegetation?..
	// scn->vegetNodes;

	obj.close();


	gui->Exp(CGui::INFO, "Export to RoR end.");
}
