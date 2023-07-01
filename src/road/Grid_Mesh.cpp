#include "pch.h"
#include "Grid.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "GraphicsSystem.h"
#include "App.h"
#include "CScene.h"
#include "CData.h"
#include "PresetsXml.h"

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
using namespace std;

#define V1tangents  // todo: compute tangents ..


//  todo: fake, just to shut up warnings
class ManResLd : public ManualResourceLoader
{
	void prepareResource( Resource *resource ) override
	{
	}
	void loadResource( Resource *resource ) override
	{
	}
} gGridManResLd;


App* GridCellLods::pApp = 0;


//  🆕 Create grid cell mesh, with lods
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void GridCellLods::Create()
{
	auto mSceneMgr = pApp->mSceneMgr;
	// for (auto& lod : lods)
	// {
	auto& lod = lods[0];
	auto& pos = lod.pos;  auto& norm = lod.norm;
	auto& tcs = lod.tcs;  auto& clr = lod.clr;
	auto& idx = lod.idx;
	auto sMtrName = mtr.name;
	bool alpha = 0, pipeGlass = 0;

	// LogO("Road -- MESH mtr: "+sMtrName+"  cnt pos "+toStr(pos.size())+" idx "+toStr(idx.size()));

	size_t i; //, si = pos.size();
	if (pos.empty() || idx.empty())
	{	LogO("Grid CreateMesh 0 verts");
		return;
	}
	
	if (!sMesh.empty())
	{	LogO("Grid!! Create sMesh != ");  return;  }
	if (it)
	{	LogO("Grid!! Create sMesh != ");  return;  }
	
	//  cell mesh name
	++ii;
	short x,y,z;  tie(x,y,z) = gpos;
	sMesh = "Gx"+iToStr(x,2)+",y"+iToStr(y,2)+",z"+iToStr(z,2)+","+iToStr(ii,2)+","+sMtrName;

	if (MeshManager::getSingleton().getByName(sMesh))
		LogO("Mesh exists !!!" + sMesh);

	LogO("Grid mesh: "+sMesh+/*"  mtr: "+sMtrName+*/"  pos "+iToStr(pos.size(),2)+"  idx "+iToStr(idx.size(),2));
	bool trail = 0; //IsTrail();

 	Aabb aabox;  Real b = 1000.f;
	aabox.setExtents(Vector3(-b,-b,-b), Vector3(b,b,b));
 	MeshPtr mesh = MeshManager::getSingleton().createManual(sMesh, "General", &gGridManResLd);
	SubMesh* subMesh = mesh->createSubMesh();

    //-----------------------------------------------------------------------------------
    bool partialMesh = false;
	RenderSystem *rs = pApp->mRoot->getRenderSystem();
	VaoManager *vaoManager = rs->getVaoManager();

	// Vertex declaration
	VertexElement2Vec vertexElements;
	bool hasClr = 0; //clr.size() > 0;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );
	if (!trail)
	{	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_NORMAL ) );
	#ifndef V1tangents
		vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_TANGENT ) );
		// vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_BINORMAL ) );
	#endif
		vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );
	}
	// vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );  //2nd uv-
	if (hasClr)
		vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );


	//  vertex data  ------------------------------------------
	uint vertCnt = pos.size();
	uint vertSize = VaoManager::calculateVertexSize( vertexElements );
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
			// LogO(toStr(i)+" "+toStr(pos[i]));
			v[a++] = norm[i].x;  v[a++] = norm[i].y;  v[a++] = norm[i].z;
		#ifndef V1tangents
			v[a++] = norm[i].x;  v[a++] = norm[i].z;  v[a++] = norm[i].y;  // tangent-
			// v[a++] = norm[i].z;  v[a++] = norm[i].y;  v[a++] = norm[i].x;  // binormal-
		#endif
			v[a++] = tcs[i].x;  v[a++] = tcs[i].y;
			// v[a++] = tcs[i].x;   v[a++] = tcs[i].y;  //2nd uv-
		}

	// for (uint i=0; i < vertCnt; ++i)
	// {
	// 	v[a++] = pos[i].x;   v[a++] = pos[i].y;   v[a++] = pos[i].z;  aabox.merge( pos[i] );
	// 	v[a++] = norm[i].x;  v[a++] = norm[i].y;  v[a++] = norm[i].z;
	// 	v[a++] = tcs[i].x;  v[a++] = tcs[i].y;
	// 	// v[a++] = tcs[i].x;   v[a++] = tcs[i].y;  //2nd uv-
	// }


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
		OGRE_FREE_SIMD( v, MEMCATEGORY_GEOMETRY );
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
		LogO("Grid!  Index out of 16bit!");
	
	uint idxSize = sizeof( uint16 ) * idxCnt;
	uint16 *indices = reinterpret_cast<uint16 *>(
		OGRE_MALLOC_SIMD( idxSize, MEMCATEGORY_GEOMETRY ) );
	
	// for (uint i=0; i < idxCnt; ++i)
	// 	indices[i] = idx[i];
	memcpy( indices, &idx[0],  idxSize);
	
	// for (uint i=0; i < idxCnt; ++i)
	// 	LogO(toStr(i)+" "+toStr(indices[i]));

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
		OGRE_FREE_SIMD( indices, MEMCATEGORY_GEOMETRY );
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
	mesh->load();

	//  tangents v1-
#ifdef V1tangents
	String s1 = sMesh+"v1", s2 = sMesh+"v2";
	v1::MeshPtr m1 = v1::MeshManager::getSingleton().create(s1, "General");
	m1->setVertexBufferPolicy( v1::HardwareBuffer::HBU_STATIC, false );  // true may decrease loading time..
	m1->setIndexBufferPolicy( v1::HardwareBuffer::HBU_STATIC, false );
 	m1->importV2(mesh.get());
	if (!trail)
		m1->buildTangentVectors();  // todo: slow in ed, 24 Fps vs 38?-
	mesh = MeshManager::getSingleton().createByImportingV1(s2, "General", m1.get(), false,false,false);
	MeshManager::getSingleton().remove(sMesh);  // not needed
#endif


	//  add mesh to scene
	//---------------------------------------------------------
	auto dyn = SCENE_STATIC;
#ifdef SR_EDITOR
	// dyn = SCENE_DYNAMIC;  // ed a bit faster?
#endif
// 
	it2 = 0;
#ifdef V1tangents
	it = mSceneMgr->createItem(s2, "General", dyn );
#else
	it = mSceneMgr->createItem( mesh, dyn );
#endif

	node = mSceneMgr->getRootSceneNode( dyn )->createChildSceneNode( dyn );
	node->attachObject(it);
	it->setVisible(true);  it->setCastShadows(true);//-
	it->setVisibilityFlags(RV_Road);
	it->setRenderQueueGroup(RQG_Road);

	//  ⭕ pipe glass 2nd item
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/*if (pipeGlass)
	{
	#ifdef V1tangents
		it2 = mSceneMgr->createItem(s2, "General", dyn );
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
	}*/

	//  wrap tex  ----
	if (!trail)
	{
		pApp->SetTexWrap(it);  if (it2)  pApp->SetTexWrap(it2);
	}

	//  replace onTer alpha  ----
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/*HlmsPbsDatablock *db = static_cast< HlmsPbsDatablock *>( it->getSubItem(0)->getDatablock() );
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
	if (db)
	{
		//  refl from presets
		auto s = sMtrName;
		auto is = s.find_last_of('_');  // drop _ter
		if (is != std::string::npos)
			s = s.substr(0,is);
		auto rd = pApp->scn->data->pre->GetRoad(s);
		if (!rd)
			LogO("Road mat error: not in presets: "+s);
		else
		if (rd->reflect && pApp->mCubeReflTex)
			db->setTexture( PBSM_REFLECTION, pApp->mCubeReflTex );  // wet, etc+
		// it->getSubItem(0)->setDatablock( db );
	}*/

	// LogO("Grid ok  " + sMesh + " " + db->getName().getReleaseText());  // hash

	// }

	// ?
	for (auto& l : lods)
		l.Clear();
}


void GridCellLods::Destroy()
{
	// LogO("Destroy Grid Cell " + mtr.name);
	if (!it || !pApp)  return;
	try
	{
		auto* mgr = pApp->mSceneMgr;
		auto& ms = MeshManager::getSingleton();
		auto& m1 = v1::MeshManager::getSingleton();

		auto& n = node;  if (n)  mgr->destroySceneNode(n);  n = 0;
		auto& i = it;    if (i)  mgr->destroyItem(i);  i = 0;
		auto& j = it2;   if (j)  mgr->destroyItem(j);  j = 0;
		auto& s = sMesh;
		if (!s.empty())
		{	String s1 = s+"v1", s2 = s+"v2";
			s = "";
			// ms.remove(s);  // already-
			ms.remove(s2);
			m1.remove(s1);
		}
	}
	catch (Exception ex)
	{
		LogO(String("# Error! Destroy Grid Cell ") + ex.what());
	}

	for (auto& l : lods)
		l.Clear();
}
