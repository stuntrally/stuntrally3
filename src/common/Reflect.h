#pragma once
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <OgrePlanarReflections.h>

namespace Ogre  {
	// class Terra;  class Atmosphere2Npr;  class HlmsPbsTerraShadows;
	class Light;  class SceneNode;  class Camera;  class SceneManager;  }
class App;


//  🪞 planar reflect  ws listener
class ReflectListener : public Ogre::CompositorWorkspaceListener
{
	App* app =0;
	Ogre::PlanarReflections *mPlanarRefl;

public:
	ReflectListener( App* app1, Ogre::PlanarReflections *p )
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
	App* app =0;
	class btDiscreteDynamicsWorld* world =0;

	//  🟢 meshes etc per fluid
	std::vector<Ogre::String/*MeshPtr*/> vsMesh, vsMesh2;
	std::vector<Ogre::Item*> vIt;
	std::vector<Ogre::SceneNode*> vNd;
	std::vector<Ogre::PlanarReflectionActor*> vActors;
	std::vector<Ogre::PlanarReflections::TrackedRenderable*> vTracked;

	void CreateFluids(), DestroyFluids(), CreateBltFluids();

	//  🪞 RTT, setup
	Ogre::PlanarReflections* mPlanarRefl =0;  // one system for all fluids
	ReflectListener *mWsListener =0;
	
	void CreateRTT(), DestroyRTT();
};
