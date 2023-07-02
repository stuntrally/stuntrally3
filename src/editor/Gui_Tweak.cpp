#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Slider.h"

#include "HlmsPbs2.h"
#include <OgreHlmsCommon.h>
#include <OgreHlmsPbsDatablock.h>
#include <MyGUI.h>
#include <utility>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  gui tweak page, material properties  new PBS
//------------------------------------------------------------------------------------------------------------

//  👆 pick material from list
void CGui::listTweakMtr(Li li, size_t val)
{
	pSet->tweak_mtr = li->getItemNameAt(val);
	updTweakMtr();
}


//  🎚️ init gui sliders
void CGui::InitGuiTweakMtr()
{
	SV* sv;
	#define TWK(n,a,b)  sv= &twk.sv##n;  sv->Init(#n,  &twk.f##n, a, b, 1.5f, 3,5);  sv->DefaultF(0.0f);  Sev(TweakMtr);

	TWK(DiffR, 0.f, 1.5f)  TWK(DiffG, 0.f, 1.5f)  TWK(DiffB, 0.f, 1.5f)
	TWK(SpecR, 0.f, 1.5f)  TWK(SpecG, 0.f, 1.5f)  TWK(SpecB, 0.f, 1.5f)
	// TWK(Fresnel, 0.f, 1.5f)
	TWK(FresR, 0.f, 1.5f)  TWK(FresG, 0.f, 1.5f)  TWK(FresB, 0.f, 1.5f)
	TWK(Rough, 0.001f, 1.5f)
	TWK(ClearCoat, 0.f, 1.5f)  TWK(ClearRough, 0.001f, 1.5f)
}

//  🔁 update db mat value
void CGui::slTweakMtr(SV* sv)
{
	if (!twk.db)  return;
	const auto& s = sv->sName;

	     if (s=="DiffR" || s=="DiffG" || s=="DiffB")   twk.db->setDiffuse(Vector3(twk.fDiffR, twk.fDiffG, twk.fDiffB));
	else if (s=="SpecR" || s=="SpecG" || s=="SpecB")   twk.db->setSpecular(Vector3(twk.fSpecR, twk.fSpecG, twk.fSpecB));
	else if (s=="FresR" || s=="FresG" || s=="FresB")   twk.db->setFresnel(Vector3(twk.fFresR, twk.fFresG, twk.fFresB), true);
	else if (s=="Rough")       twk.db->setRoughness(twk.fRough);
	else if (s=="ClearCoat")   twk.db->setClearCoat(twk.fClearCoat);
	else if (s=="ClearRough")  twk.db->setClearCoatRoughness(twk.fClearRough);
}

//  🔁🌈 set gui slider values
void CGui::updTweakMtr()
{
	auto hlms = Root::getSingleton().getHlmsManager()->getHlms( HLMS_PBS );
	twk.db = (HlmsPbsDatablock2*) hlms->getDatablock( pSet->tweak_mtr );
	if (!twk.db)  return;
	Vector3 v;  float f;

	v = twk.db->getDiffuse();   twk.fDiffR = v.x;  twk.fDiffG = v.y;  twk.fDiffB = v.z;  twk.svDiffR.Upd(); twk.svDiffG.Upd(); twk.svDiffB.Upd();
	v = twk.db->getSpecular();  twk.fSpecR = v.x;  twk.fSpecG = v.y;  twk.fSpecB = v.z;  twk.svSpecR.Upd(); twk.svSpecG.Upd(); twk.svSpecB.Upd();
	v = twk.db->getFresnel();   twk.fFresR = v.x;  twk.fFresG = v.y;  twk.fFresB = v.z;  twk.svFresR.Upd(); twk.svFresG.Upd(); twk.svFresB.Upd();
	f = twk.db->getRoughness();  twk.fRough = f;  twk.svRough.Upd();
	f = twk.db->getClearCoat();  twk.fClearCoat = f;  twk.svClearCoat.Upd();
	f = twk.db->getClearCoatRoughness();  twk.fClearRough = f;  twk.svClearRough.Upd();
	// twk.db->getDiffuseTexture()
	// twk.db->getBackgroundDiffuse()  //-

	// twk.db->getLinkedRenderables()  //?
	// twk.db->getWorkflow()  //=
	// twk.db->getUseEmissiveAsLightmap()
	// twk.db->getEmissive()
	// twk.db->getEmissiveTexture()
	
	// twk.db->getTwoSidedLighting()
	// twk.db->getTransparency()
	// twk.db->getTransparencyMode()

	// twk.db->getAlphaTest()
	// twk.db->getAlphaTestThreshold()
	// twk.db->getUseAlphaFromTextures()
	
	// twk.db->getNormalMapWeight()  //+
	// twk.db->getMetalness()  //-
	// twk.db->getRefractionStrength()
	// twk.db->getReceiveShadows()
	
	// twk.db->getNameStr()
	// twk.db->getFilenameAndResourceGroup(
	// twk.db->getBrdf()  //=
	// twk.db->getCubemapProbe()  //-
	
	// twk.db->getDetailMapBlendMode()  //`
	// twk.db->getDetailMapOffsetScale()
	// twk.db->getDetailMapWeight()
	// twk.db->getDetailNormalWeight()
}


#if 0
	ColourValue diff, spec;  //, fresn1, fresn2;
	auto c = gc.add(0];
	diff.setHSB(1.f - c.hue, c.sat, c.val);

	db->setWorkflow(  // once?
		HlmsPbsDatablock::SpecularWorkflow  // par?
		// HlmsPbsDatablock::SpecularAsFresnelWorkflow
		// HlmsPbsDatablock::MetallicWorkflow
	);
	// db->setMetalness( gc.metal );  // only in metallic
	
	// db->setIndexOfRefraction( Vector3::UNIT_SCALE * (3.f-gc.fresnel*3.f), false );
	db->setFresnel( Vector3::UNIT_SCALE * gc.fresnel, false );
#endif


//  🌈 set gui slider values  Init  only for gui list
void CGui::ClrTweakMtr()
{
	std::vector<pair<String,String>> clr;
	#define add(a,b)  clr.push_back(make_pair(a,b))
	add("base"  ,"#101010");
	add("cyan"  ,"#00F0F0");  add("green" ,"#00F000");  add("jungle","#20FF00");
	add("moss"  ,"#00FFA0");  add("blue"  ,"#1060F0");
	add("red"   ,"#FF0808");  add("yellow","#F0F000");  add("desert","#F0E090");
	add("orange","#FF9000");  add("lava"  ,"#FF4000");
	add("white" ,"#FFFFFF");  add("ice"   ,"#F9F9F9");  add("snow"  ,"#EEEEEE");
	add("viol"  ,"#8040FF");  add("pink"  ,"#FF40FF");
	add("sunset","#F0C010");  add("alien" ,"#90F020");  add("space" ,"#90C0E0");
	add("dark"  ,"#202020");  add("sand"  ,"#FFE0B0");  add("savan" ,"#A0F080");
	add("stone" ,"#C0C0C0");  add("metal" ,"#6090C0");  add("gravel","#909090");
	add("mud"   ,"#904020");  add("crysta","#2060C0");  add("marble","#F090D0");

	add("glass" ,"#F0F8FF");  add("road"  ,"#904000");  add("pipe"  ,"#C0C000");
	add("water" ,"#60A0FF");  add("river" ,"#80C0FF");  add("_ter"  ,"#403020");
	add("house" ,"#A0C0D0");  add("balloon","#F0A040"); add("pyrami","#F0C010");

	for (auto& mat : vsMaterials)
	{
		auto s = mat;
		StringUtil::toLowerCase(s);
		
		String c;
		for (auto it = clr.begin(); it != clr.end(); ++it)
		{
			if (s.find(it->first) != string::npos)
			{
				c = it->second;
				break;
		}	}
		liTweakMtr->addItem(c + mat);
	}
}
