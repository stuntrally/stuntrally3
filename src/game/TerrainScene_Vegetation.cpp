#include "TerrainGame.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

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

    //  Vegetation setup
    void TerrainGame::SetupVeget()
    {
        vegetLayers.clear();   //  OgreMeshTool args
        // sc min, max, dens, down
        vegetLayers.emplace_back(VegetLayer("jungle_tree-lod8.mesh",
            3.0f, 5.0f, 10.f, -0.1f, 5000, 0 ));  //  -v2 -l 10 -d 100 -p 11 jungle_tree.mesh
        vegetLayers.emplace_back(VegetLayer("palm2-lod8.mesh",
            7.5f,12.5f, 8.f,  -0.1f, 5000, 0 ));  //  -v2 -l 8 -d 200 -p 10 palm2.mesh

        vegetLayers.emplace_back(VegetLayer("plant_tropical-lod6.mesh",
            4.5f, 7.5f, 30.f, -0.1f, 1000, 0 ));  //  -v2 -l 6 -d 200 -p 15 plant_tropical.mesh
        vegetLayers.emplace_back(VegetLayer("fern-lod6.mesh",
            0.6f, 1.0f, 55.f, 0.0f, 600, 0 ));  //  -v2 -l 6 -d 200 -p 15 fern.mesh
        vegetLayers.emplace_back(VegetLayer("fern2-lod6.mesh",
            0.6f, 1.0f, 50.f, 0.0f, 600, 0 ));  //  -v2 -l 8 -d 200 -p 10 palm2.mesh

        vegetLayers.emplace_back(VegetLayer("rock02brown2flat.mesh",
            1.1f, 5.0f, 5.0f, 0.0f, 3000, 1 ));  //  -v2 -l 6 -d 200 -p 15 rock*.mesh
        vegetLayers.emplace_back(VegetLayer("rock25dark2Harsh2.mesh",
            0.6f, 3.0f, 5.0f, 0.0f, 3000, 1 ));
        vegetLayers.emplace_back(VegetLayer("rock30grayGreen.mesh",
            2.1f, 6.0f, 5.0f, 0.0f, 3000, 1 ));
        vegetLayers.emplace_back(VegetLayer("rock37brGr1tall.mesh",
            1.1f, 3.0f, 5.0f, 0.0f, 3000, 1 ));
        // vegetLayers.emplace_back(VegetLayer("rock18black3.mesh",
        //     1.6f,7.f, 5.f ));
        // vegetLayers.emplace_back(VegetLayer("rock_B02.mesh",
        //     0.5f,2.f, 5.f ));

        //vegetLayers.emplace_back(VegetLayer("pine2_tall_norm-lod9.mesh",
        //    2.5f,4.f, 10.f ));  //  -v2 -l 9 -d 100 -p 9 pine2_tall_norm.mesh
    }

    //  Veget
    //-----------------------------------------------------------------------------------------------------------------------------
    void TerrainGame::CreateVeget()
	{
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
        SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
    #if 0
        HlmsPbsDatablock *pbsdatablock = (HlmsPbsDatablock*)hlmsManager->getDatablock( "pine2norm" );
        pbsdatablock->setTwoSidedLighting( true );  //?
        //pbsdatablock->setMacroblock( macroblockWire );
    #endif

		const Real mult  = !mTerra ? 0.1f : 1.f;
        const Real scale = sizeXZ * 0.49f;  // world
		
        for (auto& lay : vegetLayers)
        {
            int cnt = mult * lay.density * 200.f;  // count, fake..
            for (int i=0; i < cnt; ++i)
            {
                ++lay.count;
				//  Item  ----
                Item *item;
                try 
				{   item = mgr->createItem( lay.mesh,
					    ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
					    SCENE_STATIC );
                }catch(...)
                {
                    LogO("error: mesh not found: " + lay.mesh);
                    return;
                }
                //item->setDatablock( "pine2norm" );
                item->setRenderQueueGroup( 200 );  // after terrain
                item->setRenderingDistance( lay.visFar );  // how far visible
				vegetItems.push_back(item);

				//  Node  ----
				SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
				node->attachObject( item );
                
                //  scale
                Real s = Math::RangeRandom(lay.scaleMin, lay.scaleMax) / 6.f;
				node->scale( s, s, s );
                
                //  pos
				Vector3 objPos = Vector3(
                    Math::RangeRandom(-scale, scale), 0.f,
                    Math::RangeRandom(-scale, scale));
                if (mTerra)
                    mTerra->getHeightAt( objPos );
                objPos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + lay.down;  //par
                objPos.y -= 11.f;  //par ..
                node->setPosition( objPos );
                
                //  rot
                Degree ang( Math::RangeRandom(0, 360.f) );
                Quaternion q;  q.FromAngleAxis( ang, Vector3::UNIT_Y );
                if (lay.rotAll)
                {
                    Degree ang( Math::RangeRandom(0, 180.f) );  //par ? range
                    Quaternion p;  p.FromAngleAxis( -ang, Vector3::UNIT_Z );
                    node->setOrientation( p * q );
                }else
                    node->setOrientation( q );

				vegetNodes.push_back(node);
			}
		}
	}

    //-----------------------------------------------------------------------------------
    void TerrainGame::DestroyVeget()
	{
        SceneManager *mgr = mGraphicsSystem->getSceneManager();
		
		for (auto node : vegetNodes)
			mgr->destroySceneNode(node);
		vegetNodes.clear();
		
		for (auto item : vegetItems)
			mgr->destroyItem(item);
		vegetItems.clear();
	}

}
