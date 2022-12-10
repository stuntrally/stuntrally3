#pragma once
#include "OgrePrerequisites.h"
#include "GameState.h"

namespace Ogre  {  namespace v1  {  class TextAreaOverlayElement;  }  }

class GraphicsSystem;
class CameraController;


//  Base game state for the tutorials with text and input
class TutorialGameState : public GameState
{
public:
	GraphicsSystem      *mGraphicsSystem;
protected:

	CameraController    *mCameraController;

	Ogre::uint16        mDisplayHelpMode;
	Ogre::uint16        mNumDisplayHelpModes;

	Ogre::v1::TextAreaOverlayElement *mDebugText;
	Ogre::v1::TextAreaOverlayElement *mDebugTextShadow;

	virtual void createDebugTextOverlay();

public:
	TutorialGameState();
	virtual ~TutorialGameState();

	void _notifyGraphicsSystem( GraphicsSystem *graphicsSystem );

	void createScene01() override;

	void update( float timeSinceLast ) override;

	void keyPressed( const SDL_KeyboardEvent &arg ) override;
	void keyReleased( const SDL_KeyboardEvent &arg ) override;

	void mouseMoved( const SDL_Event &arg ) override;
	void mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;
	void mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) override;
};
