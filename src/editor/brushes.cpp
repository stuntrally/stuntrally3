#include "pch.h"
#include "Def_Str.h"
#include "brushes.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <OgreString.h>
using namespace Ogre;
using namespace std;


void BrushesIni::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
	LogO("L*** Loaded brushes: "+toStr(v.size()));
}
void BrushesIni::Save(std::string sfile)
{
	CONFIGFILE c;
	Serialize(true, c);  c.Write(sfile);
}

void BrushesIni::SerBrush(bool wr, CONFIGFILE & cf, const std::string s, BrushSet& p)
{
	Param(cf,wr, s+"ed",   p.edMode);
	Param(cf,wr, s+"size", p.size);  Param(cf,wr, s+"int",  p.intens);  Param(cf,wr, s+"pow",  p.power);
	Param(cf,wr, s+"freq", p.freq);  Param(cf,wr, s+"oct",  p.octaves);
	Param(cf,wr, s+"ofsX", p.offsetX);  Param(cf,wr, s+"ofsY", p.offsetY);
	Param(cf,wr, s+"shape",p.shape);
	Param(cf,wr, s+"filt", p.filter);  Param(cf,wr, s+"hset", p.height);
	Param(cf,wr, s+"newline", p.newLine);  Param(cf,wr, s+"name", p.name);  Param(cf,wr, s+"rate", p.rate);
}


//  🖌️📄 Brushes.ini
void BrushesIni::Serialize(bool w, CONFIGFILE & c)
{
	Param(c,w, "all.perRow", perRow);
	Param(c,w, "all.imgSize", imgSize);

	auto str = [](int i)
	{
		std::ostringstream s;
		s.width(3);  s.fill('0');
		s << std::fixed << i << '.';
		return s.str();
	};

	if (w)
	{	//  write
		int si = v.size();
		for (int i=0; i < si; ++i)
		{
			auto s = str(i);
			SerBrush(w,c, s, v[i]);
		}
	}else  // read
	{	v.clear();
		v.reserve(200);
		for (int i=0; i < MaxBrushes; ++i)
		{
			auto s = str(i);
			float f;
			if (c.GetParam(s + "ed", f))
			{
				BrushSet br;
				SerBrush(w,c, s, br);
				v.push_back(br);
			}
		}
	}
}
