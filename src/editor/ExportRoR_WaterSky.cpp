#include "pch.h"
#include "ExportRoR.h"
#include "Def_Str.h"
// #include "settings.h"

#include "CGui.h"
#include "CScene.h"
#include "PresetsXml.h"

#include <OgreString.h>
#include <OgreVector3.h>
#include <string>
using namespace Ogre;
using namespace std;


//------------------------------------------------------------------------------------------------------------------------
//  🌊 Water  and  ⛅ Caelum .os
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportWaterSky()
{

	//  🌊 Fluids
	//------------------------------------------------------------
	Ywater = 0.f;
	water = 0;
	
	switch (cfg->water)
	{
	case -1:  // off
		water = 0;
		break;
	case 0:  // auto
		//  get 1 big for water level
		for (const auto& fl : sc->fluids)
		{
			if (!water && sc->fluids.size()==1 || fl.size.x > 200.f)  // pick 1st big
			{	water = 1;
				Ywater = fl.pos.y - hmin + cfg->yWaterOfs;
		}	}
		break;
	case 1:  // force custom manual
		water = 1;
		Ywater = cfg->yWaterOfs;  break;
	}
	gui->Exp(CGui::NOTE, String("Water: ")+(water ? "yes" : "no")+"  Y level: "+fToStr(Ywater));


	//  ⛅ Caelum  sun light etc  save  .os
	//------------------------------------------------------------------------------------------------------------------------
	string osFile = path + name + ".os";
	ofstream os;
	os.open(osFile.c_str(), std::ios_base::out);

	os << "caelum_sky_system " + name + ".os\n";
	os << "{\n";

	const PSky* sky = pre->GetSky(sc->skyMtr);
	float cld = sky ? sky->clouds : 0.2f;

	//  julian
	double juld = !sky ? 2458971.0 :  // may 12:00
		2458979.5 + double(sky->daytime) * 0.0416666;  // 0:00 + t * 1 h
	ostringstream ss;  ss.width(15);
	ss << fixed << setprecision(6) << juld;

	gui->Exp(CGui::TXT, String("Sky: ")+sc->skyMtr+" clouds "+fToStr(cld)+
						" time "+fToStr(sky->daytime)+" latitude "+fToStr(sky->latitude));
	os << "	julian_day " << ss.str() << "\n";
	os << "	time_scale 1\n";
	os << "	longitude 0\n";  // from sun dir?-
	os << "	latitude " << sky->latitude << "\n";
	os << "\n";

	os << "	point_starfield {\n";  // for night
	os << "		magnitude_scale 12.51189\n";
	os << "		mag0_pixel_size 16\n";
	os << "		min_pixel_size 4\n";
	os << "		max_pixel_size 6\n";
	os << "	}\n";
	os << "\n";
	os << "	manage_ambient_light true\n";
	os << "	minimum_ambient_light 0.05 0.05 0.1\n";
	os << "\n";
	// os << "	manage_scene_fog yes\n";  // todo..  unrecognized
	os << "	ground_fog_density_multiplier 0.015\n";  // cg! no diff-
	os << "	scene_fog_density_multiplier 0.015\n";
	os << "\n";

	//  🌞 sun light
	//------------------------------------------------------------
	os << "	sun {\n";
	Vector3 la = sc->lAmb.GetRGB()  * cfg->lAmb + cfg->lAmbAdd;  // par bright
	Vector3 ld = sc->lDiff.GetRGB() * cfg->lDiff;
	Vector3 ls = sc->lSpec.GetRGB() * cfg->lSpec;
	os << "		ambient_multiplier "+fToStr(la.x,3,5)+" "+fToStr(la.y,3,5)+" "+fToStr(la.z,3,5)+"\n";
	os << "		diffuse_multiplier "+fToStr(ld.x,3,5)+" "+fToStr(ld.y,3,5)+" "+fToStr(ld.z,3,5)+"\n";
	os << "		specular_multiplier "+fToStr(ls.x,3,5)+" "+fToStr(ls.y,3,5)+" "+fToStr(ls.z,3,5)+"\n";
	os << "\n";
	os << "		auto_disable_threshold 0.05\n";
	os << "		auto_disable true\n";
	os << "	}\n";
	os << "\n";

	os << "	moon {\n";
	os << "		ambient_multiplier 0.2 0.2 0.2\n";
	os << "		diffuse_multiplier 1 1 0.9\n";
	os << "		specular_multiplier 1 1 1\n";
	os << "\n";
	os << "		auto_disable_threshold 0.05\n";
	os << "		auto_disable true\n";
	os << "	}\n";
	os << "\n";
	os << "	sky_dome {\n";
	os << "		haze_enabled yes\n";
	os << "		sky_gradients_image EarthClearSky2.png\n";
	os << "		atmosphere_depth_image AtmosphereDepth.png\n";
	os << "	}\n";
	os << "\n";

	//  clouds  factor from presets.xml
	//------------------------------------------------------------
	if (cld > 0.f)
	{
		os << "	cloud_system\n";
		os << "	{\n";
		os << "		cloud_layer low\n";
		os << "		{\n";
		os << "			height 2000\n";
		os << "			coverage "<< min(cld, 0.2f) <<"\n";
		// os << "			cloud_uv_factor " << 500 <<"\n";  //..cd
		os << "		}\n";
		if (cld >= 0.4f)
		{
			os << "		cloud_layer mid\n";
			os << "		{\n";
			os << "			height 2700\n";
			os << "			coverage "<< min(cld, 0.6f) <<"\n";
			os << "		}\n";
		}
		if (cld >= 0.7f)
		{
			os << "		cloud_layer high\n";
			os << "		{\n";
			os << "			height 3500\n";  // vis far is 5000 max
			os << "			coverage "<< min(cld, 1.0f) <<"\n";
			os << "		}\n";
		}
		os << "	}\n";
	}

	//  🌧️ weather
	//------------------------------------------------------------
	int rain = 0, snow = 0;
	if ( sc->rainName.find("Rain") != string::npos)  rain += sc->rainEmit;
	if (sc->rain2Name.find("Rain") != string::npos)  rain += sc->rain2Emit;
	if ( sc->rainName.find("Snow") != string::npos)  snow += sc->rainEmit;
	if (sc->rain2Name.find("Snow") != string::npos)  snow += sc->rain2Emit;
	
	if (rain > snow && rain > 10)
	{
		rain = min(rain, 9000);
		os << "	precipitation\n";
		os << "	{\n";
		os << "		intensity " << 1.f * rain / 9000.f << "\n";
		// if (rain > 4000)  // no
		// 	os << "		texture precipitation_rain.png\n";
		// else  //  thin fine
			os << "		texture precipitation_drizzle.png\n";
		os << "		wind_speed 0.2 0 0.2\n";
		os << "		camera_speed_scale 0.01 0.01 0.01\n";  // meh
		os << "	}\n";
	}else
	if (snow > rain && snow > 10)
	{
		rain = min(snow, 5000);
		os << "	precipitation\n";
		os << "	{\n";
		os << "		intensity " << 0.1f + 0.6f * rain / 5000.f << "\n";
		os << "		texture precipitation_snow.png\n";
		os << "		wind_speed .02 0 .02\n";
		os << "		camera_speed_scale 0.0 0.0 0.0\n";
		os << "		falling_direction 0 -0.2 0.02\n";
		os << "	}\n";
	}

	os << "}\n";

	os.close();
}
