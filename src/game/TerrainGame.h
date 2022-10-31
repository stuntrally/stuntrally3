#ifndef _Demo_TerrainGame_H_
#define _Demo_TerrainGame_H_

#include "OgreHlmsDatablock.h"
#include "OgrePrerequisites.h"
#include "TutorialGameState.h"


class SETTINGS;
class GAME;
class App;
class Scene;

namespace Ogre
{
	class Terra;
	class HlmsPbsTerraShadows;
}

namespace Demo
{

	struct VegetLayer
	{
		std::string mesh;
		float scaleMin, scaleMax, density;
		float visFar, down;  // visibility max, down offset
		bool rotAll;  // allow all axes rotation
		int count;  // auto
		// range ter angle, height..
		
		VegetLayer(std::string mesh1, float scMin, float scMax,
				float dens, float dn, float vis, bool rot)
			: mesh(mesh1), scaleMin(scMin), scaleMax(scMax)
			, density(dens), visFar(vis), down(dn), rotAll(rot), count(0)
		{   }
	};

	enum IblQuality  // reflections
	{
		MipmapsLowest,
		IblLow,
		IblHigh
	};


	class TerrainGame : public TutorialGameState
	{
		//  vars
		float mPitch;  // sun dir
		float mYaw;
		Ogre::Vector3 camPos;

		//  input
		int mArrows[11] = {0,0,0,0,0,0,0,0,0,0},
			mKeys[4] = {0,0,0,0};  // sun keys
		int param = 0;  // to adjust
		bool left = false, right = false;  // arrows
		bool shift = false, ctrl = false;

		//  terrain
		Ogre::Terra *mTerra = 0;
		Ogre::Light *mSunLight = 0;

		// Listener to make PBS objects also be affected by terrain's shadows
		Ogre::HlmsPbsTerraShadows *mHlmsPbsTerraShadows = 0;

		//  wireframe
		Ogre::HlmsMacroblock macroblockWire;
		bool wireTerrain = false;

		void generateDebugText();
		Ogre::String generateFpsDebugText();

	public:
		TerrainGame();

		//  SR
		SETTINGS* pSet = 0;
		GAME* pGame = 0;
		App* pApp = 0;
		
		Scene* sc = 0;  // in pApp->scn ..

		void LoadDefaultSet(SETTINGS* settings, std::string setFile);
		void Init();
		void Destroy();


		//  main
		void createScene01() override;
		void destroyScene() override;

		void update( float timeSinceLast ) override;

		//  events
		void keyPressed( const SDL_KeyboardEvent &arg ) override;
		void keyReleased( const SDL_KeyboardEvent &arg ) override;
	protected:

		//  reflection cube  ----
		Ogre::Camera *mCubeCamera = 0;
		Ogre::TextureGpu *mDynamicCubemap = 0;
		Ogre::CompositorWorkspace *mDynamicCubemapWorkspace = 0;

		IblQuality mIblQuality = IblLow;  // par in ctor
		Ogre::CompositorWorkspace *setupCompositor();


		//  terrain  ----
		Ogre::Real sizeXZ = 1000.f;
	public:		
		void CreateTerrain(), DestroyTerrain();
	protected:
		Ogre::SceneNode *nodeTerrain = 0;

		void CreatePlane(), DestroyPlane();
		Ogre::MeshPtr planeMesh = 0;
		Ogre::Item *planeItem = 0;
		Ogre::SceneNode *planeNode = 0;

		//  sky
		void CreateSkyDome(Ogre::String sMater, float yaw);
		int iSky = 0;
		Ogre::ManualObject* moSky = 0;
		Ogre::SceneNode* ndSky = 0;
		void DestroySkyDome();
	
		//  vegetation  ----
		void SetupVeget(), CreateVeget(), DestroyVeget();

		std::vector<VegetLayer> vegetLayers;
		std::vector<Ogre::Item*> vegetItems;
		std::vector<Ogre::SceneNode*> vegetNodes;

		//  other
		void CreateManualObj(Ogre::Vector3 camPos);
		void CreateParticles();
	};
}

#endif
