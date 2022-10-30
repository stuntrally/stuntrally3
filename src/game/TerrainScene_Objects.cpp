#include "OgreCommon.h"
#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "OgreLogManager.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreCamera.h"

#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"
#include "OgreGpuProgramManager.h"

#include "OgreItem.h"
#include "OgreParticleSystem.h"

#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreManualObject2.h"

#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"

#include "Def_Str.h"
using namespace Demo;
using namespace Ogre;


namespace Demo
{

	//  Particles
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::CreateParticles()
	{
		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
		LogO("---- new Particles");

		Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
		Vector3 dir = mGraphicsSystem->getCamera()->getDirection();
		camPos += dir * 40.f;

		for (int i=0; i < 2; ++i)  // 20
		{
			ParticleSystem* parSys = mgr->createParticleSystem(
				i%2 ? "Smoke2" : "Fire2");
			//parHit->setVisibilityFlags(RV_Particles);
			SceneNode* node = rootNode->createChildSceneNode();
			node->attachObject( parSys );
			parSys->setRenderQueueGroup( 225 );  //? after Veget

			Vector3 objPos = camPos + Vector3( i/2 * 2.f, -5.f + i%2 * 4.f, 0.f);
			// if (mTerra)
			//     objPos.y += mTerra->getHeightAt( objPos ) + 5.f;
			node->setPosition( objPos );
			//parHit->getEmitter(0)->setEmissionRate(20);
		}
	}


	//  Car
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::CreateCar()
	{
		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_DYNAMIC );

		Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();
		Vector3 dir = mGraphicsSystem->getCamera()->getDirection();
		camPos += dir * 40.f;

		const String cars[nCars] = {"SX", "HI"}; //{ "ES", "SX", "HI" };
		const String car = cars[iCar];
		++iCar;
		if (iCar == nCars)  iCar = 0;  // next

		const int carParts = 3;
		const String carPart[carParts] = {"_body.mesh", "_interior.mesh", "_glass.mesh"}, sWheel = "_wheel.mesh";

		//  scale
		Real s = 1.f;

		//  pos
		Vector3 objPos = camPos, pos = objPos;
		if (mTerra)
			mTerra->getHeightAt( pos );
		Real ymin = !mTerra ? 0.85f * s : pos.y + 0.68f * s;
		// if (objPos.y < ymin)
			objPos.y = ymin;

		//  car  ------------------------
		// ndCar = rootNode->createChildSceneNode( SCENE_DYNAMIC );
		for (int i=0; i < carParts; ++i)
		{
			Item *item = mgr->createItem( car + carPart[i],
				ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_DYNAMIC );
			item->setVisibilityFlags(RV_Car);

			// SceneNode *node = ndCar->createChildSceneNode( SCENE_DYNAMIC );
			SceneNode *node = rootNode->createChildSceneNode( SCENE_DYNAMIC );
			node->attachObject( item );
			if (i==2)
				item->setRenderQueueGroup( 202 );  // glass after Veget
			
			//node->scale( s, s, s );
			
			node->setPosition( objPos );
			ndCar[i] = node;
			
			//  rot
			Quaternion q;  q.FromAngleAxis( Degree(180), Vector3::UNIT_Z );
			node->setOrientation( q );

			//  set reflection cube
			assert( dynamic_cast<Ogre::HlmsPbsDatablock *>( item->getSubItem( 0 )->getDatablock() ) );
			Ogre::HlmsPbsDatablock *datablock =
				static_cast<Ogre::HlmsPbsDatablock *>( item->getSubItem( 0 )->getDatablock() );
			datablock->setTexture( Ogre::PBSM_REFLECTION, mDynamicCubemap );

			if (mCubeCamera)
				mCubeCamera->setPosition(objPos);
		}

		//  wheels  ------------------------
		for (int i=0; i < 4; ++i)
		{
			Item *item = mgr->createItem( car + sWheel,
				ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_DYNAMIC );
			item->setVisibilityFlags(RV_Car);

			SceneNode *node = rootNode->createChildSceneNode( SCENE_DYNAMIC );
			node->attachObject( item );
			
			node->scale( s, s, s );
			
			if (car == "SX")
				node->setPosition( objPos + s * Vector3(
					i/2 ? -1.29f : 1.28f,  -0.34f,
					i%2 ? -0.8f : 0.8f ) );
			else if (car == "HI")
				node->setPosition( objPos + s * Vector3(
					i/2 ? -1.29f : 1.30f,  -0.34f,   // front+
					i%2 ? -0.88f : 0.88f ) );
			if (car == "ES")
				node->setPosition( objPos + s * Vector3(
					i/2 ? -1.21f : 1.44f,  -0.36f,
					i%2 ? -0.71f : 0.71f ) );
			
			//  rot
			Quaternion q;  q.FromAngleAxis( Degree(-180), Vector3::UNIT_Z );
			Quaternion r;  r.FromAngleAxis( Degree(i%2 ? 90 : -90), Vector3::UNIT_Y );
			node->setOrientation( r * q );
			ndWheel[i] = node;

			//  set reflection cube
			assert( dynamic_cast<Ogre::HlmsPbsDatablock *>( item->getSubItem( 0 )->getDatablock() ) );
			Ogre::HlmsPbsDatablock *datablock =
				static_cast<Ogre::HlmsPbsDatablock *>( item->getSubItem( 0 )->getDatablock() );
			datablock->setTexture( Ogre::PBSM_REFLECTION, mDynamicCubemap );
		}
	}


	//  Manual object
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::CreateManualObj(Ogre::Vector3 camPos)
	{
		SceneManager *mgr = mGraphicsSystem->getSceneManager();

		LogO("---- new Manual object");
		ManualObject *m;
		std::vector<Vector3> mVertices;

		m = mgr->createManualObject();
		m->begin("jungle_tree", OT_TRIANGLE_LIST);
		//m->begin("ParSmoke", OT_TRIANGLE_LIST);

		//m->beginUpdate(0);
		const size_t GridSize = 15;
		const float GridStep = 1.0f / GridSize;

		for (size_t i = 0; i < GridSize; i++)
		{
			for (size_t j = 0; j < GridSize; j++)
			{
				mVertices.push_back(Vector3(GridStep * i,       GridStep * j,       0.00f));
				mVertices.push_back(Vector3(GridStep * (i + 1), GridStep * j,       0.00f));
				mVertices.push_back(Vector3(GridStep * i,       GridStep * (j + 1), 0.00f * j));
				mVertices.push_back(Vector3(GridStep * (i + 1), GridStep * (j + 1), 0.00f * i));
			}
		}

		float uvOffset = 0.f;
		{
			for (size_t i = 0; i < mVertices.size(); )
			{
				m->position(mVertices[i]);
				m->normal(0.0f, 1.0f, 0.0f);
				m->tangent(1.0f, 0.0f, 0.0f);
				m->textureCoord(0.0f + uvOffset, 0.0f + uvOffset);

				m->position(mVertices[i + 1]);
				m->normal(0.0f, 1.0f, 0.0f);
				m->tangent(1.0f, 0.0f, 0.0f);
				m->textureCoord(1.0f + uvOffset, 0.0f + uvOffset);

				m->position(mVertices[i + 2]);
				m->normal(0.0f, 1.0f, 0.0f);
				m->tangent(1.0f, 0.0f, 0.0f);
				m->textureCoord(0.0f + uvOffset, 1.0f + uvOffset);

				m->position(mVertices[i + 3]);
				m->normal(0.0f, 1.0f, 0.0f);
				m->tangent(1.0f, 0.0f, 0.0f);
				m->textureCoord(1.0f + uvOffset, 1.0f + uvOffset);

				m->quad(i, i + 1, i + 3, i + 2);
				i += 4;
			}
		}
		m->end();

		SceneNode *sceneNodeGrid = mgr->getRootSceneNode( SCENE_DYNAMIC )->createChildSceneNode( SCENE_DYNAMIC );
		sceneNodeGrid->attachObject(m);
		sceneNodeGrid->scale(4.0f, 4.0f, 4.0f);
		Vector3 objPos = camPos + Vector3( 0.f, -5.f, -10.f);
		sceneNodeGrid->translate(objPos, SceneNode::TS_LOCAL);
	}

}
