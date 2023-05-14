#pragma once
#include <Compositor/OgreCompositorWorkspaceListener.h>

namespace Ogre  {
	class PlanarReflections;  class PlanarReflectionActor;
	// class Terra;  class Atmosphere2Npr;  class HlmsPbsTerraShadows;
	class Light;  class SceneNode;  class Camera;  class SceneManager;  }
class AppGui;


//  🪞 planar reflect  ws listener
class ReflectListener : public Ogre::CompositorWorkspaceListener
{
	AppGui* app =0;
	Ogre::PlanarReflections *mPlanarRefl;

public:
	ReflectListener( AppGui* app1, Ogre::PlanarReflections *p )
		:app(app1), mPlanarRefl( p )
	{	}
	virtual ~ReflectListener()
	{	}

	void workspacePreUpdate( Ogre::CompositorWorkspace *workspace ) override;
	void passEarlyPreExecute( Ogre::CompositorPass *pass ) override;
};


//  🌊 Fluids  water,mud etc
class FluidsReflect
{
public:
	AppGui* app =0;

	//  🟢 meshes etc
	// class FluidRefl
	// {
	Ogre::Item *item =0;
	Ogre::SceneNode *nd =0;
	Ogre::String sMesh;
	Ogre::PlanarReflectionActor *actor =0;
	// };
	// std::vector<FluidRefl> fluids;

	void CreateFluids(), DestroyFluids();

	//  🪞 RTT, setup
	Ogre::PlanarReflections *mPlanarRefl =0;
	ReflectListener *mWsListener =0;
	
	void CreateRTT(), DestroyRTT();
};
