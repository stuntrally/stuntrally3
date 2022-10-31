#include "TerrainGame.h"
#include "OgrePrerequisites.h"
#include "OgreVector4.h"
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
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreManualObject2.h"

#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsTerraDatablock.h"
#include "OgreTextureGpuManager.h"

#include "OgreGpuResource.h"
#include "OgreHlmsTerraPrerequisites.h"
// #include "OgreHlmsTerra.h"

//  SR
#include "CGame.h"
#include "CScene.h"
#include "SceneXml.h"
#include "SceneClasses.h"
#include "game.h"
#include "settings.h"
#include "ShapeData.h"
#include "SceneXml.h"

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include "Def_Str.h"
#include "RenderConst.h"
using namespace Demo;
using namespace Ogre;


namespace Demo
{

	//  Terrain
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::CreateTerrain()
	{
		if (mTerra) return;
		Root *root = mGraphicsSystem->getRoot();
		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
		
		HlmsManager *hlmsManager = root->getHlmsManager();
		HlmsDatablock *datablock = 0;

		LogO("---- new Terra json mat");

	#if 1
		std::string datablockName = "TerraExampleMaterial";  // from .json
		// mGraphicsSystem->hlmsTerra->loadMaterial( "FileName for logging purposes", defaultResourceGroupForLoadingTextures, jsonString, "" );
	#else
		std::string datablockName = "TerraExampleMaterial2";
		// mGraphicsSystem->hlmsTerra->setDetailTextureProperty(
		datablock = mGraphicsSystem->hlmsTerra->createDatablock(
			datablockName.c_str(), datablockName.c_str(),
			HlmsMacroblock(),
			HlmsBlendblock(),
			HlmsParamVec() );
		assert( dynamic_cast<HlmsTerraDatablock *>( datablock ) );
		// datablock->set
		HlmsTerraDatablock *tblock = static_cast<HlmsTerraDatablock *>( datablock );
		TextureGpuManager *texMgr = root->getRenderSystem()->getTextureGpuManager();

		///  Layer Textures  ----
		int ls = sc->td.layers.size();
		for (int i=0; i < ls; ++i)
		{
			TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
			// di.layerList[i].worldSize = l.tiling;

			//  combined rgb,a from 2 tex
			String path = PATHMANAGER::Data() + "/terrain/";
			String d_d, d_s, n_n, n_h;
			
			///  diff
			d_d = l.texFile;  // ends with _d
			d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");
			d_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");
			n_n = l.texNorm;  // ends with _n
			n_h = StringUtil::replaceAll(l.texNorm,"_n.","_h.");


			auto tex = texMgr->createOrRetrieveTexture(d_d,
				GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
			/*if (tex)
			{	tex->isTextureGpu();
				tex->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
				tex->scheduleTransitionTo( GpuResidency::Resident );
			}*/
			tblock->setTexture( TERRA_DETAIL0 + i, tex );
			// tblock->setSamplerblock(uint8 texType, const HlmsSamplerblock &params)

			tex = texMgr->createOrRetrieveTexture(n_n,
				GpuPageOutStrategy::Discard, CommonTextureTypes::NormalMap, "General" );
			if (tex)
				tblock->setTexture( TERRA_DETAIL0_NM + i, tex );

			/*tex = texMgr->createOrRetrieveTexture(n_h,
				GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
			if (tex)
				tblock->setTexture( TERRA_DETAIL_ROUGHNESS0 + i, tex );

			tex = texMgr->createOrRetrieveTexture(d_s,
				GpuPageOutStrategy::Discard, CommonTextureTypes::Diffuse, "General" );
			if (tex)
				tblock->setTexture( TERRA_DETAIL_METALNESS0 + i, tex );*/

			tblock->setDetailMapOffsetScale( i, Vector4(0,0, l.tiling, l.tiling) );
			tblock->setMetalness(i, 0.1);
			tblock->setRoughness(i, 0.6);
		}
		//tblock->setDiffuse(Vector3());
		// const HlmsSamplerblock *sam = tblock->getSamplerblock( TERRA_DETAIL0 );
		// sam->setTexture()

		// tblock->setDetailTextureProperty();

	#endif

		LogO("---- new Terra");

		mTerra = new Terra( Id::generateNewId<MovableObject>(),
							&mgr->_getEntityMemoryManager( SCENE_STATIC ),
							mgr, 11u, root->getCompositorManager2(),
							mGraphicsSystem->getCamera(), false );
		mTerra->setCastShadows( false );
		pApp->scn->terrain = mTerra;

		LogO("---- Terra load");

		//  Heightmap  ------------------------------------------------
		sizeXZ = sc->td.fTriangleSize * (sc->td.iVertsX-1);  //sc->td.fTerWorldSize;
		float ofs = sc->td.fTriangleSize * -0.5f;  // todo: ofs 1025 to 1024 verts
		LogO("Ter size: " + toStr(sc->td.iVertsX));// +" "+ toStr((sc->td.iVertsX)*sizeof(float))

		mTerra->load(
			sc->td.iVertsX-1, sc->td.iVertsY-1, 
			sc->td.hfHeight, sc->td.iVertsX,
			Vector3( ofs*0.f, 0.45f, -ofs),  //** y?
			Vector3( sizeXZ, 1.0f, sizeXZ),
			// true, true);
			false, false);

		SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
		node->attachObject( mTerra );

		LogO("---- Terra attach");

		datablock = hlmsManager->getDatablock( datablockName );
		mTerra->setDatablock( datablock );

		mHlmsPbsTerraShadows = new HlmsPbsTerraShadows();
		mHlmsPbsTerraShadows->setTerra( mTerra );
		//Set the PBS listener so regular objects also receive terrain shadows
		Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );
		hlmsPbs->setListener( mHlmsPbsTerraShadows );
	}

	void TerrainGame::DestroyTerrain()
	{
		LogO("---- destroy Terrain");
		Root *root = mGraphicsSystem->getRoot();
		Hlms *hlmsPbs = root->getHlmsManager()->getHlms( HLMS_PBS );

		if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
		{
			hlmsPbs->setListener( 0 );
			delete mHlmsPbsTerraShadows;  mHlmsPbsTerraShadows = 0;
		}
		delete mTerra;  mTerra = 0;
	}


	//  Plane
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::CreatePlane()
	{
		return;  //-
	#if 0
		sizeXZ = 1000.0f;
		v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
			"Plane v1", rgDef,
			Plane( Vector3::UNIT_Y, 1.0f ), sizeXZ, sizeXZ,
			20, 20, true, 1, 160.f, 160.f, Vector3::UNIT_Z,
			v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

		planeMesh = MeshManager::getSingleton().createByImportingV1(
			"Plane", rgDef,
			planeMeshV1.get(), true, true, true );
		
		planeMeshV1->unload();

		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

		planeItem = mgr->createItem( planeMesh, SCENE_STATIC );
		planeItem->setDatablock( "Ground" );
		
		planeNode = rootNode->createChildSceneNode( SCENE_STATIC );
		planeNode->setPosition( 0, -12.2f, 0 );
		planeNode->attachObject( planeItem );
	#else
	// todo ... test center and borders 
	const int num = 2;  // pos norm up
	Vector3 pos[num][3] = {
		{Vector3(0,0,0), Vector3(1,0,0), Vector3(0,1,0) },
		{Vector3(0,0,0), Vector3(-1,0,0), Vector3(0,-1,0) },
	};
	for (int i=0; i < num; ++i)
	{
		sizeXZ = sc->td.fTriangleSize * (sc->td.iVertsX-1);
		v1::MeshPtr planeMeshV1 = v1::MeshManager::getSingleton().createPlane(
			"Plane v1-"+toStr(i), rgDef,
			Plane( pos[i][1], 1.0f ), sizeXZ, sizeXZ,
			10, 10, true, 1, 160.f, 160.f, pos[i][2],
			v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );

		planeMesh = MeshManager::getSingleton().createByImportingV1(
			"Plane-"+toStr(i), rgDef,
			planeMeshV1.get(), true, true, true );
		
		planeMeshV1->unload();

		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

		planeItem = mgr->createItem( planeMesh, SCENE_STATIC );
		planeItem->setDatablock( "Ground" );
		planeItem->setCastShadows(false);
		
		planeNode = rootNode->createChildSceneNode( SCENE_STATIC );
		planeNode->setPosition( pos[i][0] );
		planeNode->attachObject( planeItem );
	}
	#endif
	}

	void TerrainGame::DestroyPlane()
	{
		LogO("---- destroy Plane");
		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		if (planeItem)
		{   mgr->destroyItem(planeItem);  planeItem = 0;  }
		if (planeNode)
		{   mgr->destroySceneNode(planeNode);  planeNode = 0;  }
	}


	//  Sky dome
	//-----------------------------------------------------------------------------------------------------------------------------
	void TerrainGame::CreateSkyDome(String sMater, float yaw)
	{
		if (moSky)  return;
		Vector3 scale = 25000 /*view_distance*/ * Vector3::UNIT_SCALE;
		
		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		ManualObject* m = mgr->createManualObject();
		m->begin(sMater, OT_TRIANGLE_LIST);

		//  divisions- quality
		int ia = 32*2, ib = 24,iB = 24 +1/*below_*/, i=0;
		
		//  angles, max
		const Real B = Math::HALF_PI, A = Math::TWO_PI;
		Real bb = B/ib, aa = A/ia;  // add
		Real a,b;
		ia += 1;

		//  up/dn y  )
		for (b = 0.f; b <= B+bb/*1*/*iB; b += bb)
		{
			Real cb = sinf(b), sb = cosf(b);
			Real y = sb;

			//  circle xz  o
			for (a = 0.f; a <= A; a += aa, ++i)
			{
				Real x = cosf(a)*cb, z = sinf(a)*cb;
				m->position(x,y,z);

				m->textureCoord(a/A, b/B);

				if (a > 0.f && b > 0.f)  // rect 2tri
				{
					m->index(i-1);  m->index(i);     m->index(i-ia);
					m->index(i-1);  m->index(i-ia);  m->index(i-ia-1);
				}
			}
		}
		m->end();
		moSky = m;

		Aabb aab(Vector3(0,0,0), Vector3(1,1,1)*1000000);
		m->setLocalAabb(aab);  // always visible
		//m->setRenderQueueGroup(230);  //?
		m->setCastShadows(false);

		ndSky = mgr->getRootSceneNode()->createChildSceneNode();
		ndSky->attachObject(m);  // SCENE_DYNAMIC
		ndSky->setScale(scale);
		Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
		ndSky->setOrientation(q);

		//  Change the addressing mode to wrap  ?
		/*Root *root = mGraphicsSystem->getRoot();
		Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_UNLIT );
		HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>(hlms->getDatablock(sMater));
		// HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(m->getDatablock() );
		HlmsSamplerblock sampler( *datablock->getSamplerblock( PBSM_DIFFUSE ) );  // hard copy
		sampler.mU = TAM_WRAP;
		sampler.mV = TAM_WRAP;
		sampler.mW = TAM_WRAP;
		datablock->setSamplerblock( PBSM_DIFFUSE, sampler );
		datablock->setSamplerblock( PBSM_NORMAL, sampler );
		datablock->setSamplerblock( PBSM_ROUGHNESS, sampler );
		datablock->setSamplerblock( PBSM_METALLIC, sampler );/**/
		// datablock->setSamplerblock(sampler);
	}

	void TerrainGame::DestroySkyDome()
	{
		LogO("---- destroy SkyDome");
		SceneManager *mgr = mGraphicsSystem->getSceneManager();
		if (moSky)
		{   mgr->destroyManualObject(moSky);  moSky = 0;  }
		if (ndSky)
		{   mgr->destroySceneNode(ndSky);  ndSky = 0;  }
	}
}
