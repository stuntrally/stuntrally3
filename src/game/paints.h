#pragma once
#include "configfile.h"


//  🎨 paint  for vehicle
struct CarPaint
{
	bool one_clr = true;  // use 1 for all, in [0]
	struct Clr
	{	float hue = 0.35f, sat = 0.98f, val = 0.8f;
	} clr[4];  //  diff, spec, fresnel1, fresnel2;
	
	float gloss = 0.9f,  // mix:  0 all diff .. 1 all spec
		metal = 1.f,  // not used-
		rough = 0.25f,
		clear_coat = 1.f,
		clear_rough = 0.01f,
		fresnel = 1.f;

};


//  🎨 paints.ini  palette for Gui
//--------------------------------------------------------------------
class PaintsIni
{
public:
	std::vector<CarPaint> v;
	int perRow = 12, imgSize = 18;  // gui params
	
//------------------------------------------
	// PaintsIni();

	template <typename T>
	static bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	static void SerPaint(bool write, CONFIGFILE & config, const std::string s, CarPaint& p);

	void Serialize(bool write, CONFIGFILE & config);
	void Load(std::string sfile);
	void Save(std::string sfile);
};
