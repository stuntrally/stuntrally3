#include "OgreHlmsSamplerblock.h"
#include "pch.h"
#include "half.hpp"
#include "Def_Str.h"
#include "HudRenderable.h"
#include <OgreRoot.h>
#include <OgreCommon.h>
#include <OgreSceneManager.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreHlmsUnlitDatablock.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVertexElements.h>
#include <Vao/OgreBufferPacked.h>
using namespace Ogre;


//  🌟 ctor
//-----------------------------------------------------------------------------------
HudRenderable::HudRenderable(
	const String& material, SceneManager* mgr,
	Ogre::OperationType oper, bool hasUV, bool colors,
	uint32 vis, uint8 rndQue,
	int count,  // of lines or faces(quads)
	bool is3D)

	: MovableObject(
		Id::generateNewId<MovableObject>(),
		&mgr->_getEntityMemoryManager( SCENE_DYNAMIC ),
		mgr, rndQue)
	, Renderable()
	, sMtr(material), mOper(oper)
	, bUV(hasUV), bColors(colors), b3D(is3D)
{
	switch (oper)
	{
	case OT_LINE_LIST:   //  no UV, lines count (2verts per line)
		iVertCount = count * 2;  // lines count * 2 verts
		iFace      = 2;
		iIndCount  = count * iFace;
		break;
	case OT_LINE_STRIP:  //  no UV
		iVertCount = count;  // lines count + 1 vert
		iFace      = 0;  // no index buffer
		iIndCount  = 0;
		break;
	case OT_TRIANGLE_LIST:  //  count faces (2tris), has tex UV
		iVertCount = count * 4;  // 4 verts per face
		iFace      = 6;  // 6 indices per face
		iIndCount  = count * iFace;
		break;
	default:
		LogO("HudR ERROR: Unsupported operation type! For material: " + material);
		break;
	}
	//  log info  ----
	LogO("HudR New  material: " + material + "  " +
		(oper == OT_TRIANGLE_LIST ? "Tri_List" :
		 oper == OT_LINE_LIST ? "Line_List" : "Line_Strip") +
		"  count: " + toStr(count));

	if (!is3D)
	{	setUseIdentityProjection(true);
		setUseIdentityView(true);
	}
	setVisibilityFlags(vis);
	setCastShadows(false);

	Aabb aabb( Aabb::BOX_INFINITE );
	mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
	mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
	mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();
	mObjectData.mWorldRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();

	createBuffers(count);

	mRenderables.push_back( this );

 	//  set clamp
 	auto hlms = Root::getSingleton().getHlmsManager()->getHlms( HLMS_UNLIT );
	HlmsUnlitDatablock* db = (HlmsUnlitDatablock*)hlms->getDatablock( material );
	HlmsSamplerblock sb;
	sb.mU = TAM_CLAMP;  sb.mV = TAM_CLAMP;  sb.mW = TAM_CLAMP;
	db->setSamplerblock( 0, sb );

	setDatablock( db );
}

//  💥 dtor
//-----------------------------------------------------------------------------------
HudRenderable::~HudRenderable()
{
	_setNullDatablock();
	destroy();  //
	
	VaoManager *mgr = mManager->getDestinationRenderSystem()->getVaoManager();

	for (auto itor = mVaoPerLod[0].begin();
		itor != mVaoPerLod[0].end(); ++itor)
	{
		VertexArrayObject *vao = *itor;

		const auto &vbs = vao->getVertexBuffers();
		for (auto it = vbs.begin(); it != vbs.end(); ++it)
			mgr->destroyVertexBuffer( *it );  // vb

		if (vao->getIndexBuffer())  // ib
			mgr->destroyIndexBuffer( vao->getIndexBuffer() );
		
		mgr->destroyVertexArrayObject( vao );
	}
}

//  🆕 Create
//-----------------------------------------------------------------------------------
void HudRenderable::createBuffers(const int count)
{
	VaoManager *mgr = mManager->getDestinationRenderSystem()->getVaoManager();
	IndexBufferPacked *ib = 0;

	//  Index buffer  -------
	if (iFace > 0)
	{
		uint16 *indices = reinterpret_cast<uint16 *>(
			OGRE_MALLOC_SIMD( sizeof( uint16 ) * iFace * count, MEMCATEGORY_GEOMETRY ) );

		if (mOper == OT_TRIANGLE_LIST)
		{
			const uint16 c_indexData[/*iFace*/ 6] = {
				0,1,3, 2,3,0  };

			for (int f = 0; f < count; ++f)
			for (int i = 0; i < iFace; ++i)
				indices[f * iFace + i] = c_indexData[i] + f * 4;
		}
		else if (mOper == OT_LINE_LIST)
		{
			for (int f = 0; f < count; ++f)
			for (int i = 0; i < 2; ++i)
				indices[f * 2 + i] = i + f * 2;
		}
		
		try
		{	ib = mgr->createIndexBuffer(
				IndexBufferPacked::IT_16BIT, iFace * count,
				BT_IMMUTABLE, indices, true );
		}
		catch( Exception &e )
		{
			OGRE_FREE_SIMD( ib, MEMCATEGORY_GEOMETRY );
			ib = 0;  throw e;
		}
	}

	//  Vertex declaration
	VertexElement2Vec vertexElements;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );
	if (bUV)
		vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES ) );
	if (bColors)
		vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );


	//  Vertex buffer  -------
	vb = 0;
	try
	{	vb = mgr->createVertexBuffer(
			vertexElements, iVertCount, BT_DYNAMIC_PERSISTENT, 0, false );
	}
	catch( Exception &e )
	{
		OGRE_FREE_SIMD( vb, MEMCATEGORY_GEOMETRY );
		vb = 0;  throw e;
	}
	
	//  Vao
	VertexBufferPackedVec vertexBuffers;
	vertexBuffers.push_back( vb );
	VertexArrayObject *vao = mgr->createVertexArrayObject(
		vertexBuffers, ib, mOper );

	mVaoPerLod[0].push_back( vao );
	// mVaoPerLod[1].push_back( vao );  // same for shadow-
}


//  💫 Update
//-----------------------------------------------------------------------------------

//  maps all verts
void HudRenderable::begin()
{
	//if( !vb || vb->getMappingState() == MS_MAPPED )
	//	return;

	vp = reinterpret_cast<float * RESTRICT_ALIAS>(
		vb->map( 0, vb->getNumElements() ) );
	iVertCur = 0;
}

void HudRenderable::position(float x, float y, float z)
{
	if (iVertCur >= iVertCount)  return;

	vp[0] = x;  vp[1] = y;  vp[2] = z;
	vp += 3;
	++iVertCur;
}
void HudRenderable::texUV(float u, float v)
{
	if (!bUV)  return;

	vp[0] = u;  vp[1] = v;
	vp += 2;
}

void HudRenderable::color(float r, float g, float b, float a)
{
	if (!bColors)  return;

	vp[0] = r;  vp[1] = g;  vp[2] = b;  vp[3] = a;
	vp += 4;
}
void HudRenderable::color(ColourValue c)
{
	if (!bColors)  return;

	vp[0] = c.r;  vp[1] = c.g;  vp[2] = c.b;  vp[3] = c.a;
	vp += 4;
}

//  end map
void HudRenderable::end()
{
	//if( !vb || vb->getMappingState() == MS_UNMAPPED )
	//	return;

	//  fill rest if needed, to not have garbage
	if (iVertCur < iVertCount)
	for (int i = iVertCur; i < iVertCount; ++i)
	{
		position(0.f, -1000.f, 0.f);  // whatever
		if (bUV)  texUV(0.f, 0.f);
		if (bColors)  color(0.f, 0.f, 0.f, 0.f);
	}
	//  check if same count as in ctor
	if (iVertCur != iVertCount)
		LogO("HudR Error: Wrong vertices: " +
			toStr(iVertCur) + " expected: " + toStr(iVertCount) +
			"  mtr: " + sMtr);

	vb->unmap( UO_KEEP_PERSISTENT );
}


//-----------------------------------------------------------------------------------
void HudRenderable::destroy()
{
	//  Permanently unmap persistent mapped buffers
	if( vb && vb->getMappingState() != MS_UNMAPPED )
		vb->unmap( UO_UNMAP_ALL );
}


//-----------------------------------------------------------------------------------
const String &HudRenderable::getMovableType() const
{   return BLANKSTRING;  }

const LightList &HudRenderable::getLights() const
{
	return queryLights();  // from MovableObject base
}

void HudRenderable::getRenderOperation( v1::RenderOperation &op, bool casterPass )
{
	//op = mOper;
	OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "HudRenderable::getRenderOperation" );
}

void HudRenderable::getWorldTransforms( Matrix4 *xform ) const
{
	OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "HudRenderable::getWorldTransforms" );
}

bool HudRenderable::getCastsShadows() const
{
	//return false;
	OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "HudRenderable::getCastsShadows" );
}
