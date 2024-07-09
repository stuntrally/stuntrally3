#pragma once
#include <OgreVector4.h>
#include <OgreAtmosphereComponent.h>
#include <OgreColourValue.h>
#include <OgreSharedPtr.h>
#include <OgreVector3.h>
#include <OgreHeaderPrefix.h>
#include <map>


//  global shader params for all
class Atmosphere final : public Ogre::AtmosphereComponent
{
public:
    //**  🌫️ fogDensStartBreak
    float fogDensity = 0.0001f;    // .x
    float fogStartDistance = 0.f;  // .y
    // todo: par gui.. 
    // .z  Very bright objects (i.e. reflect lots of light) manage to break through fog
    float fogBreakMinBrightness = 0.25f;  // [0; inf)
    //. w  How fast bright objects appear in fog
    //  Small values make objects appear very slowly after luminance > fogBreakMinBrightness
    float fogBreakFalloff = 0.1f;  // (0; inf)

    Ogre::Vector4 fogHcolor{0.5,0.75,1,1}, fogHparams{11,0.2,0,0};
    Ogre::Vector4 fogColourSun{0.99f, 0.99f, 0.98f, 1.f};
    Ogre::Vector4 fogColourAway{0.74f, 0.87f, 1.0f, 1.f};
    Ogre::Vector4 fogFluidH{-1900.f, 1.f/17.f, 0.15f, 0};
    Ogre::Vector4 fogFluidClr{0.5f, 0.6f, 0.7f, 0.f};
    
    //  🌿 grass deform,  2 collision spheres: pos,R^2
    Ogre::Vector4 posSph0{0,0,-500,-11}, posSph1{0,0,-500,-11};
    
    //  time  🌪️ wind: y offset, z amplitude, w freq
    Ogre::Vector4 timeWind{0.f, 0.f, 0.f, 0.f};
    //  wind dir x,z
    Ogre::Vector4 windDir{1.f, 0.f, 0.f, 0.f};


protected:
    ///  Contains all settings in a GPU buffer for Hlms shaders
    Ogre::ConstBufferPacked* mHlmsBuffer;
    Ogre::VaoManager* mVaoMgr;
    Ogre::SceneManager* mSceneMgr;

public:
    Atmosphere(Ogre::VaoManager* vaoMgr, Ogre::SceneManager* sceneMgr);
    ~Atmosphere() override;

    void _update(Ogre::SceneManager* sceneMgr, Ogre::Camera* camera) override;

    Ogre::uint32 preparePassHash(Ogre::Hlms *hlms, size_t constBufferSlot) override;
    Ogre::uint32 bindConstBuffers(Ogre::CommandBuffer *commandBuffer, size_t slotIdx) override;
    Ogre::uint32 getNumConstBuffersSlots() const override;

};
