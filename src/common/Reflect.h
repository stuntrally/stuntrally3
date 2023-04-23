#pragma once
#include <Compositor/OgreCompositorWorkspaceListener.h>

namespace Ogre  {
	class PlanarReflections;  class PlanarReflectionActor;
	// class Terra;  class Atmosphere2Npr;  class HlmsPbsTerraShadows;
	class Light;  class SceneNode;  class Camera;  class SceneManager;  }
class AppGui;


//  🪞 planar reflect  water
class ReflectListener : public Ogre::CompositorWorkspaceListener
{
	Ogre::PlanarReflections *mPlanarReflections;

public:
	ReflectListener( Ogre::PlanarReflections *p ) :
		mPlanarReflections( p )
	{
	}
	virtual ~ReflectListener() {}

	virtual void workspacePreUpdate( Ogre::CompositorWorkspace *workspace );

	virtual void passEarlyPreExecute( Ogre::CompositorPass *pass );
};


//  🪞 planar reflect  water/mud etc
class FluidReflect
{
public:
	AppGui* app =0;
	Ogre::Item *item =0;
	Ogre::SceneNode *nd =0;
	Ogre::String sMesh;
	Ogre::PlanarReflectionActor *actor =0;

	Ogre::PlanarReflections *mPlanarRefl =0;
	ReflectListener *mWorkspaceListener =0;

	void CreateReflect(), DestroyReflect();
};

