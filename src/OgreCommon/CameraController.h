#pragma once
#include "OgrePrerequisites.h"
#include "TutorialGameState.h"


class CameraController
{
	bool        mUseSceneNode;
	bool        mSpeed1;
	bool        mSpeed2;
	bool        mWASDQE[6];
	float       mCameraYaw;
	float       mCameraPitch;
public:
	float       mCameraBaseSpeed;
	float       mCameraSpeed1;
	float       mCameraSpeed2;

private:
	GraphicsSystem      *mGraphicsSystem;

public:
	CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode=false );

	void update( float timeSinceLast );

	/// Returns true if we've handled the event
	bool keyPressed( const SDL_KeyboardEvent &arg );
	/// Returns true if we've handled the event
	bool keyReleased( const SDL_KeyboardEvent &arg );

	void mouseMoved( const SDL_Event &arg );
};
