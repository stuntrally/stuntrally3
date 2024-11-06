#pragma once
#include "configfile.h"


//  🖌️ brush shapes, functions
enum EBrShape
{   BRS_Triangle=0, BRS_Sinus, BRS_Ngon, BRS_Noise, BRS_Noise2, BRS_ALL  };
extern const Ogre::String csBrShape[BRS_ALL];

///<>  🖌️⛰️ terrain edit, brush setup
struct BrushSet
{
	/*ED_MODE*/int edMode =0;
	float size = 20.f, intens = 20.f;  // edit
	/*EBrShape*/int shape = BRS_Sinus;  float power = 1.f;  // funct
	float freq = 0.2f;  int octaves = 3;  // 🌀 noise
	float offsetX = 1.f, offsetY = 1.f;
	float filter = 2.f, height = 10.f;  // special
	int newLine = 0;  Ogre::String name;  // for gui
	int rate = 0;
};
const int MaxBrushes = 255;  // par max ini, fit 2k prv tex


//  🖌️📄 Brushes.ini  palette for Gui 🎨
//--------------------------------------------------------------------
class BrushesIni
{
public:
	const static Ogre::String csBrShape[BRS_ALL];

	std::vector<BrushSet> v;
	int perRow = 12, imgSize = 48;  // gui params
	
//------------------------------------------
	template <typename T>
	static bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	static void SerBrush(bool write, CONFIGFILE & config, const std::string s, BrushSet& p);

	void Serialize(bool write, CONFIGFILE & config);
	void LoadDef();
	void Load(std::string sfile);
	void Save(std::string sfile);
};
