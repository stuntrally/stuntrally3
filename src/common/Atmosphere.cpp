#include "pch.h"
#include "Atmosphere.h"
#include "CommandBuffer/OgreCbShaderBuffer.h"
#include "CommandBuffer/OgreCommandBuffer.h"
#include <OgreCamera.h>
#include <OgreHlms.h>
#include <OgrePass.h>
#include <OgreSceneManager.h>
#include <OgreShaderPrimitives.h>
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreVaoManager.h"
using namespace Ogre;


    //  🌠 for all shaders in atmo.
    struct AtmoSettingsGpu  // same as AtmoSettings
    {
        float4 cameraDisplacement;  // padding

        float fogDensity;
        float fogBreakMinBrightness;
        float fogBreakFalloff;
        
		//**  fog new
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


    //  🌟 ctor
    //-------------------------------------------------------------------------
    Atmosphere::Atmosphere( VaoManager *vaoManager, SceneManager *sceneManager ) :
        mSceneManager(sceneManager),
        mHlmsBuffer( 0 ),
        mVaoManager( vaoManager )
    {
        mHlmsBuffer = vaoManager->createConstBuffer( sizeof( AtmoSettingsGpu ), BT_DEFAULT, 0, false );
        sceneManager->_setAtmosphere( this );
    }

    Atmosphere::~Atmosphere()
    {
        mSceneManager->_setAtmosphere( nullptr );

        mVaoManager->destroyConstBuffer( mHlmsBuffer );
        mHlmsBuffer = 0;
    }
    

    //  💫 Update
    //-------------------------------------------------------------------------
    void Atmosphere::_update( SceneManager *sceneManager, Camera *camera )
    {
        const Vector3 &cameraPos = camera->getDerivedPosition();

        AtmoSettingsGpu atmoGpu;

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


    //  🟢 ogre Update
    //-------------------------------------------------------------------------
    uint32 Atmosphere::preparePassHash( Hlms *hlms, size_t constBufferSlot )
    {
        hlms->_setProperty( HlmsBaseProp::Fog, 1 );
        hlms->_setProperty( "atmosky_npr", int32( constBufferSlot ) );
        return 1u;
    }

    uint32 Atmosphere::getNumConstBuffersSlots() const
    {
        return 1u;
    }

    uint32 Atmosphere::bindConstBuffers( CommandBuffer *commandBuffer, size_t slotIdx )
    {
        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(
            VertexShader, uint16( slotIdx ), mHlmsBuffer, 0, (uint32)mHlmsBuffer->getTotalSizeBytes() );
        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(
            PixelShader, uint16( slotIdx ), mHlmsBuffer, 0, (uint32)mHlmsBuffer->getTotalSizeBytes() );

        return 1u;
    }
