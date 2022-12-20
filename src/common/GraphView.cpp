#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "GraphView.h"
#include "HudRenderable.h"

#include <OgreCommon.h>
#include <OgreSceneManager.h>
#include <OgreWindow.h>
#include <OgreSceneNode.h>
#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace MyGUI;


//  🌟 ctor
GraphView::GraphView(SceneManager* pSceneMgr, Ogre::Window* pWindow, Gui* pGui)
	:mSceneMgr(pSceneMgr), mWindow(pWindow), mGui(pGui)
{	}

//  same as in graph1..5 materials
const Colour GraphView::graphClr[GraphView::iGraphClrs] = {
	Colour(0.0, 1.0, 1.0),
	Colour(0.0, 1.0, 0.0),
	Colour(1.0, 1.0, 0.0),
	Colour(1.0, 0.5, 0.0),
	Colour(1.0, 0.0, 0.0),

	Colour(0.9, 0.9, 0.9),
	Colour(0.8, 1.0, 1.0),
	Colour(0.6, 1.0, 1.0),
	Colour(0.4, 1.0, 1.0),
	Colour(0.2, 0.8, 1.0),
	Colour(0.2, 0.6, 1.0),
	Colour(0.2, 0.4, 1.0),
	Colour(0.2, 0.2, 1.0),

	Colour(1.0, 1.0, 0.9),
	Colour(1.0, 1.0, 0.6),
	Colour(1.0, 1.0, 0.3),
	Colour(1.0, 1.0, 0.0),
	Colour(1.0, 0.8, 0.0),
	Colour(1.0, 0.6, 0.0),
	Colour(1.0, 0.4, 0.0),
	Colour(1.0, 0.2, 0.0) };


//  🆕 Create
//----------------------------------------------------------------------------------------
void GraphView::Create(int length, String sMtr, float backAlpha, bool buffered1)
{
	vals.resize(length+1);
	iCurX = 0;  //size-1
	buffered = buffered1;  manualUpd = buffered;//
	node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	//  graph line  ----------------------
	if (length > 1)  // at least 2, use 1 for text only
	{
		moLine = new HudRenderable(sMtr, mSceneMgr,
			OT_LINE_STRIP,
			false, false, RV_Hud, RQG_Hud3, length + 1);

		/*moLine->begin();  // fill 0s?
		for (int i=0; i < length +1; ++i)
			moLine->position(i*0.01f,0,0);
		moLine->end();/**/

		attached = 0;
		// node->attachObject(moLine);  // after 1st update
	}

	//  backgr rect  ----------------------
	if (backAlpha > 0.01f)
	{
		moBack = new HudRenderable("graphBack", mSceneMgr,
			OT_TRIANGLE_LIST, false, true, RV_Hud, RQG_Hud1, 1);
		moBack->begin();
		moBack->position(0,0, 0);  moBack->color(0,0,0, backAlpha);
		moBack->position(1,0, 0);  moBack->color(0,0,0, backAlpha);
		moBack->position(0,1, 0);  moBack->color(0,0,0, backAlpha);
		moBack->position(1,1, 0);  moBack->color(0,0,0, backAlpha);
		moBack->end();
		node->attachObject(moBack);
	}
}


//  🪟 Grid lines  == ||
//----------------------------------------------------------------------------------------
void GraphView::CreateGrid(int numH, int numV, /*char clr,*/ float clr, float alpha)
{
	if (!node)  return;
	if (numH <= 0 && numV <= 0)  return;
	// return;  // fixme
	
	const float ya = 1.f / numH, xa = 1.f / numV;
	float x,y;

	int count = 0;  //  count lines  // = numH + numV
	if (numH > 0)  for (y = 0.f; y <= 1.f; y += ya)  ++count;
	if (numV > 0)  for (x = 0.f; x <= 1.f; x += xa)  ++count;

	moGrid = new HudRenderable("graphGrid", mSceneMgr,
		OT_LINE_LIST, false, true, RV_Hud, RQG_Hud2, count);

	//const Colour& gc = graphClr[clr];
	//ColourValue c(gc.red, gc.green, gc.blue, alpha);

	moGrid->begin();
	float a = sqrt(alpha);  //?-

	if (numH > 0)  //  fill
		for (y = 0.f; y <= 1.f; y += ya)
		{
			moGrid->position(0,y, 0);  moGrid->color(clr, clr, clr, a);
			moGrid->position(1,y, 0);  moGrid->color(clr, clr, clr, a);
		}
	if (numV > 0)
		for (x = 0.f; x <= 1.f; x += xa)
		{
			moGrid->position(x,0, 0);  moGrid->color(clr, clr, clr, a);
			moGrid->position(x,1, 0);  moGrid->color(clr, clr, clr, a);
		}
	moGrid->end();
	node->attachObject(moGrid);
}

/// todo: auto val range, auto grid lines, value texts..
// gui tab [big]= edit pos,size,value,text,range,etc. save in graphs.xml


//  🔤 Create title text
//----------------------------------------------------------------------------------------
void GraphView::CreateTitle(String title, char clr, float posX, char alignY, int fontHeight, int numLines, bool shadow)
{
	if (!mGui)  return;
	static int cntr = 0;  ++cntr;

	txPosX = posX;  txH = fontHeight;  txAlignY = alignY;

	txt = mGui->createWidget<TextBox>("TextBox",
		100,100, 360,txH*numLines +20,  //?
		Align::Center, "Back", "GrTx"+toStr(cntr));

	if (shadow)
	{	txt->setTextShadow(true);
		txt->setTextShadowColour(Colour::Black);  }

	txt->setFontHeight(fontHeight);
	txt->setTextColour(graphClr[clr % iGraphClrs]);
	txt->setCaption(title);
	txt->setVisible(true);
}

void GraphView::UpdTitle(String title)
{
	if (!txt)  return;
	txt->setCaption(title);
}

//  🗜️ Set Size
//----------------------------------------------------------------------------------------
void GraphView::SetSize(float posX,float posY,float sizeX,float sizeY)  // [0..1]  0,0 is left bottom
{
	if (!node)  return;
	float px = -1.f+posX*2.f, py = -1.f+posY*2.f;
	node->setPosition(px, py, 0.f);
	node->setScale(sizeX*2.f, sizeY*2.f, 1.f);

	//  set title text pos
	if (!txt || !mWindow)  return;
	int wx = mWindow->getWidth(), wy = mWindow->getHeight() +20;  //?
	int x = (posX + txPosX*sizeX) * wx;  float pszY = posY + sizeY;
	switch (txAlignY)
	{
		case -1:  txt->setPosition(x, wy - pszY * wy - txH );  break;  // above
		case -2:  txt->setPosition(x, wy - pszY * wy       );  break;  // 1 inside  top
		case -3:  txt->setPosition(x, wy - pszY * wy + txH );  break;  // 2 below 1
		case  0:  txt->setPosition(x, wy - pszY * wy       );  break;  // center
		case  3:  txt->setPosition(x, wy - posY * wy-2*txH );  break;  // 2 above 1
		case  2:  txt->setPosition(x, wy - posY * wy - txH );  break;  // 1 inside  bottom
		case  1:  txt->setPosition(x, wy - posY * wy       );  break;  // below
	}
}
//----------------------------------------------------------------------------------------

//  💥 Destroy
void GraphView::Destroy()
{
	manualUpd = false;
	// if (moLine)  moLine->end();  //?
	// if (moGrid)  moGrid->end();  //-
	// if (moBack)  moBack->end();  //-

	if (mGui && txt)  {  mGui->destroyWidget(txt);  txt = 0;  }
	if (moLine) {	delete moLine;  moLine = 0;  }
	if (moBack) {	delete moBack;  moBack = 0;  }
	if (moGrid) {	delete moGrid;  moGrid = 0;  }
	if (node) {		mSceneMgr->destroySceneNode(node);  node = 0;  }
}

//  show/hide
void GraphView::SetVisible(bool visible)
{
	if (node)  node->setVisible(visible);
	if (txt)   txt->setVisible(visible);
}


//  💫 Add value  (into buffer)
//------------------------------------------------------------------
void GraphView::AddVal(float val)
{
	//if (!node)  return;
	if (iCurX >= vals.size())  return;

	//assert vals empty
	vals[iCurX] = val;
	++iCurX;  if (iCurX >= vals.size())  iCurX = 0;
}

//  💫 Update  (on screen)
//------------------------------------------------------------------
void GraphView::SetUpdate()
{
	manualUpd = true;  // buffered
}
void GraphView::Update()
{
	if (!node || !moLine)  return;
	if (buffered && !manualUpd)  return;
	manualUpd = false;
	
	size_t size = vals.size();  //todo: mutex lock..
	int i = iCurX % size;  // vals id
	float fx = 0.f, fAdd = 1.f / size;  // screen x

	moLine->begin();
	moLine->position(fx, vals[i], 0.f);  // line start
	
	for (size_t n=0; n < size; ++n)
	{
		moLine->position(fx, vals[i], 0.f);

		++i;  if (i >= size)  i = 0;
		fx += fAdd;
	}
	moLine->end();

	if (!attached)  // after first full update
	{	attached = 1;
		node->attachObject(moLine);
	}
}
