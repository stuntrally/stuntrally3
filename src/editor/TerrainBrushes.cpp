#include "pch.h"
#include "Def_Str.h"
#include "paths.h"
#include "CGui.h"
#include "GuiCom.h"
#include "App.h"
// #include "settings.h"
#include <OgreTimer.h>
#include <OgreResourceGroupManager.h>
#include <OgreStagingTexture.h>
#include <MyGUI_ComboBox.h>
using namespace Ogre;
using namespace std;
using namespace MyGUI;


const Ogre::String BrushesIni::csBrShape[BRS_ALL] =
{ "Triangle", "Sinus", "N-gon", "Noise", "Noise2" };  // static


//  update all Brushes prv png  ......................................................
// todo: brush presets in xml, auto upd
void App::ToolBrushesPrv()
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
	LogO(String("::: Time ALL brush: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  // < 1s
}



///  ⛰️ upd Gui  brush presets   o o o o o o
//------------------------------------------------------------------------
void CGui::UpdBrushes()
{
	auto* scv = scvBrushes;

	//  destroy old
	for (auto img : brImgs)
		mGui->destroyWidget(img);
	brImgs.clear();
	
	for (auto txt : brTxts)
		mGui->destroyWidget(txt);
	brTxts.clear();

	//  create
	int yy=0, xx=0;  const int uv = 128;
	for (int i=0; i < app->brSets.v.size(); ++i,++xx)
	{
		const auto& st = app->brSets.v[i];  const String s = toStr(i);
		int x,y, xt,yt, sx; //, row1 = i-14;  // y,x for next lines
		/*if (row1 < 0)  // top row
		{	x = 10+ i*50;  y = 10;
			xt = x + 20;  yt = y + 50;  sx = 48;
		}else*/
		{	if (st.newLine==1 && xx > 0 || xx > 9) {  xx=0;  ++yy;  }  // 1 new line
			x = 0+ xx*70;  y = 10+ yy*70;
			xt = x + 15;  yt = y + 55;  sx = 56; //64;
			if (st.newLine < 0)  xx -= st.newLine;  // -1 empty x
		}
		Img img = scv->createWidget<ImageBox>("ImageBox", x,y, sx,sx, Align::Default, "brI"+s);
		img->eventMouseButtonClick += newDelegate(this, &CGui::btnBrushPreset);
		img->setUserString("tip", st.name);  img->setNeedToolTip(true);
		img->setImageTexture("brushes.png");
		img->setImageCoord(IntCoord( i%16*uv, i/16*uv, uv,uv ));
		if (!st.name.empty())  img->eventToolTip += newDelegate(gcom, &CGuiCom::notifyToolTip);
		gcom->setOrigPos(img, "EditorWnd");
		brImgs.push_back(img);
		
		Txt txt = scv->createWidget<TextBox>("TextBox", xt,yt, 60,22, Align::Default, "brT"+s);
		txt->setCaption(fToStr(st.Size,0,2));
		auto si = st.Size, in = st.Intens;
		txt->setCaption(
			gcom->getClrVal( si / 150.f * 17.f) + fToStr(si,0,2) + " "+ 
			gcom->getClrVal( in / 60.f * 17.f) + fToStr(in,0,2));
		txt->setFontHeight(st.Size /10 + 15);
		gcom->setOrigPos(txt, "EditorWnd");
		brTxts.push_back(txt);
	}
	if (bGI)  // resize
		gcom->doSizeGUI(scv->getEnumerator());
}

void CGui::btnBrushAdd(WP)
{

}
void CGui::btnBrushDel(WP)
{

}

void CGui::btnBrushesSave(WP)
{
	app->brSets.Save(PATHS::UserConfigDir()+"/brushes.ini");
}
void CGui::btnBrushesLoad(WP)
{
	app->brSets.Load(PATHS::UserConfigDir()+"/brushes.ini");
	UpdBrushes();
}
void CGui::btnBrushesLoadDef(WP)
{
	app->brSets.Save(PATHS::GameConfigDir()+"/brushes.ini");
	UpdBrushes();
}
