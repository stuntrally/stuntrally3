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
#include "Axes.h"
#include "Road.h"
#include <Terra.h>
#include <OgreImage2.h>

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

	//  normalize  to 2B raw:  0..65535
	for (int y=0; y < size; ++y)
	{
		int a = y * size;
		for (int x=0; x < size; ++x,++a)
			hmap[x*size1 + y] =  // flip x-y
				(td.hfHeight[a] - hmin) * hsc * 32767.f + 32767.f;
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
				ColourValue a = tbS.getColourAt(x, y, 0, pfS) * 0.1f;  // par spec mul
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
				const float a = 0.f;
				// const float a = max(0.f, 1.f - c.r - c.g);  // side
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
	int roadAdd = 1;  // 0 off  1 add road layer last
	string opgFile = path + name + "-page-0-0.otc";
	ofstream lay;
	lay.open(opgFile.c_str(), std::ios_base::out);

	lay << name + ".raw\n";
	// layers += 1;  // on ter road last
	lay << layers + roadAdd << "\n";  // todo
	lay << "; worldSize, diffusespecular, normalheight, blendmap, blendmapmode, alpha\n";

	const char rgba[5] = "RGBA";
	string roadDiff = "", roadNorm = "";  // set.. which?
	float roadTile = 5.f;
	const float mul = 1.f;  // tile par
	for (int i=0; i < layers; ++i)
	{
		const TerLayer& l = td.layersAll[td.layers[i]];
		// di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String pathTerTex = PATHS::Data() + "/terrain/";
		String diff = l.texFile;  // ends with _d
		String norm = l.texNorm;  // _n

		// if (i==0)
		// 	lay << l.tiling << " , " << diff+", "+norm+"\n";
		// else if (i == layers-1)  // last- road
		// 	lay << l.tiling << " , " << diff+", "+norm+", " +
		// 		name + "-road.png, R, 0.99\n";
		// else
			// lay << l.tiling << " , " << diff+", "+norm+", " +
			// lay << l.tiling << " , _" << toStr(i)+".png, "+norm+", " +
			// lay << l.tiling << " , _" << toStr(i)+".png, _"+ toStr(i)+"_n.png, " +
			lay << mul * l.tiling << " , " << layTexDS[i] + ", " + layTexNH[i] + ", " +
				name + "-blendmap.png, " << rgba[i] << ", 0.99\n";

		if (i == 1)  // todo gui, idk
		{	roadDiff = diff;
			roadNorm = norm;
			roadTile = l.tiling;
		}
	}
	if (roadAdd && !roadDiff.empty())
	{
		gui->Exp(CGui::TXT, "Road layer: " + fToStr(roadTile)+" , "+roadDiff+", "+roadNorm);
		lay << roadTile << " , " << roadDiff+", "+roadNorm+", " +
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
	gui->Exp(CGui::TXT, "Road image width: "+fToStr(r));


	//  📄⛰️ Terrain hmap, setup  save  .otc
	//------------------------------------------------------------------------------------------------------------------------
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
	otc << "LayerBlendMapSize=" << bleSize << "\n";
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
	otc << "LightmapEnabled=0\n";  // nope
	otc << "\n";
	otc << "# Whether to support normal mapping per layer in the shader (default true). \n";
	otc << "NormalMappingEnabled=1\n";  // yes
	otc << "\n";
	otc << "# Whether to support specular mapping per layer in the shader (default true). \n";
	otc << "SpecularMappingEnabled=1\n";  // idk DS
	otc << "\n";
	otc << "# Whether to support parallax mapping per layer in the shader (default true). \n";
	otc << "ParallaxMappingEnabled=0\n";  // no, NH meh
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
	otc << "SkirtSize=10\n";
	otc << "\n";
	otc << "#  Sets the default size of lightmaps for a new terrain, default: 1024\n";
	otc << "LightMapSize=1024\n";
	otc << "\n";
	otc << "# Whether the terrain will be able to cast shadows, default: 0\n";
	otc << "CastsDynamicShadows=0\n";  // no
	otc << "\n";
	otc << "# Set the maximum screen pixel error that should  be allowed when rendering, default:\n";
	otc << "MaxPixelError=5\n";  // > ?
	otc << "\n";
	// otc << "# dump the blend maps to files named blendmap_layer_X.png\n";
	// otc << "DebugBlendMaps=0\n";

	otc.close();


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
		gui->Exp(CGui::ERR, "Track not in tracks.ini, no id or authors set.");
	

	//  🌊 Fluids  get 1 big for water level
	//------------------------------------------------------------
	int water = 0;  float Ywater = 0.f;
	for (const auto& fl : sc->fluids)
	{
		if (!water && sc->fluids.size()==1 || fl.size.x > 200.f)  // pick 1st big
		{	water = 1;
			Ywater = fl.pos.y + Ysize;  // todo fix ?
	}	}
	gui->Exp(CGui::TXT, String("Water: ")+(water ? "yes" : "no")+"  Y level: "+fToStr(Ywater));


	//  🏞️⛅ Track/map setup  save  .terrn2
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
	trn << "AmbientColor = 0.99, 0.98, 0.97\n";  // todo  sc->lAmb  unused?
	//  ror = sr
	//  0, y, 0        = -470, y, 460
	//  959, 340 y, 950 = 487, y, -472
	Vector3 st = Axes::toOgre(sc->startPos[0]);
	trn << "StartPosition = " << fToStr(half - st.z)+", "+fToStr(st.y + Ysize)+", "+fToStr(st.x + half)+"\n";
	trn << "\n";

	trn << "#CaelumConfigFile = \n";  // todo  caelum.os
	trn << "SandStormCubeMap = tracks/skyboxcol\n";  // sky meh- dome?
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
	trn << "authors = " + authors + "\n";
	trn << "conversion = Exported from Stunt Rally 3 Track Editor, version: " << SET_VER << "\n";
	bool road = !scn->roads.empty() && scn->roads[0]->getNumPoints() > 2;
	if (road)
	{
		auto& rd = scn->roads[0];  // extra info from sr3 track
		trn << "description = "+rd->sTxtDescr+"\n";
		trn << "drive_advice = "+rd->sTxtAdvice+"\n";
	}
	trn << " \n";

	trn << "[Objects]\n";
	// trn << ""+name+".tobj=\n";
	// trn << ""+name+"-veget.tobj=\n";  // todo
	if (road)
		trn << ""+name+"-road.tobj=\n";
	trn << "\n";

	trn << "#[Scripts]\n";  // todo  road, checks
	//  if has race script define .terrn.as
	trn << "#"+name+".terrn.as=\n";

	trn.close();


	//  📄📦 Objects  save  .tobj
	//------------------------------------------------------------------------------------------------------------------------
	string objFile = path + name + ".tobj";
	ofstream obj;
	obj.open(objFile.c_str(), std::ios_base::out);

	int iodef = 0;
	std::map<string, int> once;
	for (const auto& o : sc->objects)
	{
		Vector3 p = Axes::toOgre(o.pos);
		auto q = Axes::toOgreW(o.rot);

		obj << fToStr(half - p.z)+", "+fToStr(p.y + Ysize)+", "+fToStr(p.x + half)+", ";
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
	}
	obj.close();

	gui->Exp(CGui::TXT, "Objects: "+toStr(sc->objects.size())+"  odef: "+toStr(iodef));


	//  grass
	//------------------------------------------------------------------------------------------------------------------------

	//  vegetation?..
	// scn->vegetNodes;
	// trn << ""+name+"-veget.tobj=\n";


	gui->Exp(CGui::INFO, "Export to RoR end.");
}
