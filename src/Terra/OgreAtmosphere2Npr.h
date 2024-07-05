/*
Modified by CryHam  under GPLv3
-----------------------------------------------------------------------------
This source file WAS part of OGRE-Next
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#pragma once

#include "OgreVector4.h"
#include <OgreAtmosphere2Prerequisites.h>

#include <OgreAtmosphereComponent.h>
#include <OgreColourValue.h>
#include <OgreSharedPtr.h>
#include <OgreVector3.h>

#include <map>

#include <OgreHeaderPrefix.h>

namespace Ogre
{
    OGRE_ASSUME_NONNULL_BEGIN

    class Atmosphere2Npr final : public AtmosphereComponent
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
        Vector4 fogFluidH{-1900.f, 1.f/17.f, 0.15f, 0};
        Vector4 fogFluidClr{0,0,0,0};

        Vector4 posSph0{0,0,-500,1}, posSph1{0,0,-500,1};

    protected:

        /// Contains all settings in a GPU buffer for Hlms to consume
        ConstBufferPacked *mHlmsBuffer;
        VaoManager        *mVaoManager;

        Ogre::SceneManager * mSceneManager;

    public:
        Atmosphere2Npr( VaoManager *vaoManager, Ogre::SceneManager *sceneManager );
        ~Atmosphere2Npr() override;

        Preset &getPreset() { return mPreset; }


        void _update( SceneManager *sceneManager, Camera *camera ) override;

        uint32 preparePassHash( Hlms *hlms, size_t constBufferSlot ) override;

        uint32 getNumConstBuffersSlots() const override;

        uint32 bindConstBuffers( CommandBuffer *commandBuffer, size_t slotIdx ) override;

    };

    OGRE_ASSUME_NONNULL_END

}  // namespace Ogre

#include <OgreHeaderSuffix.h>

