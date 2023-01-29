#include "pch.h"
#include "paints.h"
#include <stdio.h>
#include <iostream>
#include <OgreString.h>
using namespace Ogre;
using namespace std;


void PaintsIni::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
}
void PaintsIni::Save(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(true, c);  c.Write();
}

void PaintsIni::SerPaint(bool wr, CONFIGFILE & cf, const std::string s, CarPaint& p)
{
	Param(cf,wr, s+"one_clr", p.one_clr);
	int cn = p.one_clr ? 1 : 2;
	for (int c = 0; c < cn; ++c)  // clrs
	{
		auto& cl = p.clr[c];
		const auto sc = s + toStr(c);
		Param(cf,wr, sc+"hue", cl.hue);
		Param(cf,wr, sc+"sat", cl.sat);
		Param(cf,wr, sc+"val", cl.val);
	}

	Param(cf,wr, s+"gloss", p.gloss);
	Param(cf,wr, s+"metal", p.metal);
	Param(cf,wr, s+"rough", p.rough);

	Param(cf,wr, s+"clear_coat",  p.clear_coat);
	Param(cf,wr, s+"clear_rough", p.clear_rough);
	Param(cf,wr, s+"fresnel", p.fresnel);
}


//  📄 paints.ini
void PaintsIni::Serialize(bool w, CONFIGFILE & c)
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
			SerPaint(w,c, s, v[i]);
		}
	}else  // read
	{	v.clear();
		for (int i=0; i < 200; ++i)  // max
		{
			auto s = str(i);
			float f;
			if (c.GetParam(s + "0hue", f))
			{
				CarPaint p;
				SerPaint(w,c, s, p);
				v.push_back(p);
			}
		}
	}
}
