#include "OgreCommon.h"
#include "OgreHlmsPbsDatablock.h"
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
	VertexElement2Vec vertexElements;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_NORMAL ) );
	vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );
	// if (clr.size() > 0)
	// 	vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );


	//  vertex data  ------------------------------------------
	// For immutable buffers, it is mandatory that vertices is not a null pointer.
	uint vertCnt = pos.size();
	uint vertSize = (3 + 3 + 2) * sizeof( float );
	float *vertices = reinterpret_cast<float *>(
		OGRE_MALLOC_SIMD( sizeof( float ) * vertSize * vertCnt, MEMCATEGORY_GEOMETRY ) );
	
	// Fill the data
	uint a = 0;
	for (uint i=0; i < vertCnt; ++i)
	{
		vertices[a++] = pos[i].x;
		vertices[a++] = pos[i].y;
		vertices[a++] = pos[i].z;  aabox.merge(pos[i]);
		vertices[a++] = norm[i].x;
		vertices[a++] = norm[i].y;
		vertices[a++] = norm[i].z;
		vertices[a++] = tcs[i].x;
		vertices[a++] = tcs[i].y;
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
		// Important: Please note that we passed keepAsShadow = true to createVertexBuffer,
		// thus Ogre will free the pointer. However had we passed keepAsShadow = false,
		// it would be YOUR responsability to free the pointer, not Ogre's
		OGRE_FREE_SIMD( vertexBuffer, MEMCATEGORY_GEOMETRY );
		vertexBuffer = 0;
		throw e;
	}


	// Now the Vao. We'll just use one vertex buffer source (multi-source not working yet)
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
		// The reasons for exceptions are very rare. But we're doing this for correctness.
		// Important: Please note that we passed keepAsShadow = true to createIndexBuffer,
		// thus Ogre will free the pointer. However had we passed keepAsShadow = false,
		// it would be YOUR responsability to free the pointer, not Ogre's
		OGRE_FREE_SIMD( indexBuffer, MEMCATEGORY_GEOMETRY );
		indexBuffer = 0;
		throw e;
	}

	VertexArrayObject *vao =
		vaoManager->createVertexArrayObject( vertexBuffers, indexBuffer, OT_TRIANGLE_LIST );

	// Each Vao pushed to the vector refers to an LOD level.
	// Must be in sync with mesh->mLodValues & mesh->mNumLods if you use more than one level
	subMesh->mVao[VpNormal].push_back( vao );
	// Use the same geometry for shadow casting.
	subMesh->mVao[VpShadow].push_back( vao );

	// subMesh->arrangeEfficient(true, true, true);

	// Set the bounds to get frustum culling and LOD to work correctly.
	// mesh->_setBounds( Aabb( Vector3::ZERO, Vector3::UNIT_SCALE ), false );
	// mesh->_setBoundingSphereRadius( 1.732f );

	// return mesh;
    //-----------------------------------------------------------------------------------

	/*mesh->useSharedVertices = false;
	mesh->vertexData = new VertexData();
	mesh->vertexData->vertexStart = 0;
	mesh->vertexData->vertexCount = si;

	//  decl
	VertexDeclaration* decl = mesh->vertexData->vertexDeclaration;
	size_t offset = 0;
	offset += decl->addElement(0,offset,VET_FLOAT3,VES_POSITION).getSize();
	offset += decl->addElement(0,offset,VET_FLOAT3,VES_NORMAL).getSize();
	offset += decl->addElement(0,offset,VET_FLOAT2,VES_TEXTURE_COORDINATES).getSize();
	if (clr.size() > 0)
		offset += decl->addElement(0,offset,VET_FLOAT4,VES_DIFFUSE).getSize();

	//  vertex
	//-----------------------------------------
	HardwareVertexBufferSharedPtr vbuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0), si, HardwareBuffer::HBU_STATIC);
	float* vp = static_cast<float*>(vbuffer->lock(HardwareBuffer::HBL_DISCARD));
	
	//  fill vb, update aabb
	if (clr.size() > 0)
	for (i=0; i < si; ++i)
	{
		const Vector3& p = pos[i];
		*vp++ = p.x;  *vp++ = p.y;  *vp++ = p.z;	aabox.merge(p);
		const Vector3& n = norm[i];
		*vp++ = n.x;  *vp++ = n.y;  *vp++ = n.z;
		*vp++ = tcs[i].x;  *vp++ = tcs[i].y;
		const Vector4& c = clr[i];
		*vp++ = c.x;  *vp++ = c.y;  *vp++ = c.z;  *vp++ = c.w;
	}
	else
	for (i=0; i < si; ++i)
	{
		const Vector3& p = pos[i];
		*vp++ = p.x;  *vp++ = p.y;  *vp++ = p.z;	aabox.merge(p);
		const Vector3& n = norm[i];
		*vp++ = n.x;  *vp++ = n.y;  *vp++ = n.z;
		*vp++ = tcs[i].x;  *vp++ = tcs[i].y;
	}
	vbuffer->unlock();
	mesh->vertexData->vertexBufferBinding->setBinding(0,vbuffer);
	
	//  index
	//-----------------------------------------
	IndexData* id = mesh->indexData;
	id->indexCount = idx.size();  id->indexStart = 0;
	id->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, id->indexCount, HardwareBuffer::HBU_STATIC);
	uint16* ip = static_cast<uint16*>(id->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

	//  fill ib
	for (size_t i=0; i < idx.size(); ++i)
		*ip++ = idx[i];
	
	id->indexBuffer->unlock();*/
	subMesh->setMaterialName(sMtrName);
}


//  add mesh to scene
//---------------------------------------------------------

void SplineRoad::AddMesh(MeshPtr mesh, String sMesh, const Aabb& aabox,
	Item** pIt, SceneNode** pNode, String sEnd)
{
	mesh->_setBounds(aabox);
	mesh->_setBoundingSphereRadius((aabox.getMaximum() - aabox.getMinimum()).length() / 2.0);  
	// mesh->load();
	/*unsigned short src, dest;
	if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
		mesh->buildTangentVectors(VES_TANGENT, src, dest);*/

	*pIt = mSceneMgr->createItem(/*"rd.ent"+sEnd,*/ sMesh, "General", Ogre::SCENE_STATIC);
	*pNode = mSceneMgr->getRootSceneNode(SCENE_STATIC)->createChildSceneNode(/*"rd.node"+sEnd*/SCENE_STATIC);
	(*pNode)->attachObject(*pIt);
	(*pIt)->setVisible(false);  (*pIt)->setCastShadows(false);
	(*pIt)->setVisibilityFlags(RV_Road);

	//  wrap tex
	HlmsSamplerblock sampler;
	sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
	sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sampler.mMaxAnisotropy = 16;//app->pSet->anisotropy;
	sampler.mU = TAM_WRAP;  sampler.mV = TAM_WRAP;  sampler.mW = TAM_WRAP;
	assert( dynamic_cast<Ogre::HlmsPbsDatablock *>( (*pIt)->getSubItem( 0 )->getDatablock() ) );
	HlmsPbsDatablock *datablock =
		static_cast<Ogre::HlmsPbsDatablock *>( (*pIt)->getSubItem( 0 )->getDatablock() );
	datablock->setSamplerblock( PBSM_DIFFUSE, sampler );
	datablock->setSamplerblock( PBSM_NORMAL, sampler );/**/
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
			rs.wall[l].node->detachAllObjects();
			#ifdef SR_EDITOR
			mSceneMgr->destroyItem(rs.wall[l].ent);
			#endif
			//rs.wall[l].node->getParentSceneNode()->detachObject(0);
			mSceneMgr->destroySceneNode(rs.wall[l].node);
			//; Ogre::MeshManager::getSingleton().remove(rs.wall[l].smesh);
		}
		if (rs.blend[l].it)  // > blend
		{
			rs.blend[l].node->detachAllObjects();
			#ifdef SR_EDITOR
			mSceneMgr->destroyItem(rs.blend[l].ent);
			#endif
			mSceneMgr->destroySceneNode(rs.blend[l].node);
			//; Ogre::MeshManager::getSingleton().remove(rs.blend[l].smesh);
		}
	}
	if (rs.col.it)  // | column
	{
		#ifdef SR_EDITOR
		rs.col.node->detachAllObjects();
		mSceneMgr->destroyItem(rs.col.ent);
		#endif
		mSceneMgr->destroySceneNode(rs.col.node);
		//; Ogre::MeshManager::getSingleton().remove(rs.col.smesh);
	}
	for (int l=0; l < LODs; ++l)
	if (rs.road[l].it)
	{
		rs.road[l].node->detachAllObjects();
		if (IsTrail())
			mSceneMgr->destroyItem(rs.road[l].it);
		#ifdef SR_EDITOR  //_crash in game (destroy all ents is before)
		mSceneMgr->destroyItem(rs.road[l].ent);
		#endif
		mSceneMgr->destroySceneNode(rs.road[l].node);
		//; if (Ogre::MeshManager::getSingleton().resourceExists(rs.road[l].smesh))
			// Ogre::MeshManager::getSingleton().remove(rs.road[l].smesh);
		//Resource* r = ResourceManae::getSingleton().remove(rs.road[l].smesh);
	}
}catch (Exception ex)
{
	LogO(String("# Error! road DestroySeg") + ex.what());
}
	//LogO("Destroyed.");
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
