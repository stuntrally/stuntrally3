#include "pch.h"
#include "Def_Str.h"
#include "SceneClasses.h"
#include "Axes.h"
#include <OgreSceneNode.h>
#include <filesystem>
namespace fs = std::filesystem;
using namespace std;
using namespace Ogre;


///  bullet to ogre  ----------
void Object::SetFromBlt()
{
	if (!nd)  return;
	nd->setPosition(Axes::toOgre(pos));
	nd->setOrientation(Axes::toOgreW(rot));
}


//  TerData
//------------------------------------------
TerData::TerData()
{
	Default();
}
void TerData::Default()
{
	iVertsX = 1024;  iVertsXold = 1024;
	fTriangleSize = 1.f;  // scale
	triplanarLayer1 = 8;  triplanarLayer2 = 8;  triplCnt = 0;  // off
	// errorNorm = 1.7;  normScale = 1.f;  specularPow = 32.f;
	emissive = false;  specularPowEm = 2.f;
	// road1mtr = true;

	for (int i=0; i < ciNumLay; ++i)
	{	
		TerLayer& l = layersAll[i];  l.tiling = 8.5f;
		l.on = i==0;
		l.texFile = "";  l.texNorm = "";
		l.dust = 0.f;  l.mud = 1.f;  l.smoke = 0.f;
		l.tclr = SColor(0.16f,0.5f,0.2f,0.7f);
		l.fDamage = 0.f;
	}
	UpdVals();  UpdLayers();
}

void TerData::UpdVals()
{
	fTerWorldSize = iVertsX * fTriangleSize;
}

int TerData::getFileSize(std::string filename)
{
	int fsize = fs::file_size(filename);
	int size = fsize / sizeof(float);
	
	iVertsX = sqrt(size);  // [] square ter only
	iVertsXold = iVertsX;
	
	if (iVertsX % 2 == 1)
	{
		--iVertsX;  // 1025 to 1024 etc
		//  set if hmap old, read from xml for new
		ofsZ = fTriangleSize;
	}
	UpdVals();
	return fsize;
}


//  fill only active layers
//------------------------------------------
void TerData::UpdLayers()
{
	layers.clear();  int li = 0;
	triplanarLayer1 = 8;  triplanarLayer2 = 8;  triplCnt = 0;  // off
	for (int i=0; i < ciNumLay; ++i)
	{
		if (layersAll[i].on)
		{
			if (layersAll[i].triplanar)
			{	++triplCnt;
				if (triplanarLayer1 < 8)
					triplanarLayer2 = li;
				else
					triplanarLayer1 = li;  }
			++li;
			layers.push_back(i);
	}	}
}
