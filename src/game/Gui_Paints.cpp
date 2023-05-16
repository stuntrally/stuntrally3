#include "pch.h"
// #include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "settings.h"
#include "paths.h"

#include "paints.h"
#include "CData.h"
#include "CGui.h"
#include "CGame.h"
#include "CarModel.h"

#include <OgreMath.h>
#include <MyGUI.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


//  🎨 vehicle Paints
//---------------------------------------------------------------------
void CGui::chkPaintAdj(Ck*)
{
	bool vis = panPaintAdj->getVisible();
	panPaintAdj->setVisible(!vis);
	// btPaintRandom->setAlpha(vis ? 1.f : 0.5f);
}

//  3. apply new color to car/ghost
void CGui::SetPaint()  // Mr Pet Saint
{
	if (!bGI)  return;
	
	auto& cm = app->carModels;
	int s = cm.size(), i;
	if (iCurCar == 4)  // ghost
	{
		for (i=0; i < s; ++i)
			if (cm[i]->isGhost() && !cm[i]->isGhostTrk())
				cm[i]->SetPaint();
	}
	else if (iCurCar == 5)  // track's ghost
	{
		for (i=0; i < s; ++i)
			if (cm[i]->isGhostTrk())
				cm[i]->SetPaint();
	}else
		if (iCurCar < s)  // player
			cm[iCurCar]->SetPaint();
}

//  2. upd game set color and sliders
void CGui::UpdPaintSld(bool upd)
{
	SldUpd_Paint();
	int i = iCurCar;
	
	int c = pSet->car_clr;
	bool ok = c >= 0 && c < data->paints->v.size() && c < imgsPaint.size();
	pSet->game.clr[i] = pSet->gui.clr[i];  // copy to apply
	
	UpdPaintCur();
	if (upd)
		SetPaint();
	UpdImgClr();
}

void CGui::UpdPaintCur()
{
	int c = pSet->car_clr;
	bool ok = c >= 0 && c < data->paints->v.size() && c < imgsPaint.size();
	imgPaintCur->setVisible(ok);
	if (!ok)  return;

	Img i = imgsPaint[c];
	auto p = i->getPosition();
	auto s = i->getSize();
	imgPaintCur->setCoord(p.left-3, p.top-3, s.width+6, s.height+6);
}

//  1. upd sld and pointers after tab change
void CGui::SldUpd_Paint()
{
	auto* tab = tbColorType;
	auto& gc = pSet->gui.clr[iCurCar];
	const int t = tab->getIndexSelected();
	const bool mix = t == 5;  // tab clr chg mix
	if (!mix)  // clr hsv
	{	auto& c = t >= 2 ? gc.paints[t-2] : gc.clr[t];
		svPaintH.UpdF(&c.hue);  svPaintS.UpdF(&c.sat);  svPaintV.UpdF(&c.val);
	}
	//  hide/show clr Type tabs  [diff] [spec] [1][2][3] [Mix]
	const int typ = gc.type;
	const bool clr3 = typ == CP_3Clrs;
	tab->setButtonWidthAt(0, !clr3 ? -1 : 1);
	tab->setButtonWidthAt(1, typ == CP_DiffSpec ? -1 : 1);
	for (int i = 2; i <= 5; ++i)
		tab->setButtonWidthAt(i, clr3 ? -1 : 1);

	//  hide/show Mix pan
	panPaintHSV->setVisible(!mix);
	panPaintMix->setVisible(mix);
	svPaint1Mul.UpdF(&gc.paintMulAll);
	svPaint2Mul.UpdF(&gc.paintMul2nd);
	svPaint3Mul.UpdF(&gc.paintPow3rd);
	//  gui
	ckPaintNewLine.Upd(&gc.new_line);
	svPaintRate.UpdI(&gc.rate);
	
	//  pbs params
	svPaintGloss.setVisible(t == 0);
	svPaintGloss.UpdF(&gc.gloss);
	svPaintRough.UpdF(&gc.rough);

	svClearCoat.UpdF(&gc.clear_coat);
	svClearCoatRough.UpdF(&gc.clear_rough);
	svPaintFresnel.UpdF(&gc.fresnel);
	svPaintType.UpdI(&gc.type);
}

void CGui::slPaintType(SV*)
{
	//  set visible tab
	auto& gc = pSet->gui.clr[iCurCar];
	const int typ = gc.type;
	const bool clr3 = typ == CP_3Clrs;
	int i = clr3 ? 2 : 0;
	tbColorType->setIndexSelected(i);
	tabColorType(tbColorType, i);
	
	SldUpd_Paint();
	SetPaint();
	UpdImgClr();
	UpdPaints();
}

void CGui::slPaint(SV*)
{
	SetPaint();
	UpdImgClr();
	UpdPaints();
}

//  upd data for ini save
void CGui::UpdPaints()
{
	int i = iCurCar;  // plr
	int b = pSet->car_clr;  // btn
	if (b >= 0 && b < data->paints->v.size())
		data->paints->v[b] = pSet->gui.clr[i];
}

void CGui::tabColorType(Tab, size_t)
{
	SldUpd_Paint();
	UpdImgClr();
}

void CGui::UpdImgClr()
{
	const int i = iCurCar;
	const auto& gc = pSet->gui.clr[i];
	const int tb = tbColorType->getIndexSelected();
	
	const auto tp = gc.type;
	
	const bool vis = !(tp == CP_3Clrs && tb == 5);
	imgPaint->setVisible(vis);
	txPaintRgb->setVisible(vis);
	if (!vis)  return;
	
	//  clr
	const int t = tp == CP_OneClr ? 0 : tb;
	const auto& cl = t >= 2 ? gc.paints[t-2] : gc.clr[t];
	
	ColourValue c;  c.setHSB(1.f - cl.hue, cl.sat, cl.val);
	Colour cc(c.r, c.g, c.b);
	imgPaint->setColour(cc);
	// todo: 3clr,2 etc..  3d view render sphere

	txPaintRgb->setCaption("RGB: "+fToStr(c.r)+" "+fToStr(c.g)+" "+fToStr(c.b));  // rgb info-
	
	// int b = pSet->car_clr;  // grid btn-
	// if (b >= 0 && b < imgsPaint.size())
	// 	imgsPaint[b]->setColour(cc);
}


//  🎨 Paint img click
//---------------------------------------------------------------------
void CGui::imgBtnPaint(WP img)
{
	auto& c = pSet->car_clr;
	auto i = s2i(img->getUserString("i"));
	
	auto& v = data->paints->v;
	int si = v.size();
	if (c >= si)  c = -1;  //return;  }
	if (i < 0 || i >= si)  return;

	if (app->alt && c >= 0)  // todo: tip, info
	{
		auto cnl = v[c].new_line, inl = v[i].new_line;
		auto cgr = v[c].group, igr = v[i].group;

		//  swap
		auto cc = v[c];
		v[c] = v[i];  v[i] = cc;

		v[c].new_line = cnl;  v[i].new_line = inl;  // fixme
		v[c].group = cgr;  v[i].group = igr;
		UpdPaintImgs();  return;
	}
	if (app->shift && c >= 0)
	{
		auto cnl = v[c].new_line, inl = v[i].new_line;
		auto cgr = v[c].group, igr = v[i].group;

		//  move cur
		auto cc = v[c];
		if (i >= 0 && i < v.size())
			v.erase(v.begin() + c);
		v.insert(v.begin() + i, cc);

		v[c].new_line = cnl;  v[i].new_line = inl;  // fixme
		v[c].group = cgr;  v[i].group = igr;
		UpdPaintImgs();  return;
	}
	//  click, set clr
	{	c = i;
		pSet->gui.clr[iCurCar] = v[c];
		UpdPaintSld();
	}
}


//  🌀 Random Paint
//---------------------------------------------------------------------
void CGui::btnPaintRandom(WP)
{
	int i = iCurCar;
	auto& gc = pSet->gui.clr[i];

	//  🎨 pick random from list
	if (!pSet->paintAdj)
	{	
		auto& v = data->paints->v;
		int si = v.size() - 1;
		pSet->car_clr = Math::RangeRandom(0, si);
		pSet->gui.clr[i] = v[pSet->car_clr];
		// UpdPaintSld();
		SetPaint();
		// UpdImgClr();
		UpdPaintSld();
		return;
	}
	//  🌈 randomize values, adjust
	pSet->car_clr = -1;  //-

	// gc.type = Math::RangeRandom(0.f,3.f);  //no chg
	gc.rate = 0;
	for (int c = 0; c < 5; ++c)
	{
		auto& cl = c >= 2 ? gc.paints[c-2] : gc.clr[c];
		cl.hue = Math::UnitRandom();
		cl.sat = powf(Math::UnitRandom(), 0.3f);
		cl.val = powf(Math::UnitRandom(), 0.5f);
	}
	gc.gloss = powf(Math::UnitRandom(), 0.5f);
	gc.rough = Math::RangeRandom(0.01f,0.5f);
	
	gc.clear_coat = Math::RangeRandom(0.05f, 0.6f);
	gc.clear_rough = Math::RangeRandom(0.01f, 0.12f);
	gc.fresnel = powf(Math::RangeRandom(0.f,1.f), 0.4f);

	gc.paintMulAll = Math::RangeRandom(0.05f, 0.3f);
	gc.paintMul2nd = Math::RangeRandom(2.f,4.f);
	gc.paintPow3rd = Math::RangeRandom(2.f,5.f);
	UpdPaintSld();
}


//  save, load
void CGui::btnPaintSave(WP)
{
	auto user = PATHS::UserConfigDir() + "/paints.ini";
	data->paints->Save(user);
}
void CGui::btnPaintLoadDef(WP)
{
	data->LoadPaints(true);
	UpdPaintImgs();
}
void CGui::btnPaintLoad(WP)
{
	data->LoadPaints();
	UpdPaintImgs();
}

//  upd all
void CGui::slPaintRate(SV*)
{
	// todo: drag freezes
	UpdPaints();
	UpdPaintImgs();
}

void CGui::chkPaintNewLine(Ck*)
{
	UpdPaints();
	UpdPaintImgs();
}


//  paint add +
void CGui::btnPaintAdd(WP)
{
	CarPaint c = pSet->gui.clr[iCurCar];
	auto i = pSet->car_clr;
	auto& v = data->paints->v;
	if (i >= 0 && i < v.size()-1)
		v.insert(v.begin() + i + 1, c);
	else
		v.push_back(c);

	// insert after car_clr?
	UpdPaintImgs();
}

//  paint del -
void CGui::btnPaintDel(WP)
{
	auto& i = pSet->car_clr;
	auto& v = data->paints->v;
	if (i >= 0 && i < v.size())
		v.erase(v.begin() + i);
	
	if (i == v.size() && i > 0)
		--i;

	UpdPaintImgs();
}


//  🔁🎨 Upd All Paints palette
//---------------------------------------------------------------------
void CGui::UpdPaintImgs()
{
	Ogre::Timer ti;
	const auto* p = data->paints;
	const int all = p->v.size(),
		clrRow = p->perRow, sx = p->imgSize;
	
	//  destroy old
	auto* scv = scvPaints;
	for (auto img : imgsPaint)
		mGui->destroyWidget(img);
	imgsPaint.clear();
	
	for (auto txt : txtsPaint)
		mGui->destroyWidget(txt);
	txtsPaint.clear();
	
	//  add clr img  --------
	auto AddImg = [&](WP wp,
		int rate, bool first,
		int px,int py, int sx,int sy,
		int u, bool focus,
		const CarPaint::Clr* clr, float mul=1.f)
	{
		Img img = wp->createWidget<ImageBox>("ImageBox",
			px, !first ? py : py +(6-rate)/2,
			sx-1 +rate, sx-1 +rate, Align::Left);
		img->setImageTexture("paint_icons.png");
		img->setImageCoord(IntCoord( u*32,0, 32,32 ));
		
		gcom->setOrigPos(img, "GameWnd");
		img->setNeedMouseFocus(focus);
		
		if (clr)
		{	Ogre::ColourValue c;
			c.setHSB(1.f-clr->hue, clr->sat, clr->val * mul);
			img->setColour(Colour(c.r,c.g,c.b));
		}
		return img;
	};

	//  Grid palette  --------------------
	int x = 0, y = 0, x0 = 5, y0 = 5;
	int px = x0, py = y0, rmax = 0;  // pos
	auto inc_y = [&]()
	{
		x = 0;  px = x0;
		++y;  py += sx + 6 + 2*rmax/3;  rmax = 0;
	};
	for (int i=0; i < all; ++i)
	{
		const auto& cl = p->v[i];
		const int r = cl.rate * 6;
		rmax = max(rmax, r);

		//  group text  ----
		if (!cl.group.empty())
		{
			Txt txt = scv->createWidget<TextBox>("TextBox",
				px, py+6,
				320, sx, Align::Left);

			auto s = StringUtil::replaceAll(cl.group, "@", "#");
			txt->setCaption(TR(s));
			// txt->setFontName("font.big");
			
			gcom->setOrigPos(txt, "GameWnd");
			txt->setNeedMouseFocus(false);

			txtsPaint.push_back(txt);
			inc_y();
		}

		//  create clr img 1..3  ------
		Img img = 0, im2 = 0, im3 = 0;
		switch (cl.type)
		{
		case CP_OneClr:
			img = AddImg(scv, r,1, px,py, sx-1,sx-1, 0, 1, &cl.clr[0]);
			break;
		
		case CP_DiffSpec:
		{
			int p2 =  8 *sx/32, s2 = 24 *sx/32;
			img = AddImg(scv, r,1, px,py, sx-1,sx-1, 0, 1, &cl.clr[0]);
			im2 = AddImg(img, r,0, p2, 0, sx-1,sx-1, 3, 0, &cl.clr[1]);
		}	break;
		
		case CP_3Clrs:
		{	int p2 =  0 *sx/32, s2 = 33 *sx/32;
			int p3 = 14 *sx/32, s3 = 20 *sx/32;
			float m = min(1.f, cl.paintMulAll / 0.3f);
			img = AddImg(scv, r,1, px,py, sx-1,sx-1, 0, 1, &cl.paints[2], m);
			im2 = AddImg(img, r,0, p2,p2, s2,s2, 3, 0, &cl.paints[1]);
			im3 = AddImg(img, r,0, p3,p3, s3,s3, 3, 0, &cl.paints[0]);
		}	break;
		}
		
		//  shine  rough  * `  ----
		int s1 = cl.rough * 2.5f * sx + 2;  s1 = min(64, s1);  // 32
		im2 = AddImg(img, r,0, 0,0, s1,s1, 1, 0, 0);
		
		float a = cl.gloss * cl.clear_coat * 2.f;  a = min(1.f, a);
		im2->setAlpha(a);
		// if (!cl.one_clr)  // own spec
		// 	img2->setColour(cl.clr[1]);


		a = 1.f - cl.rough * 0.5f;
		// img->setAlpha(a);  // *
		
		//  gui main  ----
		img->eventMouseButtonClick += newDelegate(this, &CGui::imgBtnPaint);

		img->setUserString("i", toStr(i));
		img->setUserString("tip", toStr(i)+"\n"+TR("#{TipCarColor}"));
		img->setNeedToolTip(true);
		img->eventToolTip += newDelegate(gcom, &CGuiCom::notifyToolTip);
		
		imgsPaint.push_back(img);


		//  inc pos  ---
		++x;  px += sx + r;
		if (x >= clrRow || cl.new_line)
			inc_y();
	}
	if (bGI)  // resize
		gcom->doSizeGUI(tbPlrPaint->getEnumerator());
	
	UpdPaintCur();
	LogO(String("::: Time upd Paints: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
