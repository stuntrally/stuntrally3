#pragma once

class App;
namespace Ogre
{	class SceneManager;  class SceneNode;  class Item;  class Camera;
	class CompositorWorkspace;
}

//  separate 3d scene
//  in ed for object preview
//  in game could be for garage, vehicle

class PreviewScene
{
public:
	App* app = 0;

	bool Create(App* app1);
	void Destroy();

	bool Load(Ogre::String mesh);
	void Unload();

	Ogre::Vector3 dim;  // after load
	Ogre::SceneManager* mgr = 0;
	Ogre::Camera* cam = 0;
protected:
	Ogre::Light* sun = 0;
	Ogre::SceneNode *ndSun = 0;

	Ogre::SceneNode* node = 0;
	Ogre::Item* item = 0;
	Ogre::CompositorWorkspace* ws = 0;
};
