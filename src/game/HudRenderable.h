#pragma once
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
// #include <OgreVertexBufferPacked.h>
namespace Ogre {  class VertexBufferPacked;  }


class HudRenderable final
    : public Ogre::MovableObject, public Ogre::Renderable
{
public:
    HudRenderable(
        const Ogre::String& material, Ogre::SceneManager* mgr,
        bool colors,
        Ogre::uint32 vis, Ogre::uint8 rndQue,
        int faces);
    ~HudRenderable() override;

    //  from MovableObject
    const Ogre::String &getMovableType() const override;
    
    //  from Renderable
    const Ogre::LightList &getLights() const override;
    void getRenderOperation( Ogre::v1::RenderOperation &op, bool casterPass ) override;
    void getWorldTransforms( Ogre::Matrix4 *xform ) const override;
    bool getCastsShadows() const override;


    //  update
    Ogre::VertexBufferPacked *vb = 0;
    void createBuffers(bool colors=false, const int faces=1);
    void destroy();

    //  ManualObject syntax, update all verts
    float *RESTRICT_ALIAS vp = 0;
    void begin();
    void position(float x, float y, float z);
    void textureCoord(float u, float v);
    void color(float r, float g, float b, float a);
    void end();
};
