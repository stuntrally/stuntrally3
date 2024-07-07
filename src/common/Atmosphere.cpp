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
struct AtmoSettingsGpu  // same as AtmoSettings info in Atmosphere.h
{
	//**  fog
	float4 fogDensStartBreak;
	float4 fogHcolor, fogHparams;
	float4 fogColourSun, fogColourAway;
	float4 fogFluidH, fogFluidClr;

	//  grass deform 2 spheres pos,r^2
	float4 posSph0, posSph1;

	//  time, wind
	float4 timeWind, windDir;
};


//  🌟 ctor
//-------------------------------------------------------------------------
Atmosphere::Atmosphere( VaoManager* vaoMgr, SceneManager* sceneMgr )
	: mVaoMgr(vaoMgr), mSceneMgr(sceneMgr)
	, mHlmsBuffer(0)
{
	mHlmsBuffer = vaoMgr->createConstBuffer( sizeof(AtmoSettingsGpu), BT_DEFAULT, 0, false );
	sceneMgr->_setAtmosphere( this );
}

Atmosphere::~Atmosphere()
{
	mSceneMgr->_setAtmosphere( nullptr );

	mVaoMgr->destroyConstBuffer( mHlmsBuffer );
	mHlmsBuffer = 0;
}


//  💫 Update
//-------------------------------------------------------------------------
void Atmosphere::_update( SceneManager* sceneMgr, Camera* camera )
{
	const Vector3 &cameraPos = camera->getDerivedPosition();

	AtmoSettingsGpu atmoGpu;

	//**  fog
	atmoGpu.fogDensStartBreak.x = fogDensity;
	atmoGpu.fogDensStartBreak.y = fogStartDistance;
	atmoGpu.fogDensStartBreak.z = fogBreakMinBrightness * fogBreakFalloff;
	atmoGpu.fogDensStartBreak.w = -fogBreakFalloff;

	atmoGpu.fogHcolor  = fogHcolor;
	atmoGpu.fogHparams = fogHparams;
	atmoGpu.fogColourSun  = fogColourSun;
	atmoGpu.fogColourAway = fogColourAway;
	atmoGpu.fogFluidH   = fogFluidH;
	atmoGpu.fogFluidClr = fogFluidClr;

	//  grass deform
	atmoGpu.posSph0 = posSph0;
	atmoGpu.posSph1 = posSph1;
	
	//  wind
	atmoGpu.timeWind = timeWind;
	atmoGpu.windDir  = windDir;

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

uint32 Atmosphere::bindConstBuffers( CommandBuffer *commandBuffer, size_t slotIdx )
{
	*commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(
		VertexShader, uint16( slotIdx ), mHlmsBuffer, 0, (uint32)mHlmsBuffer->getTotalSizeBytes() );
	*commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer(
		PixelShader, uint16( slotIdx ), mHlmsBuffer, 0, (uint32)mHlmsBuffer->getTotalSizeBytes() );

	return 1u;
}

uint32 Atmosphere::getNumConstBuffersSlots() const
{
	return 1u;
}
