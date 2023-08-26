#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "settings.h"
#include "App.h"
#include "CGui.h"
#include "Slider.h"
#include "paths.h"

#include "HlmsPbs2.h"
#include <OgreVector3.h>
#include <OgreHlmsPbsPrerequisites.h>
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
void CGuiCom::listTweakMtr(Li li, size_t val)
{
	auto s = li->getItemNameAt(val);
	if (!s.empty() && s[0] == '#')
		s = s.substr(7);
	
	pSet->tweak_mtr = s;
	updTweakMtr();
}

void CGuiCom::editMtrFind(Ed)
{
	FillTweakMtr();
}


//  🧰🎚️ init gui sliders
//--------------------------------------------------
void CGuiCom::InitGuiTweakMtr()
{
	SV* sv;
	#define TWK(n,a,b)  sv= &twk.sv##n;  sv->Init(#n,  &twk.f##n, a, b, 1.5f, 3,5);  sv->DefaultF(0.0f);  SevC(TweakMtr);

	TWK(DiffR, 0.f, 1.5f)  TWK(DiffG, 0.f, 1.5f)  TWK(DiffB, 0.f, 1.5f)
	TWK(SpecR, 0.f, 1.5f)  TWK(SpecG, 0.f, 1.5f)  TWK(SpecB, 0.f, 1.5f)
	// TWK(Fresnel, 0.f, 1.5f)
	TWK(FresR, 0.f, 1.5f)  TWK(FresG, 0.f, 1.5f)  TWK(FresB, 0.f, 1.5f)

	TWK(Rough, 0.001f, 1.5f)  TWK(Metal, 0.f, 1.5f)
	TWK(ClearCoat, 0.f, 1.5f)  TWK(ClearRough, 0.001f, 1.5f)

	TWK(BumpScale, 0.0f, 6.f)  TWK(Transp, 0.f, 1.f)

	float a = -1.f, b = 2.f;  // par range
	int i,x;
	for (x=0; x < 4; ++x)
	{
		for (i=0; i < 3; ++i)
		{	auto s = "User" + toStr(i) + toStr(x);
			sv= &twk.svUser[i][x];  sv->Init(s, &twk.fUser[i][x], a, b, 1.5f, 3,5);  sv->DefaultF(0.0f);  SevC(TweakMtr);

			auto c = Colour(0.2f+x*0.2f, 0.5f+i*0.12f, 1.f-i*0.1f-x*0.1f);
			auto txt = fTxt(s+"Name");  txt->setTextColour(c);
			sv->setClr(c);  //txt->setCaption();  //..
		}
		for (i=0; i < 4; ++i)
		{	auto s = "Det" + toStr(i) + toStr(x);
			sv= &twk.svDet[i][x];  sv->Init(s, &twk.fDet[i][x], a, b, 1.5f, 3,5);  sv->DefaultF(0.0f);  SevC(TweakMtr);
			
			auto c = Colour(0.2f+x*0.2f, 0.5f+i*0.12f, 1.f-i*0.1f-x*0.1f);
			auto txt = fTxt(s+"Name");  txt->setTextColour(c);
			sv->setClr(c);
		}
	}

	twk.edInfo = fEd("MtrInfo");

	EdC(edMtrFind, "MtrFind", editMtrFind);
	InitClrTweakMtr();
	FillTweakMtr();
}

//  🔁 update db mat value
//--------------------------------------------------
void CGuiCom::slTweakMtr(SV* sv)
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
	else if (s=="Transp")      twk.db->setTransparency(twk.fTransp, twk.db->getTransparencyMode());
	else
	{
		if (s.substr(0,4) == "User")
		{
			int i = s.substr(4,5)[0]-'0';
			Vector4 v{twk.fUser[i][0], twk.fUser[i][1], twk.fUser[i][2], twk.fUser[i][3]};
			twk.db->setUserValue(i, v);
		}
		if (s.substr(0,3) == "Det")
		{
			int i = s.substr(3,4)[0]-'0';
			Vector4 v{twk.fDet[i][0], twk.fDet[i][1], twk.fDet[i][2], twk.fDet[i][3]};
			twk.db->setDetailMapOffsetScale(i, v);
		}
	}
}


//  🔁🌈 set Gui, slider values
//--------------------------------------------------
void CGuiCom::updTweakMtr()
{
	auto hlms = Root::getSingleton().getHlmsManager()->getHlms( HLMS_PBS );
	twk.db = (HlmsPbsDatablock*) hlms->getDatablock( pSet->tweak_mtr );
	if (!twk.db)  return;
	Vector4 u;  Vector3 v;  float f;  int i,x;

	v = twk.db->getDiffuse();   twk.fDiffR = v.x;  twk.fDiffG = v.y;  twk.fDiffB = v.z;  twk.svDiffR.Upd(); twk.svDiffG.Upd(); twk.svDiffB.Upd();
	v = twk.db->getSpecular();  twk.fSpecR = v.x;  twk.fSpecG = v.y;  twk.fSpecB = v.z;  twk.svSpecR.Upd(); twk.svSpecG.Upd(); twk.svSpecB.Upd();
	v = twk.db->getFresnel();   twk.fFresR = v.x;  twk.fFresG = v.y;  twk.fFresB = v.z;  twk.svFresR.Upd(); twk.svFresG.Upd(); twk.svFresB.Upd();

	f = twk.db->getRoughness();  twk.fRough = f;  twk.svRough.Upd();
	f = twk.db->getMetalness();  twk.fMetal = f;  twk.svMetal.Upd();
	f = twk.db->getClearCoat();  twk.fClearCoat = f;  twk.svClearCoat.Upd();
	f = twk.db->getClearCoatRoughness();  twk.fClearRough = f;  twk.svClearRough.Upd();

	f = twk.db->getNormalMapWeight();  twk.fBumpScale = f;  twk.svBumpScale.Upd();

	for (i=0; i < 3; ++i)
	{
		u = twk.db->getUserValue(i);
		twk.fUser[i][0] = u.x;  twk.fUser[i][1] = u.y;  twk.fUser[i][2] = u.z;  twk.fUser[i][3] = u.w;
		for (x=0; x < 4; ++x)  twk.svUser[i][x].Upd();
	}
	for (i=0; i < 4; ++i)
	{
		u = twk.db->getDetailMapOffsetScale(i);
		twk.fDet[i][0] = u.x;  twk.fDet[i][1] = u.y;  twk.fDet[i][2] = u.z;  twk.fDet[i][3] = u.w;
		for (x=0; x < 4; ++x)  twk.svDet[i][x].Upd();
	}

	//----------------------------------------------------------------------------------------------------
	StringStream s;  // info only ..

	s << "#20F020   Textures: #D0FFD0" << endl;
	auto* tex = twk.db->getDiffuseTexture();    if (tex)  s << "Diffuse:    " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_NORMAL);      if (tex)  s << "Normal:    " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_DETAIL_WEIGHT); if (tex)  s << "Detail Weight:  " << tex->getNameStr() << endl;
	for (i=0; i < 4; ++i)
	{
	    tex = twk.db->getTexture(PBSM_DETAIL0 + i);
		if (tex)  s << "Detail Diff:        " << tex->getNameStr() << endl;
	    tex = twk.db->getTexture(PBSM_DETAIL0_NM + i);
		if (tex)  s << "Detail Norm:     " << tex->getNameStr() << endl;
	}
    tex = twk.db->getTexture(PBSM_SPECULAR);    if (tex)  s << "Specular:  " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_ROUGHNESS);   if (tex)  s << "Roughness: " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_EMISSIVE);    if (tex)  s << "Emissive:   " << tex->getNameStr() << endl;
    tex = twk.db->getTexture(PBSM_REFLECTION);  if (tex)  s << "Reflect:    " << tex->getNameStr() << endl;
	
	s << "#40D090   Samplers: #C0FFE0" << endl;
	auto* sb = twk.db->getSamplerblock(0);
	if (sb)
	{
		const static String sFil[5] = {"none","point","lin","anis"};  // FilterOptions
		s << "Sampler:  min: " << sFil[sb->mMinFilter] << "  mag: " << sFil[sb->mMagFilter] << "  mip: " << sFil[sb->mMipFilter]
		  << "  Aniso: " << sb->mMaxAnisotropy << endl;

		const static String sWrap[5] = {"wrap","mirror","clamp","border"};  // TextureAddressingMode
		s << "Texture wrap uvw:  " << sWrap[sb->mU] << ", " << sWrap[sb->mV] << ", " << sWrap[sb->mW] << endl;
	}
	s << "#90D0F0";
	s << "Renderables:  " << twk.db->getLinkedRenderables().size() << "  Hash:  " << twk.db->getNameStr() << endl;
	const static String sWf[3] = {"Specular","SpecularAsFresnel","Metallic"};
	i = twk.db->getWorkflow();
	s << "#70B0F0PBS Workflow: " << sWf[i] << endl;
	s << endl;

	s << "#C08040   Macro: #F0C0A0" << endl;
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


	s << "#C0C040   Alpha: #F0F0A0" << endl;
	const static String sTr[4]= {"none", "transp", "fade", "refract"};  // TransparencyModes
	s << "Transparency:  " << twk.db->getTransparency()/**/ << "  mode: " << sTr[twk.db->getTransparencyMode()] << endl;

	s << "Alpha test:  " << twk.db->getAlphaTest() << "  threshold: " << twk.db->getAlphaTestThreshold()
	  << "  from tex:  " << twk.db->getUseAlphaFromTextures() << endl;
	// s << endl;

	s << "#C0C0F0   Light: #D0D0FF" << endl;
	// twk.db->getFilenameAndResourceGroup(&fname, &grp);
	s << "Brdf: " << twk.db->getBrdf()
	  << "  Refract strength: " << twk.db->getRefractionStrength() << endl;
	// s << "cubemap probe: " << (twk.db->getCubemapProbe() ? "yes" : "no") << endl;

	s << "Emissive: " << twk.db->getEmissive() << "  as lightmap: " << twk.db->getUseEmissiveAsLightmap() << endl;
	s << "Backgr Diff clr: " << twk.db->getBackgroundDiffuse() << endl;
	s << endl;


	///  user[3], detail[4]  .. remove
	s << "#C0C0C0   User: #F0F0F0" << endl;
	for (i=0; i<3; ++i)
		s << "UserValue " << i << ":    " << twk.db->getUserValue(i) << endl;

	s << "#D0D0D0   Detail: #E0E0E0" << endl;
	for (i=0; i<4; ++i)
	{
		s << "Detail offset,scale " << i << ": " << twk.db->getDetailMapOffsetScale(i) << endl;
		s << "Detail weights map " << i << ": " << twk.db->getDetailMapWeight(i)
		  << "  normal: " << twk.db->getDetailNormalWeight(i) << endl;
	}
	// twk.db->hasSeparateFresnel()
	// s << ": " << twk.db->getDetailMapBlendMode() << endl;
	
	twk.edInfo->setCaption(s.str());
}


//  🔁📃 Upd Fill materials list
//--------------------------------------------------
void CGuiCom::FillTweakMtr()
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
void CGuiCom::InitClrTweakMtr()
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
void CGuiCom::GetTweakMtr()
{
	string sData = PATHS::Data();
	String path = sData +"/materials/Pbs/";  // path

	liTweakMtr = fLi("TweakMtr");  LevC(liTweakMtr, TweakMtr);

	vsMaterials.clear();  // _Tool_  add more if needed

	// vsMaterials.push_back("#0080FF  ~~~~  Car  ~~~~");
	// GetMaterialsMat(path+"cars.material",0);  //-
	vsMaterials.push_back("#0080FFbody_realtime");
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
