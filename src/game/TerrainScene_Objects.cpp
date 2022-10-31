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
