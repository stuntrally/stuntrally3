#include "pch.h"
#include "paints.h"
#include <stdio.h>
#include <iostream>
#include <OgreString.h>
using namespace Ogre;
using namespace std;


// void PaintsIni::Load(std::string sfile)
// {
// 	CONFIGFILE c;  c.Load(sfile);
// 	Serialize(false, c);
// }
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
	
	int s = v.size();
	for (int i=0; i < s; ++i)
	{
		std::ostringstream s;
		s.width(3);  s.fill('0');
		s << std::fixed << i << '.';
		SerPaint(w,c, s.str(), v[i]);
	}
}


//  🎨 Load car colors.ini 📄
//--------------------------------------------------------------------------------------------------------------------------------------
bool PaintsIni::LoadOld(string file)
{
	v.clear();

	char s[256];

	ifstream fs(file.c_str());
	if (fs.fail())  return false;
	
	while (fs.good())
	{
		fs.getline(s,254);
		
		if (strlen(s) > 0 && s[0] != '#' && s[0] != '/' && s[0] != ' ')  //  comment
		{
			string t = s;  //  params
			     if (t.substr(0,6) == "perRow")   perRow =  s2i(t.substr(6));
			else if (t.substr(0,7) == "imgSize")  imgSize = s2i(t.substr(7));
			else
			//  color, starting with digit
			if (s[0] >= '0' && s[0] <= '9')
			{
				CarPaint c;
				sscanf(s, "%f %f %f %f %f %f",
					&c.clr[0].hue, &c.clr[0].sat, &c.clr[0].val, &c.gloss, &c.metal, &c.rough);
				if (c.metal > 1.f)  c.metal = 1.f;
				if (c.rough > 1.f)  c.rough = 1.f;

				v.push_back(c);
			}
	}	}
	return true;
}
