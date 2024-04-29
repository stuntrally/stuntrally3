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
		a = ec->Attribute("mode");   mode = s2i(a);
		a = ec->Attribute("money");	 money = s2r(a);
		a = ec->Attribute("diff");	 diff_rank = s2i(a);

		e = ec->FirstChildElement("fuels");
		a = e->Attribute("car");  if (a)  fuels.car = s2r(a);

		//  stats
		e = ec->FirstChildElement("stats");
		a = e->Attribute("time");   totalTime = s2r(a);
		a = e->Attribute("raced");  raced = s2i(a);

		a = e->Attribute("pos1");  pos1 = s2i(a);	a = e->Attribute("pos2");  pos2 = s2i(a);
		a = e->Attribute("pos3");  pos3 = s2i(a);	a = e->Attribute("pos4");  pos4 = s2i(a);
		a = e->Attribute("pos5");  pos5up = s2i(a);

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
bool ProgressCareer::SaveXml(std::string file)
{
	XMLDocument xml;
	XMLElement* root = xml.NewElement("career");

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
