#include "pch.h"
#include "Def_Str.h"
#include "GuiCom.h"
#include "SceneXml.h"
#include "CData.h"
#include "TracksXml.h"
#include "BltObjects.h"
#include "PresetsXml.h"
#include "paths.h"
#include "Road.h"
#ifndef SR_EDITOR
	#include "CGame.h"
	#include "CHud.h"
	#include "CGui.h"
	#include "timer.h"
	#include "game.h"
#else
	#include "CScene.h"
	#include "CApp.h"
	#include "CGui.h"
	#include "settings.h"
#endif
#include <OgreTimer.h>
#include <OgreResourceGroupManager.h>
#include <OgreStagingTexture.h>
#include <MyGUI_ComboBox.h>
using namespace Ogre;
using namespace std;
using namespace MyGUI;


#ifdef SR_EDITOR

//--------  Editor Tools  --------

///  _Tool_	Warnings  ......................................................
///  check all tracks for warnings
///  Warning: takes about 16 sec
void CGui::ToolTracksWarnings()
{
	Ogre::Timer ti;
	LogO("ALL tracks warnings ---------\n");
	logWarn = true;

	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	//  foreach track
		string trk = data->tracks->trks[i].name, path = gcom->pathTrk[0] +"/"+ trk +"/";
		/**/if (!(trk[0] >= 'A' && trk[0] <= 'Z'))  continue;
		/**/if (StringUtil::startsWith(trk,"test"))  continue;

		Scene sc;  sc.LoadXml(path +"scene.xml");
		SplineRoad rd(app);  rd.LoadFile(path +"road.xml");
		
		LogO("Track: "+trk);
		WarningsCheck(&sc,&rd);
	}
	LogO(String("::: Time ALL tracks: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	LogO("ALL tracks warnings ---------");
}


//  update all Brushes prv png  ......................................................
// todo: brush presets in xml, auto upd
void App::ToolBrushesPrv()
{
	Ogre::Timer ti;
	const uint si = 2048;  // 16 * 128  BrPrvSize
	uint32* data = new uint32[BrPrvSize * BrPrvSize];

	Image2 im;
	im.createEmptyImage(si, si, 1, TextureTypes::Type2D, PFG_RGBA8_UNORM);
	uint32* big = (uint32*)im.getRawBuffer();
	memset(big, 0, si*si);

	uint u = 0, v = 0;
	for (int i=0; i < brSetsNum; ++i)
	{
		SetBrushPreset(i, 0);
		updBrushData((uint8*)data);
		
		//  store prv in big
		uint a = 0;
		for (uint y=0; y < BrPrvSize; ++y)
		{
			uint b = u + (v + y) * si;
			for (uint x=0; x < BrPrvSize; ++x)
				big[b++] = data[a++];
		}
		u += BrPrvSize;  // next u,v prv pos
		if (u >= si)
		{	u = 0;  v += BrPrvSize;
			if (v >= si)
			{	LogO("!No more room for brushes on size: "+toStr(si));
				break;  // max 256
		}	}
	}

	im.save(PATHS::UserConfigDir()+"/brushes.png", 0, 0);
	delete[] data;
	LogO(String("::: Time ALL brush: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  // < 1s
}


///............................................................................................................................
///  _Tool_ scene
///  check/resave all tracks scene.xml 
///............................................................................................................................
void CGui::ToolSceneXml()
{
	//Ogre::Timer ti;
	LogO("ALL tracks scene ---------");
	std::map<string, int> noCol,minSc;
	ResourceGroupManager& rg = ResourceGroupManager::getSingleton();

	int i,n;
	for (auto trkinf : data->tracks->trks)
	{
		//  load each track xmls
		string trk = trkinf.name, path = gcom->pathTrk[0] +"/"+ trk +"/";
		Scene sc;  sc.LoadXml(path +"scene.xml");
		SplineRoad rd(app);  rd.LoadFile(path +"road.xml");
		bool modif = false;

		int l = 20 - trk.length();  // align
		for (n=0; n < l; ++n)  trk += " ";

		///  sound  ---
		if (sc.sReverbs=="")
			LogO("No reverb! "+trk);
		else
		{	int id = data->reverbs->revmap[sc.sReverbs]-1;
			if (id==-1)  LogO("Reverb not found! "+trk+"  "+sc.sReverbs);
		}		
		
		///  sky clrs  ---
		string s;
		s += sc.lAmb.Check("amb");  s += sc.lDiff.Check("dif");  s += sc.lSpec.Check("spc");
		s += sc.fogClr.Check("fog");  s += sc.fogClr2.Check("fog2");  s += sc.fogClrH.Check("foh");
		if (!s.empty())
			LogO("CLR CHK! "+trk+"  "+s);
		
		
		///  terrain  --------
		#if 0  // used
		for (n=0; n < sc.td.layers.size(); ++n)
		{	const TerLayer& l = sc.td.layersAll[sc.td.layers[n]];
		#else  // all
		for (n=0; n < TerData::ciNumLay; ++n)
		{	TerLayer& l = sc.tds[0].layersAll[n];  // 1st ter only
		#endif
			bool e = l.texFile.empty();
			if (!e && !rg.resourceExistsInAnyGroup(l.texFile))
				LogO("Ter: " + trk + " Not Found !!!  " + l.texFile);

			if (!l.texNorm.empty() && !rg.resourceExistsInAnyGroup(l.texNorm))
				LogO("Ter: " + trk + " Not Found !!!  " + l.texNorm);
				
			const PTer* p = data->pre->GetTer(l.texFile.substr(0, l.texFile.length()-4));
			if (!e && !p)
				LogO("Ter: " + trk + " Not Found in presets !!!  " + l.texFile);

			if (!e && l.surfName == "Default")
			{
				LogO("Ter: " + trk + " Default surface !!!  " + l.texFile);
			#if 0  //  fix from presets
				l.surfName = p->surfName;
				l.dust = p->dust;   l.dustS = p->dustS;
				l.mud = p->mud;  l.smoke = 0.f;  l.tclr = p->tclr;
				modif = true;
				LogO("Ter:  Fixed");
			#endif
			}
			#if 0
			if (!e && p && l.surfName != p->surfName)
				LogO("Ter: " + trk + " Different surface !  " + l.texFile + " " + l.surfName + " pre: " + p->surfName);
			#endif
		}
		
		///  road  ----
		int iLch = 0;
		for (n=0; n < rd.mP.size(); ++n)
			if (rd.mP[n].chkR > 0.f && rd.mP[n].loop > 0)
				++iLch;
		//LogO("Road: " + trk + "  Lch " + toStr(iLch));
		if (iLch % 2 == 1)
			LogO("Road: " + trk + " Not even loop chks count !  ");

		for (n=0; n < MTRs; ++n)
		{
			String s = rd.sMtrRoad[n];
			if (!s.empty() && !data->pre->GetRoad(s))
				LogO("Road: " + trk + " Not Found in presets !!!  " + s);

			s = rd.sMtrPipe[n];
			if (!s.empty() && !data->pre->GetRoad(s))
				LogO("Pipe: " + trk + " Not Found in presets !!!  " + s);
	
			//sc.td.layerRoad
		}
		//sMtrWall,sMtrWallPipe, sMtrCol

		
		///  grass  ----
		for (n=0; n < Scene::ciNumGrLay; ++n)
		{	const SGrassLayer& l = sc.grLayersAll[n];

			String s = l.material;
			if (!s.empty() && l.on && !data->pre->GetGrass(s))
				LogO("Grs: " + trk + " Not Found in presets !!!  " + s);
		}

		///  veget  --------
		for (n=0; n < Scene::ciNumPgLay; ++n)
		{
			const PagedLayer& l = sc.pgLayersAll[n];
			const String& s = l.name;  //.mesh
				
			//  checks
			if (!s.empty())
			{
				if (l.on && !rg.resourceExistsInAnyGroup(s))
					LogO("Veg: " + trk + " Not Found !!!  " + s);

				if (l.on &&/**/ !data->pre->GetVeget(s.substr(0,s.length()-5)))
					LogO("Veg: " + trk + " Not Found in presets !!!  " + s);

				#if 0
				if (l.on && !data->objs->Find(s) && noCol[s]==0)
				if (!(s.length() > 4 && s.substr(0,4) == "rock"))
				{	noCol[s] = 1;
					LogO("Veg: " + trk + " no collision.xml for  " + s);
				}
				#endif

				/*if (l.minScale < 0.3f && minSc[s]==0)
				{	minSc[s] = 1;
					LogO("Veg: " + trk + " scale < 0.3  model  " + s + "  val " + fToStr(l.minScale,2,4) +" "+ fToStr(l.maxScale,2,4));
				}*/
				if (l.maxScale > 4.f)
					LogO("All: " + trk + "  scale > 4  model  "   + s + "  val " + fToStr(l.maxScale,2,4));
				
				//  rescale ..
				/**if (s.substr(0,3)=="fir")
				{
					lay.minScale *= 10.f;  lay.maxScale *= 10.f;
					lay.windFx *= 0.1f;  lay.windFy *= 0.1f;
				}/**/
		}	}

		if (modif)
			sc.SaveXml(path +"scene.xml");  /// resave
		//SplineRoad rd(this);  rd.LoadFile(path+"road.xml");
		//rd.SaveFile(path+"road1.xml");  // resave
	}
	
	
	//LogO(String("::: Time ALL tracks: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	LogO("ALL tracks scene ---------");
}


#else
//--------  Game Tools  --------

///............................................................................................................................
///  _Tool_ ghosts times (user, to check times and final place)
///............................................................................................................................
void CGui::ToolGhosts()
{
	LogO("ALL ghosts ---------");
	using namespace std;
	const string sim = 1 /**/ ? "normal" : "easy";
	String msg="\n";  const float tMax = 10000.f;
	TIMER tim;
	
	//  all cars
	std::vector<string> cars;
	std::vector<float> plc;
	for (int c=0; c < data->cars->cars.size(); ++c)
	{	cars.push_back(data->cars->cars[c].id);
		plc.push_back(0.f);  }

	//  foreach track
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	string trk = data->tracks->trks[i].name;
		if (trk.substr(0,4) == "Test" && trk.substr(0,5) != "TestC")  continue;

		//  records
		tim.Load(PATHS::Records()+"/"+ sim+"/"+ trk+".txt", 0.f);
		float timeES=tMax, timeBest=tMax;
		for (int c=0; c < cars.size(); ++c)
		{
			tim.AddCar(cars[c]);
			float t = tim.GetBestLap(c, false);  //not reverse
			plc[c] = t;
			if (t == 0.f)  continue;

			if (t < timeBest)  timeBest = t;
			if (cars[c] == "ES" || cars[c] == "S1")
				if (t < timeES)  timeES = t;
		}
		if (timeES==tMax)  timeES=0.f;
		if (timeBest==tMax)  timeBest=0.f;
		//  T= from tracks.ini
		float timeTrk = data->tracks->times[trk];// + 2;

		//float timeB = timeTrk * 1.1f;  // champs factor mostly 0.1
		//const float decFactor = 1.5f;
		//float score = max(0.f, (1.f + (timeB-timeES)/timeB * decFactor) * 100.f);
		//float place = app->GetRacePos(timeES,timeTrk,1.f,false);

		///  write
	#if 0
		//  format trk times (old)
		ostringstream s;
		s << "\t<track name=\""+trk+"\"";
		for (int i=0; i < 18-trk.length(); ++i)
			s << " ";  //align
		s << "time=\""+fToStr(timeES,1)+"\" />";
		msg += s.str()+"\n";
	#else
		//  stats ..
		ostringstream s;
		s << fixed << left << setw(18) << trk;  //align
		#if 0
		s << "  E " << CHud::GetTimeShort(timeES);  // Expected car ES or S1
		s << "  T " << CHud::GetTimeShort(timeTrk);  // trk time from .xml
		s << "  b " << CHud::GetTimeShort(timeES == timeBest ? 0.f : timeBest);
		s << "  E-b " << (timeES > 0.f && timeES != timeBest ?
						fToStr(timeES - timeBest ,0,2) : "  ");
		s << "  T-E " << (timeES > 0.f ?
						fToStr(timeTrk - timeES  ,0,2) : "  ");
		s << "  pET " << (timeES > 0.f ? fToStr(place,1,3) : "   ");
		#endif
		
		//  race pos for all cars from cur ghosts
		for (int c=0; c < cars.size(); ++c)
		{
			float t = plc[c];
			float cmul = app->GetCarTimeMul(cars[c], sim);
			float pl = app->GetRacePos(t,timeTrk, cmul,false);
			s << cars[c] << " " << (t > 0.f ? (pl > 20 ? " ." : fToStr(pl,0,2)) : "  ") << " ";
		}										  //90
		
		//s << (score > 135.f ? " ! " : "   ");
		msg += s.str()+"\n";
	#endif
	}
	LogO(msg);
	//LogO("ALL ghosts ---------");
}

///............................................................................................................................
///  _Tool_ convert ghosts to track's ghosts (less size and frame data)
//  put original ghosts into  data/ghosts/original/*_ES.rpl
//  (ES, normal sim, 1st lap, no boost, use rewind with _Tool_ go back time)
//  time should be like in tracks.ini or less (last T= )
///............................................................................................................................
void CGui::ToolGhostsConv()
{
	LogO("ALL ghosts Convert ---------");
	Replay2 ghost;  TrackGhost trg;
	for (int r=0; r < 2; ++r)
	{
		string sRev = r==1 ? "_r" : "";

		//  for each track
		for (int i=0; i < data->tracks->trks.size(); ++i)
		{	string track = data->tracks->trks[i].name;
			if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
			
			//  load
			ghost.Clear();  trg.Clear();
			string file = PATHS::TrkGhosts()+"/original/"+ track + sRev + "_ES.rpl";
			if (!PATHS::FileExists(file))
			{}	//LogO("NOT found: "+file);
			else
			{	LogO("---------  "+track+"  ---------");
				ghost.LoadFile(file);
				
				//  convert
				MATHVECTOR<float,3> oldPos;  float oldTime = 0.f;
				int num = ghost.GetNumFrames(), jmp = 0;
				
				for (int i=0; i < num; ++i)
				{
					const ReplayFrame2& fr = ghost.GetFrame0(i);
					TrackFrame tf;
					tf.time = fr.time;
					tf.pos = fr.pos;  tf.rot = fr.rot;
					tf.brake = fr.get(b_braking) ? 1 : 0;
					tf.steer = fr.steer * 127.f;
					//LogO(toStr(fr.braking)+ " st " +fToStr(fr.steer,2,5));

					#define Nth 2  // old was 1/3 from 160, new 1/2 from 80
					if (i % Nth == Nth-1)  /// write every n-th frame only
						trg.AddFrame(tf);

					//  check for sudden pos jumps  (rewind used but not with _Tool_ go back time !)
					if (i > 10 && i < num-1)  // ignore jumps at start or end
					{	float dist = (fr.pos - oldPos).MagnitudeSquared();
						if (dist > 16.f)  //1.f small
						{	
							LogO("!Jump at "+StrTime2(fr.time)+"  d "+fToStr(sqrt(dist),0)+"m");
							++jmp;
					}	}
					//  check vel at start
					if (i==50)
					{
						float dist = (fr.pos - oldPos).Magnitude();
						float vel = 3.6f * dist / (fr.time - oldTime);
						bool bad = vel > 30;
						if (bad)
							LogO("!Vel at "+StrTime(fr.time)+" kmh "+fToStr(vel,0) + (bad ? "  BAD":""));
					}
					oldPos = fr.pos;  oldTime = fr.time;
				}
				if (jmp > 0)
					LogO("!Jumps: "+toStr(jmp));
			
				//  save
				string fsave = PATHS::TrkGhosts()+"/"+ track + sRev + ".gho";
				trg.header.ver = 1;
				trg.SaveFile(fsave);
			}
		}

		//  check missing
		for (int i=0; i < data->tracks->trks.size(); ++i)
		{	string track = data->tracks->trks[i].name;
			if (track.substr(0,4) == "Test" /*&& track.substr(0,5) != "TestC"*/)  continue;
			
			string fsave = PATHS::TrkGhosts()+"/"+ track + sRev + ".gho";
			if (!PATHS::FileExists(fsave))
				if (r==1)	LogO("!Rev Missing for track: " + track);
				else		LogO("!!   Missing for track: " + track);
		}
	}
}

///  _Tool_ check tracks ghosts
///............................................................................................................................
void CGui::ToolTestTrkGhosts()
{
	LogO("ALL tracks ghosts Test ---------");
	TrackGhost gho;
	
	//  foreach track
	String ss;
	for (int r=0; r < 2; ++r)
	{	
		string sRev = r==1 ? "_r" : "";
		for (int i=0; i < data->tracks->trks.size(); ++i)
		{	string track = data->tracks->trks[i].name;
			//if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
			
			//  load
			gho.Clear();
			string file = PATHS::TrkGhosts()+"/"+ track + sRev + ".gho";
			if (!PATHS::FileExists(file))
			{	/*LogO("NOT found: "+file);/**/  }
			else
			{	LogO("---------  "+track+"  ---------");
				gho.LoadFile(file);
				
				//  test
				MATHVECTOR<float,3> oldPos;  float oldTime = 0.f;
				int frames = gho.getNumFrames(), jmp = 0;
				for (int i=0; i < frames; ++i)
				{
					const TrackFrame& fr = gho.getFrame0(i);

					//  check for sudden pos jumps  (rewind used but not with _Tool_ go back time !)
					if (i > 10 && i < frames-1)  // ignore jumps at start or end
					{	float dist = (fr.pos - oldPos).MagnitudeSquared();
						if (dist > 6.f*6.f)  //par
						{	
							LogO("!Jump at "+StrTime2(fr.time)+"  d "+fToStr(sqrt(dist),0)+"m");
							++jmp;
					}	}
					//  check vel at start
					if (i==50/3)
					{
						float dist = (fr.pos - oldPos).Magnitude();
						float vel = 3.6f * dist / (fr.time - oldTime);
						bool bad = vel > 30;
						if (bad)
							LogO("!Vel at "+StrTime(fr.time)+" kmh "+fToStr(vel,0) + (bad ? "  BAD":""));
					}
					oldPos = fr.pos;  oldTime = fr.time;
				}
				if (jmp > 0)
				{	LogO("!Jumps: "+toStr(jmp));
					ss += "\n" + track;
			}	}
		}
	}
	LogO("!! Jumps on tracks:"+ss);
}

#endif
