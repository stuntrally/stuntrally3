#include "pch.h"
#include "Def_Str.h"
#include "PaintsIni.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <OgreString.h>
using namespace Ogre;
using namespace std;


//  default under glow  same as main
void CarPaint::GlowDef()
{
	if (type == CP_3Clrs)
		glow = paints[1];  //0 1 2?
	else
		glow = clr[0];
}

void PaintsIni::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
}
void PaintsIni::Save(std::string sfile)
{
	CONFIGFILE c;
	Serialize(true, c);  c.Write(sfile);
}

void PaintsIni::SerPaint(bool wr, CONFIGFILE & cf, const std::string s, CarPaint& p)
{
	Param(cf,wr, s+"new_line", p.new_line);
	Param(cf,wr, s+"rate", p.rate);
	Param(cf,wr, s+"group", p.group);
	
	Param(cf,wr, s+"type", p.type);
	int n = 3;
	switch (p.type)
	{
	case CP_OneClr:    n = 1;  break;
	case CP_DiffSpec:  n = 2;  break;
	}

	for (int c = 0; c < n; ++c)  // clrs
	{
		auto& cl = n == 3 ? p.paints[c] : p.clr[c];
		const auto sc = s + toStr(c);
		Param(cf,wr, sc+"hue", cl.hue);
		Param(cf,wr, sc+"sat", cl.sat);
		Param(cf,wr, sc+"val", cl.val);
	}

	Param(cf,wr, s+"gloss", p.gloss);
	Param(cf,wr, s+"rough", p.rough);

	Param(cf,wr, s+"clear_coat",  p.clear_coat);
	Param(cf,wr, s+"clear_rough", p.clear_rough);
	Param(cf,wr, s+"fresnel", p.fresnel);
	
	if (n==3)
	{	Param(cf,wr, s+"p1all", p.paintMulAll);
		Param(cf,wr, s+"p2mul", p.paintMul2nd);
		Param(cf,wr, s+"p3pow", p.paintPow3rd);
	}
	p.GlowDef();
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
		v.reserve(500);
		for (int i=0; i < MaxPaints; ++i)
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
