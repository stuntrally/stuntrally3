#include "pch.h"
#include "Def_Str.h"
#include "TracksXml.h"
#include "CollectXml.h"
#include "SceneXml.h"
#include "paths.h"
#include <regex>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace Ogre;
using namespace std;


//  Load collections
//-------------------------------------------------------------------------------------------------------------
bool CollectXml::LoadXml(std::string file, TracksIni* trks, bool check)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	all.clear();

	///  challs
	const char* a;
	XMLElement* eCol = root->FirstChildElement("collection");
	while (eCol)
	{
		Collection c;  // defaults in ctor
		a = eCol->Attribute("name");
		if (a)
		{	c.nameGui = std::string(a);
			regex re("#[0-9a-fA-F]{6}");  // remove colors from name
			c.name = regex_replace(c.nameGui, re, "");
		}
		a = eCol->Attribute("descr");		if (a)  c.descr = std::string(a);
		a = eCol->Attribute("ver");			if (a)  c.ver = s2i(a);
		a = eCol->Attribute("difficulty");	if (a)  c.diff = s2i(a);
		a = eCol->Attribute("type");		if (a)  c.type = s2i(a);
		a = eCol->Attribute("groups");		if (a)  c.groups = s2i(a);
		
		XMLElement* eSim = eCol->FirstChildElement("sim");
		if (eSim)
		{
			a = eSim->Attribute("mode");	if (a)  c.sim_mode = std::string(a);

			a = eSim->Attribute("damage");	if (a)  c.damage_type = s2i(a);  // range chk..
			
			a = eSim->Attribute("boost");	if (a)  c.boost_type = s2i(a);
			a = eSim->Attribute("flip");	if (a)  c.flip_type = s2i(a);
			a = eSim->Attribute("rewind");	if (a)  c.rewind_type = s2i(a);
		}

		c.cars.LoadXml(eCol);  //  allowed
		
		XMLElement* ePass = eCol->FirstChildElement("pass");
		if (ePass)
		{
			a = ePass->Attribute("need");    if (a)  c.need = s2i(a) > 0;  // 0 infinite  1 timed
			a = ePass->Attribute("time");    if (a)  c.time = s2time(a);
			a = ePass->Attribute("prizes");  if (a)  c.prizes = s2i(a);
			a = ePass->Attribute("next");    if (a)  c.next = s2r(a);
		}

		//  track
		XMLElement* eTr = eCol->FirstChildElement("track");
		if (eTr)
		{
			a = eTr->Attribute("name");		if (a)  c.track = std::string(a);
		}

		//----  gems, load scene.xml  meh..
		string path = PATHS::Tracks() + "/"+ c.track +"/";
		Scene sc;  sc.LoadXmlCollects(path +"scene.xml");
		
		for (auto& q : sc.qcols)
		if ( (1u << q.group) & c.groups)
			c.collects.push_back(q);
		//--------
		
		all.push_back(c);
		eCol = eCol->NextSiblingElement("collection");
	}
	return true;
}


//  Load progress
//-------------------------------------------------------------------------------------------------------------
bool ProgressCXml::LoadXml(std::string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;

	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	col.clear();  icol.clear();

	const char* a;
	XMLElement* eCol = root->FirstChildElement("collect");
	while (eCol)
	{
		ProgressCollect pc;
		a = eCol->Attribute("name");	if (a)  pc.name = std::string(a);
		a = eCol->Attribute("track");	if (a)  pc.track = std::string(a);
		a = eCol->Attribute("ver");		if (a)  pc.ver = s2i(a);

		a = eCol->Attribute("t");	if (a)  pc.bestTime = s2r(a);
		a = eCol->Attribute("e");	if (a)  pc.fin = s2i(a);
		
		XMLElement* eG = eCol->FirstChildElement("g");
		while (eG)  // gems
		{
			int i = 0;  bool z =0;
			a = eG->Attribute("i");  if (a)  i = s2r(a);
			a = eG->Attribute("z");  if (a)  z = s2i(a) > 0;  // collected
			
			pc.gems[i] = z;
			eG = eG->NextSiblingElement("g");
		}

		col.push_back(pc);
		icol[pc.name] = col.size();
		eCol = eCol->NextSiblingElement("collect");
	}
	return true;
}

//  Save progress
bool ProgressCXml::SaveXml(std::string file)
{
	XMLDocument xml;
	XMLElement* root = xml.NewElement("progress");

	for (int i=0; i < col.size(); ++i)
	{
		const ProgressCollect& pc = col[i];
		XMLElement* eCol = xml.NewElement("collect");
			eCol->SetAttribute("name", pc.name.c_str() );
			eCol->SetAttribute("track", pc.track.c_str() );
			eCol->SetAttribute("ver",  toStrC( pc.ver ));

			eCol->SetAttribute("t",  fToStr( pc.bestTime, 1).c_str());
			eCol->SetAttribute("e",  toStrC( pc.fin ));

			for (auto& g : pc.gems)
			{
				XMLElement* eG = xml.NewElement("g");

				eG->SetAttribute("i",  iToStr( g.first ).c_str());
				eG->SetAttribute("z",  g.first ? "1" : "0");  // collected
				eCol->InsertEndChild(eG);
			}
		root->InsertEndChild(eCol);
	}
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}

ProgressCollect* ProgressCXml::Get(std::string name)
{
	auto f = icol.find(name);
	return f != icol.end() ? &col[f->second -1] : 0;
}
