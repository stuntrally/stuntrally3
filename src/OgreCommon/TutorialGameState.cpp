#include "pch.h"
#include "TutorialGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"

#include "OgreOverlayManager.h"
#include "OgreOverlay.h"
#include "OgreOverlayContainer.h"
#include "OgreTextAreaOverlayElement.h"

#include "OgreRoot.h"
#include "OgreFrameStats.h"

#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreHlmsCompute.h"
#include "OgreGpuProgramManager.h"

#include <MyGUI_InputManager.h>


TutorialGameState::TutorialGameState() :
	mGraphicsSystem( 0 ),
	mCameraController( 0 ),
	mDisplayHelpMode( 1 ),
	mNumDisplayHelpModes( 2 ),
	mDebugText( 0 )
{
}
//-----------------------------------------------------------------------------------
TutorialGameState::~TutorialGameState()
{
	delete mCameraController;
	mCameraController = 0;
}
//-----------------------------------------------------------------------------------
void TutorialGameState::_notifyGraphicsSystem( GraphicsSystem *graphicsSystem )
{
	mGraphicsSystem = graphicsSystem;
}
//-----------------------------------------------------------------------------------
void TutorialGameState::createScene01()
{
	createDebugTextOverlay();
}
//-----------------------------------------------------------------------------------
void TutorialGameState::createDebugTextOverlay()
{
	Ogre::v1::OverlayManager &overlayManager = Ogre::v1::OverlayManager::getSingleton();
	Ogre::v1::Overlay *overlay = overlayManager.create( "DebugText" );

	Ogre::v1::OverlayContainer *panel = static_cast<Ogre::v1::OverlayContainer*>(
		overlayManager.createOverlayElement("Panel", "DebugPanel"));
	mDebugText = static_cast<Ogre::v1::TextAreaOverlayElement*>(
		overlayManager.createOverlayElement( "TextArea", "DebugText" ) );
	mDebugText->setFontName( "DebugFont" );
	mDebugText->setCharHeight( 0.022f );

	mDebugTextShadow = static_cast<Ogre::v1::TextAreaOverlayElement*>(
		overlayManager.createOverlayElement( "TextArea", "0DebugTextShadow" ) );
	mDebugTextShadow->setFontName( "DebugFont" );
	mDebugTextShadow->setCharHeight( 0.022f );
	mDebugTextShadow->setColour( Ogre::ColourValue::Black );
	mDebugTextShadow->setPosition( 0.001f, 0.001f );

	panel->addChild( mDebugTextShadow );
	panel->addChild( mDebugText );
	overlay->add2D( panel );
	overlay->show();
}
//-----------------------------------------------------------------------------------
void TutorialGameState::update( float timeSinceLast )
{
	if( mCameraController )
		mCameraController->update( timeSinceLast );
}
//-----------------------------------------------------------------------------------
void TutorialGameState::keyPressed( const SDL_KeyboardEvent &arg )
{
	bool handledEvent = false;

	if( mCameraController )
		handledEvent = mCameraController->keyPressed( arg );

	if( !handledEvent )
		GameState::keyPressed( arg );
}
//-----------------------------------------------------------------------------------
void TutorialGameState::keyReleased( const SDL_KeyboardEvent &arg )
{
	if( arg.keysym.scancode == SDL_SCANCODE_F1 && (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) == 0 )
	{
		mDisplayHelpMode = (mDisplayHelpMode + 1) % mNumDisplayHelpModes;
	}
	else if( arg.keysym.scancode == SDL_SCANCODE_F1 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
	{
		//Hot reload of PBS shaders. We need to clear the microcode cache
		//to prevent using old compiled versions.
		Ogre::Root *root = mGraphicsSystem->getRoot();
		Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

		Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_PBS );
		Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}
	else if( arg.keysym.scancode == SDL_SCANCODE_F2  && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
	{
		//Hot reload of Unlit shaders.
		Ogre::Root *root = mGraphicsSystem->getRoot();
		Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

		Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_UNLIT );
		Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}
	else if( arg.keysym.scancode == SDL_SCANCODE_F3 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
	{
		//Hot reload of Compute shaders.
		Ogre::Root *root = mGraphicsSystem->getRoot();
		Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

		Ogre::Hlms *hlms = hlmsManager->getComputeHlms();
		Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
		hlms->reloadFrom( hlms->getDataFolder() );
	}
	else if( arg.keysym.scancode == SDL_SCANCODE_F5 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
	{
		//Force device reelection
		Ogre::Root *root = mGraphicsSystem->getRoot();
		root->getRenderSystem()->validateDevice( true );
	}
	else
	{
		bool handledEvent = false;

		if( mCameraController )
			handledEvent = mCameraController->keyReleased( arg );

		if( !handledEvent )
			GameState::keyReleased( arg );
	}
}


//  input events
//-----------------------------------------------------------------------------------
MyGUI::MouseButton sdlButtonToMyGUI(Uint8 button)
{
	//  The right button is the second button, according to MyGUI
	if (button == SDL_BUTTON_RIGHT)  button = SDL_BUTTON_MIDDLE;
	else if (button == SDL_BUTTON_MIDDLE)  button = SDL_BUTTON_RIGHT;
	//  MyGUI's buttons are 0 indexed
	return MyGUI::MouseButton::Enum(button - 1);
}

void TutorialGameState::mouseMoved( const SDL_Event &arg )
{
	if( mCameraController )
		mCameraController->mouseMoved( arg );

// if (bGuiFocus)
	MyGUI::InputManager::getInstance().injectMouseMove(
		arg.motion.x, arg.motion.y, arg.wheel.y);

	GameState::mouseMoved( arg );
}

void TutorialGameState::mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id )
{
	MyGUI::InputManager::getInstance().injectMousePress(
		arg.x, arg.y, sdlButtonToMyGUI(arg.button));
}
void TutorialGameState::mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id )
{
	MyGUI::InputManager::getInstance().injectMouseRelease(
		arg.x, arg.y, sdlButtonToMyGUI(arg.button));
}
