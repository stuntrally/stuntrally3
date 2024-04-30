#include "pch.h"
#include "Def_Str.h"
#include "TracksXml.h"
#include "CareerXml.h"
#include <regex>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace Ogre;
using namespace std;


//  Load career xml
//-------------------------------------------------------------------------------------------------------------
bool ProgressCareer::LoadXml(std::string file)
{
	XMLDocument doc;
	XMLError er = doc.LoadFile(file.c_str());
	if (er != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	// all.clear();  // defaults..
	cars.clear();

	const char* a;
	XMLElement* ec = root->FirstChildElement("career"), *e;
	if (ec)
	{
		e = ec->FirstChildElement("cur");
		a = e->Attribute("mode");   if (a)  mode = s2i(a);
		a = e->Attribute("money");  if (a)  money = s2r(a);
		a = e->Attribute("diff");   if (a)  diff_rank = s2i(a);

		e = ec->FirstChildElement("fuels");
		a = e->Attribute("car");   if (a)  fuels.car = s2r(a);

		//  stats
		e = ec->FirstChildElement("stats");
		a = e->Attribute("time");   if (a)  totalTime = s2r(a);
		a = e->Attribute("raced");  if (a)  raced = s2i(a);

		a = e->Attribute("pos1");  if (a)  pos1 = s2i(a);	a = e->Attribute("pos2");  if (a)  pos2 = s2i(a);
		a = e->Attribute("pos3");  if (a)  pos3 = s2i(a);	a = e->Attribute("pos4");  if (a)  pos4 = s2i(a);
		a = e->Attribute("pos5");  if (a)  pos5up = s2i(a);

		//  cars / vehicles
		XMLElement* ev = ec->FirstChildElement("veh");
		while (ev)
		{
			ProgressCar car;
			a = ev->Attribute("id");   if (a)  car.name = std::string(a);
			a = ev->Attribute("dmg");  if (a)  car.damage = s2r(a);
			// int cur_paint =0;
			// std::vector<CarPaint> paints;  // ..
			
			cars.push_back(car);
			ev = ev->NextSiblingElement("veh");
		}
	}
	return true;
}

//  Save career xml
//-------------------------------------------------------------------------------------------------------------
bool ProgressCareer::SaveXml(std::string file)
{
	XMLDocument xml;
	XMLElement* root = xml.NewElement("career"), *e;

	e = xml.NewElement("cur");
		e->SetAttribute("mode",  toStrC( mode ));
		e->SetAttribute("money", toStrC( money ));
		e->SetAttribute("diff",  toStrC( diff_rank ));
	root->InsertEndChild(e);

	e = xml.NewElement("fuels");
		e->SetAttribute("car",   toStrC( fuels.car ));
	root->InsertEndChild(e);

	e = xml.NewElement("stats");
		e->SetAttribute("time",  toStrC( totalTime ));
		e->SetAttribute("raced", toStrC( raced ));

		e->SetAttribute("pos1",  toStrC( pos1 ));
		e->SetAttribute("pos2",  toStrC( pos2 ));
		e->SetAttribute("pos3",  toStrC( pos3 ));
		e->SetAttribute("pos4",  toStrC( pos4 ));
		e->SetAttribute("pos5",  toStrC( pos5up ));
	root->InsertEndChild(e);

	for (int i=0; i < cars.size(); ++i)
	{
		const ProgressCar& car = cars[i];
		XMLElement* ev = xml.NewElement("veh");
			ev->SetAttribute("id",  car.name.c_str() );
			ev->SetAttribute("dmg", fToStr( car.damage ).c_str());
			//..
		root->InsertEndChild(ev);
	}
	
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}
