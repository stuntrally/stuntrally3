#include "pch.h"
#include "CAllowed.h"
#include "CData.h"
#include "Def_Str.h"
#include "TracksXml.h"

#include <OgreString.h>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace Ogre;


//  CAllowed
//---------------------------------------------------------------------
void CAllowed::LoadXml(XMLElement* xml)
{
	//  cars
	XMLElement* eCarT = xml->FirstChildElement("cartype");
	const char* a;
	if (eCarT)
	{
		a = eCarT->Attribute("names");
		if (a)
		{	String s = a;
			types = StringUtil::split(s, "|");
		}
		//  wheels
		a = eCarT->Attribute("whMin");	if (a)  whMin = s2i(a);
		a = eCarT->Attribute("whMax");	if (a)  whMax = s2i(a);
	}
	//<car names="ES|HI" />
	XMLElement* eCar = xml->FirstChildElement("car");
	if (eCar)
	{
		a = eCar->Attribute("names");
		if (a)
		{	String s = a;
			cars = StringUtil::split(s, "|");
		}
		a = eCar->Attribute("deny");
		if (a)
		{	String s = a;
			carsDeny = StringUtil::split(s, "|");
	}	}
	if (!eCarT && !eCar)
		all = true;
}

#ifndef SR_EDITOR
//  list allowed cars types and cars
String CAllowed::GetStr(CarsXml* xml) const
{
	String str;
	int i,s;

	s = types.size();
	for (i=0; i < s; ++i)
	{
		const String& ct = types[i];
			str += xml->colormap[ct];  // car type color
		str += ct;
		if (i+1 < s)  str += ",";
	}
	//int id = data->carsXml.carmap[*i];
	//data->carsXml.cars[id-1];
	if (!str.empty())
		str += " ";
	
	s = cars.size();
	for (i=0; i < s; ++i)
	{
		const String& c = cars[i];
			int id = xml->carmap[c]-1;  // get car color from type
			if (id >= 0)  str += xml->colormap[ xml->cars[id].type ];
		str += c;
		if (i+1 < s)  str += ",";
	}
	return str;
}

//  check if car allowed
bool CAllowed::Allows(CarsXml* xml, std::string name) const
{
	int i,s;
	if (!cars.empty())
		for (auto& c : cars)  // allow specified
			if (c == name)  return true;
	
	if (!carsDeny.empty())
		for (auto& c : carsDeny)  // deny specified
			if (c == name)  return false;

	if (!types.empty())
	{	s = types.size();

		int id = xml->carmap[name]-1;
		if (id >= 0)
		{
			const auto& ci = xml->cars[id];
			String type = ci.type;

			if (ci.wheels < whMin || ci.wheels > whMax)
				return false;  // deny type if wheels not allowed

			for (i=0; i < s; ++i)
				if (type == types[i])  return true;
	}	}
	return all;
}
#endif
