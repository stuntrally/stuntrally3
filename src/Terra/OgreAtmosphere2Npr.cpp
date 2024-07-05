/*
-----------------------------------------------------------------------------
This source file is part of OGRE-Next
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

#include <OgreAtmosphere2Npr.h>

#include "CommandBuffer/OgreCbShaderBuffer.h"
#include "CommandBuffer/OgreCommandBuffer.h"
#include <OgreCamera.h>
#include <OgreHlms.h>
#include <OgreLogManager.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreRectangle2D2.h>
#include <OgreSceneManager.h>
#include <OgreShaderPrimitives.h>
#include <OgreTechnique.h>
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreVaoManager.h"

namespace Ogre
{
    struct AtmoSettingsGpu  // same as AtmoSettings
    {
        float densityCoeff;  // not used.. remove?
        float lightDensity;
        float sunHeight;
        float sunHeightWeight;

        float4 skyLightAbsorption;
        float4 sunAbsorption;
        float4 cameraDisplacement;
        float4 packedParams1;
        float4 packedParams2;
        float4 packedParams3;

        float fogDensity;
        float fogBreakMinBrightness;
        float fogBreakFalloff;
        
		//**  fog new, SR3 added
   		float fogStartDistance;
        float4 fogHcolor;
        float4 fogHparams;
        float4 fogColourSun;
        float4 fogColourAway;
        float4 fogFluidH;
        float4 fogFluidClr;

		//**  new other
		float globalTime;  // for water, grass, wind etc

		float4 posSph0;   // grass deform, 2 collision spheres pos,r^2
		float4 posSph1;   //    for 1 car only  // todo: splitscreen meh-

		// float2 windDir;   // x,z  new ..
		// float windSpeed;  // freq, amp, turbulence-
    };

    Atmosphere2Npr::Atmosphere2Npr( VaoManager *vaoManager ) :
        mSunDir( Ogre::Vector3( 0, 1, 1 ).normalisedCopy() ),
        mNormalizedTimeOfDay( std::asin( mSunDir.y ) ),
        mSceneManager(0),
        mAtmosphereSeaLevel( 0.0f ),
        mAtmosphereHeight( 110.0f * 1000.0f ),  // in meters (actually in units)
        mHlmsBuffer( 0 ),
        mVaoManager( vaoManager )
    {
        mHlmsBuffer = vaoManager->createConstBuffer( sizeof( AtmoSettingsGpu ), BT_DEFAULT, 0, false );
    }
    //-------------------------------------------------------------------------
    Atmosphere2Npr::~Atmosphere2Npr()
    {
        mSceneManager->_setAtmosphere( nullptr );

        mVaoManager->destroyConstBuffer( mHlmsBuffer );
        mHlmsBuffer = 0;
    }
    
    //-------------------------------------------------------------------------
    void Atmosphere2Npr::setSky( Ogre::SceneManager *sceneManager, bool bEnabled )
    {
        mSceneManager = sceneManager;

        if( bEnabled )
            sceneManager->_setAtmosphere( this );
        else
            sceneManager->_setAtmosphere( nullptr );
    }

    //-------------------------------------------------------------------------
    void Atmosphere2Npr::destroySky( Ogre::SceneManager *sceneManager )
    {

    }

  
    //-------------------------------------------------------------------------
    void Atmosphere2Npr::_update( SceneManager *sceneManager, Camera *camera )
    {
        // const Vector3 *corners = camera->getWorldSpaceCorners();
        const Vector3 &cameraPos = camera->getDerivedPosition();

        Vector3 cameraDisplacement( Vector3::ZERO );
        {
            float camHeight = cameraPos[1];
            camHeight *= 1.0f;
            camHeight = ( camHeight - mAtmosphereSeaLevel ) / mAtmosphereHeight;
            cameraDisplacement[1] = camHeight;
        }

        AtmoSettingsGpu atmoGpu;

        const float sunHeight = std::sin( mNormalizedTimeOfDay * Math::PI );
        const float sunHeightWeight = std::exp2( -1.0f / sunHeight );
        // Gets smaller as sunHeight gets bigger
        const float lightDensity =
            mPreset.densityCoeff / std::pow( std::max( sunHeight, 0.0035f ), 0.75f );

        atmoGpu.densityCoeff = mPreset.densityCoeff;
        atmoGpu.lightDensity = lightDensity;
        atmoGpu.sunHeight = sunHeight;
        atmoGpu.sunHeightWeight = sunHeightWeight;

        const Vector3 mieAbsorption =
            std::pow( std::max( 1.0f - lightDensity, 0.1f ), 4.0f ) *
            Math::lerp( mPreset.skyColour, Vector3::UNIT_SCALE, sunHeightWeight );
        const float finalMultiplier = ( 0.5f + Math::smoothstep( 0.02f, 0.4f, sunHeightWeight ) ) * mPreset.skyPower;
        const Vector4 packedParams1( mieAbsorption, finalMultiplier );
        const Vector4 packedParams2( mSunDir, mPreset.horizonLimit );
        const Vector4 packedParams3( mPreset.skyColour, mPreset.densityDiffusion );

        atmoGpu.skyLightAbsorption = Vector4( 1.f,1.f,1.f, cameraPos.x );
        atmoGpu.sunAbsorption =      Vector4( 1.f,1.f,1.f, cameraPos.y );
        atmoGpu.cameraDisplacement = Vector4( cameraDisplacement, cameraPos.z );
        atmoGpu.packedParams1 = packedParams1;
        atmoGpu.packedParams2 = packedParams2;
        atmoGpu.packedParams3 = packedParams3;

        atmoGpu.fogDensity = mPreset.fogDensity;
        atmoGpu.fogBreakMinBrightness = mPreset.fogBreakMinBrightness * mPreset.fogBreakFalloff;
        atmoGpu.fogBreakFalloff = -mPreset.fogBreakFalloff;

        //**  fog new
        atmoGpu.fogStartDistance = mPreset.fogStartDistance;
        atmoGpu.fogHcolor = mPreset.fogHcolor;
        atmoGpu.fogHparams = mPreset.fogHparams;
        atmoGpu.fogColourSun = mPreset.fogColourSun;
        atmoGpu.fogColourAway = mPreset.fogColourAway;
        atmoGpu.fogFluidH   = fogFluidH;
        atmoGpu.fogFluidClr = fogFluidClr;

        atmoGpu.globalTime  = globalTime;

        atmoGpu.posSph0 = posSph0;
        atmoGpu.posSph1 = posSph1;

        mHlmsBuffer->upload( &atmoGpu, 0u, sizeof( atmoGpu ) );
    }
    
    //-------------------------------------------------------------------------
    uint32 Atmosphere2Npr::preparePassHash( Hlms *hlms, size_t constBufferSlot )
    {
        hlms->_setProperty( HlmsBaseProp::Fog, 1 );
        hlms->_setProperty( "atmosky_npr", int32( constBufferSlot ) );
        return 1u;
    }

    //-------------------------------------------------------------------------
    uint32 Atmosphere2Npr::getNumConstBuffersSlots() const
    {
        return 1u;
    }

    //-------------------------------------------------------------------------
    uint32 Atmosphere2Npr::bindConstBuffers( CommandBuffer *commandBuffer, size_t slotIdx )
    {
        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(
            VertexShader, uint16( slotIdx ), mHlmsBuffer, 0, (uint32)mHlmsBuffer->getTotalSizeBytes() );
        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(
            PixelShader, uint16( slotIdx ), mHlmsBuffer, 0, (uint32)mHlmsBuffer->getTotalSizeBytes() );

        return 1u;
    }
    
}  // namespace Ogre
