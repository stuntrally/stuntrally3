#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CData.h"
#include "TracksXml.h"
#include "PresetsXml.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "pathmanager.h"
#include "MultiList2.h"
#include "Slider.h"

#include <filesystem>
#include <MyGUI.h>
#include <MyGUI_InputManager.h>
using namespace MyGUI;
// using namespace Ogre;
using Ogre::String;  using Ogre::StringUtil;
using namespace std;


///  📃 Gui Init Lists
//----------------------------------------------------------------------------------------------------------------------
void CGui::InitGuiLists() 
{
	///  Fill Combo boxes  . . . . . . .
	string sData = PATHMANAGER::Data();
	String sMat = sData +"/materials/Pbs/";  // path
	int u,n;
	Btn btn;  Ck* ck;

	
	//---------------------  🔊 Game, Reverbs  ---------------------
	txtRevebDescr = fTxt("txtRevebDescr");
	Cmb(cmbReverbs, "CmbReverbs", comboReverbs);

	for (u=0; u < data->reverbs->revs.size(); ++u)
		cmbReverbs->addItem(data->reverbs->revs[u].name);


	//---------------------  🌧️ Weather  ---------------------
	Cmb(cmbRain1, "Rain1Cmb", comboRain1);  cmbRain1->addItem("");
	Cmb(cmbRain2, "Rain2Cmb", comboRain2);  cmbRain2->addItem("");

	GetMaterials("weather.particle", true, "particle_system");
	for (u=0; u < vsMaterials.size(); ++u)
	{	const String& s = vsMaterials[u];
		cmbRain1->addItem(s);  cmbRain2->addItem(s);
	}	

	//---------------------  🔥 Emitters  ---------------------
	GetMaterials("emitters.particle", true, "particle_system");
	app->vEmtNames.clear();
	for (u=0; u < vsMaterials.size(); ++u)
		app->vEmtNames.push_back(vsMaterials[u]);


	//---------------------  ⛰️ Terrain  ---------------------
	Cmb(cmbTexNorm, "TexNormal", comboTexNorm);  cmbTexNorm->addItem("flat_n.png");

	strlist li;
	PATHMANAGER::DirList(sData + "/terrain", li);
	for (auto q : li)
	{
		if (StringUtil::match(q, "*_n.*", false))
			cmbTexNorm->addItem(q);
		//else
		//if (StringUtil::match(*q, "*_d.*", false))  //_T
		//	cmbTexDiff->addItem(*q);
	}
	
	//  particles
	GetMaterials("tires.particle", true, "particle_system");
	for (u=0; u < vsMaterials.size(); ++u)
	{	const String& s = vsMaterials[u];
		cmbParDust->addItem(s);  cmbParMud->addItem(s);  cmbParSmoke->addItem(s);
	}
	
	//  surfaces
	for (u=0; u < app->surfaces.size(); ++u)
		cmbSurface->addItem(app->surfaces[u].name);
	

	//---------------------  🌿 Grass  ---------------------
	PATHMANAGER::DirList(sData + "/grass", li);
	for (auto q : li)
	{
		if (StringUtil::startsWith(q, "grClr", false))
			cmbGrassClr->addItem(q);
	}


	//---------------------  📦 Objects  ---------------------
	app->vObjNames.clear();  strlist lo;
	PATHMANAGER::DirList(sData + "/objects2", lo);
	PATHMANAGER::DirList(sData + "/objects", lo);
	for (auto q : lo)
		if (StringUtil::endsWith(q, ".mesh"))
		{	string name = q.substr(0, q.length()-5);  //no .ext
			if (name != "sphere")
				app->vObjNames.push_back(name);
		}
	
	objListDyn = fLi("ObjListDyn");  Lev(objListDyn, ObjsChng);
	objListSt  = fLi("ObjListSt");   Lev(objListSt,  ObjsChng);
	objListBld = fLi("ObjListBld");  Lev(objListBld, ObjsChng);
	objListCat = fLi("ObjListCat");  Lev(objListCat, ObjsCatChng);
	objPan = fWP("objPan");


	for (u=0; u < app->vObjNames.size(); ++u)
	{	const string& name = app->vObjNames[u];
		if (name != "sphere" && PATHMANAGER::FileExists(sData+"/objects/"+ name + ".bullet"))
			objListDyn->addItem("#A0E0FF"+name);  // dynamic
	}
	
	//  🏢 buildings  group categories  more with same prefix  ----
	app->vBuildings.clear();
	objListCat->removeAllItems();

	auto AddPath = [&](auto path)
	{
		std::map<string, int> cats;  // yeah cats are fun
		int b0 = app->vBuildings.size();
		lo.clear();
		PATHMANAGER::DirList(sData + path, lo);

		for (auto q : lo)
			if (StringUtil::endsWith(q,".mesh"))
			{	string name = q.substr(0, q.length()-5);  //no .ext
				if (name != "sphere" && !PATHMANAGER::FileExists(sData+"/objects/"+ name + ".bullet"))
				{	// no dynamic

					// auto id = name.find('_') ?..
					string cat = name.substr(0,4);
					++cats[cat];
					app->vBuildings.push_back(name);
					app->vObjNames.push_back(name);  // in -,=
			}	}
		//  get cats  ----
		for (auto it : cats)
		{
			string cat = it.first;  int n = it.second;
			//LogO(cat+" "+toStr(n));
			if (n > 1)  // add category (> 1 Bld with this prefix)
				objListCat->addItem("#F0B0B0"+cat);
		}
		//  push Bld back to Obj list (if <= 1, rare cat)
		for (u = b0; u < app->vBuildings.size(); ++u)
		{
			const string& name = app->vBuildings[u];
			string cat = name.substr(0,4);
			if (cats[cat] <= 1)
				objListSt->addItem("#D0D0C8"+name);
		}
		{	int il = objListCat->findItemIndexWith("#E09090"+pSet->objGroup);
			objListCat->setIndexSelected(il);
			listObjsCatChng(objListCat, il);
		}
	};
	AddPath("/objects");
	AddPath("/objects2");
	objListCat->addItem("#FFB060---- "+TR("#{ObjRocks}"));
	AddPath("/rocks");
	objListCat->addItem("#C0C8C0---- City");
	AddPath("/objectsC");
	objListCat->addItem("#E0E0A0---- 0 A.D.");
	AddPath("/objects0");


	//---------------------  ⚫💭 Surfaces  ---------------------
	surfList = fLi("SurfList");  Lev(surfList, Surf);
	for (n=0; n < 4; ++n)  surfList->addItem("#80FF00"+TR("#{Layer} ")+toStr(n));
	for (n=0; n < 4; ++n)  surfList->addItem("#FFB020"+TR("#{Road} ")+toStr(n));
	for (n=0; n < 4; ++n)  surfList->addItem("#FFFF80"+TR("#{Pipe} ")+toStr(n));
	surfList->setIndexSelected(0);
	
	
	//---------------------  🔧 Tweak  ---------------------
	ComboBoxPtr cmbTwk;
	Cmb(cmbTwk, "TweakMtr", comboTweakMtr);

	GetMaterialsMat(sMat+"water.material");
	GetMaterialsMat(sMat+"pipe.material",false);
	GetMaterialsMat(sMat+"road.material",false);
	GetMaterialsMat(sMat+"objects_static.material",false);

	cmbTwk->addItem("");
	for (u=0; u < vsMaterials.size(); ++u)
		cmbTwk->addItem(vsMaterials[u]);

	cmbTwk->setIndexSelected( cmbTwk->findItemIndexWith(pSet->tweak_mtr) );

	
	///  👆 Pick window
	///------------------------------------------------------------------------------------------------------------
	//  Pick btns
	Btn("PickSky", btnPickSky);      btn->eventMouseWheel += newDelegate(this, &CGui::wheelSky);  btnSky = btn;
	Btn("PickTex", btnPickTex);      btn->eventMouseWheel += newDelegate(this, &CGui::wheelTex);  btnTexDiff = btn;
	Btn("PickGrass", btnPickGrass);  btn->eventMouseWheel += newDelegate(this, &CGui::wheelGrs);  btnGrassMtr = btn;
	Btn("PickVeget", btnPickVeget);  btn->eventMouseWheel += newDelegate(this, &CGui::wheelVeg);  btnVeget = btn;
	
	auto btnWhRd = [&](){  btn->eventMouseWheel += newDelegate(this, &CGui::wheelRd);  };
	for (n=0; n < 4; ++n)
	{	Btn(toStr(n)+"RdMtr",  btnPickRoad);  btnRoad[n] = btn;  btnWhRd();
		Btn(toStr(n)+"RdMtrP", btnPickPipe);  btnPipe[n] = btn;  btnWhRd();
	}
	Btn("0RdMtrW",  btnPickRoadW);    btnRoadW = btn;    btnWhRd();
	Btn("0RdMtrPW", btnPickPipeW);    btnPipeW = btn;    btnWhRd();
	Btn("0RdMtrC",  btnPickRoadCol);  btnRoadCol = btn;  btnWhRd();

	ck= &ckPickSetPar;	ck->Init("PickSetPar",	&pSet->pick_setpar);
	panPick = fWP("PanelPick");

	FillPickLists();

    //TrackListUpd(true);  //upd
	//listTrackChng(trkList,0);
}


///  👆📃 Fill Pick Lists
///------------------------------------------------------------------------------------------------------------
void CGui::FillPickLists()
{

	///  ⛅ Sky Mtr  --------
	Mli2 lp;  int l,u;
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liSky = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickSky);
	lp->setColour(Colour(0.7,0.85,1.0));  lp->setInheritsAlpha(false);
	const int rt = 22, sl = 21;
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0", 15);
	lp->addColumn("#E0F0FF"+TR("#{Sky}"), 200);
	lp->addColumn("#E0F0FF"+TR("#{Pitch}"), 60);
	lp->addColumn(" ", sl);
	liPickX[P_Sky] = 0.45f;  liPickW[P_Sky] = 280;

	for (u=0; u < data->pre->sky.size(); ++u)
	{	const PSky& s = data->pre->sky[u];
		String c = s.clr;
		lp->addItem(c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ s.mtr);
		lp->setSubItemNameAt(2,l, c+ fToStr( s.ldPitch, 0,2));
	}


	///  🏔️ Terrain layers  Tex Diff  --------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liTex = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickTex);
	lp->setColour(Colour(0.8,0.9,0.7));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#E0FFE0"+TR("#{Diffuse}"), 190);
	lp->addColumn("#80E0E0"+TR("#{Scale}"), 40);
	lp->addColumn("#80FF80|", 27);
	lp->addColumn(" ", sl);
	liPickX[P_Tex] = 0.45f;  liPickW[P_Tex] = 330;

	for (u=0; u < data->pre->ter.size(); ++u)
	{	const PTer& t = data->pre->ter[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.texFile.substr(0, t.texFile.length()-2));  // no _d
		if (t.rate)
		{	lp->setSubItemNameAt(2,l, gcom->getClrLong(t.tiling * 0.15f) + fToStr(t.tiling, 0,2));
			lp->setSubItemNameAt(3,l, (t.triplanar ? "#E0E0E01" : "#6565650"));
	}	}


	///  🌿 Grass  --------------------------------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liGrs = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickGrs);
	lp->setColour(Colour(0.7,0.9,0.7));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#E0FFE0"+TR("#{GrMaterial}"), 152);
	lp->addColumn("#D0FFD0-", 41);  lp->addColumn("#C0F0C0|", 41);
	lp->addColumn(" ", sl);
	liPickX[P_Grs] = 0.36f;  liPickW[P_Grs] = 310;

	for (u=0; u < data->pre->gr.size(); ++u)
	{	const PGrass& t = data->pre->gr[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.mtr);
		if (t.rate)
		{	lp->setSubItemNameAt(2,l, c+ fToStr(t.maxSx, 1,3));
			lp->setSubItemNameAt(3,l, c+ fToStr(t.maxSy, 1,3));
	}	}


	///  🌳🪨 Vegetation  --------------------------------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liVeg = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickVeg);
	lp->setColour(Colour(0.7,0.9,0.9));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#E0FFE0"+TR("#{Model}"), 207);
	lp->addColumn("#80E0E0"+TR("#{Scale}"), 40);
	lp->addColumn(" ", sl);
	liPickX[P_Veg] = 0.36f;  liPickW[P_Veg] = 320;

	for (u=0; u < data->pre->veg.size(); ++u)
	{	const PVeget& t = data->pre->veg[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.name);
		if (t.rate)
			lp->setSubItemNameAt(2,l, c+ fToStr(t.maxScale, 1,3));
			//lp->setSubItemNameAt(3,l, c+ fToStr(t.maxTerAng, 0,2));
	}


	///  🛣️ Road  --------------------------------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liRd = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickRd);
	lp->setColour(Colour(0.9,0.8,0.7));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#FFE0D0"+TR("#{GrMaterial}"), 257);
	//lp->addColumn("#80E0E0"+TR("#{Surface}"), 80);
	lp->addColumn(" ", sl);
	liPickX[P_Rd] = 0.36f;  liPickW[P_Rd] = 320;

	for (u=0; u < data->pre->rd.size(); ++u)
	{	const PRoad& t = data->pre->rd[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.mtr);
		/*String su = t.surfName;  if (su.substr(0,4)=="road")  su = su.substr(4, su.length());
		if (t.rate && t.mtr.substr(0,5) != "River")
			lp->setSubItemNameAt(2,l, c+ su);*/
	}
	
	// todo: auto groups chks gui? to filter pick lists
}
