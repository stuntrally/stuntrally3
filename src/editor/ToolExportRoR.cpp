#include "pch.h"
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


//  gui events
void CGui::editRoRPath(Ed ed)
{
	pSet->pathExportRoR = ed->getCaption();
}
void CGui::btnExport(WP)
{
	app->ToolExportRoR();
}

//  Export current track for Rigs Of Rods
//------------------------------------------------------------------------------------------------------------------------
void App::ToolExportRoR()
{
	Ogre::Timer ti;
	const auto& sc = scn->sc;
	const auto& ror = sc->ror;  // scene.xml config vars for RoR

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
	const string name = pSet->gui.track;
	const string dirTrk = dirRoR + trk;
	if (!PATHS::CreateDir(dirTrk))
	{	gui->Exp(CGui::ERR, "Can't create track dir: "+dirTrk);
		return;
	}
	const string path = dirTrk + "/";  // main


	//  ⛰️ Heightmap convert to .raw
	//------------------------------------------------------------------------------------------------------------------------
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
	const float Ysize = (hmax - hmin);
	const float XZsize = size *  td.fTriangleSize,
		half = XZsize * 0.5f;  // half ter size offset

	gui->Exp(CGui::TXT, "Hmap size   " + toStr(size));
	gui->Exp(CGui::TXT, "Height min  " + toStr(hmin));
	gui->Exp(CGui::TXT, "Height max  " + toStr(hmax));
	gui->Exp(CGui::TXT, "World XZ size  " + toStr(XZsize));
	gui->Exp(CGui::TXT, "World Y size   " + toStr(Ysize));

	//  normalize  to 2B raw
	//  SR           ->  .raw     ->  RoR
	//  hmin .. hmax ->  0..65535 ->  0.f .. Ysize
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
	gui->Exp(CGui::TXT, "Terrain layers  " + toStr(td.layers.size()));

	string layTexDS[4], layTexNH[4];  // new filenames for ds,nh

	//  copy layer textures
	const int layers = std::min(4, (int)td.layers.size());
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];

		//  diffuse _d, normal _n, specular _s
		String pathTer = PATHS::Data() + "/terrain/";
		String d_d, d_s, d_r, n_n, n_s;
		d_d = l.texFile;  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");  // _s
		n_n = l.texNorm;  // _n
		n_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");  // _s
		
		string ext = 1 ? "png" : "dds";  // todo as dds fails..
		layTexDS[i] = StringUtil::replaceAll(l.texFile,"_d.jpg","_ds."+ext);
		layTexNH[i] = StringUtil::replaceAll(l.texNorm,"_n.jpg","_nh."+ext);

		gui->Exp(CGui::TXT, "layer " + toStr(i+1) + " diff, norm:  " + d_d + "  " + n_n);
		
		String diff = d_d, norm = n_n;
		string from, to;
		try
		{	//  copy _d _n
			from = pathTer + diff;  to = path + diff;
			if (!fs::exists(to.c_str()))
				fs::copy_file(from.c_str(), to.c_str());

			from = pathTer + norm;  to = path + norm;
			if (!fs::exists(to.c_str()))
				fs::copy_file(from.c_str(), to.c_str());
		}
		catch (const fs::filesystem_error & ex)
		{
			String s = "Error: Copying file " + from + " to " + to + " failed ! \n" + ex.what();
			gui->Exp(CGui::WARN, s);
		}

		//  find _s  for specular
		String spec = d_s;
		if (!PATHS::FileExists(pathTer + spec))
		{	spec = n_s;
			if (!PATHS::FileExists(pathTer + spec))
			{	spec = d_d;
				gui->Exp(CGui::TXT, "layer " + toStr(i+1) + " spec:  " + spec + "  " + fToStr(l.tiling));
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
				ColourValue a = tbS.getColourAt(x, y, 0, pfS) * 0.5f;  // par spec mul ..
				ColourValue ds(rgb.r, rgb.g, rgb.b, a.r);
				tbDS.setColourAt(ds, x, y, 0, pfA);
			}
			imDS.save(path + layTexDS[i], 0, 0);
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in blendmap flip: ") + ex.what());
		}

		//  combine Norm+H
		//------------------------------------------------------------
        Image2 imN, imH, imNH;
        imD.load(norm, "General");  //imS.load(spec, "General");
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
				// const float a = 0.f;
				const float a = max(0.f, 1.f - c.r - c.g);  // par side ?..
				ColourValue nh(c.r, c.g, c.b, a);
				tbNH.setColourAt(nh, x, y, 0, pfA);
			}
			imNH.save(path + layTexNH[i], 0, 0);
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in blendmap flip: ") + ex.what());
		}
	}

	//  🏔️ Blendmap  save as .png
	//------------------------------------------------------------
	int bleSize = 1024;
	if (scn->ters[0]->blendmap.texture)
	{
		auto bleFile = path + name + "-blendmap.png";
		// scn->ters[0]->blendmap.texture->writeContentsToFile(ble, 0, 0);
		
        Image2 img, im2;
        img.convertFromTexture(scn->ters[0]->blendmap.texture, 0, 0);
		im2 = img;
		try
		{	//  rotate +90 deg, -y x
			const int xx = img.getWidth(), yy = img.getHeight();
			bleSize = xx;
			TextureBox tb = img.getData(0), tb2 = im2.getData(0);
			auto pf = img.getPixelFormat();
			for (int y = 0; y < yy; ++y)
			for (int x = 0; x < xx; ++x)
			{
				ColourValue cv = tb.getColourAt(x, y, 0, pf);
				// ColourValue cv = tb.getColourAt(xx-1 - x, yy-1 - y, 0, pf);
				// ColourValue cv = tb.getColourAt(x, y, 0, pf);
				ColourValue c2(cv.r, cv.g, cv.b, cv.a);
				tb2.setColourAt(c2, yy-1-y, x, 0, pf);
			}
			im2.save(bleFile, 0, 0);  // ter blend map
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in blendmap flip: ") + ex.what());
		}
	}


	//  📄🏔️ Terrain layers setup  save  page.otc
	//------------------------------------------------------------------------------------------------------------------------
	int roadAdd = 1;  // par-  0 off  1 add road layer last
	string opgFile = path + name + "-page-0-0.otc";
	ofstream lay;
	lay.open(opgFile.c_str(), std::ios_base::out);

	lay << name + ".raw\n";
	// layers += 1;  // on ter road last
	lay << layers + roadAdd << "\n";  // todo
	lay << "; worldSize,  diffusespecular, normalheight,  blendmap, blendmapmode,  alpha\n";

	const char rgba[5] = "RGBA";
	string roadDiff = "", roadNorm = "";  // set.. which?
	float roadTile = 5.f;
	const float mul = ror.tileMul;  // scale
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];

		//  combined rgb,a from 2 tex
		// String diff = l.texFile;  // ends with _d
		// String norm = l.texNorm;  // _n
		// lay << l.tiling << " , " << diff+", "+norm+", " +  // 2 jpg
		// lay << l.tiling << " , _" << toStr(i)+".png, "+norm+", " +
		// lay << l.tiling << " , _" << toStr(i)+".png, _"+ toStr(i)+"_n.png, " +  // 2 png

		lay << mul * l.tiling << " , " << layTexDS[i] + ", " + layTexNH[i] + ", " +  // 2 rgba png
			name + "-blendmap.png, " << rgba[i] << ", 0.99\n";

		if (i == ror.roadTerTexLayer)  // par
		// {	roadDiff = diff;  roadNorm = norm;  // 2 jpg
		{	roadDiff = layTexDS[i];  roadNorm = layTexNH[i];
			roadTile = l.tiling;
		}
	}
	if (roadAdd && !roadDiff.empty())
	{
		gui->Exp(CGui::TXT, "Road layer: " + fToStr(roadTile)+" , "+roadDiff+", "+roadNorm);
		lay << mul * roadTile << " , " << roadDiff+", "+roadNorm+", " +
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
	otc << "LightmapEnabled=0\n";  // nope
	otc << "\n";
	otc << "# Whether to support normal mapping per layer in the shader (default true). \n";
	otc << "NormalMappingEnabled=1\n";  // yes
	otc << "\n";
	otc << "# Whether to support specular mapping per layer in the shader (default true). \n";
	otc << "SpecularMappingEnabled=1\n";  // idk DS
	otc << "\n";

	otc << "# Whether to support parallax mapping per layer in the shader (default true). \n";
	otc << "ParallaxMappingEnabled=0\n";  // no meh,  no real H just guessed
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
	int trkId = 0;  // N from ini
	int id = scn->data->tracks->trkmap[pSet->gui.track];
	if (id > 0)
	{	const TrackInfo& ti = scn->data->tracks->trks[id-1];

		authors = ti.author=="CH" ? "CryHam" : ti.author;
		trkId = ti.n;
	}else
		gui->Exp(CGui::ERR, "Track not in tracks.ini, no guid id or authors set.");
	

	//  🌊 Fluid
	//------------------------------------------------------------
	float Ywater = 0.f;
	int water = 0;
	switch (ror.water)
	{
	case -1:  // off
		water = 0;
		break;
	case 0:  // auto
		//  get 1 big for water level
		for (const auto& fl : sc->fluids)
		{
			if (!water && sc->fluids.size()==1 || fl.size.x > 200.f)  // pick 1st big
			{	water = 1;
				Ywater = fl.pos.y - hmin + ror.yWaterOfs;
		}	}
		break;
	case 1:  // force custom manual
		water = 1;
		Ywater = ror.yWaterOfs;  break;
	}
	gui->Exp(CGui::TXT, String("Water: ")+(water ? "yes" : "no")+"  Y level: "+fToStr(Ywater));


	//  ⛅ Caelum  sun light etc  save  .os
	//------------------------------------------------------------------------------------------------------------------------
	string osFile = path + name + ".os";
	ofstream os;
	os.open(osFile.c_str(), std::ios_base::out);

	os << "caelum_sky_system " + name + ".os\n";
	os << "{\n";
	os << "	// .75 = 6:00\n";
	os << "	julian_day 2458850\n";  //?
	os << "	time_scale 1\n";
	os << "	longitude 30\n";  // todo from sun dir ?
	os << "	latitude 10\n";
	os << "\n";
	os << "	point_starfield {\n";
	os << "		magnitude_scale 12.51189\n";
	os << "		mag0_pixel_size 16\n";
	os << "		min_pixel_size 4\n";
	os << "		max_pixel_size 6\n";
	os << "	}\n";
	os << "\n";
	os << "	manage_ambient_light true\n";
	os << "	minimum_ambient_light 0.05 0.05 0.1\n";
	os << "\n";
	os << "	manage_scene_fog yes\n";  // todo..
	os << "	ground_fog_density_multiplier 0.015\n";  // cg! no diff-
	os << "	scene_fog_density_multiplier 0.015\n";
	os << "\n";
	os << "	sun {\n";  // 🌞 sun light
	Vector3 la = sc->lAmb.GetRGB()  * ror.lAmb + ror.lAmbAdd;  // par bright
	Vector3 ld = sc->lDiff.GetRGB() * ror.lDiff;
	Vector3 ls = sc->lSpec.GetRGB() * ror.lSpec;
	os << "		ambient_multiplier "+fToStr(la.x,3,5)+" "+fToStr(la.y,3,5)+" "+fToStr(la.z,3,5)+"\n";
	os << "		diffuse_multiplier "+fToStr(ld.x,3,5)+" "+fToStr(ld.y,3,5)+" "+fToStr(ld.z,3,5)+"\n";
	os << "		specular_multiplier "+fToStr(ls.x,3,5)+" "+fToStr(ls.y,3,5)+" "+fToStr(ls.z,3,5)+"\n";
	os << "\n";
	os << "		auto_disable_threshold 0.05\n";
	os << "		auto_disable true\n";
	os << "	}\n";
	os << "\n";
	os << "	moon {\n";
	os << "		ambient_multiplier 0.2 0.2 0.2\n";
	os << "		diffuse_multiplier 1 1 0.9\n";
	os << "		specular_multiplier 1 1 1\n";
	os << "\n";
	os << "		auto_disable_threshold 0.05\n";
	os << "		auto_disable true\n";
	os << "	}\n";
	os << "\n";
	os << "	sky_dome {\n";
	os << "		haze_enabled yes\n";
	os << "		sky_gradients_image EarthClearSky2.png\n";
	os << "		atmosphere_depth_image AtmosphereDepth.png\n";
	os << "	}\n";
	os << "\n";

	//  clouds  factor from presets.xml
	auto* sky = scn->data->pre->GetSky(sc->skyMtr);
	float cld = sky ? sky->clouds : 0.2f;
	if (cld > 0.f)
	{
		os << "	cloud_system\n";
		os << "	{\n";
		os << "		cloud_layer low\n";
		os << "		{\n";
		os << "			height 2000\n";
		os << "			coverage "<< min(cld, 0.2f) <<"\n";
		os << "		}\n";
		if (cld >= 0.4f)
		{
			os << "		cloud_layer mid\n";
			os << "		{\n";
			os << "			height 3000\n";
			os << "			coverage "<< min(cld, 0.6f) <<"\n";
			os << "		}\n";
		}
		if (cld >= 0.7f)
		{
			os << "		cloud_layer high\n";
			os << "		{\n";
			os << "			height 4000\n";
			os << "			coverage "<< min(cld, 1.0f) <<"\n";
			os << "		}\n";
		}
		os << "	}\n";
	}
	os << "}\n";

	os.close();


	//  📄📦 Objects  save  .tobj
	//------------------------------------------------------------------------------------------------------------------------
	// ReadTxts("objects");  ReadTxts("objects2");  ReadTxts("objectsC");  ReadTxts("objects0");
	// ReadTxts("obstacles");  ReadTxts("rocks");  ReadTxts("rockshex");
	//- ReadTxts("skies");  
	// ReadTxts("grass");  ReadTxts("terrain");  ReadTxts("road");
	// ReadTxts("trees");  ReadTxts("trees2");   ReadTxts("trees-old");  //todo: replace-
	
	string objFile = path + name + ".tobj";
	ofstream obj;
	obj.open(objFile.c_str(), std::ios_base::out);

	int iodef = 0;
	std::map<string, int> once;
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
		
		//  object  save  .odef
		//------------------------------------------------------------
		string odefFile = path + o.name + ".odef";
		// if (!fs::exists(odefFile))
		if (once.find(o.name) != once.end())
		{
			once[o.name] = 1;
			ofstream odef;
			odef.open(odefFile.c_str(), std::ios_base::out);
			
			odef << o.name + ".mesh\n";
			odef << "1, 1, 1\n";
			odef << "\n";
			odef << "beginmesh\n";
			odef << "mesh " + o.name + ".mesh\n";
			odef << "stdfriction concrete\n";
			odef << "endmesh\n";
			odef << "\n";
			odef << "end\n";
			
			odef.close();  ++iodef;
		}
		// todo: once copy .mesh, texture, material ...
		// from old SR dir ..
	}
	obj.close();

	gui->Exp(CGui::TXT, "Objects: "+toStr(sc->objects.size())+"  odef: "+toStr(iodef));


	//  🌳🪨 Vegetation setup  save  .tobj .png
	//------------------------------------------------------------------------------------------------------------------------
	const bool veget = 1;  // par.. not in FPS variant
	if (veget)
	{
		string vegFile = path + name + "-veget.tobj";
		ofstream veg;
		veg.open(vegFile.c_str(), std::ios_base::out);

		string matFile = path + name + "-veget.material";
		ofstream mat;
		mat.open(matFile.c_str(), std::ios_base::out);


		//  🌿 Grass layers
		//------------------------------------------------------------------------------------------------------------------------
		veg << "// format:  grass range,  SwaySpeed, SwayLength, SwayDistribution,   Density,  minx, miny, maxx, maxy,   fadetype, minY, maxY,   material colormap densitymap\n";

		const SGrassLayer* g0 = &sc->grLayersAll[0];
		for (int i=0; i < sc->ciNumGrLay; ++i)
		{
			const SGrassLayer* gr = &sc->grLayersAll[i];
			if (gr->on)
			{
				const SGrassChannel* ch = &scn->sc->grChan[gr->iChan];
				string mapName = name + "-grass"+toStr(gr->iChan)+".png";

				// ch->angMin, ch->angMax
				//grass 600,  0.5, 0.15, 10,  0.3, 0.3, 0.3, 1.2, 1.2,  1, 10, 0, grass4 RoRArizona-SCRUB1.dds RoRArizona-VEGE1.dds
				//grass 200,  0.5, 0.05, 10,  0.1, 0.2, 0.2, 1, 1,  1, 0, 9, seaweed none none
				//grass 200,  0.5, 0.05, 10,  0.3, 0.2, 0.2, 1, 1,  1, 10, 0, grass1 aspen.jpg aspen_grass_density.png

				//  range,
				veg << "grass " << "300,  ";  // par mul
				//  Sway: Speed, Length, Distribution,
				veg << "0.5, 0.1, 10,  ";  
				
				//  Density,  minx, miny, maxx, maxy,
				veg << gr->dens * sc->densGrass * ror.grassMul << ",  ";
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
					const Real tws = sc->tds[0].fTerWorldSize, hws = tws * 0.5f;  //par-
					// const Real mrg = 0.98f;
					const auto* terrain = scn->ters[0];

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

				//  copy grass*.png
				//------------------------------------------------------------
				String pathGrs = PATHS::Data() + "/grass/";
				String grassPng = gr->material + ".png";  // same as mtr name
				string from, to;
				try
				{	//  copy _d _n
					from = pathGrs + grassPng;  to = path + grassPng;
					if (!fs::exists(to.c_str()))
						fs::copy_file(from.c_str(), to.c_str());
				}
				catch (const fs::filesystem_error & ex)
				{
					String s = "Error: Copying file " + from + " to " + to + " failed ! \n" + ex.what();
					gui->Exp(CGui::WARN, s);
				}				
				
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
			}
		}

		mat.close();

		//  🌳🪨 Vegetation
		//------------------------------------------------------------------------------------------------------------------------

		//  veget map
		//------------------------------------------------------------
		/*Image2 img, im2;
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
				ColourValue cv = tb.getColourAt(xx-1 - x, y, 0, pf);  // flip x
				cv.g = cv.b = cv.r;  // white
				tb.setColourAt(cv, x, y, 0, pf);
			}

			im2.save(path + name + "-grass"+toStr(0)+".png", 0, 0);
		}
		catch (exception ex)
		{
			gui->Exp(CGui::WARN, string("Exception in grass dens map: ") + ex.what());
		}*/


		//  Veget Layers  ------------------------------------
		for (size_t l=0; l < sc->vegLayers.size(); ++l)
		{
			VegetLayer& vg = sc->vegLayersAll[sc->vegLayers[l]];
			String file = vg.name;  //, fpng = file+".png";
			// vg.cnt = 0;

			//  copy mesh from old SR ?  or convert v2 to v1-
		}

		#if 0
			//  check ter angle  ------------
			float ang = ter0->getAngle(pos.x, pos.z, td.fTriangleSize);
			if (ang > vg.maxTerAng)
				add = false;

			// if (!add)  LogO("ter ang");
			if (!add)  continue;  //

			//  check ter height  ------------
			bool in = ter0->getHeightAt(pos);
			// LogO(fToStr(pos.y));
			if (!in)  add = false;  // outside
			
			if (pos.y < vg.minTerH || pos.y > vg.maxTerH)
				add = false;
			
			// if (!add)  LogO("ter h");
			if (!add)  continue;  //
			
			//  check if in fluids  ------------
			float fa = sc->GetDepthInFluids(pos);
			if (fa > vg.maxDepth)
				add = false;

			// if (!add)  LogO("in fl");

			//  check if on road - uses roadDensity.png
			if (imgRoad && r > 0)  //  ----------------
			{
			int mx = (0.5*tws + pos.x)/tws*r,
				my = (0.5*tws + pos.z)/tws*r;

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
						xx = std::max(0,std::min(r-1, my+ii)),
						yy = std::max(0,std::min(r-1, mx+jj));

					const float cr = tb.getColourAt(
						xx, yy, 0, Ogre::PFG_RGBA8_UNORM_SRGB ).r;
					
					if (cr < 0.75f)  // par-
					{
						rr = abs(ii)+abs(jj);
						//rr = sqrt(float(ii*ii+jj*jj));  // much slower
						rmin = std::min(rmin, rr);
					}
				}
				if (rmin <= c)
					add = false;

				if (bMax && /*d > 1 &&*/ rmin > d-1)  // max dist (optional)
					add = false;
			}
			// if (!add)  LogO("on rd");
			if (!add)  continue;  //

			//  check if outside of terrain?
			// if (pos.x < 
			
			// if (!add)  continue;  //


			//  **************  add  **************
			++all;
		#endif

		// veg << "";
		// trn << ""+name+"-veget.tobj=\n";
		// trees yawFrom, yawTo, scaleFrom, scaleTo, highDensity, distance1, distance2, meshName colormap densitymap
		//trees 0, 360, 0.1, 0.12, 2, 60, 3000, fir05_30.mesh aspen-test.dds aspen_grass_density2.png 
		//  todo save densitymap  0 blk .. 1 wh

		// Trees

		//trees yawFrom, yawTo, scaleFrom, scaleTo, highDensity, distance1, distance2, meshName colormap densitymap 
		// trees 0, 360, 0.1, 0.12, 2, 60, 3000, fir05_30.mesh aspen-test.dds aspen_grass_density2.png 

		veg.close();
	}


	//  🛣️ Road  points
	//------------------------------------------------------------------------------------------------------------------------
	std::vector<string> chks;
	const bool roadtxt = !scn->roads.empty();
	const bool road = roadtxt && scn->roads[0]->getNumPoints() > 2;
	bool hasRoad = 0;
	if (road)
	{	const auto& rd = scn->roads[0];

		//  if all points on ter, skip road, is on blendmap
		bool roadOnTer = 1;
		for (int i=0; i < rd->getNumPoints(); ++i)
			if (!rd->getPoint(i).onTer)
				roadOnTer = 0;

		//  has some bridges  ------------------------
		if (!roadOnTer)
		{	hasRoad = 1;
			string roadFile = path + name + "-road.tobj";
			ofstream trd;
			trd.open(roadFile.c_str(), std::ios_base::out);

			trd << "//  position x,y,z   rotation rx,ry,rz,   width,   border width, border height,  type\n";
			bool begin = 0;  int iroads = 0;

			for (int i=0; i < rd->getNumPoints() + 1; ++i)  // loop it
			{
				//  i0,1
				const int i0 = rd->getAdd(i,0), i1 = rd->getNext(i);
				const auto& p = rd->getPoint(i0), p1 = rd->getPoint(i1);
				bool bridge = !p.onTer || !p1.onTer;

				if ( (i==0 || !begin) && bridge)
				{	begin = 1;  ++iroads;
					trd << "begin_procedural_roads\n";
					// gui->Exp(CGui::TXT, "Road begin");
				}

				//  length steps  |
				Real len = rd->GetSegLen(i0);
				const float fLenDim = ror.roadStepDist;  // par ! quality, points density

				// gui->Exp(CGui::TXT, "Road i0: "+toStr(i0)+"  ter: "+toStr(p.onTer?1:0) +"  l "+fToStr(len) );

				//  add points on bridge  ----------
				if (begin)
				{
					//  len steps
					// const int il = 2;  // const LQ
					const int il = 1 + (len / fLenDim);  // var, by dist
					const int ila = il + (!bridge ? 1 : 0);  // will end, all len
					// gui->Exp(CGui::TXT, "Road il: "+toStr(il) );
					
					for (int l = 0; l < ila; ++l)
					{	//  pos
						float fl = l;
						Vector3 vP = rd->interpolate(i0, fl / il);
						Vector3 vP1 = rd->interpolate(i0, (fl+1.f) / il);
						Vector3 dir = vP1 - vP;  // along length

						// float width = p.width + 2.f;  //-
						const float width = rd->interpWidth(i0, fl / il);

						// if (p.onTer)  // ?
						// 	vP.y = scn->getTerH(vP.x, vP.z);
						vP.y = ror.roadHadd;  // lower for entry  // -0.9f + rd->g_Height;

						//  rot y
						float yaw = TerUtil::GetAngle(dir.x, dir.z) *180.f/PI_d - 45.f - 15.f;  // par ? 45

						//  write  ------
						//  pos
						trd << half - vP.z << ", " << vP.y - hmin << ", " << vP.x + half << ",   ";
						//  rot
						trd << "0, " << yaw << ", 0,  ";  // todo  p.aRoll
						trd << width << ",   ";
						
						//  bridge
						/*if (p.onTer)
							trd << "0.5,  0.2,  flat\n";
						else*/
						/*if (p.cols)  // too much, bad
							trd << "0.6,  1.0,  bridge\n";
						else*/
							// trd << "0.6,  1.0,  bridge_no_pillars\n";
							trd << ror.wallX << ",  " << (p.onTer ? 0.f : ror.wallY) << 
								(ror.roadCols ? ",  bridge\n" : ",  bridge_no_pillars\n");
				}	}

	
				if (begin && !bridge)
				{	begin = 0;
					trd << "end_procedural_roads\n";
					// gui->Exp(CGui::TXT, "Road end");
				}
			}
			if (begin)
				trd << "end_procedural_roads\n";

			/*	0,0,0,         0,0,0,    10.0,  0,            0,             flat
				0,0,0,         0,0,0,    10.0,  0.25,         1.0,           both
				flat - none
				left, right, both - borders
				bridge, bridge_no_pillars
			*/
			trd.close();
			gui->Exp(CGui::TXT, "Roads: "+toStr(iroads));
		}

		//  get checkpoints
		for (int i=0; i < rd->getNumPoints() + 1; ++i)  // loop it
		{
			const auto& p = rd->getPoint(i);
			if (p.chkR > 0.1f)
			{
				// Vector3 vP = p.pos;
				Vector3 vP = rd->interpolate(i, 0.f);
				Vector3 vP1 = rd->interpolate(i, 0.2f);  // par-
				Vector3 dir = vP1 - vP;  // along length
				float yaw = TerUtil::GetAngle(dir.x, dir.z) *180.f/PI_d - 45.f - 15.f;  // par ? 45
				// pos 
				string s = fToStr(half - vP.z) + ", " + fToStr(vP.y - hmin) + ", " + fToStr(vP.x + half) + ",   ";
				// rot
				s += "0.0,  " + fToStr(yaw) + ",  0.0";
				chks.push_back(s);
		}	}
	}

	//  Road script  checks
	//------------------------------------------------------------
	string asFile = path + name + ".as";
	ofstream as;
	as.open(asFile.c_str(), std::ios_base::out);

	as << "#include \"base.as\"\n";
	as << "#include \"races.as\"\n";
	as << "\n";
	as << "racesManager races();\n";
	as << "\n";
	as << "void createRaces()\n";
	as << "{\n";
	as << "	//races.showCheckPointInfoWhenNotInRace = true;\n";
	as << "\n";
	as << "	races.setCallback(\"RaceFinish\", on_raceFinish);\n";
	as << "	\n";
	as << "	double[][] race_autocross = \n";
	as << "		{\n";
#if 1
	int i = 0, n = chks.size();
	for (auto ch : chks)
	{
		bool last = i == n-1;  // no ,
		as << "			{ " << ch << (last ? " }\n" : " },\n");
		++i;
	}
#else
	as << "			{916.0,     9.0,   454.0,     0.0,     0,  0.0},\n";
	as << "			{871.0,     9.0,   475.0,     0.0,   180,  0.0},\n";
	as << "			{897.0,     9.0,   524.0,     0.0,  -120,  0.0}\n";
#endif
	as << "		};\n";
	as << "	int raceID = races.addRace(\"Autocross\", race_autocross, races.LAPS_Unlimited, \"31-checkpoint\", \"31-checkpoint\", \"31-checkpoint\", \"CryHam-1.0\");\n";
	as << "	\n";
	as << "	races.setVersion(raceID, \"CryHam-1.0\");\n";
	as << "	races.finalize(raceID);\n";
	as << "}\n";
	as << "\n";
	as << "void main()\n";
	as << "{\n";
	as << "	createRaces();\n";
	as << "	//races.submitScore(false);\n";
	as << "}\n";
	as << "\n";
	as << "void eventCallback(int eventnum, int value)\n";
	as << "{\n";
	as << "	// required for the races!\n";
	as << "	races.eventCallback(eventnum, value);\n";
	as << "}\n";
	as << "\n";
	as << "//  finishing a race\n";
	as << "void on_raceFinish(dictionary@ info)\n";
	as << "{\n";
	as << "    int raceID;\n";
	as << "    info.get(\"raceID\", raceID);\n";
	as << "    \n";
	as << "    float lapTime = game.getTime() - races.lapStartTime;\n";
	as << "    float raceTime = game.getTime() - races.raceStartTime;\n";
	as << "    \n";
	as << "    // Race specific stuff\n";
	as << "    if(raceID == races.getRaceIDbyName(\"Autocross\"))\n";
	as << "    {\n";
	as << "        // Calculate the average speeds (0.25 mile = 0.40... kilometers)\n";
	as << "        float averageSpeed_mph = 0.25 / lapTime * 3600;\n";  // todo road length * 0.6
	as << "        float averageSpeed_kph = 0.402336 / lapTime * 3600;\n";
	as << "        \n";
	as << "        game.message(\"Average speed: \" + averageSpeed_mph + \" mph (\" + averageSpeed_kph + \" km/h).\", \"chart_bar.png\", 30000, true);\n";
	as << "    }\n";
	as << "}  \n";
	as << "\n";

	as.close();


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
	trn << "Version = 1\n";
	// todo, random hash from trk name?
	trn << "GUID = 11223344-5566-7788-" << fToStr(trkId,0,4,'0') <<"-012345678901\n";
	trn << "\n";
	//  if has groundmodel, define landuse file
	trn << "#TractionMap = landuse.cfg\n";  // todo  surfaces.cfg
	trn << "\n";

	trn << "[Authors]\n";
	trn << "Authors = " + authors + "\n";
	trn << "Conversion = Exported from Stunt Rally 3 Track Editor, version: " << SET_VER << "\n";
	if (roadtxt)
	{
		auto& rd = scn->roads[0];  // extra info from SR3 track
		auto len = rd->st.Length;  // road stats

		trn << "stat1 = " << "Length: " <<  fToStr(len * 0.001f,2,4) << " km  /  " << fToStr(len * 0.000621371f,2,4) << " mi\n";
		trn << "stat2 = " << "Width average: " << fToStr(rd->st.WidthAvg,1,3) << " m\n";
		trn << "stat3 = " << "Height range: " << fToStr(rd->st.HeightDiff,0,3) << " m\n";
		trn << "stat4 = " << "on Terrain: " << fToStr(rd->st.OnTer,0,3) << " %\n";
		// trn << "stat5 = " << "bank_angle_avg: " << fToStr(rd->st.bankAvg,0,2) << "\n";
		trn << "stat5 = " << "bank_angle_max: " << fToStr(rd->st.bankMax,0,2) << "'\n";

		trn << "Description = "+rd->sTxtDescr+"\n";  // text
		trn << "drive_Advice = "+rd->sTxtAdvice+"\n";
	}
	trn << " \n";

	trn << "[Objects]\n";
	if (hasRoad)
		trn << name+"-road.tobj=\n";
	if (veget)
		trn << name+"-veget.tobj=\n";
	//if (objs)
	//	trn << name+".tobj=\n";
	trn << "\n";

	trn << "[Scripts]\n";
	trn << name + ".as=\n";

	//  if has race script define .terrn.as
	// trn << "#"+name+".terrn.as=\n";

	trn.close();


	gui->Exp(CGui::INFO, "Export to RoR end. Time: " + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
