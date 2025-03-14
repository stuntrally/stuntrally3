#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "settings.h"
#include "CGui.h"
#include "CApp.h"
#include "GuiCom.h"
#include "CScene.h"
#include "CData.h"
#include "Road.h"
#include "Slider.h"
#include "MultiList2.h"
#include "paths.h"

#include "Terra.h"
#include "GraphicsSystem.h"
#include "OgreHlmsTerra.h"
#include "OgreHlmsTerraDatablock.h"

#include <fstream>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;


///  ⛰️ Terrain
//-----------------------------------------------------------------------------------------------------------
bool CGui::notd()  // no td yet
{
	return sc->tds.empty();
}
TerData& CGui::td()
{
	assert(!sc->tds.empty());
	return sc->tds[scn->terCur];
}

///  Change terrain texture layer, update values
void CGui::tabTerLayer(Tab wp, size_t id)
{
	idTerLay = id;  // help var
	if (notd())  return;
	TerLayer* l = &td().layersAll[idTerLay];

	noBlendUpd = true;
	SldUpd_TerL();

	//  tex
	String s = l->texFile.substr(0, l->texFile.length()-6);
	btnTexDiff->setCaption(s);  // no _d.jpg
	cmbTexNorm->setIndexSelected( cmbTexNorm->findItemIndexWith(l->texNorm) );

	for (int i=0; i < liTex->getItemCount(); ++i)  /// upd pick
		if (liTex->getSubItemNameAt(1,i).substr(7) == s)
			liTex->setIndexSelected(i);

	//  auto norm check
	String sTex,sNorm, sExt;
	StringUtil::splitBaseFilename(l->texFile,sTex,sExt);
	StringUtil::splitBaseFilename(l->texNorm,sNorm,sExt);

	String sTexN = StringUtil::replaceAll(sTex,"_d","_n");
	bool bAuto = sTexN == sNorm;  //_T
	bTexNormAuto = bAuto;
	ckTexNormAuto.Upd();

	//  image
    imgTexDiff->setImageTexture(l->texFile);

	updUsedTer();
	SldUpd_TerLNvis();

	noBlendUpd = false;
}

void CGui::SldUpd_TerLNvis()
{
	//  upd vis of layer noise sliders
	//  check for valid +1,-1,+2 layers
	if (notd())  return;
	int ll = td().layers.size();
	int l1 = -1, last = 8, last_2 = 8,  nu = 0, ncl = 0;

	for (int i=0; i < TerData::ciNumLay; ++i)
	if (td().layersAll[i].on)
	{	++nu;
		if (i==idTerLay)  ncl = nu;
		if (nu==1)  l1 = i;
		if (nu==ll)  last = i;
		if (nu==ll-2)  last_2 = i;
	}
	bool ok = idTerLay >= l1 && idTerLay <= last;
	svTerLNoise.setVisible(ok && idTerLay < last);
	svTerLNprev.setVisible(ok && idTerLay > l1);
	svTerLNnext2.setVisible(ok && idTerLay <= last_2 && nu > 2);
	//  dbg img clr
	const static Colour lc[5] = {Colour::Black, Colour::Red, Colour::Green, Colour::Blue, Colour(0.5,0.5,0.5)};
	dbgLclr->setColour(lc[ncl]);
}

//  all ter layer sliders
void CGui::SldUpd_TerL()
{
	if (notd())  return;
	TerLayer* l = &td().layersAll[idTerLay];
	ckTerLayOn.Upd(&l->on);
	svTerLScale.UpdF(&l->tiling);
	ckTerLayTripl.Upd(&l->triplanar);
	//  blmap
	svTerLAngMin.UpdF(&l->angMin);  svTerLHMin.UpdF(&l->hMin);
	svTerLAngMax.UpdF(&l->angMax);  svTerLHMax.UpdF(&l->hMax);
	svTerLAngSm.UpdF(&l->angSm);    svTerLHSm.UpdF(&l->hSm);
	ckTerLNOnly.Upd(&l->nOnly);
	//  noise
	svTerLNoise.UpdF(&l->noise);  svTerLNprev.UpdF(&l->nprev);
	svTerLNnext2.UpdF(&l->nnext2);
	svTerLDmg.UpdF(&l->fDamage);
	//  noise params
	for (int i=0; i<2; ++i)
	{	svTerLN_Freq[i].UpdF(&l->nFreq[i]);
		svTerLN_Oct[i].UpdI(&l->nOct[i]);
		svTerLN_Pers[i].UpdF(&l->nPers[i]);
		svTerLN_Pow[i].UpdF(&l->nPow[i]);
	}
}

//  Ter properties, size
//----------------------------------------------------------------------------------------------------------

//  one ter
void CGui::SldUpd_Ter()
{
	if (notd())  return;
	auto& t = td();

	//  horiz, collis
	svTerHoriz.UpdI(&t.iHorizon);
	ckTerCollis.Upd(&t.collis);
	//  pos
	svTerPosX.UpdF(&t.posX);
	svTerPosZ.UpdF(&t.posZ);
	//  borders
	ckTerBL.Upd(&t.bL);
	ckTerBR.Upd(&t.bR);
	ckTerBF.Upd(&t.bF);
	ckTerBB.Upd(&t.bB);
	svTerVeget.UpdF(&t.fVeget);
}

void CGui::slTerHoriz(SV* sv)
{
	if (notd())  return;
	// td().iHorizon = sv->getF();
}
void CGui::slTerPosX(SV* sv)
{
	if (notd())  return;
	// scn->ter->SetPos(td().posX, td().posZ);
	// upd ?..
}
void CGui::slTerPosZ(SV* sv)
{
	if (notd())  return;
	// scn->ter->SetPos(td().posX, td().posZ);
}

void CGui::chkTerCollis(Ck* ck)
{
}
void CGui::chkTerBL(Ck* ck)
{
	// todo: upd visible border planes
}
void CGui::chkTerBR(Ck* ck)
{
}
void CGui::chkTerBF(Ck* ck)
{
}
void CGui::chkTerBB(Ck* ck)
{
}


//  🔺 Tri size
void CGui::slTerTriSize(SV* sv)
{
	if (notd())  return;
	td().fTriangleSize = sv->getF();
	LogO(fToStr(td().fTriangleSize));
	td().UpdVals();
	UpdTxtTerSize();
}

int CGui::UpdTxtTerSize(float mul)
{
	if (notd())  return 0;
	int size = getHMapSizeTab() * mul;
	float res = td().fTriangleSize * size;  // result size
	svTerTriSize.setText(fToStr(res*0.001f,2,4));
	
	valHmapMB->setCaption(toStr(size*size*4/1000000)+" MB");  // floats
	return size;
}

//  HMap size tab
void CGui::updTabHmap()
{
	if (notd())  return;
	static std::map<int,int> h;
	if (h.empty())
	{	h[128]=0; h[256]=1; h[512]=2; h[1024]=3; h[2048]=4; h[4096]=5; h[8192]=6;  }
	tabsHmap->setIndexSelected( h[ td().iVertsX ] );
	tabHmap(0,0);
}
int CGui::getHMapSizeTab()
{
	static std::map<int,int> h;
	if (h.empty())
	{	h[0]=128; h[1]=256; h[2]=512; h[3]=1024; h[4]=2048; h[5]=4096; h[6]=8192;  }
	return h[ tabsHmap->getIndexSelected() ];
}
void CGui::tabHmap(Tab, size_t)
{
	UpdTxtTerSize();
}


//  upd ter sh par
void CGui::slTerPar(SV*sv)
{
	// ter_scaleNormal      1.f / td().normScale
	// ter_specular_pow     td().specularPow
	// ter_specular_pow_em  td().specularPowEm
}

//  save new hmap
void CGui::saveNewHmap(float* hfData, int size, int add, bool bNew)
{
	auto file = scn->getHmap( //add ? scn->sc->tds.size() :  // if add, last
		scn->terCur, bNew);
	LogO("TER+ New Hmap  " + toStr(size) +"  "+ file);
	std::ofstream of;
	of.open(file.c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], size);
	of.close();
	scn->iNewHmap = add ? -1 : scn->terCur;
}


//  🛠️ Ter Hmap tools  - - - -
//----------------------------------------------------------------------------------------------------------
void CGui::btnAlignTerToRoad(WP)
{
	app->AlignTerToRoad();
}
void CGui::btnAlignHorizonToTer(WP)
{
	app->AlignHorizonToTer();
}

void CGui::btnTerrainNew(WP)
{
	if (notd())  return;
	int si = UpdTxtTerSize();

	auto& s = td().iVertsX;
	s = si;  td().UpdVals();  // new hf

	float* hfData = new float[s * s];
	int size = s * s * sizeof(float);
	
	//  generate Hmap
	for (int j=0; j < s; ++j)
	{
		int a = j * s;
		for (int i=0; i < s; ++i,++a)
			hfData[a] = 0.f;  //td().getHeight(i,j);
	}
	// ++scn->terCur;
	saveNewHmap(hfData, size);

	delete[] hfData;
	app->UpdateTrack();
}


//  Terrain  half  --------------------------------
void CGui::btnTerrainHalf(WP)
{
	if (notd())  return;
	int so = td().iVertsX;
	if (so <= 128)  return;
	int s = so / 2;
	float* hfData = new float[s * s];
	int size = s * s * sizeof(float);
	
	//  resize Hmap by half
	for (int j=0; j < s; ++j)
	{
		int a = j * s,
			a2 = j*2 * so;
		for (int i=0; i < s; ++i,++a)
		{	hfData[a] = td().hfHeight[a2];  a2+=2;  }
	}
	saveNewHmap(hfData, size);
	delete[] hfData;

	td().fTriangleSize *= 2.f;
	td().iVertsX = s;  td().UpdVals();
	
	updTabHmap();  svTerTriSize.Upd();
	app->UpdateTrack();
}

//  Terrain  double  --------------------------------
void CGui::btnTerrainDouble(WP)
{
	if (notd())  return;
	int so = td().iVertsX;
	if (so >= 4096)  return;

	int s = so * 2, ofs4 = s / 4;

	const int size = s * s;
	float* hfData = new float[size];
	for (int i=0; i < size; ++i)
		hfData[i] = 0.f;
	
	//  resize Hmap by half
	for (int j=0; j < so; ++j)
	{
		int a = (j + ofs4) * s + ofs4,
			a2 = j * so;
		for (int i=0; i < so; ++i,++a)
		{	hfData[a] = td().hfHeight[a2];  ++a2;  }
	}
	saveNewHmap(hfData, size * sizeof(float));
	delete[] hfData;

	td().iVertsX = s;  td().UpdVals();
	updTabHmap();
	app->UpdateTrack();
}

#if 0
//  Terrain  resize ..  --------------------------------
void CGui::btnTerrainResize(WP)
{
	if (notd())  return;
	int size = getHMapSizeTab() / 2;
	if (valTerTriSize)
		valTerTriSize->setCaption(fToStr(td().fTriangleSize * size,2,4));

	int oldSize = td().iVertsX, osi = oldSize*oldSize,
		newSize = oldSize * 2;
	float scale = 1.f / 2.f / 2.f;
	float* hfData = new float[si];
	for (int i=0; i < si; ++i)  hfData[i] = 0.f;  // clear out
	
	//  resize
	int i,j,x,y,a;
	for (j=0; j < newSize; ++j)
	{
		y = scale * j;
		a = j * newSize;
		for (i=0; i < newSize; ++i,++a)
		{
			x = y * oldSize + scale * i;
			x = std::max(0, std::min(osi-1, x));
			hfData[a] = td().hfHeight[ x ];
		}
	}
	std::ofstream of;
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], si * sizeof(float));
	of.close();
	delete[] hfData;
	
	td().fTriangleSize * scale * 2.f;
	td().iVertsX = newSize;  td().UpdVals();
	updTabHmap();  svTerTriSize.Upd();
	app->bNewHmap = true;	app->UpdateTrack();  //SetGuiFromXmls();
}
#endif


//  Terrain  move  --------------------------------
void CGui::btnTerrainMove(WP)
{
	if (notd())  return;
	Ed ex = (Ed)app->mWndEdit->findWidget("TerMoveX");
	Ed ey = (Ed)app->mWndEdit->findWidget("TerMoveY");
	int mx = ex ? s2i(ex->getCaption()) : 0;
	int my = ey ?-s2i(ey->getCaption()) : 0;
	
	int s = td().iVertsX, size = s * s;
	float* hfData = new float[size];
	
	//  resize
	int i,j,a,aa;
	for (j=0; j < s; ++j)
	{
		a = j * s;
		for (i=0; i < s; ++i,++a)
		{
			aa = std::max(0, std::min(size-1, (j-mx) * s + i+my));
			hfData[a] = td().hfHeight[aa];
		}
	}
	saveNewHmap(hfData, size);
	delete[] hfData;
	
	app->scn->road->SelAll();
	app->scn->road->Move(Vector3(my,0,mx) * -td().fTriangleSize);
	app->scn->road->SelClear();
	//start, objects-

	app->UpdateTrack();
}

//  Terrain  height scale  --------------------------------
void CGui::btnScaleTerH(WP)
{
	return;  // todo: fixme crash..
	if (notd())  return;
	if (!app->scn->road)  return;
	Real sf = std::max(0.1f, fScaleTer);  // scale mul
	int i;

	//  road
	for (i=0; i < app->scn->road->getNumPoints(); ++i)
		app->scn->road->Scale1(i, 0.f, sf);
	app->scn->road->bSelChng = true;
	
	//  fluids
	for (i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.y *= sf;  fb.size.y *= sf;
	}
	
	//  objs h
	for (i=0; i < sc->objects.size(); ++i)
	{
		Object& o = sc->objects[i];
		o.pos[2] *= sf;
		o.SetFromBlt();
	}

	//  1st ter only  ---
	int si = td().iVertsX;
	float* hfData = new float[si];
	int size = si * si * sizeof(float);
	
	//  generate Hmap
	for (int j=0; j < si; ++j)
	{
		int a = j * si;
		for (i=0; i < si; ++i,++a)
			hfData[a] = td().hfHeight[a] * sf;
	}
	saveNewHmap(hfData, size);
	delete[] hfData;

	app->UpdateTrack();

	//  start,end pos
	for (i=0; i < 2; ++i)
		app->scn->sc->startPos[i][2] *= sf;
	app->UpdStartPos();
}
//----------------------------------------------------------------------------------------------------------


//  generator  . . . . . . .
void CGui::slTerGen(SV*)
{
	app->bUpdTerPrv = true;
}

//  blendmap_test
void CGui::chkDebugBlend(Ck*)
{
	app->scn->ter->tDb->setBlendmapTest(bDebugBlend);
	dbgLclr->setVisible(bDebugBlend);
}


///  🏔️ Terrain layers  -----------------------------
//
void CGui::chkTerLayOn(Ck* ck)
{
	td().UpdLayers();
	if (td().layers.size()==0)  // would crash, surf
	{
		*ck->pBool = true;  // turn it back on
		ck->Upd();
	}

	td().UpdLayers();
	updUsedTer();

	app->UpdateTrack();

	SldUpd_TerLNvis();
	UpdSurfList();
}

void CGui::updUsedTer()
{
	if (notd())  return;
	SetUsedStr(valTerLAll, td().layers.size(), 3);
	int t = td().triplCnt;
	valTriplAll->setCaption(TR("#{Used}") + ": " + toStr(t));
	valTriplAll->setTextColour(sUsedClr[ t > 2 ? 6 : (t + 2)]);
}

void CGui::chkTerLayTripl(Ck*)
{
	td().UpdLayers();
	updUsedTer();
}

void CGui::comboTexNorm(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	td().layersAll[idTerLay].texNorm = s;
}

//  Terrain BlendMap
void CGui::slTerLay(SV*)
{
	app->bTerUpdBlend = true;
}
void CGui::chkTerLNOnly(Ck*)
{
	slTerLay(0);
}

//  move layer order
void CGui::btnTerLmoveL(WP)  // -1
{
	if (idTerLay <= 0)  return;

	TerLayer& t = td().layersAll[idTerLay], st,
			&t1 = td().layersAll[idTerLay-1];
	st = t;  t = t1;  t1 = st;
	
	td().UpdLayers();  NumTabNext(-1);
	app->scn->UpdBlendmap();
}

void CGui::btnTerLmoveR(WP)  // +1
{
	if (idTerLay >= TerData::ciNumLay-1)  return;

	TerLayer& t = td().layersAll[idTerLay], st,
			&t1 = td().layersAll[idTerLay+1];
	st = t;  t = t1;  t1 = st;
	
	td().UpdLayers();  NumTabNext(1);
	app->scn->UpdBlendmap();
}


///  🌀 Noise preset buttons
void CGui::radN1(WP) {  Radio2(bRn1, bRn2, bRn2->getStateSelected());  }
void CGui::radN2(WP) {  Radio2(bRn1, bRn2, bRn2->getStateSelected());  }

const static float ns[15][4] = {  //  freq, oct, pers, pow
{ 30.4f, 3, 0.33f, 1.5f },{ 36.6f, 4, 0.49f, 1.9f },{ 30.7f, 3, 0.30f, 1.5f },{ 29.5f, 2, 0.13f, 1.8f },{ 40.5f, 3, 0.43f, 2.0f },
{ 25.3f, 3, 0.30f, 1.2f },{ 31.3f, 5, 0.70f, 2.0f },{ 28.4f, 4, 0.70f, 1.5f },{ 34.5f, 4, 0.40f, 0.9f },{ 34.3f, 4, 0.54f, 1.0f },
{ 44.6f, 2, 0.30f, 1.1f },{ 48.2f, 3, 0.12f, 1.6f },{ 56.6f, 4, 0.49f, 2.0f },{ 60.4f, 4, 0.51f, 2.0f },{ 62.6f, 3, 0.12f, 2.1f }};

void CGui::btnNpreset(WP wp)
{
	if (notd())  return;
	int l = bRn2->getStateSelected() ? 1 : 0;
	String s = wp->getName();  //"TerLN_"
	int i = s2i(s.substr(6));

	TerLayer& t = td().layersAll[idTerLay];
	t.nFreq[l] = ns[i][0];
	t.nOct[l]  = int(ns[i][1]);
	t.nPers[l] = ns[i][2];
	t.nPow[l]  = ns[i][3];
	SldUpd_TerL();
	app->scn->UpdBlendmap();
}
void CGui::btnNrandom(WP wp)
{
	if (notd())  return;
	int l = bRn2->getStateSelected() ? 1 : 0;

	TerLayer& t = td().layersAll[idTerLay];
	t.nFreq[l] = Math::RangeRandom(20.f,70.f);
	t.nOct[l]  = Math::RangeRandom(2.f,5.f);
	t.nPers[l] = Math::RangeRandom(0.1f,0.7f);
	t.nPow[l]  = Math::RangeRandom(0.8f,2.4f);
	SldUpd_TerL();
	app->scn->UpdBlendmap();
}

//  swap noise 1 and 2 params
void CGui::btnNswap(WP wp)
{
	if (notd())  return;
	TerLayer& t = td().layersAll[idTerLay];
	std::swap(t.nFreq[0], t.nFreq[1]);
	std::swap(t.nOct[0] , t.nOct[1] );
	std::swap(t.nPers[0], t.nPers[1]);
	std::swap(t.nPow[0] , t.nPow[1] );
	SldUpd_TerL();
	app->scn->UpdBlendmap();
}


///  ⚫💭 Terrain Particles  -----------------------------
//
void CGui::comboParDust(Cmb cmb, size_t val)  // par type
{
	String s = cmb->getItemNameAt(val);
	String n = cmb->getName();
		 
		 if (n=="CmbParDust")   sc->sParDust = s;
	else if (n=="CmbParMud")    sc->sParMud = s;
	else if (n=="CmbParSmoke")  sc->sParSmoke = s;
}


///  Terrain Surface  -----------------------------
//
void CGui::comboSurface(Cmb cmb, size_t val)
{
	TerLayer* l = GetTerRdLay();
	std::string s = cmb->getItemNameAt(val);
	// LogO("SURF cmb  = "+s);
	l->surfName = s;
	UpdSurfInfo();
}

void CGui::UpdSurfInfo()
{
	TerLayer* l = GetTerRdLay();
	int id = app->surf_map[l->surfName]-1;
	if (id == -1)  return;  //not found..
	const TRACKSURFACE& su = app->surfaces[id];

	txtSurfTire->setCaption(su.tireName);
	txtSuBumpWave->setCaption(fToStr(su.bumpWaveLength, 1,3));
	txtSuBumpAmp->setCaption(fToStr(su.bumpAmplitude, 2,4));
	txtSuRollDrag->setCaption(fToStr(su.rollingDrag, 1,3));
	txtSuFrict->setCaption(fToStr(su.friction, 2,4));
	txtSurfType->setCaption(csTRKsurf[su.type]);
}


///  Surfaces  all in data/cars/surfaces.cfg
//------------------------------------------------------------------------------------------------------------------------------
bool App::LoadAllSurfaces()
{
	surfaces.clear();
	surf_map.clear();

	std::string path = PATHS::CarSim() + "/normal/surfaces.cfg";
	CONFIGFILE param;
	if (!param.Load(path))
	{
		LogO("Can't find surfaces configfile: " + path);
		return false;
	}
	
	std::list <std::string> sectionlist;
	param.GetSectionList(sectionlist);
	
	for (const auto& section : sectionlist)
	{
		TRACKSURFACE surf;
		surf.name = section;
		
		int id;
		param.GetParam(section + ".ID", id);  // for sound..
		//-assert(indexnum >= 0 && indexnum < (int)tracksurfaces.size());
		surf.setType(id);
		
		float temp = 0.0;
		param.GetParam(section + ".BumpWaveLength", temp);		surf.bumpWaveLength = temp;
		param.GetParam(section + ".BumpAmplitude", temp);		surf.bumpAmplitude = temp;

		//frictionX, frictionY, bumpWaveLength2, bumpAmplitude2, not shown ..
		param.GetParam(section + ".FrictionTread", temp);		surf.friction = temp;
		
		if (param.GetParam(section + ".RollResistance", temp))	surf.rollingResist = temp;
		param.GetParam(section + ".RollingDrag", temp);		surf.rollingDrag = temp;

		///---  Tire  ---
		std::string tireFile;
		if (!param.GetParam(section + "." + "Tire", tireFile))
			tireFile = "Default";  // default surface if not found
		surf.tireName = tireFile;
		///---
		
		surfaces.push_back(surf);
		surf_map[surf.name] = (int)surfaces.size();  //+1, 0 = not found
	}
	return true;
}
