#pragma once
#include <OgreString.h>
#include <OgreStringVector.h>


namespace tinyxml2
{	class XMLElement;  }

class CarsXml;


//  Allowed type(s) or specific vehicle(s), 1 or more
class CAllowed
{
public:
	Ogre::StringVector types, cars, carsDeny;
	int whMin = 0, whMax = 10;  // allowed vehicle wheels count range [min..max]
	bool all = false;  // allowed

	void LoadXml(tinyxml2::XMLElement* xml);

#ifndef SR_EDITOR
	Ogre::String GetStr(CarsXml* xml) const;
	bool Allows(CarsXml* xml, std::string car) const;
#endif
};
