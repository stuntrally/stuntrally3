#pragma once
#include "Gui_Def.h"
#include "AppGui.h"
#include "GameState.h"

#include <OgreVector3.h>
#include <OgreString.h>
#include "enums.h"
#include "SDL_scancode.h"

struct SDL_Window;
namespace Ogre {  class SceneNode;  class Overlay;  class OverlayElement;  }
class SplineRoad;
	

class BaseApp : public AppGui, public GameState
{
public:
	BaseApp();
	virtual ~BaseApp();
	// virtual void Run( bool showDialog );

	friend class CGui;
	friend class CGuiCom;
	friend class CScene;

	bool bWindowResized =1;
	
protected:	
	// bool mShowDialog =0;
	// bool setup(), configure();
	
	bool bFirstRenderFrame =1;
	
	///  create
	// virtual void createScene() = 0;
	// virtual void destroyScene() = 0;

	// void createCamera(), createFrameListener();
	// void setupResources(), loadResources();

	void onCursorChange(const std::string& name);
	// virtual void windowResized(int x, int y);
	// virtual void windowClosed();

protected:	
	SDL_Window* mSDLWindow =0;
	Ogre::Vector3 mCamPosOld{0,0,0}, mCamDirOld{0,0,0};


	///  🕹️ Input  ----------------
	///  input events
	void mouseMoved( const SDL_Event &arg ) override;
	void mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;
	void mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;

	// void textEditing( const SDL_TextEditingEvent& arg ) override;
	void textInput( const SDL_TextInputEvent& arg ) override;
	void BaseKeyPressed( const SDL_KeyboardEvent &arg );
	virtual void keyReleased (const SDL_KeyboardEvent &arg ) override;
	
	int iKeys[SDL_NUM_SCANCODES] = {0,};  // all keys state
	int IsKey(SDL_Scancode key) {  return iKeys[key];  }

	//  input vars
	bool alt =0, ctrl =0, shift =0;  // key modifiers
	bool mbLeft =0, mbRight =0, mbMiddle =0;  // mouse buttons

	Ogre::String  mDebugText;	// info texts


	///  🎥 camera upd
	int mKeys[10] = {0,0,0,0,0,0, 0,0,0,0};  // cam arrows
	bool bMoveCam  =0;
	int mx =0, my =0, mz =0;  double mDTime =0.0;
	Ogre::Real mRotX =0, mRotY =0,  mRotKX =0, mRotKY =0,  moveMul =0, rotMul =0;
	Ogre::Vector3 mTrans{0,0,0};

	float fStFade =0.f;  // status


	///  🎛️ Gui  ..........................
	bool bGuiFocus =0;  // gui shown
	void baseInitGui();

	Wnd mWndBrush =0, mWndCam =0, mWndStart =0,  // tool windows
		mWndRoadCur =0, mWndRoadStats =0,
		mWndFluids =0, mWndObjects =0, mWndParticles =0,
		mWndCollects =0, mWndFields =0;

	//MyGUI::VectorWidgetPtr
	std::vector<WP> vwGui;  // all widgets to destroy
	Img imgCur =0, bckInput =0, bckCamPos =0;
	Txt txCamPos =0, txInput =0;


	//  🪧 main menu  1 level
	Wnd mWMainMenu =0, mWndTrack =0, mWndEdit =0;
	Img mMainPanels[WND_ALL] ={0,};
	Btn mMainBtns[WND_ALL] ={0,};

	///  wnds  ----
	Wnd mWndHelp =0, mWndOpts =0,  // common in game & ed
		mWndMaterials =0, mWndTrkFilt =0;  // common 🛠️ tools
	Wnd mWndPick =0;
	Tab mTabsTrack =0, mTabsEdit =0,  // main tabs on windows
		mTabsHelp =0, mTabsOpts =0, mTabsMat =0;  // common tabs

	
	ED_MODE	edMode = ED_Deform, edModeOld = ED_Deform;
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
