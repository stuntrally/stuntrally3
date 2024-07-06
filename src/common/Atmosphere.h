#pragma once
#include <OgreVector4.h>
#include <OgreAtmosphereComponent.h>
#include <OgreColourValue.h>
#include <OgreSharedPtr.h>
#include <OgreVector3.h>
#include <OgreHeaderPrefix.h>
#include <map>


class Atmosphere final : public Ogre::AtmosphereComponent
{
public:
    struct Preset
    {
        /// Affects objects' fog (not sky)
        float fogDensity;
        /// Very bright objects (i.e. reflect lots of light)
        /// manage to "breakthrough" the fog.
        ///
        /// A value of fogBreakMinBrightness = 3 means that pixels
        /// that have a luminance of >= 3 (i.e. in HDR) will start
        /// becoming more visible
        ///
        /// Range: [0; inf)
        float fogBreakMinBrightness;
        /// How fast bright objects appear in fog.
        /// Small values make objects appear very slowly after luminance > fogBreakMinBrightness
        /// Large values make objects appear very quickly
        ///
        /// Range: (0; inf)
        float fogBreakFalloff;

        //**  fog new  ----
        float fogStartDistance;
        Ogre::Vector4 fogHcolor, fogHparams;
        Ogre::Vector4 fogColourSun;
        Ogre::Vector4 fogColourAway;
        Ogre::Vector4 fogFluidH;
        Ogre::Vector4 fogFluidClr;

        Ogre::Vector4 posSph0;   // grass deform 2 spheres pos,R^2
        Ogre::Vector4 posSph1;

        Preset() :               
            fogDensity( 0.0001f ),
            fogBreakMinBrightness( 0.25f ),
            fogBreakFalloff( 0.1f ),
            
            //**  fog new  ----
            fogStartDistance( 0.0f ),
            fogHcolor(0.5,0.75,1,1),
            fogHparams(11,0.2,0,0),
            fogColourSun( 0.99f, 0.99f, 0.98f, 1.f ),
            fogColourAway( 0.74f, 0.87f, 1.0f, 1.f ),
            fogFluidH(-1900.f, 1.f/17.f, 0.15f, 0 ),
            fogFluidClr( 0.5f, 0.6f, 0.7f, 0.f),

            posSph0(0,0,500,-1),
            posSph1(0,0,500,-1)
        {
        }
    };

protected:
    Preset  mPreset;

public:
    //**  new
    float globalTime = 0.f;
    Ogre::Vector4 fogFluidH{-1900.f, 1.f/17.f, 0.15f, 0};
    Ogre::Vector4 fogFluidClr{0,0,0,0};

    Ogre::Vector4 posSph0{0,0,-500,1}, posSph1{0,0,-500,1};

protected:
    ///  Contains all settings in a GPU buffer for Hlms shaders
    Ogre::ConstBufferPacked *mHlmsBuffer;
    Ogre::VaoManager        *mVaoManager;

    Ogre::SceneManager *mSceneManager;

public:
    Atmosphere( Ogre::VaoManager *vaoManager, Ogre::SceneManager *sceneManager );
    ~Atmosphere() override;

    Preset &getPreset() {  return mPreset;  }


    void _update( Ogre::SceneManager *sceneManager, Ogre::Camera *camera ) override;

    Ogre::uint32 preparePassHash( Ogre::Hlms *hlms, size_t constBufferSlot ) override;

    Ogre::uint32 getNumConstBuffersSlots() const override;

    Ogre::uint32 bindConstBuffers( Ogre::CommandBuffer *commandBuffer, size_t slotIdx ) override;

};
