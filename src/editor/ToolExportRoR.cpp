#include "pch.h"
#include "enums.h"
#include "BaseApp.h"
#include "settings.h"
#include "paths.h"
#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "Road.h"
#include "paths.h"
#include <string>
using namespace Ogre;
using namespace std;


//  Export current track for RigsOfRods
void App::ToolExportRoR()
{
	//  gui status
	gui->Status("RoR Export..", 1,0.5,1);
	gui->edWarn->setCaption("");
	gui->Warn(CGui::INFO, "Export to RoR started..");


	//  heightmap convert to .raw
	//------------------------------------------------------------
	auto& td = scn->sc->tds[0];  // 1st ter only

	const int size  = td.iVertsX;  // org  e.g. 1024
	const int size1 = size + 1;  // e.g. 1025
	const int hsize = size1 * size1;

	ushort* hmap = new ushort[hsize];
	memset(hmap, 0, hsize * 2);

	//  find h min, max
	float hmin = 100000.f, hmax = -100000.f;
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
	const float Ysize = hmax - hmin;
	// const float hsc = max( fabs(hmax), fabs(hmin) );
	// const float hmul = 12767.f / hsc;

	gui->Warn(CGui::TXT, "H min    " + toStr(hmin));
	gui->Warn(CGui::TXT, "H max    " + toStr(hmax));
	gui->Warn(CGui::TXT, "H scale   " + toStr(Ysize));

	//  normalize  to 2B raw:  0..65535
	for (int y=0; y < size; ++y)
	{
		int a = y * size;
		// int a1 = y * size1;
		// int a1 = (size-1 - y) * size1 + size-1;  // flip x,y
		
		// for (int x=0; x < size; ++x,++a,--a1)
		// 	hmap[a1] = (td.hfHeight[a] - hmin) * hsc * 32767.f + 32767.f;

		for (int x=0; x < size; ++x,++a)
			hmap[x*size1 + y] = (td.hfHeight[a] - hmin) * hsc * 32767.f + 32767.f;
	}

	//  add extra +1 col  |
	for (int y=0; y < size; ++y)  hmap[y*size + size-1] = hmap[y*size + size-2];
	//  add extra +1 row __
	const int a = (size-1)*size, a1 = (size-2)*size;
	for (int x=0; x < size; ++x)  hmap[x + a] = hmap[x + a1];


	//  save hmap .raw 16bit
	//------------------------------------------------------------
	string path = "/home/ch/_p/rigs-of-rods-dev/content/``0";  // adj..
	string hmapFile = path + "/heightmap.raw";
	ofstream of;
	of.open(hmapFile.c_str(), std::ios_base::binary);
	of.write((const char*)&hmap[0], hsize * 2);
	of.close();
	delete[] hmap;  hmap = 0;

	
	//  world size
	const float XZsize = size *  td.fTriangleSize;

	//  save  .otc  Terrain setup
	//------------------------------------------------------------
	string otcFile = path + "/0.otc";
	ofstream otc;
	otc.open(otcFile.c_str(), std::ios_base::out);

	otc << "# the amount of pages in this terrain\n";
	otc << "# 0, 0 means that you only have one page\n";
	otc << "PagesX=0\n";
	otc << "PagesZ=0\n";
	otc << "\n";
	otc << "PageFileFormat=0-page-0-0.otc\n";
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
	otc << "LayerBlendMapSize=2048\n";
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
	otc << "MaxPixelError=3\n";
	otc << "\n";
	otc << "# dump the blend maps to files named blendmap_layer_X.png\n";
	otc << "DebugBlendMaps=0\n";
	otc << "NormalMappingEnabled=1\n";
	otc << "\n";

	otc.close();


	gui->Warn(CGui::INFO, "Export to RoR end.");
}
