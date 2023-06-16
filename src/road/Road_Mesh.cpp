#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "Road.h"
#include "App.h"
#include "CData.h"
#include "PresetsXml.h"
#ifndef SR_EDITOR
#else
	#include "game.h"
#endif
#include "CScene.h"
#include "GraphicsSystem.h"

#include <OgreCommon.h>
#include <OgreHlmsManager.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsPbsPrerequisites.h>
#include <OgreMeshManager.h>
#include <OgreException.h>
#include <OgreItem.h>
#include <OgreSceneNode.h>
#include <OgreMesh.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager2.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexElements.h>
using namespace Ogre;

#define V1tangents  // todo: compute tangents ..
// WARNING: Mesh2 instance 'road.6041_0' was defined as manually loaded, but no manual loader was provided. This Resource will be lost if it has to be reloaded.


//  🏗️ Create Mesh
//---------------------------------------------------------------------------------------------------------------------------------
void SplineRoad::CreateMesh( int lod, SegData& sd, Ogre::String sMesh,
	Ogre::String sMtrName, bool alpha, bool pipeGlass,
	const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
	const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
	const std::vector<Ogre::uint16>& idx)
{
	// LogO("Road -- MESH mtr: "+sMtrName+"  cnt pos "+toStr(pos.size())+" idx "+toStr(idx.size()));

	size_t i;
	if (pos.empty())
	{	LogO("Road CreateMesh 0 verts !");
		return;
	}
	if (MeshManager::getSingleton().getByName(sMesh))
		LogR("Mesh exists !!!" + sMesh);

	//LogO("RD mesh: "+sMesh+" "+sMtrName);
	bool trail = IsTrail();

 	Aabb aabox;
 	MeshPtr mesh = MeshManager::getSingleton().createManual(sMesh, "General");
		//ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	SubMesh* subMesh = mesh->createSubMesh();

    //-----------------------------------------------------------------------------------
    bool partialMesh = false;
	RenderSystem *rs = pApp->mRoot->getRenderSystem();
	VaoManager *vaoManager = rs->getVaoManager();

	// Vertex declaration
	uint vertSize = 0;
	VertexElement2Vec vertexElements;
	bool hasClr = clr.size() > 0;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );  vertSize += 3;
	if (!trail)
	{	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_NORMAL ) );    vertSize += 3;
	#ifndef V1tangents
		vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_TANGENT ) );    vertSize += 3;
		// vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_BINORMAL ) );    vertSize += 3;
	#endif
		vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );  vertSize += 2;
	}
	// vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );  vertSize += 2;  //2nd uv-
	if (hasClr)
	{	vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );  vertSize += 4;  }


	//  vertex data  ------------------------------------------
	uint vertCnt = pos.size();
	vertSize *= sizeof( float );
	uint size = vertSize * vertCnt;
	float *v = reinterpret_cast<float *>(  // vertices
		OGRE_MALLOC_SIMD( size, MEMCATEGORY_GEOMETRY ) );
	
	uint a = 0;
	if (trail)
		for (uint i=0; i < vertCnt; ++i)
		{
			v[a++] = pos[i].x;  v[a++] = pos[i].y;  v[a++] = pos[i].z;  aabox.merge( pos[i] );
			v[a++] = clr[i].x;  v[a++] = clr[i].y;  v[a++] = clr[i].z;  v[a++] = clr[i].w;
		}
	else if (hasClr)
		for (uint i=0; i < vertCnt; ++i)
		{
			v[a++] = pos[i].x;   v[a++] = pos[i].y;   v[a++] = pos[i].z;  aabox.merge( pos[i] );
			v[a++] = norm[i].x;  v[a++] = norm[i].y;  v[a++] = norm[i].z;
		#ifndef V1tangents
			v[a++] = norm[i].x;  v[a++] = norm[i].z;  v[a++] = norm[i].y;  // tangent-
			//v[a++] = norm[i].z;  v[a++] = norm[i].y;  v[a++] = norm[i].x;  // binormal-
		#endif
			v[a++] = tcs[i].x;  v[a++] = tcs[i].y;
			v[a++] = clr[i].x;  v[a++] = clr[i].y;  v[a++] = clr[i].z;  v[a++] = clr[i].w;
		}
	else
		for (uint i=0; i < vertCnt; ++i)
		{
			v[a++] = pos[i].x;   v[a++] = pos[i].y;   v[a++] = pos[i].z;  aabox.merge( pos[i] );
			v[a++] = norm[i].x;  v[a++] = norm[i].y;  v[a++] = norm[i].z;
		#ifndef V1tangents
			v[a++] = norm[i].x;  v[a++] = norm[i].z;  v[a++] = norm[i].y;  // tangent-
			//v[a++] = norm[i].z;  v[a++] = norm[i].y;  v[a++] = norm[i].x;  // binormal-
		#endif
			v[a++] = tcs[i].x;  v[a++] = tcs[i].y;
			// v[a++] = tcs[i].x;   v[a++] = tcs[i].y;  //2nd uv-
		}

	VertexBufferPacked *vertexBuffer = 0;
	try
	{
		vertexBuffer = vaoManager->createVertexBuffer(
			vertexElements, vertCnt, partialMesh ? BT_DEFAULT : BT_IMMUTABLE, &v[0], true );
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
	if (idxCnt >= 65530)
		LogO("Road!  Index out of 16bit! "+toStr(idxCnt));
	
	uint idxSize = sizeof( uint16 ) * idxCnt;
	uint16 *indices = reinterpret_cast<uint16 *>(
		OGRE_MALLOC_SIMD( idxSize, MEMCATEGORY_GEOMETRY ) );
	
	// for (uint i=0; i < idxCnt; ++i)
	// 	indices[i] = idx[i];
	memcpy( indices, &idx[0],  idxSize);

	try
	{
		indexBuffer = vaoManager->createIndexBuffer(
			IndexBufferPacked::IT_16BIT, idxCnt, BT_IMMUTABLE, indices, true );
	}
	catch (Exception &e)
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

#ifndef SR_EDITOR
	subMesh->arrangeEfficient(false, false, false);  // same-
#endif


	//  add mesh to scene
	//---------------------------------------------------------
	subMesh->setMaterialName(sMtrName);

	mesh->_setBounds(aabox, false);  //?
	mesh->_setBoundingSphereRadius((aabox.getMaximum() - aabox.getMinimum()).length() / 2.0);  

	//  tangents  v2 to v1 to v2 meh-
#ifdef V1tangents
	String s1 = sMesh+"v1", s2 = sMesh+"v2";

	sd.mesh1 = v1::MeshManager::getSingleton().create(s1, "General");
	sd.mesh1->setVertexBufferPolicy( v1::HardwareBuffer::HBU_STATIC, false );  // true may decrease loading time..
	sd.mesh1->setIndexBufferPolicy( v1::HardwareBuffer::HBU_STATIC, false );

 	sd.mesh1->importV2(mesh.get());
	MeshManager::getSingleton().remove(mesh);  // not needed
	mesh.reset();

	if (!trail)
		sd.mesh1->buildTangentVectors();  // todo: slow in ed, 24,30 Fps  vs 60 Fps v2 only

	sd.mesh = MeshManager::getSingleton().createManual(s2, "General", 0);  // todo: loader for device lost-
	sd.mesh->importV1(sd.mesh1.get(), false,false,false);
	// v1::MeshManager::getSingleton().remove(sd.mesh1);  // not needed
	// sd.mesh1.reset();
	//  old
	// sd.mesh = MeshManager::getSingleton().createByImportingV1(
	// 	s2, "General", sd.mesh1.get(), false,false,false);
#else
	sd.mesh = mesh;
#endif


	//  add mesh to scene
	//---------------------------------------------------------
	auto dyn = SCENE_STATIC;
#ifdef SR_EDITOR
	// dyn = SCENE_DYNAMIC;  // ed a bit faster?
#endif

	Item *it2 =0, *it34[2] ={0,0};
#ifdef V1tangents
	Item *it = mSceneMgr->createItem( s2, "General", dyn );
#else
	Item *it = mSceneMgr->createItem( mesh, dyn );
#endif

	SceneNode* node = mSceneMgr->getRootSceneNode( dyn )->createChildSceneNode( dyn );
	it->setVisible(false);  it->setCastShadows(false);//-
	it->setVisibilityFlags(RV_Road);
	node->attachObject(it);


	//  ed road for mini and density
	//---------------------------------------------------------
#ifdef SR_EDITOR
	if (lod == 0)
	for (int i=0; i<2; ++i)
	{
		// LogO("LOD 0 ed "+toStr(clr.size()));
		auto*& it = it34[i];
		#ifdef V1tangents
			it = mSceneMgr->createItem( s2, "General", dyn );
		#else
			it = mSceneMgr->createItem( mesh, dyn );
		#endif
		it->setRenderQueueGroup(RQG_Road);
		it->setVisibilityFlags(i ? RV_EdRoadPreview : RV_EdRoadDens);
		it->setCastShadows(false);
		it->setDatablockOrMaterialName(i ? "ed_RoadPreview" : "ed_RoadDens");
		node->attachObject(it);
	}
#endif

	//  ⭕ pipe glass 2nd item
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (pipeGlass)
	{
	#ifdef V1tangents
		it2 = mSceneMgr->createItem( s2, "General", dyn );
	#else
		it2 = mSceneMgr->createItem( mesh, dyn );
	#endif
		
		String sMtr2 = sMtrName + "2";
		auto hlms = pApp->mRoot->getHlmsManager()->getHlms( HLMS_PBS );
		if (!hlms->getDatablock(sMtr2))
		{	//  clone if not already
			LogO(sMtr2);
			HlmsDatablock *db2 = it2->getSubItem(0)->getDatablock()->clone(sMtr2);
			HlmsMacroblock mb;  // = db2->getMacroblock();
			mb.mDepthBiasConstant = 3.f;  //.. mDepthBiasSlopeScale
			mb.mDepthWrite = false;
			mb.mCullMode = CULL_ANTICLOCKWISE;  // set opposite cull
			db2->setMacroblock(mb);
			it2->setDatablock(db2);
		}
		it2->setDatablockOrMaterialName(sMtr2);

		node->attachObject(it2);
		it2->setVisible(false);  it2->setCastShadows(false);//-
		it2->setVisibilityFlags(RV_Road);
	}

	//  wrap tex  ----
	if (!trail)
	{
		pApp->SetTexWrap(it);  if (it2)  pApp->SetTexWrap(it2);
	}

	//  replace onTer alpha  ----
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HlmsPbsDatablock *db = static_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() );
	if (alpha && db)
	{
		const TextureGpu *diffTex = db->getDiffuseTexture(),
			*normTex = db->getTexture(PBSM_NORMAL);
		if (diffTex && normTex)
		{
			const String sAlpha = "roadAlpha2.png", sFlat = "flat_n.png",
				sDiff = diffTex->getNameStr(), sNorm = normTex->getNameStr();

			if (sDiff != sAlpha)  // once
			{
				//LogO("RD mtr: "+ sMtrName+" tex: "+sDiff+" norm: "+sNorm);
				db->setTexture(PBSM_DIFFUSE, sAlpha);  // same for all
				db->setTexture(PBSM_NORMAL, sFlat);

				db->setDetailMapBlendMode(0, PBSM_BLEND_MULTIPLY);  //PBSM_BLEND_NORMAL_NON_PREMUL);
				db->setTexture(PBSM_DETAIL_WEIGHT, "roadAlpha2y.png");
				db->setDetailMapWeight(0, 1.0);
				const Real v = 33.3;  // 1.f / 0.03 = alpha tc in rebuild
				db->setDetailMapOffsetScale(0, Vector4(0,0, 1,v));

				db->setTexture(PBSM_DETAIL0, sDiff);
				db->setTexture(PBSM_DETAIL0_NM, sNorm);
				//todo: PBSM_SPECULAR ?.. stretched
				// db->setFresnel(Vector3(1.f,1,1), false);
		}	}
	}
	if (db)  //  set reflection, from presets  ----
	{
		auto s = sMtrName;
		auto is = s.find_last_of('_');  // drop _ter
		if (is != std::string::npos)
			s = s.substr(0,is);
		auto rd = pApp->scn->data->pre->GetRoad(s);
		if (!rd)
		{  }  // LogO("Road mat error: not in presets: "+s);
		else
		if (rd->reflect && pApp->mCubeReflTex)
			db->setTexture( PBSM_REFLECTION, pApp->mCubeReflTex );  // wet, etc+
		// it->getSubItem(0)->setDatablock( db );
	}

	sd.it = it;  sd.it2 = it2;
	sd.it3r = it34[0];  sd.it4d = it34[1];
	sd.node = node;
	// sd.mesh = mesh;  // sd.mesh1 = m1;

	//LogO(sMesh + " " + datablock->getName().getReleaseText());  // hash
}
//---------------------------------------------------------------------------------------------------------------------------------


//  🔺 add triangle to bullet
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


///  💥 Destroy
//-----------------------------------------------------------------------------------
void SplineRoad::Destroy()  // full and markers
{
	LogO("D--- destroy Road");
	if (ndSel)	mSceneMgr->destroySceneNode(ndSel);
	if (ndChosen)	mSceneMgr->destroySceneNode(ndChosen);
	if (ndRot)	mSceneMgr->destroySceneNode(ndRot);
	if (ndHit)	mSceneMgr->destroySceneNode(ndHit);
	if (ndChk)	mSceneMgr->destroySceneNode(ndChk);
	if (itSel)  mSceneMgr->destroyItem(itSel);
	if (itChs)  mSceneMgr->destroyItem(itChs);
	if (itRot)  mSceneMgr->destroyItem(itRot);
	if (itHit)  mSceneMgr->destroyItem(itHit);
	if (itChk)  mSceneMgr->destroyItem(itChk);
	DestroyMarkers();
	DestroyRoad();
}

//  one seg
void SplineRoad::DestroySeg(int id)
{
	// LogO("DestroySeg" + toStr(id));
	RoadSeg& rs = vSegs[id];
	if (rs.empty)  return;
	try
	{
		auto mgr = mSceneMgr;
		auto& ms = MeshManager::getSingleton();
		auto& m1 = v1::MeshManager::getSingleton();

		for (int l=0; l < LODs; ++l)
		{
			{	// ] wall
				auto& w = rs.wall[l];
				auto& n = w.node;   if (n)  mgr->destroySceneNode(n);  n = 0;
				auto& i = w.it;     if (i)  mgr->destroyItem(i);  i = 0;
				if (w.mesh)  ms.remove(w.mesh);
				w.mesh.reset();
				if (w.mesh1)  m1.remove(w.mesh1);
				w.mesh1.reset();
			}
			{	// > blend
				// auto& n = rs.blend[l].node;  if (n)  mgr->destroySceneNode(n);  n = 0;
				// auto& i = rs.blend[l].it;    if (i)  mgr->destroyItem(i);  i = 0;
				// auto& s = rs.blend[l].smesh;
				// if (!s.empty())  ms.remove(s);  s = "";
			}
			{	// - road
				// LogO("---- Destroy Road seg " + rs.road[l].smesh);
				auto& rd = rs.road[l];
				auto& n = rd.node;  if (n)  mgr->destroySceneNode(n);  n = 0;
				auto& i = rd.it;    if (i)  mgr->destroyItem(i);  i = 0;
				auto& j = rd.it2;   if (j)  mgr->destroyItem(j);  j = 0;
			#ifdef SR_EDITOR
				auto& r = rd.it3r;  if (r)  mgr->destroyItem(r);  r = 0;
				auto& d = rd.it4d;  if (d)  mgr->destroyItem(d);  d = 0;
			#endif
				if (rd.mesh)  ms.remove(rd.mesh);
				rd.mesh.reset();
				if (rd.mesh1)  m1.remove(rd.mesh1);
				rd.mesh1.reset();
			}
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


//  all segs
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
