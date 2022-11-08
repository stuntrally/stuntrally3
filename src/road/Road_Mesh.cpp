#include "OgreCommon.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsPbsPrerequisites.h"
#include "OgreMeshManager.h"
#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "Road.h"

#include "CGame.h"
#include "game.h"
#include "TerrainGame.h"
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

void SplineRoad::CreateMesh(SubMesh* subMesh, Aabb& aabox,
	const std::vector<Vector3>& pos, const std::vector<Vector3>& norm, const std::vector<Vector4>& clr,
	const std::vector<Vector2>& tcs, const std::vector<Ogre::uint16>& idx, String sMtrName)
{
	size_t i, si = pos.size();
	if (si == 0)
	{	LogO("Error:  Road CreateMesh 0 verts !");
		return;
	}

    //-----------------------------------------------------------------------------------
    bool partialMesh = false;
	Root *root = pGame->app->mainApp->mGraphicsSystem->getRoot();
	RenderSystem *renderSystem = root->getRenderSystem();
	VaoManager *vaoManager = renderSystem->getVaoManager();

	// Create the mesh
	/*MeshPtr mesh = MeshManager::getSingleton().createManual(
		partialMesh ? "My PartialMesh" : "My StaticMesh",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );*/

	// Create one submesh
	// SubMesh *subMesh = mesh->createSubMesh();


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
		if(caps && caps->hasCapability(RSC_UMA))
		{
			vertexBuffer = vaoManager->createVertexBuffer(
				vertexElements, vertCnt, BT_DEFAULT_SHARED, &vertices[0], true );
		}else
	#endif
			vertexBuffer = vaoManager->createVertexBuffer(
				vertexElements, vertCnt, partialMesh ? BT_DEFAULT : BT_IMMUTABLE, &vertices[0], true );
	}
	catch( Exception &e )
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

	subMesh->arrangeEfficient(false, false, true);  // no?

	/*  old
	HardwareVertexBufferSharedPtr vbuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0), si, HardwareBuffer::HBU_STATIC);
	float* vp = static_cast<float*>(vbuffer->lock(HardwareBuffer::HBL_DISCARD));
		HardwareIndexBuffer::IT_16BIT, id->indexCount, HardwareBuffer::HBU_STATIC);*/
	subMesh->setMaterialName(sMtrName);
}


//  add mesh to scene
//---------------------------------------------------------

String SplineRoad::AddMesh(MeshPtr mesh, String sMesh, const Aabb& aabox,
	Item** pIt, SceneNode** pNode, String sEnd)
{
	mesh->_setBounds(aabox, false);  //?
	mesh->_setBoundingSphereRadius((aabox.getMaximum() - aabox.getMinimum()).length() / 2.0);  
	//mesh->setManuallyLoaded(false);
	//mesh->arrangeEfficient(false, false, true);

	//  tangents-
	v1::MeshPtr m1 = v1::MeshManager::getSingleton().create("v1"+sMesh+sEnd, "General");
	m1->importV2(mesh.get());
	unsigned short src, dest;
	m1->buildTangentVectors(VES_TANGENT, src, dest);
	mesh->importV1(m1.get(), false, false, false);
	// mesh->importV1(m1.get(), true, true, true);
	// m1->unload();  //?
	//mesh->load();

	//  item
	*pIt = mSceneMgr->createItem( mesh, Ogre::SCENE_STATIC );
	*pNode = mSceneMgr->getRootSceneNode( SCENE_STATIC )->createChildSceneNode( SCENE_STATIC );
	(*pNode)->attachObject(*pIt);
	(*pIt)->setVisible(false);  (*pIt)->setCastShadows(false);
	(*pIt)->setVisibilityFlags(RV_Road);

	//  wrap tex-
	HlmsSamplerblock sampler;
	sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
	sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sampler.mMaxAnisotropy = pGame->pSet->anisotropy;
	sampler.mU = TAM_WRAP;  sampler.mV = TAM_WRAP;  sampler.mW = TAM_WRAP;
	assert( dynamic_cast<Ogre::HlmsPbsDatablock *>( (*pIt)->getSubItem( 0 )->getDatablock() ) );
	HlmsPbsDatablock *datablock =
		static_cast<Ogre::HlmsPbsDatablock *>( (*pIt)->getSubItem( 0 )->getDatablock() );
	datablock->setSamplerblock( PBSM_DIFFUSE, sampler );
	datablock->setSamplerblock( PBSM_NORMAL, sampler );
	datablock->setSamplerblock( PBSM_METALLIC, sampler );
	datablock->setSamplerblock( PBSM_ROUGHNESS, sampler );/**/

	//  alpha ?..
	// datablock->setDetailMapBlendMode(0, Ogre::PBSM_BLEND_NORMAL_NON_PREMUL/* PBSM_BLEND_NORMAL_PREMUL*/);
	// datablock->setDetailMapWeight(0, 0.5);
	// datablock->setDetailMapOffsetScale(0, Vector4(0,0,2,4));
	// datablock->setTexture(PBSM_DETAIL_WEIGHT, const String &name)
	// datablock->setTexture(PBSM_DETAIL0, "roadAlpha.png");
	//  datablock->setTexture(PBSM_DETAIL0, "roadGlassC_dirt2.png");

	return datablock->getName().getReleaseText();
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
			MeshManager::getSingleton().remove(rs.wall[l].smesh);
		}
		if (rs.blend[l].it)  // > blend
		{
			mSceneMgr->destroySceneNode(rs.blend[l].node);
			mSceneMgr->destroyItem(rs.blend[l].it);
			MeshManager::getSingleton().remove(rs.blend[l].smesh);
		}
	}
	if (rs.col.it)  // | column
	{
		mSceneMgr->destroySceneNode(rs.col.node);
		mSceneMgr->destroyItem(rs.col.it);
		MeshManager::getSingleton().remove(rs.col.smesh);
	}
	for (int l=0; l < LODs; ++l)
	if (rs.road[l].it)  // - road
	{
		mSceneMgr->destroySceneNode(rs.road[l].node);
		mSceneMgr->destroyItem(rs.road[l].it);
		MeshManager::getSingleton().remove(rs.road[l].smesh);
	}
}catch (Exception ex)
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
