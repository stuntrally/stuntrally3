#include "OgreCommon.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsPbsPrerequisites.h"
#include "OgreMeshManager.h"
#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "Road.h"

#include "CGame.h"
#include "CScene.h"
#include "game.h"
#include "Game.h"
#include "GraphicsSystem.h"

#include <OgreException.h>
#include <OgreItem.h>
#include <OgreSceneNode.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager2.h>
#include <Vao/OgreVaoManager.h>
#include <OgreMesh.h>
using namespace Ogre;

#define USE_UMA_SHARED_BUFFERS 1

//  Create Mesh
//---------------------------------------------------------

void SplineRoad::CreateMesh( SegData& sd, Ogre::String sMesh,
	Ogre::String sMtrName, bool alpha,
	const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
	const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
	const std::vector<Ogre::uint16>& idx)
{
	size_t i, si = pos.size();
	if (si == 0)
	{	LogO("Error:  Road CreateMesh 0 verts !");
		return;
	}
	if (MeshManager::getSingleton().getByName(sMesh))
		LogR("Mesh exists !!!" + sMesh);

 	Aabb aabox;
 	MeshPtr mesh = MeshManager::getSingleton().createManual(sMesh, "General");
		//ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	SubMesh* subMesh = mesh->createSubMesh();

    //-----------------------------------------------------------------------------------
    bool partialMesh = false;
	Root *root = pGame->app->mainApp->mGraphicsSystem->getRoot();
	RenderSystem *renderSystem = root->getRenderSystem();
	VaoManager *vaoManager = renderSystem->getVaoManager();

	// Vertex declaration
	uint vertSize = 0;
	VertexElement2Vec vertexElements;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );  vertSize += 3;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_NORMAL ) );    vertSize += 3;
	vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );  vertSize += 2;
	// vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );  vertSize += 2;  //2nd uv-
	bool hasClr = clr.size() > 0;
	if (hasClr)
	{	vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );  vertSize += 4;  }


	//  vertex data  ------------------------------------------
	uint vertCnt = pos.size();
	vertSize *= sizeof( float );
	float *vertices = reinterpret_cast<float *>(
		OGRE_MALLOC_SIMD( sizeof( float ) * vertSize * vertCnt, MEMCATEGORY_GEOMETRY ) );
	
	uint a = 0;
	if (hasClr)
	for (uint i=0; i < vertCnt; ++i)
	{
		vertices[a++] = pos[i].x;   vertices[a++] = pos[i].y;   vertices[a++] = pos[i].z;  aabox.merge(pos[i]);
		vertices[a++] = norm[i].x;  vertices[a++] = norm[i].y;  vertices[a++] = norm[i].z;
		vertices[a++] = tcs[i].x;   vertices[a++] = tcs[i].y;
		vertices[a++] = clr[i].x;   vertices[a++] = clr[i].y;   vertices[a++] = clr[i].z;   vertices[a++] = clr[i].w;
	}else
	for (uint i=0; i < vertCnt; ++i)
	{
		vertices[a++] = pos[i].x;   vertices[a++] = pos[i].y;   vertices[a++] = pos[i].z;  aabox.merge(pos[i]);
		vertices[a++] = norm[i].x;  vertices[a++] = norm[i].y;  vertices[a++] = norm[i].z;
		vertices[a++] = tcs[i].x;   vertices[a++] = tcs[i].y;
		// vertices[a++] = tcs[i].x;   vertices[a++] = tcs[i].y;  //2nd uv-
	}

	VertexBufferPacked *vertexBuffer = 0;
	try
	{
	#if USE_UMA_SHARED_BUFFERS
		const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
		if (caps && caps->hasCapability(RSC_UMA))
		{
			vertexBuffer = vaoManager->createVertexBuffer(
				vertexElements, vertCnt, BT_DEFAULT_SHARED, &vertices[0], true );
		}else
	#endif
			vertexBuffer = vaoManager->createVertexBuffer(
				vertexElements, vertCnt, partialMesh ? BT_DEFAULT : BT_IMMUTABLE, &vertices[0], true );
	}
	catch (Exception &e)
	{
		// we passed keepAsShadow = true to createVertexBuffer, thus Ogre will free the pointer
		// if keepAsShadow = false, YOU need to free the pointer
		OGRE_FREE_SIMD( vertexBuffer, MEMCATEGORY_GEOMETRY );
		vertexBuffer = 0;
		throw e;
	}


	//  Vao, one vertex buffer source (multi-source not working yet)
	VertexBufferPackedVec vertexBuffers;
	vertexBuffers.push_back( vertexBuffer );


	//  index data  ------------------------------------------
	IndexBufferPacked *indexBuffer = 0;

	uint idxCnt = idx.size();
	uint idxSize = sizeof( uint16 ) * idxCnt;
	uint16 *indices = reinterpret_cast<uint16 *>(
		OGRE_MALLOC_SIMD( idxSize, MEMCATEGORY_GEOMETRY ) );
	
	// for (uint i=0; i < idxCnt; ++i)
	// 	indices[i] = idx[i];
	memcpy( indices, &idx[0],  idxSize);

	try
	{
	#ifdef USE_UMA_SHARED_BUFFERS
		const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
		if(caps && caps->hasCapability(RSC_UMA))
		{
			indexBuffer = vaoManager->createIndexBuffer(
				IndexBufferPacked::IT_16BIT, idxCnt, BT_DEFAULT_SHARED, indices, true );
		}else
	#endif
			indexBuffer = vaoManager->createIndexBuffer(
				IndexBufferPacked::IT_16BIT, idxCnt, BT_IMMUTABLE, indices, true );
	}
	catch( Exception &e )
	{
		// When keepAsShadow = true, the memory will be freed when the index buffer is destroyed.
		// However if for some weird reason there is an exception raised, the memory will
		// not be freed, so it is up to us to do so.
		OGRE_FREE_SIMD( indexBuffer, MEMCATEGORY_GEOMETRY );
		indexBuffer = 0;
		throw e;
	}

	VertexArrayObject *vao =
		vaoManager->createVertexArrayObject( vertexBuffers, indexBuffer, OT_TRIANGLE_LIST );

	// Each Vao pushed to the vector refers to an LOD level.
	// todo: all LODs here
	// Must be in sync with mesh->mLodValues & mesh->mNumLods if > 1 level
	subMesh->mVao[VpNormal].push_back( vao );
	subMesh->mVao[VpShadow].push_back( vao );  // same geometry for shadow casting

	subMesh->arrangeEfficient(false, false, false);  // no?


	//  add mesh to scene
	//---------------------------------------------------------
	subMesh->setMaterialName(sMtrName);

	mesh->_setBounds(aabox, false);  //?
	mesh->_setBoundingSphereRadius((aabox.getMaximum() - aabox.getMinimum()).length() / 2.0);  

	//  tangents-
	String s1 = sMesh+"v1", s2 = sMesh+"v2";
	v1::MeshPtr m1 = v1::MeshManager::getSingleton().create(s1, "General");
	/*v1::MeshPtr m1 = static_cast<v1::MeshPtr>(v1::MeshManager::getSingleton().createOrRetrieve(s1, "General",
		true, 0, 0,
		v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC ).first);*/
 	m1->importV2(mesh.get());
	m1->buildTangentVectors();
	mesh = MeshManager::getSingleton().createByImportingV1(s2, "General", m1.get(), false,false,false);
	MeshManager::getSingleton().remove(sMesh);  // not needed


	//  add mesh to scene
	//---------------------------------------------------------
	Item *it = mSceneMgr->createItem(s2, "General", SCENE_STATIC );
	// Item *it = mSceneMgr->createItem( mesh, SCENE_STATIC );
	SceneNode* node = mSceneMgr->getRootSceneNode( SCENE_STATIC )->createChildSceneNode( SCENE_STATIC );
	node->attachObject(it);
	it->setVisible(false);  it->setCastShadows(false);//-
	it->setVisibilityFlags(RV_Road);

	//  wrap tex  ----
	pGame->app->scn->SetTexWrap(it);
	HlmsPbsDatablock *datablock =
		static_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() );

	//  replace alpha  ----
	if (alpha)
	{
		TextureGpu *diffTex = datablock->getDiffuseTexture(),
			*normTex = datablock->getTexture(PBSM_NORMAL);
		const String sAlpha = "roadAlpha2.png", sFlat = "flat_n.png",
			sDiff = diffTex->getNameStr(), sNorm = normTex->getNameStr();

		if (sDiff != sAlpha)  // once
		{
			//LogO("RD mtr: "+ sMtrName+" tex: "+sDiff+" norm: "+sNorm);
			datablock->setTexture(PBSM_DIFFUSE, sAlpha);  // same for all
			datablock->setTexture(PBSM_NORMAL, sFlat);

			datablock->setDetailMapBlendMode(0, PBSM_BLEND_MULTIPLY);  //PBSM_BLEND_NORMAL_NON_PREMUL);
			datablock->setTexture(PBSM_DETAIL_WEIGHT, "roadAlpha2y.png");  //"HeightmapBlendmap.png");
			datablock->setDetailMapWeight(0, 1.0);
			const Real v = 33.3;  // 1.f / 0.03 = alpha tc in rebuild
			datablock->setDetailMapOffsetScale(0, Vector4(0,0, 1,v));

			datablock->setTexture(PBSM_DETAIL0, sDiff);
			datablock->setTexture(PBSM_DETAIL0_NM, sNorm);
		}
	}
	sd.it = it;
	sd.node = node;
	sd.smesh = sMesh;
	// sd.mesh = mesh;  sd.mesh1 = m1;

	//LogO(sMesh + " " + datablock->getName().getReleaseText());  // hash
}


//  add triangle to bullet
//---------------------------------------------------------
void SplineRoad::addTri(int f1, int f2, int f3, int i)
{
	/*bool ok = true;  const int fmax = 65530; //16bit
	if (f1 >= at_size || f1 > fmax)  {  LogO("idx too big: "+toStr(f1)+" >= "+toStr(at_size));  ok = 0;  }
	if (f2 >= at_size || f2 > fmax)  {  LogO("idx too big: "+toStr(f2)+" >= "+toStr(at_size));  ok = 0;  }
	if (f3 >= at_size || f3 > fmax)  {  LogO("idx too big: "+toStr(f3)+" >= "+toStr(at_size));  ok = 0;  }
	if (!ok)  return;/**/

	idx.push_back(f1);  idx.push_back(f2);  idx.push_back(f3);
	if (blendTri)
	{
		idxB.push_back(f1);  idxB.push_back(f2);  idxB.push_back(f3);
	}

	if (bltTri && i > 0 && i < at_ilBt)
	{
		posBt.push_back((*at_pos)[f1]);
		posBt.push_back((*at_pos)[f2]);
		posBt.push_back((*at_pos)[f3]);
	}
}


///  Destroy
//---------------------------------------------------------

void SplineRoad::Destroy()  // full
{
	LogO("---- Destroy Road");
	if (ndSel)	mSceneMgr->destroySceneNode(ndSel);
	if (ndChosen)	mSceneMgr->destroySceneNode(ndChosen);
	if (ndRot)	mSceneMgr->destroySceneNode(ndRot);
	if (ndHit)	mSceneMgr->destroySceneNode(ndHit);
	if (ndChk)	mSceneMgr->destroySceneNode(ndChk);
	if (entSel)  mSceneMgr->destroyItem(entSel);
	if (entChs)  mSceneMgr->destroyItem(entChs);
	if (entRot)  mSceneMgr->destroyItem(entRot);
	if (entHit)  mSceneMgr->destroyItem(entHit);
	if (entChk)  mSceneMgr->destroyItem(entChk);
	DestroyMarkers();
	DestroyRoad();
}

void SplineRoad::DestroySeg(int id)
{
	//LogO("DestroySeg" + toStr(id));
	RoadSeg& rs = vSegs[id];
	if (rs.empty)  return;
try
{
	for (int l=0; l < LODs; ++l)
	{
		if (rs.wall[l].it)  // ] wall
		{
			mSceneMgr->destroySceneNode(rs.wall[l].node);
			mSceneMgr->destroyItem(rs.wall[l].it);

			String sMesh = rs.wall[l].smesh, s1 = sMesh+"v1", s2 = sMesh+"v2";
			MeshManager::getSingleton().remove(sMesh);
			MeshManager::getSingleton().remove(s2);
			v1::MeshManager::getSingleton().remove(s1);
		}
		if (rs.blend[l].it)  // > blend
		{
			mSceneMgr->destroySceneNode(rs.blend[l].node);
			mSceneMgr->destroyItem(rs.blend[l].it);
			MeshManager::getSingleton().remove(rs.blend[l].smesh);
		}
		if (rs.road[l].it)  // - road
		{
			// LogO("---- Destroy Road seg " + rs.road[l].smesh);
			mSceneMgr->destroySceneNode(rs.road[l].node);
			mSceneMgr->destroyItem(rs.road[l].it);

			String sMesh = rs.road[l].smesh, s1 = sMesh+"v1", s2 = sMesh+"v2";
			// MeshManager::getSingleton().remove(sMesh);
			MeshManager::getSingleton().remove(s2);
			v1::MeshManager::getSingleton().remove(s1);
		}
	}
	if (rs.col.it)  // | column
	{
		mSceneMgr->destroySceneNode(rs.col.node);
		mSceneMgr->destroyItem(rs.col.it);
		MeshManager::getSingleton().remove(rs.col.smesh);
	}
}
catch (Exception ex)
{
	LogO(String("# Error! road DestroySeg") + ex.what());
}
	// LogO("---- road Destroyed");
	rs.empty = true;
	rs.lpos.clear();
}


void SplineRoad::DestroyRoad()
{
	
#ifndef SR_EDITOR
	for (int i=0; i < vbtTriMesh.size(); ++i)
		delete vbtTriMesh[i];
	vbtTriMesh.clear();
#endif
	for (size_t seg=0; seg < vSegs.size(); ++seg)
		DestroySeg(seg);
	vSegs.clear();

	idStr = 0;
	st.Reset();
}
