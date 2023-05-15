#pragma once
#include "configfile.h"


//  brush shapes, functions
enum EBrShape
{   BRS_Triangle=0, BRS_Sinus, BRS_Ngon, BRS_Noise, BRS_Noise2, BRS_ALL  };
extern const Ogre::String csBrShape[BRS_ALL];

///<>  ⛰️ terrain edit, brush setup
struct BrushSet
{
	/*ED_MODE*/int edMode;
	float size,intens,power, freq,offset;
	int octaves;  /*EBrShape*/int shape;
	float filter, height;
	int newLine;  // gui
	Ogre::String name;
};
const int MaxBrushes = 255;  // par max ini


//  🎨 Brushes.ini  palette for Gui
//--------------------------------------------------------------------
class BrushesIni
{
public:
	const static Ogre::String csBrShape[BRS_ALL];

	std::vector<BrushSet> v;
	int perRow = 12, imgSize = 18;  // gui params
	
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
