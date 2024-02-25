#include "pch.h"
#include "ExportRoR.h"
#include "Def_Str.h"
#include "settings.h"
#include "paths.h"

#include "CGui.h"
#include "CScene.h"
#include "CData.h"
#include "PresetsXml.h"

#include <Terra.h>
#include <OgreImage2.h>
#include <OgreVector3.h>
#include <OgreColourValue.h>

#include <exception>
#include <string>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//------------------------------------------------------------------------------------------------------------------------
//  ⛰️ Terrain  Heightmap, Blendmap, Road image, scripts
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportTerrain()  // whole, full
{
	Ogre::Timer ti;

	//  ⛰️ Heightmap convert to .raw
	//------------------------------------------------------------------------------------------------------------------------
	const auto& td = sc->tds[0];  // 1st ter only

	const int size  = td.iVertsX;  // org  e.g. 1024
	const int size1 = size + 1;  // e.g. 1025
	const int hsize = size1 * size1;

	ushort* hmap = new ushort[hsize];
	memset(hmap, 0, hsize * 2);

	//  find h min, max
	hmin = 100000.f;  float hmax = -hmin;
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
	const float Ysize = (hmax - hmin);
	const float XZsize = size *  td.fTriangleSize;
	half = XZsize * 0.5f;  // half ter size offset

	gui->Exp(CGui::TXT, "Hmap size   " + toStr(size));
	gui->Exp(CGui::TXT, "Height min  " + toStr(hmin));
	gui->Exp(CGui::TXT, "Height max  " + toStr(hmax));
	gui->Exp(CGui::TXT, "World XZ size  " + toStr(XZsize));
	gui->Exp(CGui::TXT, "World Y size   " + toStr(Ysize));

	//  normalize  to 2B raw
	for (int y=0; y < size; ++y)
	{
		int a = y * size;
		for (int x=0; x < size; ++x,++a)
			hmap[x*size1 + y] =  // flip x-y
				(td.hfHeight[a] - hmin) * hsc * 65535.f;
	}

	//  add extra +1 col  |
	const int s = size1;
	for (int y=0; y < s; ++y)
		hmap[y * s + s-1] = hmap[y * s + s-2];
	
	//  add extra +1 row __
	const int a = (s-1) * s, a1 = (s-2) * s;
	for (int x=0; x < s; ++x)
		hmap[x + a] = hmap[x + a1];


	//  save Hmap  .raw  16bit 2B
	//------------------------------------------------------------
	const string hmapFile = path + name + ".raw";
	
	ofstream of;
	of.open(hmapFile.c_str(), std::ios_base::binary);
	of.write((const char*)&hmap[0], hsize * 2);
	of.close();
	delete[] hmap;  hmap = 0;


	//  🏔️ Blendmap  Layers
	//------------------------------------------------------------------------------------------------------------------------
	gui->Exp(CGui::NOTE, "Terrain layers  " + toStr(td.layers.size()));

	string layTexDS[4], layTexNH[4];  // new filenames for ds, nh
	string ext = 1 ? "png" : "dds";  // todo as dds fails..

	//  layer textures  copy & convert
	const int layers = std::min(4, (int)td.layers.size());
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];

		ConvertTerTex(l.texFile, l.texNorm, path,
					layTexDS[i], layTexNH[i], copyTerTex);
		AddPackForTer(layTexDS[i]);
	}

	//  🏔️ Blendmap  save as .png  also  -surfaces.png
	//------------------------------------------------------------------------------------------------------------------------
	int bleSize = 1024;
	if (scn->ters[0]->blendmap.texture)
	{
		auto bleFile = path + name + "-blendmap.png";
		// scn->ters[0]->blendmap.texture->writeContentsToFile(ble, 0, 0);
		auto surfFile = path + name + "-surfaces.png";
		
        Image2 img, imb, imr, isu;
		try
		{	//  blendmap rtt
	        img.convertFromTexture(scn->ters[0]->blendmap.texture, 0, 0);
			const int xx = img.getWidth(), yy = img.getHeight();
			auto pb = img.getPixelFormat();
			imb.createEmptyImage(xx, yy, 1, TextureTypes::Type2D, pb);

			//  rotate +90 deg, -y x  ----
			bleSize = xx;
			auto tb = img.getData(0), tb2 = imb.getData(0);

			for (int y = 0; y < yy; ++y)
			for (int x = 0; x < xx; ++x)
			{
				ColourValue cb = tb.getColourAt(x, y, 0, pb);
				// ColourValue cv = tb.getColourAt(xx-1 - x, yy-1 - y, 0, pf);
				// ColourValue cv = tb.getColourAt(x, y, 0, pf);
				ColourValue c2(cb.r, cb.g, cb.b, cb.a);
				tb2.setColourAt(c2, yy-1-y, x, 0, pb);
			}

			//  road  --------
			imr.load(String("roadDensity.png"), "General");
			const int xr = imr.getWidth(), yr = imr.getHeight();
			auto pr = imr.getPixelFormat();
			auto tr = imr.getData(0);
			isu.createEmptyImage(xr, yr, 1, TextureTypes::Type2D, pr);
			auto su = isu.getData(0);
			
			const ColourValue cv[6] = {
				ColourValue(1, 0, 0, 1),  // ter 0
				ColourValue(0, 1, 0, 1),  // ter 1
				ColourValue(0, 0, 1, 1),  // ter 2
				ColourValue(1, 1, 0, 1),  // ter 3
				ColourValue(0, 1, 1, 1),  // road
				ColourValue(1, 1, 1, 1)};

			//  surf  ----
			for (int y = 0; y < yr; ++y)
			for (int x = 0; x < xr; ++x)
			{
				ColourValue cr = tr.getColourAt(xr-1 - x, y, 0, pr);
				int xb = float(x)/(xr-1) * (xx-1);
				int yb = float(y)/(yr-1) * (yy-1);
				ColourValue cb = tb.getColourAt(xb, yb, 0, pb);

				int i = 0;
				if (cr.r < 0.5f)  i = 4;
				else if (cb.r > 0.5f)  i = 0;
				else if (cb.g > 0.5f)  i = 1;
				else if (cb.b > 0.5f)  i = 2;
				else /*if (cb.a > 0.5f)*/  i = 3;

				su.setColourAt(cv[i], x, y, 0, pr);
			}
			imb.save(bleFile, 0, 0);  // ter blend map
			isu.save(surfFile, 0, 0);  // road + ter surfaces
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in blendmap rotate: ") + ex.what());
		}
	}

	//  groundmodel.cfg  from  surfaces.cfg
	//  in tool, once for all tracks
	//------------------------------------------------------------

	//  landuse.cfg
	//------------------------------------------------------------
	string landFile = path + name + "-landuse.cfg";
	ofstream land;
	land.open(landFile.c_str(), std::ios_base::out);

	land << "[config]\n";
	land << "texture = " << name << "-surfaces.png\n";
	land << "\n";
	land << "#  default terrain to be used, if not using landuse its 'gravel'\n";
	land << "defaultuse = gravel\n";
	land << "\n";
	land << "#  friction configs, common for all sr tracks\n"; //, multiple lines for multiple files\n";
	// land << "loadGroundModelsConfig = " << name << "-surfaces.cfg\n";
	land << "loadGroundModelsConfig = " << "sr-surfaces.cfg\n";  // common for all sr tracks
	land << "\n";
	land << "#  colour to ground surface coupling\n";
	land << "[use-map]\n";

	//  get ter surf  ----
	string ter[4];
	int t = 0;
	for (auto i : td.layers)
		ter[t++] = td.layersAll[i].surfName;

	//  write  ----
	land << "0xffff0000 = " << "sr-" << ter[0] << "\n";
	if (!ter[1].empty())
	land << "0xff00ff00 = " << "sr-" << ter[1] << "\n";
	if (!ter[2].empty())
	land << "0xff0000ff = " << "sr-" << ter[2] << "\n";
	if (!ter[3].empty())
	land << "0xffffff00 = " << "sr-" << ter[3] << "\n";
	land << "0xff00ffff = " << "sr-" << sc->layerRoad[0].surfName << "\n";
	// land << "0xff00ffff = " << scn->roads[0].surf << "\n";

	land.close();


	//------------------------------------------------------------------------------------------------------------------------
	//  📄🏔️ Terrain layers setup  save  page.otc
	//------------------------------------------------------------------------------------------------------------------------
	int roadAdd = 1;  // 0 off  1 add road layer last - always some on terrain
	string opgFile = path + name + "-page-0-0.otc";
	ofstream lay;
	lay.open(opgFile.c_str(), std::ios_base::out);

	lay << name + ".raw\n";
	// layers += 1;  // on ter road last
	lay << layers + roadAdd << "\n";  // todo
	lay << "; worldSize,  Diffuse+Specular, Normal+Height,  blendmap, blendmapmode,  alpha\n";

	const char rgba[5] = "RGBA";
	String roadDS, roadNH;
	float roadTile = 5.f;
	
	const int ilr = cfg->roadTerTexLayer;  // ter layer for road
	const float mul = cfg->tileMul;  // scale
	
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];

		// ConvertTerTex(l.texFile, l.texNorm, path,
		// 			layTexDS[i], layTexNH[i], 0);
		
		lay << mul * l.tiling << " , " << layTexDS[i] + ", " + layTexNH[i] + ", " +  // 2 rgba png
			name + "-blendmap.png, " << rgba[i] << ", 0.99\n";

		if (i == ilr)
		{	roadDS = layTexDS[i];  roadNH = layTexNH[i];
			roadTile = l.tiling;
		}
	}
	//  if custom,  on last 5, 6 ter layer
	if (ilr >= 4)
	{
		const TerLayer& l = td.layersAll[ilr];
		// roadDS = l.texFile;  roadNH = l.texNorm;
		roadTile = l.tiling;

		ConvertTerTex(l.texFile, l.texNorm, path,
					roadDS, roadNH, copyTerTex);
		AddPackForTer(roadDS);
	}

	if (roadAdd && !roadDS.empty())
	{
		gui->Exp(CGui::NOTE, "Road layer: " + fToStr(roadTile)+" , "+roadDS+", "+roadNH);
		lay << roadTile << " , " << roadDS+", "+roadNH+", " +
			name + "-road.png, R, 0.99\n";
	}
	lay.close();


	//  🛣️ Road image,  blend last
	//------------------------------------------------------------
	Image2 img, im2;  // fixme outside road
	try
	{
		img.load(String("roadDensity.png"), "General");
	#if 0  //  doesnt work-
		img.flipAroundX();  // img.flipAroundY();
		img.save(path + name + "-road.png", 0, 0);
	#else
		im2.load(String("roadDensity.png"), "General");

		const int xx = img.getWidth(), yy = img.getHeight();
		TextureBox tb = img.getData(0), tb2 = im2.getData(0);
		auto pf = img.getPixelFormat();
		for (int y = 0; y < yy; ++y)
		for (int x = 0; x < xx; ++x)
		{
			ColourValue cv = tb.getColourAt(xx-1 - x, y, 0, pf);  // flip x
			ColourValue c2(1.f - cv.r, 0.f, 0.f, 1.f);  // invert, red only
			tb2.setColourAt(c2, x, y, 0, pf);
		}
		im2.save(path + name + "-road.png", 0, 0);  // road blend map
	#endif
	}
	catch (exception ex)
	{
		gui->Exp(CGui::WARN, string("Exception in road Dens flip: ") + ex.what());
	}
	int r = img.getWidth();
	gui->Exp(CGui::TXT, "Road image width: "+toStr(r));


	//  📄⛰️ Terrain hmap, setup  save  .otc
	//------------------------------------------------------------------------------------------------------------------------
	string otcFile = path + name + ".otc";
	ofstream otc;
	otc.open(otcFile.c_str(), std::ios_base::out);

	otc << "# Amount of pages in this terrain, 0 0 means only one page\n";
	otc << "PagesX=0\n";
	otc << "PagesZ=0\n";
	otc << "PageFileFormat=" + name + "-page-0-0.otc\n";
	otc << "\n";
	otc << "Heightmap.0.0.raw.size=" << size1 << "\n";
	otc << "Heightmap.0.0.raw.bpp=2\n";
	otc << "Heightmap.0.0.flipX=0\n";
	otc << "\n";
	otc << "# World size of terrain\n";
	otc << "WorldSizeX=" << XZsize << "\n";
	otc << "WorldSizeZ=" << XZsize << "\n";
	otc << "\n";
	otc << "# Heightmap values multiply factor\n";
	otc << "WorldSizeY=" << Ysize << "\n";
	otc << "\n";

	otc << "# Default size of blend maps for terrain. This is the resolution of each blending layer for a new terrain. default: 1024\n";
	otc << "LayerBlendMapSize=" << bleSize << "\n";
	otc << "\n";
	otc << "# disableCaching=1 will always enforce regeneration of the terrain, useful if you want to change the terrain config (.otc) and test it. Does not cache the objects on it.\n";
	otc << "disableCaching=1\n";
	otc << "\n";

	otc << "#optimizations ----\n";
	otc << "\n";
	otc << "# Must be 2^n+1. Minimum terrain tile size in vertices (at any LOD). default: 17\n";
	otc << "minBatchSize=17\n";
	otc << "\n";
	otc << "# Must be 2^n+1 and <= 65. Maximum terrain tile size in vertices (at any LOD). default: 65\n";
	otc << "maxBatchSize=65\n";
	otc << "\n";
	otc << "# Whether to support a light map over the terrain in the shader, if it's present (default true).\n";
	otc << "LightmapEnabled=0\n";  // no ?  no change-
	otc << "\n";
	otc << "# Whether to support normal mapping per layer in the shader (default true). \n";
	otc << "NormalMappingEnabled=1\n";  // yes
	otc << "\n";
	otc << "# Whether to support specular mapping per layer in the shader (default true). \n";
	otc << "SpecularMappingEnabled=0\n";  // idk DS,  no change-
	otc << "\n";

	otc << "# Whether to support parallax mapping per layer in the shader (default true). \n";
	otc << "ParallaxMappingEnabled=0\n";  // maybe, but no real H just computed
	otc << "\n";
	otc << "# Whether to support a global colour map over the terrain in the shader, if it's present (default true). \n";
	otc << "GlobalColourMapEnabled=0\n";  // no
	otc << "\n";
	otc << "# Whether to use depth shadows (default false). \n";
	otc << "ReceiveDynamicShadowsDepth=0\n";  // no ?
	otc << "\n";

	otc << "# Sets the default size of composite maps for a new terrain, default: 1024\n";
	otc << "CompositeMapSize=1024\n";
	otc << "\n";
	otc << "# Set the distance at which to start using a composite map if present, default: 4000\n";
	otc << "CompositeMapDistance=5000\n";  // off
	otc << "\n";
	otc << "# the default size of 'skirts' used to hide terrain cracks, default: 30\n";
	otc << "SkirtSize=10\n";  // ok
	otc << "\n";
	otc << "#  Sets the default size of lightmaps for a new terrain, default: 1024\n";
	otc << "LightMapSize=1024\n";
	otc << "\n";
	otc << "# Whether the terrain will be able to cast shadows, default: 0\n";
	otc << "CastsDynamicShadows=0\n";  // no
	otc << "\n";

	otc << "# Set the maximum screen pixel error that should  be allowed when rendering, default:\n";
	otc << "MaxPixelError=5\n";  // %  > ?
	otc << "\n";
	// otc << "# dump the blend maps to files named blendmap_layer_X.png\n";
	// otc << "DebugBlendMaps=0\n";

	otc.close();

	gui->Exp(CGui::INFO, "Terrain Time: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s\n");
}
