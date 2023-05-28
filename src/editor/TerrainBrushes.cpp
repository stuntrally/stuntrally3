#include "pch.h"
#include "Def_Str.h"
#include "paths.h"
#include "brushes.h"
#include "CGui.h"
#include "GuiCom.h"
#include "App.h"
#include "settings.h"

#include <OgreTimer.h>
#include <OgreResourceGroupManager.h>
#include <OgreStagingTexture.h>
#include <MyGUI_ComboBox.h>
using namespace Ogre;
using namespace std;
using namespace MyGUI;


const Ogre::String BrushesIni::csBrShape[BRS_ALL] =
{ "Triangle", "Sinus", "N-gon", "Noise", "Noise2" };  // static


//  🖌️🖼️ generate all Brushes prv png  ::
//---------------------------------------------------------------------
void App::GenBrushesPrv()
{
	Ogre::Timer ti;
	const uint siTex = 2048, siIco = BrIcoSize;  // 16 * 128
	uint32* data = new uint32[siIco * siIco];

	Image2 im;
	im.createEmptyImage(siTex, siTex, 1, TextureTypes::Type2D, PFG_RGBA8_UNORM);
	uint32* big = (uint32*)im.getRawBuffer();
	memset(big, 0, siTex * siTex);

	uint u = 0, v = 0, all = brSets.v.size();
	for (int i=0; i < all; ++i)
	{
		// SetBrushPreset(i, 0);
		updBrushData((uint8*)data, i, 1);
		
		//  store prv in big
		uint a = 0;
		for (uint y=0; y < siIco; ++y)
		{
			uint b = u + (v + y) * siTex;
			for (uint x=0; x < siIco; ++x)
				big[b++] = data[a++];
		}
		u += siIco;  // next u,v prv pos
		if (u >= siTex)
		{	u = 0;  v += siIco;
			if (v >= siTex)
			{	LogO("!No more room for brushes on size: "+toStr(siTex));
				break;  // max 256
		}	}
	}

	im.save(PATHS::UserConfigDir()+"/brushes.png", 0, 0);
	delete[] data;

	prvBrushes.Load(PATHS::UserConfigDir()+"/brushes.png",1);

	LogO(String("::: Time ALL brush: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  // < 1s
}



///  🖌️🎨 upd Gui  brush presets   o o o o o o
//--------------------------------------------------------------------------------------------------------------------------
void CGui::UpdBrushesImgs()
{
	auto* scv = scvBrushes;

	//  destroy old
	for (auto img : brImgs)
		mGui->destroyWidget(img);
	brImgs.clear();
	
	for (auto txt : brTxts)
		mGui->destroyWidget(txt);
	brTxts.clear();

	//  create  --------
	const auto& brs = app->brSets;
	const int uv = App::BrIcoSize, all = brs.v.size();
	const int sset = app->brSets.imgSize;

	int yy=0, xx=0, ss=0;
	for (int i=0; i < all; ++i)
	{
		const auto& br = brs.v[i];
		int xi,yi, xt,yt, si;

		si = sset + br.rate * 10;  // img size
		if (si > ss)  ss = si;

		if (br.newLine==1 && xx > 0 || xx >= brs.perRow * 45)  //-
		{	xx = 0;  yy += ss + 10;  ss = 0;  }  // par 1 new line
	
		xi = xx;  yi = yy;
		xt = xi + 15;  yt = yi + si + 0;
		xx += si + 5;
		
		if (br.newLine < 0)
			xx -= br.newLine * 60;  // -1 empty x

		const auto st = toStr(i);
		Img img = scv->createWidget<ImageBox>("ImageBox", xi,yi, si,si, Align::Default, "brI"+st);
		img->eventMouseButtonClick += newDelegate(this, &CGui::btnBrushPreset);
		img->setUserString("tip", br.name);  img->setNeedToolTip(true);
		img->setImageTexture("PrvBrushes");
		img->setImageCoord(IntCoord( i%16*uv, i/16*uv, uv,uv ));
		if (!br.name.empty())
			img->eventToolTip += newDelegate(gcom, &CGuiCom::notifyToolTip);
		gcom->setOrigPos(img, "EditorWnd");
		brImgs.push_back(img);
		
		auto sz = br.size, in = br.intens;
		xt -= min(15.f, sz /13.f);
		auto str = 
			gcom->getClrVal( sz / 150.f * 17.f) + fToStr(sz,0,2) + " "+ 
			gcom->getClrVal( in / 60.f * 17.f) + fToStr(in,0,2);
		Txt txt = scv->createWidget<TextBox>("TextBox", xt,yt, 60,22, Align::Default, "brT"+st);
		txt->setCaption(fToStr(br.size,0,2));
		txt->setCaption(str);
		auto ts = min(43.f, br.size / 16.f + sset / 4/*12*/);
		txt->setFontHeight(ts);
		gcom->setOrigPos(txt, "EditorWnd");
		brTxts.push_back(txt);
	}
	scv->setCanvasSize(1200, yy*2 + 100);  //-?

	if (bGI)  // resize
		gcom->doSizeGUI(scv->getEnumerator());

	setBrCur();
}


//  🖌️🎨 Brush img click
//---------------------------------------------------------------------
void CGui::btnBrushPreset(WP img)
{
	int i = 0, c = iBrGui;
	sscanf(img->getName().c_str(), "brI%d", &i);
	auto& v = app->brSets.v;
	int si = v.size();
	if (c >= si)  c = 0;  //return;  }
	if (i < 0 || i >= si)  return;
	
	if (app->alt && c >= 0)
	{
		// auto cnl = v[c].new_line, inl = v[i].new_line;
		// auto cgr = v[c].group, igr = v[i].group;

		//  swap
		auto cc = v[c];
		v[c] = v[i];  v[i] = cc;

		// v[c].new_line = cnl;  v[i].new_line = inl;  // fixme
		// v[c].group = cgr;  v[i].group = igr;
		app->GenBrushesPrv();
		UpdBrushesImgs();  return;
	}
	if (app->shift && c >= 0)
	{
		// auto cnl = v[c].new_line, inl = v[i].new_line;
		// auto cgr = v[c].group, igr = v[i].group;

		//  move cur
		auto cc = v[c];
		if (i >= 0 && i < v.size())
			v.erase(v.begin() + c);
		v.insert(v.begin() + i, cc);

		// v[c].new_line = cnl;  v[i].new_line = inl;  // fixme
		// v[c].group = cgr;  v[i].group = igr;
		app->GenBrushesPrv();
		UpdBrushesImgs();  return;
	}
	//  click, set
	iBrGui = i;
	setBrCur();
	app->SetBrushPreset(i);
}


//  gui events
//--------------------------------------------------------------------------------------------------------------------------

//  img cur [] set pos
void CGui::setBrCur()
{
	WP img = brImgs.empty() ? (WP)scvBrushes :
		iBrGui < brImgs.size() ? brImgs[iBrGui] : brImgs[0];

	auto p = img->getPosition();
	auto s = img->getSize();
	imgBrCur->setCoord(p.left, p.top, s.width, s.height);
}


void CGui::chkBrushAdj(Ck*)
{
	panBrushAdj->setVisible(pSet->br_adjust);
}
void CGui::slBrNewLine(SV*)
{
	UpdBrushesImgs();
}

void CGui::editBrName(Ed ed)
{
	auto i = iBrGui;
	auto& v = app->brSets.v;
	if (i >= 0 && i < v.size())
		v[i].name = ed->getCaption();
}


//  brush add +
void CGui::btnBrushAdd(WP)
{
	BrushSet b = app->curBr();
	auto i = iBrGui;
	auto& v = app->brSets.v;
	if (i >= 0 && i < v.size()-1)
		v.insert(v.begin() + i + 1, b);
	else
		v.push_back(b);

	app->GenBrushesPrv();
	UpdBrushesImgs();
}

//  brush del -
void CGui::btnBrushDel(WP)
{
	auto& i = iBrGui;
	auto& v = app->brSets.v;
	if (i >= 0 && i < v.size())
		v.erase(v.begin() + i);
	
	if (i == v.size() && i > 0)
		--i;

	app->GenBrushesPrv();
	UpdBrushesImgs();
}


void CGui::btnBrushesSave(WP)
{
	app->brSets.Save(PATHS::UserConfigDir()+"/brushes.ini");
}
void CGui::btnBrushesLoad(WP wp)
{
	app->brSets.Load(PATHS::UserConfigDir()+"/brushes.ini");
	if (wp)
		app->GenBrushesPrv();
	// else  // at start
	// 	app->prvBrushes.Load(PATHS::UserConfigDir()+"/brushes.png",1);
	
	UpdBrushesImgs();
}
void CGui::btnBrushesLoadDef(WP)
{
	app->brSets.Load(PATHS::GameConfigDir()+"/brushes.ini");
	app->GenBrushesPrv();
	UpdBrushesImgs();
}


//  brush random
void CGui::btnBrushRandom(WP)
{
	app->SetBrushRandom(0);
}
void CGui::btnBrushRandom2(WP)
{
	app->SetBrushRandom(1);
}
void App::SetBrushRandom(int n)
{
	float r = Math::UnitRandom();
	curBr().shape = n ? (r > 0.5f ? BRS_Noise2 : BRS_Noise) :
		(EBrShape)(Math::UnitRandom() * BRS_Noise);
	curBr().power = Math::RangeRandom(0.2, 6.0);
	
	curBr().freq  = pow(Math::RangeRandom(0.2, 0.9), 2.0);
	curBr().octaves = Math::RangeRandom(1, 9);
	curBr().offsetX = Math::RangeRandom(-20.0, 20.0);
	curBr().offsetY = Math::RangeRandom(-10.0, 10.0);
	UpdBr();
}


void App::UpdBr()
{
	gui->SldUpdBr();  updBrush();  UpdEditWnds();
}

//  set brush preset
void App::SetBrushPreset(int id, bool upd)
{
	const BrushSet& st = brSets.v[id];  // copy params
	if (!ctrl)  SetEdMode((ED_MODE)st.edMode);

	curBr().size = st.size;  curBr().intens = st.intens;  curBr().shape = st.shape;
	curBr().power = st.power;  curBr().freq = st.freq;  curBr().octaves = st.octaves;
	curBr().offsetX = st.offsetX;  curBr().offsetY = st.offsetY;

	if (st.filter > 0.f)  curBr().filter = st.filter;
	if (st.height != -0.01f)  curBr().height = st.height;

	// curBr().name = st.name;
	curBr().newLine = st.newLine;
	if (upd)  UpdBr();
}

//  ter brush pars
void CGui::slUpdBr(SV*)
{
	app->updBrush();
}
void CGui::SldUpdBr()
{
	if (!app)  return;
	svBrSize.UpdF(&app->curBr().size);     svBrForce.UpdF(&app->curBr().intens);
	svBrPower.UpdF(&app->curBr().power);   svBrShape.UpdI(&app->curBr().shape);
	svBrFreq.UpdF(&app->curBr().freq);     svBrOct.UpdI(&app->curBr().octaves);
	svBrOfsX.UpdF(&app->curBr().offsetX);  svBrOfsY.UpdF(&app->curBr().offsetY);
	// svBrSetH.UpdF(&app->curBr().height);
	svBrFilt.UpdF(&app->curBr().filter);

	edBrName->setCaption(app->brSets.v[iBrGui].name);
	svBrRate.UpdI(&app->brSets.v[iBrGui].rate);
	svBrNewLn.UpdI(&app->brSets.v[iBrGui].newLine);
}
