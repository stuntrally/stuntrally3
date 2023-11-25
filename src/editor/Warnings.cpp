#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
#include "Axes.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
// #include <OgreTerrain.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_EditBox.h>
using namespace Ogre;


///...........................................................................................................................
//  check track, and report warnings
///...........................................................................................................................

const static String clrWarn[CGui::WARN_ALL] = {"#F01010","#FF4040","#FFA040","#E0E040","#80F040","#60A0E0"};
const static String strWarn[CGui::WARN_ALL] = {"FATAL ", "ERR   ", "WARN  ", "Info  ", "Note  ", "Txt   "};
void CGui::Warn(eWarn type, String text)
{
	if (logWarn)
		LogO(strWarn[type]+text);
	else
		edWarn->addText(clrWarn[type]+text+"\n");
	if (type == FATAL || type == ERR || type == WARN)  ++cntWarn;  // count serious only
}
	
CGui::TrackWarn CGui::WarningsCheck(const Scene* sc, const std::vector<SplineRoad*>& vRoads)
{
	CGui::TrackWarn tw;
	if (!edWarn && !logWarn)  return tw;
	
	cntWarn = 0;
	if (!logWarn)
		edWarn->setCaption("");
	
	//  High Quality, for each component
	bool hqTerrain =0, hqHoriz =0, hqGrass =0, hqVeget =0,
		hqMainRoad =0, hqRoads =0, hqRiver =0,
		hqParticles =0, hqFluids =0, hqObjects =0;

	//  🛣️ Roads
	//----------------------------------------------------------------------------
	int roads = 0;  // cnt
	int roadMtrs = 0, pipeMtrs = 0;
	for (int r = 0; r < (int)vRoads.size(); ++r)
	{
		const auto road = vRoads[r];
		int cnt = road->getNumPoints();
		
		if (cnt < 2)
		{
			Warn(ERR, "---- Road: " + toStr(r) + " empty, has < 2 points");
			continue;  // nothing
		}
		bool river = road->IsRiver();
		if (river)  hqRiver = 1;
		++roads;
		if (river)
			Warn(INFO, "== River: " + toStr(r));
		else
			Warn(INFO, "~~ Road: " + toStr(r));

		
		//  count materials  todo: map by names
		bool mtrUsed[MTRs] ={0,0,0,0}, mtrPipe[MTRs] ={0,0,0,0};
		for (int i=0; i < road->mP.size(); ++i)
		{
			const SplinePoint& p = road->mP[i];
			if (p.idMtr >= 0 && p.idMtr < MTRs)
			if (p.pipe > 0.f)
				mtrPipe[p.idMtr] = true;
			else
				mtrUsed[p.idMtr] = true;
		}
		int rdm = 0, pim = 0;
		for (int i=0; i < MTRs; ++i)
		{	if (mtrUsed[i])  ++rdm;
			if (mtrPipe[i])  ++pim;
		}
		roadMtrs += rdm;
		pipeMtrs += pim;


		///-  🏁 start  ---------------------------------------
		if (r == 0)  // first main only
		{
			Vector3 stPos = Axes::toOgre(scn->sc->startPos[0]);  // todo: [1] too?
			Quaternion q1 = Axes::toOgre(scn->sc->startRot[0]);
			Vector3 vx,vy,vz;  q1.ToAxes(vx,vy,vz);  Vector3 stDir = -vx;
			Plane stPla(stDir, stPos);

			int num = road->getNumPoints();
			int iP1 = -1;  // find 1st chk id
			for (int i=0; i < num; ++i)
				if (road->mP[i].chk1st)
					iP1 = i;

			if (iP1 >= 0 && iP1 < cnt  && road->mP[iP1].chkR >= 1.f)
			{
				Vector3 ch1 = road->mP[iP1].pos;
				float d1 = stPla.getDistance(ch1);
				Warn(TXT,"Start to 1st check distance: "+fToStr(d1,2,4));
				if (d1 < 0.f)
					Warn(WARN,"Start isn't facing first checkpoint\n (wrong direction or first checkpoint), distance: "+fToStr(d1,2,4));
				//Warn(NOTE,"Check1 pos "+fToStr(ch0.x,2,5)+" "+fToStr(ch0.y,2,5)+" "+fToStr(ch0.z,2,5));

				//-  road dir  ----
				Vector3 pPrev = road->mP[(iP1 - road->iDir + cnt) % cnt].pos;
				float dPrev = stPla.getDistance(pPrev), diff = d1-dPrev;

				Warn(TXT,"Distance between 1st check and its prev point: "+fToStr(diff,2,4));
				if (diff < 0.f)
					Warn(WARN,"Road dir check wrong, road dir is likely opposite");
			}
			//Warn(TXT,"Start pos "+fToStr(stPos.x,2,5)+" "+fToStr(stPos.y,2,5)+" "+fToStr(stPos.z,2,5));
			//Warn(TXT,"Start dir "+fToStr(vx.x,3,5)+" "+fToStr(vx.y,3,5)+" "+fToStr(vx.z,3,5));


			//-  🏁 start pos  ----
			float tws = 0.5f * sc->tds[0].fTerWorldSize;  // 1st ter-
			if (stPos.x < -tws || stPos.x > tws || stPos.z < -tws || stPos.z > tws)
				Warn(ERR,"Start is outside Terrain 1 area - Whoa :o");
			
			//  won't work in tool..
			if (app && app->scn && !app->scn->ters.empty())
			{	
				{	float yt = app->scn->getTerH(stPos.x, stPos.z), yd = stPos.y - yt - 0.5f;
					//Warn(TXT,"Start to terrain distance "+fToStr(yd,1,4));
					if (yd < 0.f)   Warn(ERR, "Start below terrain - Whoa :o");
					if (yd > 0.3f)  Warn(INFO,"Start far above terrain\n (skip this if on bridge or in pipe), distance: "+fToStr(yd,1,4));
				}
				
				//-  other start places inside terrain (split screen)  ----
				for (int i=1; i < MAX_Players; ++i)
				{
					Vector3 p = stPos + i * stDir * 6.f;  //par dist
					float yt = app->scn->getTerH(p.x, p.z), yd = p.y - yt - 0.5f;
					String si = toStr(i);
									//Warn(TXT, "Car "+si+" start to ter dist "+fToStr(yd,1,4));
					//if (yd < 0.f)   Warn(WARN,"Car "+si+" start below terrain !");  // moved above in game
					if (yd > 0.3f)  Warn(INFO,"Car "+si+" start far above terrain");//\n (skips bridge/pipe), distance: "+fToStr(yd,1,4));
				}
			}			
			
			//-  🔵 first chk  ----
			if (iP1 < 0 || iP1 >= cnt)
				Warn(ERR,"First checkpoint not set  (use ctrl-0)");
			else
			if (road->mP[iP1].chkR < 0.1f)
				Warn(ERR,"First checkpoint not set  (use ctrl-0)");

			
			///-  road, 🔵 checkpoints  -------------
			int numChks = 0, iClosest=-1;  float stD = FLT_MAX;
			for (int i=0; i < road->mP.size(); ++i)
			{
				const SplinePoint& p = road->mP[i];
				if (p.chkR > 0.f && p.chkR < 1.f)
					Warn(WARN,"Too small checkpoint at road point "+toStr(i+1)+", chkR = "+fToStr(p.chkR,1,3));
				//.. in pipe > 2.f on bridge = 1.f
				
				if (p.chkR >= 1.f)
				{	++numChks;
					float d = stPos.squaredDistance(p.pos);
					if (d < stD) {  stD = d;  iClosest = i;  }
				}
			}
			if (numChks==0)
				Warn(ERR,"No checkpoints set  (use K,L on some road points)");
			if (numChks < 3)
				Warn(INFO,"Too few checkpoints, count "+toStr(numChks) + " - add more");
			

			//-  🏁 start width, height  ----
			float width = road->vStartBoxDim.z, height = road->vStartBoxDim.y;

			float rdW = 100.f;
			if (iClosest >= 0)  {  rdW = road->mP[iClosest].width;
				Warn(TXT,"Closest road point  width: "+fToStr(rdW,1,4)+",  distance "+fToStr(stPos.distance(road->mP[iClosest].pos),0,3));
			}
			if (width < 8.f || width < rdW * 1.4f)  //par, on bridge ok, pipe more..
				Warn(WARN,"Start  width small "+fToStr(width,0,2));
			if (height < 4.5f)
				Warn(WARN,"Start  height small "+fToStr(height,0,2));


			//-  🔵 road, chk cnt  ----
			float ratio = float(numChks)/cnt;
			Warn(TXT,"Road points to checkpoints ratio: "+fToStr(ratio,2,4)+"  = "+toStr(numChks)+"/"+toStr(cnt));
			if (ratio < 0.1f)  //par  // 1 chk for 10 points
				Warn(WARN,"Extremely low checkpoints ratio (< 0.1), add more");
			else if (ratio < 0.2f)  //par  1 chk for 5 points
				Warn(WARN,"Very few checkpoints ratio (< 0.2), add more");
			else if (ratio > 0.4f)  //par  1 chk for 3 points
				Warn(WARN,"High checkpoints ratio (> 0.4), remove not needed");
		}
		//-  🛣️ road points too far
		float len = road->st.Length;
		float ptLen = len/float(cnt);
		Warn(TXT,"Road length: "+fToStr(len,0,4)+ " points to length ratio: "+fToStr(ptLen,2,4));
		if (ptLen > 85.f)
			Warn(WARN,"Road points (on average) are very far\n (corners could behave sharp and straights become not straight).");
		else if (ptLen > 60.f)
			Warn(INFO,"Road points are far.");

		//-  🛣️ big road, merge len
		if (cnt > 200 && road->g_MergeLen < 600.f)
			Warn(INFO,"Road has over 200 points, use recommended merge length 600 or more.");
		else if (cnt > 120 && road->g_MergeLen < 300.f)
			Warn(INFO,"Road has over 120 points, use recommended merge length 300 or more.");
		else if (cnt > 50 && road->g_MergeLen < 80.f)
			Warn(INFO,"Road has over 50 points, use recommended merge length 80 or more.");
	}


	//-  🛣️ roads  ----
	if (hqRiver)  Warn(INFO, "HQ, River present :)");
	Warn(INFO, "Roads - count: " + toStr(roads));
	if (roads > 6)	Warn(WARN, "High roads count: " + toStr(roads) + " - not recommended");
	
	//-  🛣️ road materials  ----
	Warn(INFO, "Road materials used: " + toStr(roadMtrs) + " Pipe materials: " + toStr(pipeMtrs));
	int rdMtr = roadMtrs + pipeMtrs;
	if (rdMtr > 8)
		Warn(WARN, "High Road+Pipe materials used: " + toStr(rdMtr) + " - not recommended");

	
	///  ⛰️ Terrains
	//---------------------------------------------------------------
	int i = 0, ters = sc->tds.size();
	Warn(INFO, "---- Terrains - count: " + toStr(ters));
	for (const auto& td : sc->tds)
	{
		int horiz = td.iHorizon;
		if (horiz)  hqHoriz = 1;
		
		//  info
		Warn(INFO, "[] Terrain: " + toStr(i+1)+ " - Horizon: " + toStr(horiz));
		
		//  hmap filesize
		int sz = td.iVertsX * td.iVertsX * sizeof(float) / 1024/1024;
		if (td.iVertsX > 4000)
			Warn(ERR, "using huge 4k heightmap: "+toStr(td.iVertsX-1)+", file size is: "+toStr(sz)+" MB !!");
		else if (td.iVertsX > 2000)
			Warn(WARN, "using very big 2k heightmap: "+toStr(td.iVertsX-1)+", file size is: "+toStr(sz)+" MB !");

		if (td.iVertsX < 512)
			Warn(INFO,"Using too small < 512 heightmap: "+toStr(td.iVertsX-1));

		//-  🔺 tri size  ----
		float tri = td.fTriangleSize;
		if (tri < 0.9f && !horiz)	Warn(INFO, "triangle size < 0.9 Small:  "+fToStr(tri,2,4));
		if (tri > 1.7f && !horiz)	Warn(INFO, "triangle size > 1.7 Big: "+fToStr(tri,2,4)+" - not recommended");

		if (tri <  5.f && horiz == 1)	Warn(INFO, "horizon tri size < 5  Small: "+fToStr(tri,2,4));
		if (tri > 21.f && horiz == 1)	Warn(INFO, "horizon tri size > 21 Big: "+fToStr(tri,2,4)+" - not recommended, better to make 2nd");

		if (tri < 30.f && horiz == 2)	Warn(INFO, "horizon2 tri size < 30 Small: "+fToStr(tri,2,4));
		if (tri > 51.f && horiz == 2)	Warn(INFO, "horizon2 tri size > 51 Big: "+fToStr(tri,2,4)+" - not recommended");

		if (tri > 5.f && horiz == 0)  Warn(ERR,"big triangle but not Horizon > 0");

		///-  🏔️ ter layers  -------------
		int lay = 0;
		for (int l=0; l < TerData::ciNumLay; ++l)
			if (td.layersAll[l].on)  ++lay;
		Warn(NOTE, "Terrain - layers used: "+toStr(lay));

		hqTerrain = lay >= 4;
		if (hqTerrain)  Warn(INFO,"HQ Terrain");
		if (lay >= 5)  Warn(ERR,"Too many terrain layers used - max 4 are supported");
		if (lay <= 2)  Warn(INFO,"Too few terrain layers used");
		++i;
	}
	if (!hqHoriz && ters == 1){  Warn(WARN,"No Horizon terrain");  tw.horiz++;  }
	if (!hqHoriz && ters > 1){  Warn(WARN,"No Horizon terrain or no terrain marked as Horizon > 0");  tw.horiz++;  }
	if (hqHoriz)  Warn(INFO,"HQ Horizon");

	
	///  🌳🪨 Vegetation
	//---------------------------------------------------------------
	{
		//  layers  ----
		int veg = sc->densTrees > 0.f ? sc->vegLayers.size() : 0;
		Warn(NOTE,"---- Vegetation - models used: "+toStr(veg));  // works only when shown
		// if (sc->densTrees < 0.01f)  Warn(WARN,"No Vegetation - feels empty :(");
		hqVeget = veg >= 6;
		if (hqVeget)   Warn(INFO,"HQ Vegetation");
		if (veg >= 12)  Warn(WARN,"Too many models used - not recommended");  //par..
		if (veg <= 4)  Warn(INFO,"Too few models used - feels empty :(");
		
		//  total count
		//  todo: this needs getTerH & angle, without Terra loaded
		//  works only in ed (with veget on and at 1.0 mul), not in cmd tool
		int all = 0;
		for (auto l : sc->vegLayersAll)
		if (l.on)
		{
			all += l.cnt;
			if (l.cnt > 3000)  Warn(WARN,"High count of 1 model: "+l.name+" = "+toStr(l.cnt)+" - better to use 2 different models");
		}
		Warn(INFO,"Total Models count: "+toStr(all));
		if (all < 800)  Warn(WARN,"Low models count - feels empty :(");
		else if (all > 10000)  Warn(ERR,"Very high models count: "+toStr(all)+" - not recommended (low Fps on old GPUs)");  //par..
		else if (all > 7000)  Warn(WARN,"High models count: "+toStr(all));
	}

	//  🌿 Grass
	//---------------------------------------------------------------
	int gr = 0;  // cnt layers
	if (sc->densGrass > 0.01)
		for (int i=0; i < sc->ciNumGrLay; ++i)
			if (sc->grLayersAll[i].on)  ++gr;
	Warn(NOTE,"---- Grass - layers used: "+toStr(gr));
	hqGrass = gr >= 4;
	if (hqGrass)  Warn(INFO,"HQ Grass");
	if (gr >= 6)  Warn(WARN,"Too many grasses used (>= 6) - not recommended");
	if (gr <= 2)  Warn(INFO,"Too few grasses used (<= 2)");

	//..  page size small, dist big
	

	//  🌊 Fluids
	//---------------------------------------------------------------
	int fl = sc->fluids.size();
	Warn(NOTE,"---- Fluids - used: "+toStr(fl));
	if (fl > 6)  Warn(WARN,"Many fluids present (> 6), disable reflection on smaller");
	hqFluids = fl > 0;
	if (hqFluids)  Warn(INFO,"HQ, fluid present");
	
	//  📦 Objects
	//---------------------------------------------------------------
	// int objs = sc->objects.size();
	std::set<std::string> objVar;
	for (const auto& obj : sc->objects)
	{
		objVar.insert(obj.name);
		// todo: test if under terrains ..
	}
	int objs = objVar.size();
	Warn(NOTE,"---- Objects - used: "+toStr(objs));
	if (objs >= 60)  Warn(WARN,"Very many objects used (>= 60) - not recommended");
	if (objs <= 2)  Warn(INFO,"Too few objects placed (<= 2)");
	hqObjects = objs > 7;
	if (hqObjects)  Warn(INFO,"HQ objects (> 7 present)");

	//  🔥 Particles, clouds
	//---------------------------------------------------------------
	int pars = sc->emitters.size();
	Warn(NOTE,"---- Particles - used: "+toStr(pars));
	hqParticles = pars > 0;
	if (hqParticles)  Warn(INFO,"HQ, particles present");


	//  🏞️ Final Quality
	//---------------------------------------------------------------
	int hq = 0;
	if (hqTerrain) hq+=2;   if (hqHoriz)  ++hq;
	if (hqGrass)  ++hq;     if (hqVeget)  ++hq;
	if (hqMainRoad)  ++hq;  if (hqRoads)  ++hq;  if (hqRiver)  ++hq;
	if (hqParticles) ++hq;  if (hqFluids) ++hq;  if (hqObjects)  ++hq;
	tw.hq = hq;  tw.warn = cntWarn;

	Warn(NOTE,"---- HQ Overall: "+toStr(hq)+ "  of 10 max");
	if (hq > 8)
		Warn(INFO,"Quality very high (> 8)  (possibly low Fps) - check densities of models, objects, grasses, layers counts");
	else if (hq > 6)
		Warn(INFO,"Great quality (> 6) - but check for some optimisations");
	else if (hq < 5)
		Warn(WARN,"Low quality (< 5)  (ignore for deserts etc)\n - try to add something: layers, vegetation, fluids, objects, particles");
	

	///-  end  ----------------
	if (logWarn)
	{
		LogO("Warnings: "+toStr(cntWarn)+"\n");
		return tw;
	}
	bool warn = cntWarn > 0;
	if (!warn)
		Warn(NOTE,"#A0C0FF""No warnings.");
	else  //  show warn overlay
		txWarn->setCaption(TR("#{Warnings}: ")+toStr(cntWarn)+"  (alt-J)");

	txWarn->setVisible(warn);
	imgInfo->setVisible(!warn);  imgWarn->setVisible(warn);
	return tw;
}
