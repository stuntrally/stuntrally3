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


//  🖌️🖼️ generate all Brushes prv png  ......................................................
void App::GenBrushesPrv()
{
	Ogre::Timer ti;
	const uint si = 2048;  // 16 * 128  BrPrvSize
	uint32* data = new uint32[BrPrvSize * BrPrvSize];

	Image2 im;
	im.createEmptyImage(si, si, 1, TextureTypes::Type2D, PFG_RGBA8_UNORM);
	uint32* big = (uint32*)im.getRawBuffer();
	memset(big, 0, si*si);

	uint u = 0, v = 0;
	for (int i=0; i < brSets.v.size(); ++i)
	{
		SetBrushPreset(i, 0);
		updBrushData((uint8*)data);
		
		//  store prv in big
		uint a = 0;
		for (uint y=0; y < BrPrvSize; ++y)
		{
			uint b = u + (v + y) * si;
			for (uint x=0; x < BrPrvSize; ++x)
				big[b++] = data[a++];
		}
		u += BrPrvSize;  // next u,v prv pos
		if (u >= si)
		{	u = 0;  v += BrPrvSize;
			if (v >= si)
			{	LogO("!No more room for brushes on size: "+toStr(si));
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

	//  add user png dir
	// auto& resMgr = ResourceGroupManager::getSingleton();
	// resMgr.addResourceLocation(PATHS::GameConfigDir(), "FileSystem", "Popular");

	//  create
	int yy=0, xx=0;  const int uv = 128, s = 10;
	for (int i=0; i < app->brSets.v.size(); ++i,++xx)
	{
		const auto& st = app->brSets.v[i];  const String s = toStr(i);
		int x,y, xt,yt, sx; //, row1 = i-14;  // y,x for next lines
		/*if (row1 < 0)  // top row
		{	x = 10+ i*50;  y = 10;
			xt = x + 20;  yt = y + 50;  sx = 48;
		}else*/
		{	if (st.newLine==1 && xx > 0 || xx > 9) {  xx=0;  ++yy;  }  // 1 new line
			x = 0+ xx*70;  y = 0+ yy*70;
			xt = x + 15;  yt = y + 55;  sx = 56; //64;
			if (st.newLine < 0)  xx -= st.newLine;  // -1 empty x
		}
		Img img = scv->createWidget<ImageBox>("ImageBox", x,y, sx,sx, Align::Default, "brI"+s);
		img->eventMouseButtonClick += newDelegate(this, &CGui::btnBrushPreset);
		img->setUserString("tip", st.name);  img->setNeedToolTip(true);
		img->setImageTexture("PrvBrushes"); //"brushes.png");
		img->setImageCoord(IntCoord( i%16*uv, i/16*uv, uv,uv ));
		if (!st.name.empty())  img->eventToolTip += newDelegate(gcom, &CGuiCom::notifyToolTip);
		gcom->setOrigPos(img, "EditorWnd");
		brImgs.push_back(img);
		
		Txt txt = scv->createWidget<TextBox>("TextBox", xt,yt, 60,22, Align::Default, "brT"+s);
		txt->setCaption(fToStr(st.size,0,2));
		auto si = st.size, in = st.intens;
		txt->setCaption(
			gcom->getClrVal( si / 150.f * 17.f) + fToStr(si,0,2) + " "+ 
			gcom->getClrVal( in / 60.f * 17.f) + fToStr(in,0,2));
		txt->setFontHeight(st.size /10 + 15);
		gcom->setOrigPos(txt, "EditorWnd");
		brTxts.push_back(txt);
	}
	if (bGI)  // resize
		gcom->doSizeGUI(scv->getEnumerator());
}

//  gui events
//--------------------------------------------------------------------------------------------------------------------------
void CGui::chkBrushAdj(Ck*)
{
	panBrushAdj->setVisible(pSet->br_adjust);
}
void CGui::slBrNewLine(SV*)
{
	UpdBrushesImgs();
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
	
	curBr().freq  = pow(Math::RangeRandom(0.01, 1.0), 2.0);
	curBr().offset = Math::RangeRandom(-30.0, 30.0);
	curBr().octaves = Math::RangeRandom(1, 9);
	UpdBr();
}

void App::UpdBr()
{
	gui->SldUpdBr();  updBrush();  UpdEditWnds();
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
	
	//  img cur [] set pos
	auto p = img->getPosition();
	auto s = img->getSize();
	imgBrCur->setCoord(p.left, p.top, s.width, s.height);

	if (app->alt && c >= 0)
	{
		// auto cnl = v[c].new_line, inl = v[i].new_line;
		// auto cgr = v[c].group, igr = v[i].group;

		//  swap
		auto cc = v[c];
		v[c] = v[i];  v[i] = cc;

		// v[c].new_line = cnl;  v[i].new_line = inl;  // fixme
		// v[c].group = cgr;  v[i].group = igr;
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
		UpdBrushesImgs();  return;
	}
	//  click, set
	iBrGui = i;
	app->SetBrushPreset(i);
}


//  set brush preset
void App::SetBrushPreset(int id, bool upd)
{
	const BrushSet& st = brSets.v[id];  // copy params
	if (!shift)  SetEdMode((ED_MODE)st.edMode);
	curBr().size = st.size;  curBr().intens = st.intens;  curBr().shape = st.shape;
	curBr().power = st.power;  curBr().freq = st.freq;
	curBr().offset = st.offset;  curBr().octaves = st.octaves;
	if (st.filter > 0.f)  curBr().filter = st.filter;
	if (st.height != -0.01f)  curBr().height = st.height;
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
	svBrSize.UpdF(&app->curBr().size);    svBrForce.UpdF(&app->curBr().intens);
	svBrPower.UpdF(&app->curBr().power);  svBrShape.UpdI(&app->curBr().shape);
	svBrFreq.UpdF(&app->curBr().freq);    svBrOct.UpdI(&app->curBr().octaves);
	svBrOfs.UpdF(&app->curBr().offset);
	svBrSetH.UpdF(&app->curBr().height);  svBrFilt.UpdF(&app->curBr().filter);
	// svBrNewLine.UpdI(&app->curBr().newLine);
	svBrNewLine.UpdI(&app->brSets.v[iBrGui].newLine);
}
