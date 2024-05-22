#pragma once
#include "Gui_Def.h"
#include "AppGui.h"
#include "GameState.h"
#include <unordered_map>
#include <vector>
#include <MyGUI_KeyCode.h>
#include <SDL_keycode.h>
#include <SDL_joystick.h>
#include <boost/scoped_ptr.hpp>

namespace ICS {  class InputControlSystem;  class DetectingBindingListener;  }
namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class Window;  }
class MasterClient;  class P2PGameClient;
class CarModel;  class SETTINGS;  class GraphicsSystem;


//  main, race menus
enum Menu_Btns {  Menu_Setup, Menu_Replays, Menu_Help, Menu_Options,  ciMainBtns };
enum Setup_Btns {  Setup_Games,  Setup_HowToPlay,  Setup_Difficulty, Setup_Simulation,  Setup_Back, ciSetupBtns };
enum Games_Btns {  Games_Single, Games_SplitScreen, Games_Multiplayer, 
					Games_Tutorial, Games_Champ, Games_Challenge, Games_Collection, /*Games_Career,*/  Games_Back, ciGamesBtns };
//  gui
enum TAB_Game    {  TAB_Back=0, TAB_Track,TAB_Car, TAB_Setup, TAB_Split,TAB_Multi, TAB_Champs,TAB_Stages,TAB_Stage  };
enum TAB_Options {  TABo_Back=0, TABo_Screen, TABo_Input, TABo_View, TABo_Graphics, TABo_Sound, TABo_Settings, TABo_Tweak  };
enum LobbyState  {  DISCONNECTED, HOSTING, JOINED  };



class BaseApp : public AppGui, public GameState
	// public Ogre::FrameListener, public SFO::WindowListener
{
public:
	BaseApp();
	virtual ~BaseApp();
	// virtual void Run(bool showDialog);


	bool bLoading =0, bLoadingEnd =0, bSimulating =0;  int iLoad1stFrames =0;
	
	//  🚗 Cars / vehicles
	//  is in BaseApp for camera mouse move
	typedef std::vector<CarModel*> CarModels;
	CarModels carModels;
	
	void showMouse(), hideMouse(), updMouse();
	
	//  wnd, hud, upl
	bool bWindowResized =1, bSizeHUD =1, bRecreateHUD =0;
	// float roadUpdTm =0.f;
	

	// bool mShowDialog =0, mShutDown =0;
	// bool setup(), configure();;

	// void setupResources(), createResourceListener(), loadResources();
	void LoadingOn(), LoadingOff();

	///  frame events
	// void onCursorChange(const std::string& name);


	// SDL_Window* mSDLWindow =0;

	// virtual void windowResized (int x, int y);
	// virtual void windowClosed();


	///  🕹️ Input
	//------------------------------------------------------------
	ICS::InputControlSystem* mInputCtrl =0;
	ICS::InputControlSystem* mInputCtrlPlayer[MAX_Players] ={0,};
	std::vector<SDL_Joystick*> mJoysticks;
	
	// this is set to true when the user is asked to assign a new key
	bool bAssignKey =0;
	ICS::DetectingBindingListener* mInputBindListner =0;

	///  input  vars
	bool alt =0, ctrl =0, shift =0;  // key modifiers  for FollowCamera move
	bool mbLeft =0, mbRight =0, mbMiddle =0;  // mouse buttons
	int iCurCam = 0;  // move
	void CreateInputs();

	///  ⚡ input events  ------------
	void mouseMoved( const SDL_Event &arg ) override;
	void mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;
	void mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;

	// void textEditing( const SDL_TextEditingEvent& arg ) override;
	void textInput( const SDL_TextInputEvent& arg ) override;

	// virtual void keyPressed( const SDL_KeyboardEvent &arg ) override;  // in App
	// virtual void keyReleased(const SDL_KeyboardEvent &arg ) override;

	void joyButtonPressed( const SDL_JoyButtonEvent &evt, int button ) override;
	void joyButtonReleased( const SDL_JoyButtonEvent &evt, int button ) override;
	void joyAxisMoved( const SDL_JoyAxisEvent &arg, int axis ) override;
	// void joyPovMoved( const SDL_JoyHatEvent &arg, int index ) override;


	///  🎛️ Gui  ..........................
	bool isFocGui = 0, isFocRpl = 0;  // gui shown
	bool IsFocGuiInput()  {  return isFocGui || isFocRpl;  }
	bool IsFocGui();
	bool isTweak();
	
	void baseInitGui(), baseSizeGui();

	//  ⏳ Loading  backgr, bar
	Img imgBack =0, bckLoad =0, bckLoadBar =0, barLoad =0, imgLoad =0;
	Txt txLoadBig =0, txLoad =0;
	int barSizeX =0, barSizeY =0, barHeight =0;
	void SetLoadingBar(float pecent);


	///  🪧 main menu  // pSet->inMenu  3 levels
	Wnd mWMainMenu =0, mWMainSetup =0, mWMainGames =0;  WP mWndGameInfo =0;
	Img mMainPanels[ciMainBtns] ={0,}, mMainSetupPanels[ciSetupBtns] ={0,}, mMainGamesPanels[ciGamesBtns] ={0,};
	Btn mMainBtns  [ciMainBtns] ={0,}, mMainSetupBtns  [ciSetupBtns] ={0,}, mMainGamesBtns  [ciGamesBtns] ={0,};
	void updPanGames(int yToSet=-1);

	///  wnds  ----
	Wnd mWndGame =0, mWndReplays =0,
		mWndHelp =0, mWndOpts =0,  // common in game & ed
		mWndMaterials =0, mWndTrkFilt =0,  // common 🛠️ tools
		
		mWndWelcome =0, mWndHowTo =0,
		mWndRpl =0, mWndRplTxt =0,  // 📽️ rpl controls
		mWndNetEnd =0, mWndTweak =0,  // 📡 netw
		mWndChampStage =0, mWndChampEnd =0,  // 🏆champ
		mWndChallStage =0, mWndChallEnd =0;  // 🥇chall

	Tab mTabsGame =0, mTabsRpl =0,  // main tabs on windows
		mTabsHelp =0, mTabsOpts =0, mTabsMat =0;  // common tabs
	
	//MyGUI::VectorWidgetPtr
	std::vector<WP> vwGui;  // all widgets to destroy


	///  👥 multiplayer  networking
	boost::scoped_ptr<MasterClient> mMasterClient;
	boost::scoped_ptr<P2PGameClient> mClient;
	LobbyState mLobbyState = DISCONNECTED;
};
