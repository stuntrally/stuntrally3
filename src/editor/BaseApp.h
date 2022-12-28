#pragma once
#include "Gui_Def.h"
#include "AppGui.h"
#include "GameState.h"

#include <OgreVector3.h>
#include <OgreString.h>
#include "enums.h"

struct SDL_Window;
namespace Ogre {  class SceneNode;  class Overlay;  class OverlayElement;  }
class SplineRoad;
	

class BaseApp : public AppGui, public GameState
{
public:
	virtual ~BaseApp();
	// virtual void Run( bool showDialog );

	friend class CGui;
	friend class CGuiCom;
	friend class CScene;

	bool bWindowResized =1;
	
	bool mShutDown =0;
protected:	
	// bool mShowDialog =0;
	// bool setup(), configure();
	void updateStats();
	
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
	Ogre::Vector3 mCamPosOld, mCamDirOld;


	///  🕹️ Input  ----------------
	///  input events
	void mouseMoved( const SDL_Event &arg ) override;
	void mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;
	void mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;

	// void textEditing( const SDL_TextEditingEvent& arg ) override;
	void textInput( const SDL_TextInputEvent& arg ) override;
	void BaseKeyPressed( const SDL_KeyboardEvent &arg );
	virtual void keyReleased (const SDL_KeyboardEvent &arg ) override;

	//  input vars
	bool alt =0, ctrl =0, shift =0;  // key modifiers
	bool mbLeft =0, mbRight =0, mbMiddle =0;  // mouse buttons

	Ogre::String  mDebugText;	// info texts


	///  🎥 camera upd
	int mKeys[10] = {0,0,0,0,0,0, 0,0,0,0};  // cam arrows
	bool bMoveCam  =0;
	int mx =0, my =0, mz =0;  double mDTime =0.0;
	Ogre::Real mRotX =0, mRotY =0,  mRotKX =0, mRotKY =0,  moveMul =0, rotMul =0;
	Ogre::Vector3 mTrans;

	///  overlay
	Ogre::Overlay *ovBrushPrv =0, *ovTerPrv =0;
	Ogre::OverlayElement *ovBrushMtr =0, *ovTerMtr =0;
	float fStFade =0.f;


	///  🎛️ Gui  ..........................
	bool bGuiFocus =0;  // gui shown
	void baseInitGui();

	Wnd mWndBrush =0, mWndCam =0, mWndStart =0,  // tool windows
		mWndRoadCur =0, mWndRoadStats =0,
		mWndFluids =0, mWndObjects =0, mWndParticles =0;

	//MyGUI::VectorWidgetPtr
	std::vector<WP> vwGui;  // all widgets to destroy
	Img imgCur =0, bckInput =0;
	Txt txCamPos =0, txInput =0;


	///  🪧 main menu
	Wnd mWndMain =0, mWndTrack =0, mWndEdit =0, mWndHelp =0, mWndOpts =0;  // menu, windows
	Wnd mWndTrkFilt =0, mWndPick =0;
	Tab mWndTabsTrack =0, mWndTabsEdit =0, mWndTabsHelp =0, mWndTabsOpts =0;  // main tabs on windows

	WP mWndMainPanels[WND_ALL] ={0,};
	Btn mWndMainBtns[WND_ALL] ={0,};

	
	ED_MODE	edMode = ED_Deform, edModeOld = ED_Deform;
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
