#include "pch.h"
#include "Def_Str.h"
#include "paths.h"
#include "CGame.h"
#include "CGui.h"
#include "CData.h"
#include "TracksXml.h"
#include "GuiCom.h"

#include "MultiList2.h"
#include "gameclient.hpp"
#include <OgreTimer.h>
#include <MyGUI.h>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  🚗📃  Car list
//-----------------------------------------------------------------------------------------------------------
//  common sort code,  no info only by name
#define sArg  const CarL& c2, const CarL& c1
#define sortDef  bool t = false;  if (!c1.ci || !c2.ci)  return c1.name > c2.name || t;
const int allSortFunc = 7;

//  sorting functions for columns
/* 0  Id      */  bool Sort0 (sArg){  sortDef  return c1.ci->id     < c2.ci->id     || t;  }
/* 1  Name    */  bool Sort1 (sArg){  sortDef  return c1.name       < c2.name       || t;  }
/* 2  > speed */  bool Sort2 (sArg){  sortDef  return c1.ci->speed  < c2.ci->speed  || t;  }
/* 3  * rate  */  bool Sort3 (sArg){  sortDef  return c1.ci->rating < c2.ci->rating || t;  }
/* 4  ! diff  */  bool Sort4 (sArg){  sortDef  return c1.ci->diff   < c2.ci->diff   || t;  }
/* 5  - width */  bool Sort5 (sArg){  sortDef  return c1.ci->width  < c2.ci->width  || t;  }
/* 6  o wheels*/  bool Sort6 (sArg){  sortDef  return c1.ci->wheels < c2.ci->wheels || t;  }
// /* 7  % drivab*/  bool Sort7 (sArg){  sortDef  return c1.ci->type   < c2.ci->type   || t;  }

//  sorting functions array (to access by column index)
bool (*CarSort[allSortFunc])(const CarL& c1, const CarL& c2) =
{	Sort0, Sort1, Sort2, Sort3, Sort4, Sort5, Sort6  };


//  done every list sort column change or find edit text change
//  fills gui cars list
//-----------------------------------------------------------------------------------------------------------
void CGui::CarListUpd(bool resetNotFound)
{
	bool filter = isChallGui();
		
	if (carList)
	{	carList->removeAllItems();
		int ii = 0, si = -1;  bool bFound = false;

		//  sort
		int numFunc = min(allSortFunc-1, (int)carList->mSortColumnIndex);
		std::list<CarL> liCar2 = liCar;  // copy
		liCar2.sort(CarSort[numFunc]);
		if (carList->mSortUp)  liCar2.reverse();
		
		//  original
		for (auto i = liCar2.begin(); i != liCar2.end(); ++i)
		{
			String name = (*i).name;  //, nlow = name;  StringUtil::toLowerCase(nlow);
			//if (sTrkFind == "" || strstr(nlow.c_str(), sTrkFind.c_str()) != 0)

			///  filter for challenge
			if (!filter || IsChallCar(name))
			{
				AddCarL(name, (*i).ci);
				if (name == pSet->gui.car[0])  {  si = ii;
					carList->setIndexSelected(si);
					bFound = true;  }
				++ii;
		}	}

		//  not found last car, set last
		if (resetNotFound && !bFound)
			pSet->gui.car[0] = carList->getItemNameAt(carList->getItemCount()-1).substr(7);

		if (si > -1)  // center
			carList->beginToItemAt(max(0, si-5));
	}
}

//  ➕ Add
void CGui::AddCarL(string name, const CarInfo* ci)
{
	MultiList2* li = carList;
	CarInfo cci;
	if (!ci)  ci = &cci;  //  details
	String clr = data->cars->colormap[ci->type];  if (clr.length() != 7)  clr = "#C0D0E0";
	
	li->addItem(clr+ name);  int l = li->getItemCount()-1;
	li->setSubItemNameAt(1,l, clr+ TR("#{"+ ci->name +"}"));
	// li->setSubItemNameAt(2,l, gcom->getClrDiff(ci->speed *0.76f)+ fToStr(ci->speed,1,3));
	li->setSubItemNameAt(2,l, gcom->getClrVal(ci->speed *1.3f)+ fToStr(ci->speed,1,3));

	li->setSubItemNameAt(3,l, gcom->getClrRating(ci->rating)+ " "+toStr(ci->rating));
	li->setSubItemNameAt(4,l, gcom->getClrDiff(ci->diff )+ " "+toStr(ci->diff));

	li->setSubItemNameAt(5,l, gcom->getClrLong(ci->width *2.f)+ " "+toStr(ci->width));
	li->setSubItemNameAt(6,l, gcom->getClrSum(ci->wheels *2.f)+ " "+toStr(ci->wheels));

	float drv = max(0.f, data->GetDrivability(name, gcom->sListTrack, gcom->bListTrackU));
	float drvp = (1.f - drv) * 100.f;  int fd = 1 + drv * 7.f;
	li->setSubItemNameAt(7,l, gcom->getClrDiff(fd)+" "+ fToStr(drvp, 0,3));
	//li->setSubItemNameAt(7,l, gcom->getClrRating(min(4, max(0,1+(ci->year-1990)/10))) + toStr(ci->year));
	//li->setSubItemNameAt(7,l, clr+ TR("#{CarType_"+ci->type+"}"));
}

//  📃 Fill
void CGui::FillCarList()
{
	liCar.clear();
	strlist li;
	PATHS::DirList(PATHS::Cars(), li);
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	{
		if (PATHS::FileExists(PATHS::Cars() + "/" + *i + "/about.txt"))
		{	String s = *i;
			CarL c;  c.name = *i;  //c.pA = this;
			int id = data->cars->carmap[*i];
			if (id)
			{	c.ci = &data->cars->cars[id-1];
				liCar.push_back(c);
	}	}	}
}
//-----------------------------------------------------------------------------------------------------------


//  👻 ghost filename
//
string ghostFile(SETTINGS* pSet, string sim_mode, string car)
{
	return PATHS::Ghosts()+"/" +sim_mode+"/"
		+ pSet->game.track
		+ (pSet->game.track_user ? "_u" : "")
		+ (pSet->game.track_reversed ? "_r" : "")
		+ "_" + car + ".rpl";
}

const String& CGui::GetGhostFile(string* ghCar)
{
	static String file;
	string sim_mode = pSet->game.sim_mode, car = pSet->game.car[0];
	file = ghostFile(pSet, sim_mode, car);
	if (PATHS::FileExists(file))
		return file;
	
	if (!ghCar)
		return file;

	///--  if doesnt exist look for other cars, then other sim modes

	//  cars list sorted by car speed
	std::list<CarL> liCar2 = liCar;
	liCar2.sort(CarSort[1]);

	std::vector<string> cars;
	for (std::list<CarL>::iterator i = liCar2.begin(); i != liCar2.end(); ++i)
	{
		String name = (*i).name;
		cars.push_back(name);
		//LogO(name);
	}

	//  find current
	int i = 0, si = cars.size(), ci = 0;
	while (i < si)
	{	if (cars[i] == car)
		{	ci = i;  break;  }
		++i;
	}
	//LogO(toStr(ci)+" ci "+cars[ci]+" all "+toStr(si));

	std::vector<string> cars2;
	int a = ci, b = ci;  i = 0;
	cars2.push_back(cars[ci]);  // 1st cur
	while (cars2.size() < si)  // same size
	{	// +1, -1, +2, -2 ..
		if (i % 2 == 0)
		{	++a;  // next faster car
			if (a < si)  cars2.push_back(cars[a]);
		}else
		{	--b;  // next slower car
			if (b >= 0)  cars2.push_back(cars[b]);
		}	++i;
	}
	//for (i=0; i < cars2.size(); ++i)
	//	LogO(toStr(i)+"> "+cars2[i]);
	
	bool srch = true;
	i = 0;  a = 0;
	while (srch)
	{
		const string& car = cars2[i];
		file = ghostFile(pSet, sim_mode, car);

		if (PATHS::FileExists(file))
		{	srch = false;  *ghCar = car;  }
		++i;
		if (i >= si)
		{	i = 0;
			if (sim_mode == "easy")  sim_mode = "normal";
			else  sim_mode = "easy";
			++a;  if (a==2)  srch = false;  // only those 2
		}
	}
	return file;
}

string CGui::GetRplListDir()
{
	return (pSet->rpl_listghosts
		? (PATHS::Ghosts() + "/" + pSet->gui.sim_mode)
		: PATHS::Replays() );
}


//  [Game] 	. . . . . . . . . . . . . . . . . . . .    --- lists ----    . . . . . . . . . . . . . . . . . . 

//  🚗📃 car
void CGui::listCarChng(MultiList2* li, size_t)
{
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	Ogre::Timer ti;
	sListCar = li->getItemNameAt(i).substr(7);

	if (imgCar && !pSet->dev_no_prvs)  imgCar->setImageTexture(sListCar+".jpg");
	if (app->mClient)  app->mClient->updatePlayerInfo(pSet->nickname, sListCar);
	
	//  car desc txt
	String sd = String("#BFD3E5")+TR("#{CarDesc_"+sListCar+"}");

	//  car info
	bool car = true;
	int id = data->cars->carmap[sListCar];
	if (id > 0 && txCarSpeed && barCarSpeed)
	{	const CarInfo& ci = data->cars->cars[id-1];

		txCarAuthor->setCaption(ci.author);
		txCarSpeed->setCaption(gcom->getClrDiff(ci.speed *0.76f)+ fToStr(ci.speed,1,3));
		txCarType->setCaption(data->cars->colormap[ci.type]+ TR("#{CarType_"+ci.type+"}"));
		txCarYear->setCaption(gcom->getClrRating(min(4, max(0,1+(ci.year-1990)/10))) + toStr(ci.year));
		
		txCarDiff->setCaption(gcom->getClrDiff(ci.diff)+ toStr(ci.diff) +TR("  #{Diff"+toStr(ci.diff)+"}"));
		txCarRating->setCaption(gcom->getClrRating(ci.rating)+ toStr(ci.rating));
		txCarWidth->setCaption(gcom->getClrDiff(ci.width)+ toStr(ci.width));

		UpdDrivability(gcom->sListTrack, gcom->bListTrackU);

		if (ci.type == "Spaceship" || ci.type == "Other")
		{	car = false;  sd += TR("#E0E060 \n#{CarDesc_Pipes}");  }

		float v = std::max(0.f, 1.f - ci.speed/13.f);
		barCarSpeed->setImageCoord(IntCoord(v*128.f,0,128,16));
		barCarSpeed->setColour(Colour(1.f, 0.2f + 0.8f * v, v * 0.3f));
	}
	carDesc->setCaption(sd);

	changeCar();
	UpdCarStats(car);
	// LogO(String(":::* Time car tab upd: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}	
void CGui::changeCar()
{
	if (iCurCar < MAX_Players)
		pSet->gui.car[iCurCar] = sListCar;
}


//  📐 Drivability  ------------------------
void CGui::UpdDrivability(std::string trk, bool user)
{
	float drv = max(0.f, data->GetDrivability(sListCar, trk, user));
	float drvp = (1.f - drv) * 100.f;  int fd = std::min(7.f, 1.f + drv * 7.f);
	auto sdrv = drv > 0.85f ? TR("#{Undrivable}") : TR("#{Diff"+toStr(fd)+"}");
	// txCarTrkdrv->setCaption(drv < 0.f ? "" : gcom->getClrDiff(fd)+ fToStr(drv, 1,3) +"   " +sdrv);
	
	for (int i=0; i < iDrvSt; ++i)
	{	txTrkDrivab[i]->setCaption(drv < 0.f ? "" : gcom->getClrDiff(fd)+ fToStr(drvp, 0,3) +"%   " +sdrv);
		imgTrkDrivab[i]->setColour(Colour(1.f, 1.f - drv*drv*0.8f, 1.f - drv*0.7f, drv));
}	}


///  🚗📈 load car stats xml
//-----------------------------------------------------------------------------------------------------------
void CGui::UpdCarStats(bool car)
{
	string path = PATHS::CarSim() + "/" + pSet->gui.sim_mode + "/cars/" + sListCar + "_stats.xml";
	float f;
	#define  vis(i,v)  \
		{  txCarStTxt[i]->setVisible(v);  txCarStVals[i]->setVisible(v);  barCarSt[i]->setVisible(v);  }

	XMLDocument doc;  int i;
	XMLError er = doc.LoadFile(path.c_str());
	if (er != XML_SUCCESS)
	{
		for (i=0; i < iCarSt; ++i)	vis(i,false);
		return;
	}
	XMLElement* root = doc.RootElement();
	if (!root)  return;

	for (i=0; i < iCarSt; ++i)	vis(i,true);

	//  read xml  ------------------------
	XMLElement* e;  const char* a;
	float mass=0.f, comFront=0.f,  maxTrq=0.f, rpmMaxTq=0.f, maxPwr=0.f, rpmMaxPwr=0.f, bhpPerTon=0.f,
		maxVel=0.f, tiMaxVel=0.f,  t0to60=0.f, t0to100=0.f, t0to160=0.f, t0to200=0.f,
		stop60=0.f, stop100=0.f, stop160=0.f,  down100=0.f, down160=0.f, down200=0.f;

	e = root->FirstChildElement("car");
	if (e)
	{	a = e->Attribute("mass");	if (a)  mass = s2r(a);
		//a = e->Attribute("inertia");	if (a)  inert = s2v(a);
	}
	e = root->FirstChildElement("com");
	if (e)
	{	a = e->Attribute("frontPercent");	if (a)  comFront = s2r(a);
		//a = e->Attribute("pos");	if (a)  com = s2v(a);
		//a = e->Attribute("whf");	if (a)  whf = s2r(a);
		//a = e->Attribute("whr");	if (a)  whr = s2r(a);
	}
	e = root->FirstChildElement("torque");
	if (e)
	{	a = e->Attribute("max");	if (a)  maxTrq = s2r(a);
		//a = e->Attribute("rpm");	if (a)  rpmMaxTq = s2r(a);
		//a = e->Attribute("mul");	if (a)  mul = s2r(a);
	}
	e = root->FirstChildElement("power");
	if (e)
	{	a = e->Attribute("max");	if (a)  maxPwr = s2r(a);
		//a = e->Attribute("rpm");	if (a)  rpmMaxPwr = s2r(a);
	}
	e = root->FirstChildElement("bhpPerTon");
	if (e)
	{	a = e->Attribute("val");	if (a)  bhpPerTon = s2r(a);
	}
	e = root->FirstChildElement("top");
	if (e)
	{	a = e->Attribute("speed");	if (a)  maxVel = s2r(a);
		//a = e->Attribute("time");	if (a)  tiMaxVel = s2r(a);
	}
	/*e = root->FirstChildElement("quarterMile");
	if (e)
	{	a = e->Attribute("time");	if (a)  timeQM = s2r(a);
		a = e->Attribute("vel");	if (a)  velAtQM = s2r(a);
	}*/
	e = root->FirstChildElement("accel");
	if (e)
	{	a = e->Attribute("t60");	if (a)  t0to60 = s2r(a);
		a = e->Attribute("t100");	if (a)  t0to100 = s2r(a);
		a = e->Attribute("t160");	if (a)  t0to160 = s2r(a);
		a = e->Attribute("t200");	if (a)  t0to200 = s2r(a);
	}
	/*e = root->FirstChildElement("downForce");
	if (e)
	{	a = e->Attribute("d100");	if (a)  down100 = s2r(a);
		a = e->Attribute("d160");	if (a)  down160 = s2r(a);
		a = e->Attribute("d200");	if (a)  down200 = s2r(a);
	}*/
	e = root->FirstChildElement("stop");
	if (e)
	{	//a = e->Attribute("s160");	if (a)  stop160 = s2r(a);
		a = e->Attribute("s100");	if (a)  stop100 = s2r(a);
		//a = e->Attribute("s60");	if (a)  stop60 = s2r(a);
	}

	//  speed graph points
	e = root->FirstChildElement("velGraph");
	std::vector<float> ttim,tkmh;
	if (e)
	{	float t,v;
		XMLElement* p = e->FirstChildElement("p");
		while (p)
		{	a = p->Attribute("t");  if (a) {  t = s2r(a);
			a = p->Attribute("v");	if (a) {  v = s2r(a);
				ttim.push_back(t);  tkmh.push_back(v);
			}	}
			p = p->NextSiblingElement("p");
	}	}


	///  📉 upd vel graph  ~~~
	float xs = 10.f, ys = 0.4f, yo = 166.f, x2 = 500.f;
	const IntSize& wi = app->mWndOpts->getSize();
	const float sx = wi.width/1248.f, sy = wi.height/935.f;
	xs *= sx;  ys *= sy;  yo *= sy;  x2 *= sx;

	std::vector<FloatPoint> points,grid;
	points.push_back(FloatPoint(0.f, yo));
	for (i = 0; i < (int)ttim.size(); ++i)
		points.push_back(FloatPoint(ttim[i] * xs, yo - ys * tkmh[i]));
	graphVel->setPoints(points);

	//  grid lines
	const int y1 = yo +10, y2 = -10, x1 = -10;  //outside
	for (i = 0; i < 6; ++i)  // ||
	{	float fi = i > 2 ? 10.f*(i-1) : 5.f*i;
		grid.push_back(FloatPoint(fi * xs,  i%2==0 ? y1 : y2));
		grid.push_back(FloatPoint(fi * xs,  i%2==0 ? y2 : y1));
	}
	grid.push_back(FloatPoint(x2, y1));
	grid.push_back(FloatPoint(x1, y1));
	for (i = 0; i < 4; ++i)  // ==
	{	grid.push_back(FloatPoint(i%2==0 ? x1 : x2,  yo - ys * i * 100.f));
		grid.push_back(FloatPoint(i%2==0 ? x2 : x1,  yo - ys * i * 100.f));
	}
	graphVGrid->setPoints(grid);
	

	//  upd gui texts  ------------------------
	bool kmh = !pSet->show_mph;  float k2m = 0.621371f;
	String s[iCarSt], v[iCarSt];
	float sm = pSet->gui.sim_mode == "easy" ? 0.75f : 1.f;

	#define bar(n,sc, r,g,b)  \
		f = std::max(0.f, (1.f - sc)) * 128.f;  barCarSt[n]->setImageCoord(IntCoord(f,0,128,16));  \
		barCarSt[n]->setColour(Colour(r,g,b));

	s[0]= "#80E080"+ TR("#{Car_Mass}");
	v[0]= "#90FF90"+ fToStr(mass,0,3) +TR(" #{UnitKg}");
	bar(0, mass / 3000.f, 0.6,1.0,0.6);
	s[1]= "#B0E0B0"+ TR("#{Car_MassFront}");
	v[1]= "#C0FFC0"+ fToStr(comFront,0,3) +"%";
	bar(1, comFront / 100.f, 0.8,1.0,0.8);

	s[2]= "#E0C0A0"+ TR("#{Car_MaxTorque}");
	v[2]= "#F0D0B0"+ fToStr(maxTrq,0,3) +TR(" #{UnitNm}");  //  #{at} ")+ fToStr(rpmMaxTq,0,3) +TR(" #{UnitRpm} ");
	bar(2, maxTrq / 970.f, 0.9,0.8,0.6);  vis(2,car);
	s[3]= "#E0B090"+ TR("#{Car_MaxPower}");
	v[3]= "#F0C0A0"+ fToStr(maxPwr,0,3) +TR(" #{UnitBhp}");  //  #{at} ")+ fToStr(rpmMaxPwr,0,3) +TR(" #{UnitRpm} ");
	bar(3, maxPwr / 700.f, 0.9,0.7,0.5);  vis(3,car);
	s[4]= "#E0E0A0"+ TR("#{Car_BhpPerTon}");
	v[4]= "#F0F0B0"+ fToStr(bhpPerTon,0,3);
	bar(4, bhpPerTon / 450.f, 1.0,1.0,0.6);  vis(4,car);

	#define sVel(s,v)  \
		if (kmh)  s += fToStr(v, 0,3) +TR(" #{UnitKmh}");  \
		    else  s += fToStr(v*k2m, 0,3) +TR(" #{UnitMph}");

	s[5]= "#80C0FF"+ TR("#{Car_TopSpeed}");
	v[5]= "#90D0FF";  sVel(v[5], maxVel);  //v[5]+= TR("  #{at} ")+ fToStr(tiMaxVel,1,4) +TR(" #{UnitS} ");
	bar(5, maxVel / 300.f, 0.6,0.9,1.0);

	s[6]= "#8ECEFE"+ TR("#{Car_TimeTo} ");  sVel(s[6], 100.f);
	v[6]= "#9EDEFF"+ fToStr(t0to100,1,4) +TR(" #{UnitS} ");
	bar(6, t0to100 / 8.f * sm, 0.8,1.0,1.0);

	s[7]= "#88C8F8"+ TR("#{Car_TimeTo} ");  sVel(s[7], 160.f);
	v[7]= "#98D8FF"+ fToStr(t0to160,1,4) +TR(" #{UnitS} ");
	bar(7, t0to160 / 17.f * sm, 0.75,1.0,1.0);  vis(7, t0to160 > 0.f);
		  
	s[8]= "#80C0F0"+ TR("#{Car_TimeTo} ");  sVel(s[8], 200.f);
	v[8]= "#90D0FF"+ fToStr(t0to200,1,4) +TR(" #{UnitS} ");
	bar(8, t0to200 / 27.f * sm, 0.7,1.0,1.0);  vis(8, t0to200 > 0.f);
		  
	s[9]= "#70E0E0"+ TR("#{Car_StopTimeFrom} ");  sVel(s[9], 100.f);
	v[9]= "#80F0F0"+ fToStr(stop100,1,4) +TR(" #{UnitS} ");
	bar(9, stop100 / 5.f * sm, 0.5,1.0,1.0);

	for (i=0; i < iCarSt; ++i)	
	{	txCarStTxt[i]->setCaption(s[i]);  txCarStVals[i]->setCaption(v[i]);  }
}
