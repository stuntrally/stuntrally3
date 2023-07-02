#include "OgreHlmsPbsPrerequisites.h"
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
	auto s = li->getItemNameAt(val);
	if (!s.empty() && s[0] == '#')
		s = s.substr(7);
	
	pSet->tweak_mtr = s;
	updTweakMtr();
}

void CGui::editMtrFind(Ed)
{
	FillTweakMtr();
}


//  🧰🎚️ init gui sliders
//--------------------------------------------------
void CGui::InitGuiTweakMtr()
{
	SV* sv;
	#define TWK(n,a,b)  sv= &twk.sv##n;  sv->Init(#n,  &twk.f##n, a, b, 1.5f, 3,5);  sv->DefaultF(0.0f);  Sev(TweakMtr);

	TWK(DiffR, 0.f, 1.5f)  TWK(DiffG, 0.f, 1.5f)  TWK(DiffB, 0.f, 1.5f)
	TWK(SpecR, 0.f, 1.5f)  TWK(SpecG, 0.f, 1.5f)  TWK(SpecB, 0.f, 1.5f)
	// TWK(Fresnel, 0.f, 1.5f)
	TWK(FresR, 0.f, 1.5f)  TWK(FresG, 0.f, 1.5f)  TWK(FresB, 0.f, 1.5f)

	TWK(Rough, 0.001f, 1.5f)  TWK(Metal, 0.f, 1.5f)
	TWK(ClearCoat, 0.f, 1.5f)  TWK(ClearRough, 0.001f, 1.5f)

	TWK(BumpScale, 0.0f, 6.f)

	auto cmb = fCmb("Workflow");  twk.cbWorkflow = cmb;
	cmb->addItem("Specular");  cmb->addItem("SpecularAsFresnel");  cmb->addItem("Metallic");

	twk.edInfo = fEd("MtrInfo");

	Edt(edMtrFind, "MtrFind", editMtrFind);
	InitClrTweakMtr();
	FillTweakMtr();
}

//  🔁 update db mat value
//--------------------------------------------------
void CGui::slTweakMtr(SV* sv)
{
	if (!twk.db)  return;
	const auto& s = sv->sName;

	     if (s=="DiffR" || s=="DiffG" || s=="DiffB")   twk.db->setDiffuse(Vector3(twk.fDiffR, twk.fDiffG, twk.fDiffB));
	else if (s=="SpecR" || s=="SpecG" || s=="SpecB")   twk.db->setSpecular(Vector3(twk.fSpecR, twk.fSpecG, twk.fSpecB));
	else if (s=="FresR" || s=="FresG" || s=="FresB")   twk.db->setFresnel(Vector3(twk.fFresR, twk.fFresG, twk.fFresB), true);

	else if (s=="Rough")       twk.db->setRoughness(twk.fRough);
	else if (s=="Metal")       twk.db->setMetalness(twk.fMetal);
	else if (s=="ClearCoat")   twk.db->setClearCoat(twk.fClearCoat);
	else if (s=="ClearRough")  twk.db->setClearCoatRoughness(twk.fClearRough);

	else if (s=="BumpScale")   twk.db->setNormalMapWeight(twk.fBumpScale);
}


//  🔁🌈 set Gui, slider values
//--------------------------------------------------
void CGui::updTweakMtr()
{
	auto hlms = Root::getSingleton().getHlmsManager()->getHlms( HLMS_PBS );
	twk.db = (HlmsPbsDatablock2*) hlms->getDatablock( pSet->tweak_mtr );
	if (!twk.db)  return;
	Vector3 v;  float f;  int i;

	v = twk.db->getDiffuse();   twk.fDiffR = v.x;  twk.fDiffG = v.y;  twk.fDiffB = v.z;  twk.svDiffR.Upd(); twk.svDiffG.Upd(); twk.svDiffB.Upd();
	v = twk.db->getSpecular();  twk.fSpecR = v.x;  twk.fSpecG = v.y;  twk.fSpecB = v.z;  twk.svSpecR.Upd(); twk.svSpecG.Upd(); twk.svSpecB.Upd();
	v = twk.db->getFresnel();   twk.fFresR = v.x;  twk.fFresG = v.y;  twk.fFresB = v.z;  twk.svFresR.Upd(); twk.svFresG.Upd(); twk.svFresB.Upd();

	f = twk.db->getRoughness();  twk.fRough = f;  twk.svRough.Upd();
	f = twk.db->getMetalness();  twk.fMetal = f;  twk.svMetal.Upd();
	f = twk.db->getClearCoat();  twk.fClearCoat = f;  twk.svClearCoat.Upd();
	f = twk.db->getClearCoatRoughness();  twk.fClearRough = f;  twk.svClearRough.Upd();

	f = twk.db->getNormalMapWeight();  twk.fBumpScale = f;  twk.svBumpScale.Upd();
	i = twk.db->getWorkflow();  twk.cbWorkflow->setIndexSelected(i);

	//----------------------------------------------------------------------------------------------------
	StringStream s;  // info only ..
	auto* tex = twk.db->getDiffuseTexture();    if (tex)  s << "Diffuse:    " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_NORMAL);      if (tex)  s << "Normal:    " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_SPECULAR);    if (tex)  s << "Specular:  " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_ROUGHNESS);   if (tex)  s << "Roughness: " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_EMISSIVE);    if (tex)  s << "Emissive:   " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_REFLECTION);  if (tex)  s << "Reflect:    " << tex->getNameStr() << endl;
	
	auto* sb = twk.db->getSamplerblock(0);
	if (sb)
	{
		const static String sFil[5] = {"none","point","lin","anis"};  // FilterOptions
		s << "Sampler:  min: " << sFil[sb->mMinFilter] << "  mag: " << sFil[sb->mMagFilter] << "  mip: " << sFil[sb->mMipFilter]
		<< "  Aniso: " << sb->mMaxAnisotropy << endl;

		const static String sWrap[5] = {"wrap","mirror","clamp","border"};  // TextureAddressingMode
		s << "Texture wrap uvw:  " << sWrap[sb->mU] << ", " << sWrap[sb->mV] << ", " << sWrap[sb->mW] << endl;
		
		s << "Renderables:  " << twk.db->getLinkedRenderables().size() << "  Name:  " << twk.db->getNameStr() << endl;  // meh hash
		s << endl;
	}

	//auto* bb = twk.db->getBlendblock();
	auto* mb = twk.db->getMacroblock();
	if (mb)
	{
		// mb->mDepthClamp;  mDepthFunc
		const static String sCull[4] = {"none","cw","ccw"};  // CullingMode
		s << "Depth:  check " << mb->mDepthCheck << "  write " << mb->mDepthWrite << "  bias " << mb->mDepthBiasConstant
		<< "  slope " << mb->mDepthBiasSlopeScale << "  Cull: " << sCull[mb->mCullMode] << endl;
	}
	s << "Two sided:  " << twk.db->getTwoSidedLighting()
	  << "  receive shadows:  " << twk.db->getReceiveShadows() << endl;

	const static String sTr[4]= {"none", "transp", "fade", "refract"};  // TransparencyModes
	s << "Transparency:  " << twk.db->getTransparency() << "  mode: " << sTr[twk.db->getTransparencyMode()] << endl;

	s << "Alpha test:  " << twk.db->getAlphaTest() << "  threshold: " << twk.db->getAlphaTestThreshold()
	  << "  from tex:  " << twk.db->getUseAlphaFromTextures() << endl;
	s << endl;

	// twk.db->getFilenameAndResourceGroup(&fname, &grp);
	s << "Brdf: " << twk.db->getBrdf()
	  << "  Refract strength: " << twk.db->getRefractionStrength() << endl;
	// s << "cubemap probe: " << (twk.db->getCubemapProbe() ? "yes" : "no") << endl;

	s << "Emissive: " << twk.db->getEmissive() << "  as lightmap: " << twk.db->getUseEmissiveAsLightmap() << endl;
	s << "Backgr Diff clr: " << twk.db->getBackgroundDiffuse() << endl;

	s << "Detail offset,scale: " << twk.db->getDetailMapOffsetScale(0) << endl;
	s << "Detail weights map: " << twk.db->getDetailMapWeight(0)
	  << "  normal: " << twk.db->getDetailNormalWeight(0) << endl;
	// twk.db->hasSeparateFresnel()
	// s << ": " << twk.db->getDetailMapBlendMode() << endl;
	
	twk.edInfo->setCaption(s.str());
}


//  🔁📃 Upd Fill materials list
//--------------------------------------------------
void CGui::FillTweakMtr()
{
	liTweakMtr->removeAllItems();
	liTweakMtr->addItem("");

	String srch = edMtrFind->getCaption();
	StringUtil::toLowerCase(srch);

	for (const auto& m : vsMaterials)
	if (m != "road" && m != "road_terrain" && m != "column" && 
		m != "pipe_base" && m != "pipe_glass")  // no base mtr
	{
		auto s = m;
		StringUtil::toLowerCase(s);

		if (srch.empty() || s.find(srch) != string::npos)  // find match
		{
			String c;
			for (auto it = clrTweakMtr.begin(); it != clrTweakMtr.end(); ++it)
			{
				if (s.find(it->first) != string::npos)
				{
					c = it->second;
					break;
			}	}
			liTweakMtr->addItem(c + m);
	}	}

	for (size_t i=0; i < liTweakMtr->getItemCount(); ++i)
	if (StringUtil::endsWith(liTweakMtr->getItemNameAt(i), pSet->tweak_mtr))
	{
		liTweakMtr->setIndexSelected(i);  break;
	}
}


//  🆕🌈 init mtr colors  only for gui list
//--------------------------------------------------
void CGui::InitClrTweakMtr()
{
	auto& clr = clrTweakMtr;
	#define add(a,b)  clr.push_back(make_pair(a,b))
	// add("base"  ,"#000000");
	add("cyan"  ,"#00F0F0");  add("green" ,"#00F000");  add("jungle","#20FF00");
	add("moss"  ,"#00FFA0");  add("blue"  ,"#1060F0");
	add("mud"   ,"#904020");  add("crysta","#2060C0");  add("marble","#F090D0");
	add("red"   ,"#FF0808");  add("yellow","#F0F000");  add("desert","#F0E090");
	add("orange","#FF9000");  add("lava"  ,"#FF4000");
	add("white" ,"#FFFFFF");  add("ice"   ,"#F9F9F9");  add("snow"  ,"#EEEEEE");
	add("viol"  ,"#8040FF");  add("pink"  ,"#FF40FF");
	add("sunset","#F0C010");  add("alien" ,"#90F020");  add("space" ,"#90C0E0");
	add("dark"  ,"#202020");  add("sand"  ,"#FFE0B0");  add("savan" ,"#A0F080");
	add("stone" ,"#C0C0C0");  add("metal" ,"#6090C0");  add("gravel","#909090");

	add("glass" ,"#F0F8FF");  add("road"  ,"#904000");  add("pipe"  ,"#C0C000");
	add("water" ,"#60A0FF");  add("river" ,"#80C0FF");  add("_ter"  ,"#403020");
	add("house" ,"#A0C0D0");  add("balloon","#F0A040"); add("pyrami","#F0C010");
}

//  🆕📃 Get all materials  once  for gui list
//--------------------------------------------------
void CGui::GetTweakMtr(String path)
{
	liTweakMtr = fLi("TweakMtr");  Lev(liTweakMtr, TweakMtr);

	vsMaterials.clear();
	vsMaterials.push_back("#0080FF  ~~~~  Water  ~~~~");
	GetMaterialsMat(path+"water.material",0);
	vsMaterials.push_back("#FFFF00    ---  o O   Pipe  O o  ---");
	GetMaterialsMat(path+"pipe.material",0);
	vsMaterials.push_back("#807010  -----===  Road  ===-----");
	GetMaterialsMat(path+"road.material",0);

	vsMaterials.push_back("#808080  ----[]  objects static  []----");
	GetMaterialsMat(path+"objects_static.material",0);
	vsMaterials.push_back("#808080  ----[]  objects static2  []----");
	GetMaterialsMat(path+"objects_static2.material",0);
	vsMaterials.push_back("#808080  ----x  objects dynamic  x----");
	GetMaterialsMat(path+"objects_dynamic.material",0);

	vsMaterials.push_back("#80FF80  --------Y  trees  Y--------");
	GetMaterialsMat(path+"trees.material",0);
	vsMaterials.push_back("#C0F080  --------Y  trees ch  Y--------");
	GetMaterialsMat(path+"trees_ch.material",0);
	vsMaterials.push_back("#80C080  --------Y  trees old  Y--------");
	GetMaterialsMat(path+"trees_old.material",0);
	vsMaterials.push_back("#C0C0C0  ----cc  rocks  cc----");
	GetMaterialsMat(path+"rocks.material",0);

	vsTweakMtrs = vsMaterials;
}
