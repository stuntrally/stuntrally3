#include "pch.h"
#include "HudRenderable.h"
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVertexElements.h>
using namespace Ogre;


//-----------------------------------------------------------------------------------
HudRenderable::HudRenderable(
    const String& material, SceneManager* mgr,
    bool colors,
    uint32 vis, uint8 rndQue,
    int faces)

    : MovableObject(
        Id::generateNewId<MovableObject>(),
        &mgr->_getEntityMemoryManager( SCENE_DYNAMIC ),
        mgr, rndQue)
    , Renderable()
{
    setUseIdentityProjection(true);
    setUseIdentityView(true);
    setVisibilityFlags(vis);

    Aabb aabb( Aabb::BOX_INFINITE );
    mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
    mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
    mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();
    mObjectData.mWorldRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();

    createBuffers(colors, faces);

    mRenderables.push_back( this );

    auto hlms = Root::getSingleton().getHlmsManager()->getHlms( HLMS_UNLIT );
    auto db = hlms->getDatablock( material );
    setDatablock( db );
}

//-----------------------------------------------------------------------------------
HudRenderable::~HudRenderable()
{
    destroy();
    
    VaoManager *mgr = mManager->getDestinationRenderSystem()->getVaoManager();

    auto itor = mVaoPerLod[0].begin();
    auto end = mVaoPerLod[0].end();
    while( itor != end )
    {
        VertexArrayObject *vao = *itor;

        const auto &vertexBuffers = vao->getVertexBuffers();
        auto itBuffers = vertexBuffers.begin();
        auto enBuffers = vertexBuffers.end();

        while( itBuffers != enBuffers )
        {
            mgr->destroyVertexBuffer( *itBuffers );
            ++itBuffers;
        }

        if( vao->getIndexBuffer() )
            mgr->destroyIndexBuffer( vao->getIndexBuffer() );
        mgr->destroyVertexArrayObject( vao );
        ++itor;
    }
}

//-----------------------------------------------------------------------------------
void HudRenderable::createBuffers(bool colors, const int faces)
{
    //  Index buffer  -------
    const int face = 6;
    const uint16 c_indexData[face] = {
        0,1,3, 2,3,0  };

    uint16 *indices = reinterpret_cast<uint16 *>(
        OGRE_MALLOC_SIMD( sizeof( uint16 ) * face * faces, MEMCATEGORY_GEOMETRY ) );

    for (int f = 0; f < faces; ++f)
    for (int i = 0; i < face; ++i)
        indices[f * 6 + i] = c_indexData[i] + f * 4;
    

    VaoManager *mgr = mManager->getDestinationRenderSystem()->getVaoManager();
    
    IndexBufferPacked *ib = 0;
    try
    {
        ib = mgr->createIndexBuffer(
            IndexBufferPacked::IT_16BIT, face * faces,
            BT_IMMUTABLE, indices, true );
    }
    catch( Exception &e )
    {
        OGRE_FREE_SIMD( ib, MEMCATEGORY_GEOMETRY );
        ib = 0;  throw e;
    }


    //  Vertex declaration
    VertexElement2Vec vertexElements;
    vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );
    vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES ) );
    if (colors)
        vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );


    //  Vertex buffer  -------
    const int verts = faces * 4;

    vb = 0;
    try
    {
        vb = mgr->createVertexBuffer(
            vertexElements, verts, BT_DYNAMIC_PERSISTENT, 0, false );
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
        vertexBuffers, ib, OT_TRIANGLE_LIST );

    mVaoPerLod[0].push_back( vao );
    mVaoPerLod[1].push_back( vao );  // same for shadow-
}


//-----------------------------------------------------------------------------------
//  maps all verts
void HudRenderable::begin()
{
    vp = reinterpret_cast<float * RESTRICT_ALIAS>(
        vb->map( 0, vb->getNumElements() ) );
}
void HudRenderable::position(float x, float y, float z)
{
    vp[0] = x;  vp[1] = y;  vp[2] = z;
    vp += 3;
}
void HudRenderable::textureCoord(float u, float v)
{
    vp[0] = u;  vp[1] = v;
    vp += 2;
}
void HudRenderable::color(float r, float g, float b, float a)
{
    vp[0] = r;  vp[1] = g;  vp[2] = b;  vp[3] = a;
    vp += 4;
}
void HudRenderable::end()
{
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
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "HudRenderable::getRenderOperation" );
}

void HudRenderable::getWorldTransforms( Matrix4 *xform ) const
{
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "HudRenderable::getWorldTransforms" );
}

bool HudRenderable::getCastsShadows() const
{
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "HudRenderable::getCastsShadows" );
}
