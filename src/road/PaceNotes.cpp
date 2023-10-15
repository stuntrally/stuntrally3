#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "PaceNotes.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
#else
	#include "CGame.h"
	#include "settings.h"
#endif
#include "HudRenderable.h"
#include <OgreTimer.h>
#include <OgreVector3.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreWindow.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
#include <tinyxml2.h>
using namespace tinyxml2;
using namespace std;
using namespace Ogre;
using namespace MyGUI;


//  🌟 ctor  ---------
// PaceNote::PaceNote()
// 	:pos(0,0,0), size(4.f,4.f), clr(0,0,0,0), ofs(0,0), uv(0,0)
// {	}
PaceNote::PaceNote(int i, int t, Vector3 p,  //id,use, pos
		float sx,float sy,  float r,float g,float b,float a,  //size, clr
		float ox,float oy, float u,float v)  //ofs:dir,bar width, tex uv
	:pos(p), size(sx,sy), clr(r,g,b,a), ofs(ox,oy), uv(u,v)
	,use(t), id(i), start(0), jump(0), vel(0.f), text(0)
{	}


//  🆕 Create 1  ---------
void PaceNotes::Create(PaceNote& n)
{
	// if (n.use == 1)
	// 	LogO("PP "+toStr(n.id)+(n.start?" ST":""));
	++ii;
	if (n.text)
	{	n.txt = mGui->createWidget<TextBox>("TextBox",
			100,100, 120,64, Align::Center, "Back", "jvel"+toStr(ii));
		n.txt->setVisible(false);
		n.txt->setFontHeight(34);  //par
		n.txt->setTextShadow(true);  n.txt->setTextShadowColour(Colour::Black);
	}
}

//  💥 Destroy 1
void PaceNotes::Destroy(PaceNote& n)
{
	if (n.txt){  mGui->destroyWidget(n.txt);  n.txt = 0;  }
}


//  💫 Update 1
void PaceNotes::Update(PaceNote& n)
{
	UpdateTxt(n);
}
void PaceNotes::UpdateTxt(PaceNote& n)
{
	if (n.txt && n.vel > 0.f)
		n.txt->setCaption(fToStr(n.vel * (pSet->show_mph ? 2.23693629f : 3.6f),0,3));
}


//  💫 update geometry, visibility  ---------
void PaceNotes::UpdVis(Vector3 carPos, bool hide)
{
	const Real dd = pSet->pace_dist, dd2 = dd*dd;
	int i,s;

#ifndef SR_EDITOR
	//  game  ----
	const int rng = pSet->pace_next;  // vis next count
	const Real radiusA = 9.f*9.f;  //par 15 pace sphere radius

	s = vPS.size();
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPS[i];
		bool vis = pSet->pace_show;

		//  Advance to next sign  ~ ~ ~
		//        a    iCur   s-1  s=7
		//  0  1  2  3  4  5  6   -id
		int r = rewind ? 1 : 0;
		bool vrng = iDir > 0 ?  // inside cur..cur+range with cycle
			(i >= iCur-r && i <= iCur+rng || i < iCur+rng-iAll) :
			(i <= iCur+r && i >= iCur-rng || i > iCur-rng+iAll);
		vis &= vrng;

		if (vrng)
		{	Real d = p.pos.squaredDistance(carPos);
			bool vnear = d < dd2;
			vis &= vnear;
			
			if (d < radiusA && i != iCur)  // close next only
			{
				//LogO("iCur "+iToStr(i,3)+"  d "+fToStr(sqrt(d))+"  <> "+iToStr(i-iCur));
				iCur = i;
		}	}
		
		if (p.txt)  updTxt(p, vis);
		p.vis = vis;
	}
#else  //  ed  ----
	
	s = vPN.size();
	if (hide)
	{	for (i=0; i < s; ++i)
		{	PaceNote& p = vPN[i];
			p.vis = false;
			if (p.txt)  p.txt->setVisible(false);
		}
	}
	const Vector3& c = cam->cam->getPosition();
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPN[i];
		bool vis = p.use <= pSet->pace_show;

		const Vector3& o = p.pos;
		Real d = c.squaredDistance(o);
		bool vnear = d < dd2;
		vis &= vnear;

		if (p.txt)  updTxt(p, vis);
		p.vis = vis;
	}
#endif

	//  upd geom verts
	//..........................................................
	if (!hr)  return;
	bool vis = !hide && pSet->pace_show;
	if (vis)
	{
		const Real si = pSet->pace_size * 0.5f;
		const Vector3 vx = si * cam->cam->getRight(),
					  vy = si * cam->cam->getUp();
		hr->begin();

		auto add = [&](const PaceNote& pc)
		{
			float dist = cam->cam->getPosition().distance(pc.pos);
			dist = std::max(0.f, dist - 5.f / pSet->pace_near);
			float fade = std::min(1.f,  // close fade
				pSet->pace_near * dist * 0.03f);
			float a = pc.clr.w * pSet->pace_alpha * fade;
			// oc.type  sharp turns: blink, a = 1

			for (int y=-1; y <= 1; y+=2)  // 4 verts
			for (int x=-1; x <= 1; x+=2)
			{
				Vector3 p = pc.pos + vx * x * pc.size.x -
									 vy * y * pc.size.y;
				hr->position(p.x, p.y, p.z);

				int iu = pc.ofs.x > 0.f ? (x+1)/2 : (1-x)/2, iv = (y+1)/2;
				Real u = pc.uv.x + iu * 0.125f,
					 v = pc.uv.y + iv * 0.125f;
				hr->texUV(u, v);

				hr->color(pc.clr.x, pc.clr.y, pc.clr.z, a);
			}
		};

	#ifdef SR_EDITOR  // ed all
		for (i=0; i < s; ++i)
			add(vPN[i]);
	
	#else  // game count
		i = 0;  int cnt = 0;
		while (i < s && cnt < count)
		{
			const PaceNote& pc = vPS[i];
			if (pc.vis)
			{
				add(pc);
				++cnt;
			}
			++i;
		}
	#endif
		hr->end();
	}
	ndHR->setVisible(vis);
}


//  🆕 create hudrend
//----------------------------------------
void PaceNotes::CreateHR()
{
#ifdef SR_EDITOR
	count = vPN.size();
#else
	count = std::min((int)vPN.size(), pSet->pace_next);
#endif
	hr = new HudRenderable(
		"pacenote", mSceneMgr,
		OT_TRIANGLE_LIST, true,true, RV_Car,RQ_OVERLAY,
		count, true);

	SceneNode* rt = mSceneMgr->getRootSceneNode();
	ndHR = rt->createChildSceneNode();
	ndHR->attachObject(hr);
}


//  kmh/mph change  --------
void PaceNotes::UpdTxt()
{
	int i, s = vPS.size();
	for (i=0; i < s; ++i)
		UpdateTxt(vPS[i]);
}

//  🔤 text pos upd  3d to 2d
void PaceNotes::updTxt(PaceNote& n, bool vis)
{
	if (!vis || !cam->cam->isVisible(n.pos))
	{	n.txt->setVisible(false);
		return;
	}
	Vector3 pos2D = cam->cam->getProjectionMatrix() * (cam->cam->getViewMatrix() * n.pos);
	Real x =  pos2D.x * 0.5f + 0.5f;
	Real y = -pos2D.y * 0.5f + 0.5f;

	Vector3 p = Vector3(x * mWindow->getWidth(), y * mWindow->getHeight(), 1.f);
	//p.x = p.x * /*mSplitMgr->mDims[0].width **/ 0.5f;  //1st viewport dims 1,1 only
	//p.y = p.y * /*mSplitMgr->mDims[0].height **/ 0.5f;
	
	n.txt->setPosition(p.x+40, p.y-16);
	n.txt->setVisible(true);

	//  vel diff clr					   //par sens
	float d = std::max(-1.f, std::min(1.f, 0.12f*(carVel - n.vel)));  
	ColourValue c;			///par  HUD jump vel clrs
	if (d > 0.f)  c.setHSB(0.40f+d*0.2f, 1.f-d*1.f,1.f);  // above ok
	else		  c.setHSB(0.3f +d*0.3f, 1.f+d*0.3f,1.f);  // too low
	n.txt->setTextColour(Colour(c.r,c.g,c.b,1.f));
}


//  setup  ---------
void PaceNotes::Setup(SceneManager* sceneMgr, Cam* camera,
	Terra* terrain, MyGUI::Gui* gui, Ogre::Window* window)
{
	mSceneMgr = sceneMgr;  cam = camera;
	mTerrain = terrain;  mGui = gui;  mWindow = window;
}
void PaceNotes::SetupTer(Terra* terrain)
{
	mTerrain = terrain;
}


//  🌟 ctor  ---------
PaceNotes::PaceNotes(SETTINGS* pset)
	:pSet(pset)
{	}

//  💥 destroy all  ---------
void PaceNotes::Destroy()
{
	for (size_t i=0; i < vPN.size(); ++i)
		Destroy(vPN[i]);
	vPN.clear();
	vPS.clear();
	ii = 0;

	delete hr;  hr = 0;
}

//  💥 dtor
PaceNotes::~PaceNotes()
{
	Destroy();
}
