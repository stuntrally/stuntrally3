#pragma once
#include <OgreString.h>

namespace Ogre {  class SceneManager;  class Camera;  }


//  🎥 camera, grouped
struct Cam
{
	Ogre::SceneNode* nd = 0;
	Ogre::Camera* cam = 0;
	Ogre::String name;
};
