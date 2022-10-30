
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

using namespace Demo;

namespace Demo
{

	CameraController::CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode ) :
		mUseSceneNode( useSceneNode ),
		mSpeed1( false ),
		mSpeed2( false ),
		mCameraYaw( 0 ),
		mCameraPitch( 0 ),
		mCameraBaseSpeed( 90 ),
		mCameraSpeed1( 0.1 ),
		mCameraSpeed2( 10 ),
		mGraphicsSystem( graphicsSystem )
	{
		memset( mWASDQE, 0, sizeof(mWASDQE) );
	}

	//-----------------------------------------------------------------------------------
	void CameraController::update( float timeSinceLast )
	{
		Ogre::Camera *camera = mGraphicsSystem->getCamera();

		if( mCameraYaw != 0.0f || mCameraPitch != 0.0f )
		{
			if( mUseSceneNode )
			{
				Ogre::Node *cameraNode = camera->getParentNode();

				// Update now as yaw needs the derived orientation.
				cameraNode->_getFullTransformUpdated();
				cameraNode->yaw( Ogre::Radian( mCameraYaw ), Ogre::Node::TS_WORLD );
				cameraNode->pitch( Ogre::Radian( mCameraPitch ) );
			}else
			{
				camera->yaw( Ogre::Radian( mCameraYaw ) );
				camera->pitch( Ogre::Radian( mCameraPitch ) );
			}
			mCameraYaw   = 0.0f;
			mCameraPitch = 0.0f;
		}

		int camMovementZ = mWASDQE[2] - mWASDQE[0];
		int camMovementX = mWASDQE[3] - mWASDQE[1];
		int camMovementY = mWASDQE[5] - mWASDQE[4];

		if( camMovementZ || camMovementX || camMovementY )
		{
			Ogre::Vector3 camMovementDir( camMovementX, camMovementY, camMovementZ );
			camMovementDir.normalise();
			camMovementDir *= timeSinceLast * mCameraBaseSpeed
				* (mSpeed1 ? mCameraSpeed1 : mSpeed2 ? mCameraSpeed2 : 1.f);

			if( mUseSceneNode )
			{
				Ogre::Node *cameraNode = camera->getParentNode();
				cameraNode->translate( camMovementDir, Ogre::Node::TS_LOCAL );
			}
			else
				camera->moveRelative( camMovementDir );
		}
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
	//-----------------------------------------------------------------------------------
	bool CameraController::keyPressed( const SDL_KeyboardEvent &arg )
	{
		switch (arg.keysym.scancode)
		{
		case SDL_SCANCODE_LSHIFT:  mSpeed1 = true;  return true;
		case SDL_SCANCODE_LCTRL:   mSpeed2 = true;  return true;

		case SDL_SCANCODE_W:  mWASDQE[0] = true;  return true;
		case SDL_SCANCODE_A:  mWASDQE[1] = true;  return true;
		case SDL_SCANCODE_S:  mWASDQE[2] = true;  return true;
		case SDL_SCANCODE_D:  mWASDQE[3] = true;  return true;
		case SDL_SCANCODE_Q:  mWASDQE[4] = true;  return true;
		case SDL_SCANCODE_E:  mWASDQE[5] = true;  return true;
		}    
		return false;
	}

	//-----------------------------------------------------------------------------------
	bool CameraController::keyReleased( const SDL_KeyboardEvent &arg )
	{
		switch (arg.keysym.scancode)
		{
		case SDL_SCANCODE_LSHIFT:  mSpeed1 = false;  return true;
		case SDL_SCANCODE_LCTRL:   mSpeed2 = false;  return true;

		case SDL_SCANCODE_W:  mWASDQE[0] = false;  return true;
		case SDL_SCANCODE_A:  mWASDQE[1] = false;  return true;
		case SDL_SCANCODE_S:  mWASDQE[2] = false;  return true;
		case SDL_SCANCODE_D:  mWASDQE[3] = false;  return true;
		case SDL_SCANCODE_Q:  mWASDQE[4] = false;  return true;
		case SDL_SCANCODE_E:  mWASDQE[5] = false;  return true;
		}    
		return false;
	}
#pragma GCC diagnostic pop

	//-----------------------------------------------------------------------------------
	void CameraController::mouseMoved( const SDL_Event &arg )
	{
		float width  = static_cast<float>( mGraphicsSystem->getRenderWindow()->getWidth() );
		float height = static_cast<float>( mGraphicsSystem->getRenderWindow()->getHeight() );

		mCameraYaw   += -arg.motion.xrel / width  *6;
		mCameraPitch += -arg.motion.yrel / height *6;
	}
}
