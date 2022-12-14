#pragma once
// #include "HudRenderable.h"
#include <vector>
#include <OgreString.h>
#include <MyGUI_Colour.h>

namespace Ogre  {  class SceneManager;  class ManualObject;  class SceneNode;  class Window;  }
namespace MyGUI {  class Gui;  class TextBox;  }
class HudRenderable;


class GraphView
{
public:
	//  🌟 ctor
	GraphView(Ogre::SceneManager* pSceneMgr,
		Ogre::Window* pWindow=0,
		MyGUI::Gui* pGui=0);

	//  🆕 Create
	void Create(int length,			// values buffer length
				Ogre::String sMtr,	// material for line
				float backAlpha,	// background transparency 0..1
				bool buffered=false);	// for static graphs, if true no update is done, do it after adding all new values

	void CreateGrid(int numH,		// == horizontal lines number   1 only frame
					int numV,		// || vertical lines number		2 frame with center, 4 frame and 3 lines, etc.
					float clr,		// color (rgb gray)
					float alpha);	// transparency

	void CreateTitle(Ogre::String title, // title text
				char clr,			// text color id
				float posX,			// pos in graph rect 0..1
				char alignY,		// top:    -1 above rect, -2 in rect, -3 in rect 2nd line (below -2)
									// bottom:  1 below rect,  2 in rect,  3 in rect 2nd line (above 2)
				int fontHeight,		// in pixels eg.24
				int numLines=2,		// max number of text lines
				bool shadow=false);	// shadow under text
	void Destroy();

	//  Set position and size on screen
	void SetSize(float posX,float posY,
				float sizeX,float sizeY);  // [0..1]  0,0 is left bottom

	void SetVisible(bool visible);  // show/hide
	
	//void SetRange(bool autoMin=false, bool autoMax=false,
	//				float fMin=0.f, float fMax=1.f);  // TODO ..
	//void CreteValTxt();  // TODO for static range .. auto ?
	
	//  Update, fill
	void AddVal(float val);  // adds value at end of graph, moves left (in buffer)
	void Update();		// ogre update (on screen)
	void SetUpdate();	// request update when buffered

	void UpdTitle(Ogre::String title);

protected:
	std::vector<float> vals;  // values buffer
	int iCurX = 0;  // cur id to insert new val
	bool buffered =0, manualUpd =0;
	bool attached = 0;
	
	//float rmin, rmax, range;  // SetRange
	//bool autoMin, autoMax;  float timeUpdAuto; //?

	Ogre::SceneManager* mSceneMgr =0;  // for creating
	Ogre::Window* mWindow =0;  // gui resolution-
	MyGUI::Gui* mGui =0;       // for text only

	const static int iGraphClrs = 5+8+8;
	const static MyGUI::Colour graphClr[iGraphClrs];  // text colors
	
	HudRenderable* hrLine =0, *hrBack =0, *hrGrid =0;  //graph line, background, grid	
	Ogre::SceneNode* node =0;
	// void moSetup(Ogre::ManualObject* mo, bool dynamic, Ogre::uint8 RQG);  // helper

	MyGUI::TextBox* txt =0;
	float txPosX = 0.f;
	int txH = 0, txAlignY = -2;  // title text pos, height
};
