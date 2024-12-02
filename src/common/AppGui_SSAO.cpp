#include "pch.h"
#include "App.h"
#include "settings.h"
#include "SceneXml.h"
#include "GraphicsSystem.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>

#include <OgreHlmsManager.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorNode.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>

#include <OgreItem.h>
#include <OgreVector2.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreRenderable.h>
#include <OgreBitwise.h>
#include <OgreStagingTexture.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
using namespace Ogre;


//  🕳️ SSAO
//--------------------------------------------------------------------------------------------------------------
void AppGui::InitSSAO()
{
	//  Create SSAO kernel samples
	float kernelSamples[64][4];
	for (size_t i = 0; i < 64u; ++i)
	{
		// Vector3 sample( 10, 10, 10 );
		// while( sample.squaredLength() > 1.0f )
		// {
		//     sample = Vector3( Math::RangeRandom(  -1.0f, 1.0f ),
		//                       Math::RangeRandom(  -1.0f, 1.0f ),
		//                       Math::RangeRandom(  0.01f, 1.0f ) );
		////     sample = Vector3( Math::RangeRandom(  -0.1f, 0.1f ),
		////                       Math::RangeRandom(  -0.1f, 0.1f ),
		////                       Math::RangeRandom(  0.5f, 1.0f ) );
		// }
		Vector3 sample = Vector3( Math::RangeRandom( -1.0f, 1.0f ),
								Math::RangeRandom( -1.0f, 1.0f ),
								Math::RangeRandom( 0.0f, 1.0f ) );
		sample.normalise();

		float scale = (float)i / 64.0f;
		scale = Math::lerp( 0.3f, 1.0f, scale * scale );
		sample = sample * scale;

		kernelSamples[i][0] = sample.x;
		kernelSamples[i][1] = sample.y;
		kernelSamples[i][2] = sample.z;
		kernelSamples[i][3] = 1.0f;
	}

	//  Generate noise texture
	Root *root = mGraphicsSystem->getRoot();
	TextureGpuManager *textureManager = root->getRenderSystem()->getTextureGpuManager();
	TextureGpu *noiseTexture = textureManager->createTexture(
		"noiseTexture", GpuPageOutStrategy::SaveToSystemRam, 0, TextureTypes::Type2D );
	noiseTexture->setResolution( 2u, 2u );
	noiseTexture->setPixelFormat( PFG_RGBA8_SNORM );
	noiseTexture->_transitionTo( GpuResidency::Resident, (uint8 *)0 );
	noiseTexture->_setNextResidencyStatus( GpuResidency::Resident );

	StagingTexture *stagingTexture =
		textureManager->getStagingTexture( 2u, 2u, 1u, 1u, PFG_RGBA8_SNORM );
	stagingTexture->startMapRegion();
	TextureBox texBox = stagingTexture->mapRegion( 2u, 2u, 1u, 1u, PFG_RGBA8_SNORM );

	for (size_t j = 0; j < texBox.height; ++j)
	{
		for (size_t i = 0; i < texBox.width; ++i)
		{
			Vector3 noise = Vector3( Math::RangeRandom( -1.0f, 1.0f ),
									Math::RangeRandom( -1.0f, 1.0f ), 0.0f );
			noise.normalise();

			int8 *pixelData = reinterpret_cast<int8 *>( texBox.at( i, j, 0 ) );
			pixelData[0] = Bitwise::floatToSnorm8( noise.x );
			pixelData[1] = Bitwise::floatToSnorm8( noise.y );
			pixelData[2] = Bitwise::floatToSnorm8( noise.z );
			pixelData[3] = Bitwise::floatToSnorm8( 1.0f );
		}
	}

	stagingTexture->stopMapRegion();
	stagingTexture->upload( texBox, noiseTexture, 0, 0, 0 );
	textureManager->removeStagingTexture( stagingTexture );
	stagingTexture = 0;


	//---------------------------------------------------------------------------------
	//  Set uniforms
	MaterialPtr material =
		std::static_pointer_cast<Material>( MaterialManager::getSingleton().load(
			"SSAO/HS", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ) );

	Pass *pass = material->getTechnique( 0 )->getPass( 0 );
	mSSAOPass = pass;
	GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

	TextureUnitState *noiseTextureState = pass->getTextureUnitState( "noiseTexture" );
	noiseTextureState->setTexture( noiseTexture );

	//  Reconstruct position from depth. Position is needed in SSAO
	//  We need to set the parameters based on camera to the
	//  shader so that the un-projection works as expected
	Camera *camera = mCamera;  //mGraphicsSystem->getCamera();
	Vector2 projectionAB = camera->getProjectionParamsAB();
	//  The division will keep "linearDepth" in the shader in the [0; 1] range.
	projectionAB.y /= camera->getFarClipDistance();
	psParams->setNamedConstant( "projectionParams", projectionAB );

	//  other uniforms
	psParams->setNamedConstant( "kernelRadius", pSet->ssao_radius );
	psParams->setNamedConstant(
		"noiseScale",
		Vector2(
			( Real( mGraphicsSystem->getRenderWindow()->getWidth() ) * 0.5f ) / 2.0f,
			( Real( mGraphicsSystem->getRenderWindow()->getHeight() ) * 0.5f ) / 2.0f ) );
	psParams->setNamedConstant( "invKernelSize", 1.0f / 64.0f );
	psParams->setNamedConstant( "sampleDirs", (float *)kernelSamples, 64, 4 );

	//  blur shader uniforms
	MaterialPtr materialBlurH =
		std::static_pointer_cast<Material>( MaterialManager::getSingleton().load(
			"SSAO/BlurH", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ) );

	Pass *passBlurH = materialBlurH->getTechnique( 0 )->getPass( 0 );
	GpuProgramParametersSharedPtr psParamsBlurH = passBlurH->getFragmentProgramParameters();
	psParamsBlurH->setNamedConstant( "projectionParams", projectionAB );

	MaterialPtr materialBlurV =
		std::static_pointer_cast<Material>( MaterialManager::getSingleton().load(
			"SSAO/BlurV", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ) );

	Pass *passBlurV = materialBlurV->getTechnique( 0 )->getPass( 0 );
	GpuProgramParametersSharedPtr psParamsBlurV = passBlurV->getFragmentProgramParameters();
	psParamsBlurV->setNamedConstant( "projectionParams", projectionAB );

	//  apply shader uniforms
	MaterialPtr materialApply =
		std::static_pointer_cast<Material>( MaterialManager::getSingleton().load(
			"SSAO/Apply", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ) );

	Pass *passApply = materialApply->getTechnique( 0 )->getPass( 0 );
	mApplyPass = passApply;
	GpuProgramParametersSharedPtr psParamsApply = passApply->getFragmentProgramParameters();
	psParamsApply->setNamedConstant( "powerScale", pSet->ssao_scale );
}


//  🕳️💫 Update
//-----------------------------------------------------------------------------------
void AppGui::UpdSSAO(Camera* camera)
{
	GpuProgramParametersSharedPtr psParams = mSSAOPass->getFragmentProgramParameters();
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
	//  We don't render to render window directly, thus we need to get the projection
	//  matrix with phone orientation disable when calculating SSAO
	camera->setOrientationMode( OR_DEGREE_0 );
#endif
	psParams->setNamedConstant( "projection", camera->getProjectionMatrix() );
	psParams->setNamedConstant( "kernelRadius", pSet->ssao_radius );

	GpuProgramParametersSharedPtr psParamsApply = mApplyPass->getFragmentProgramParameters();
	psParamsApply->setNamedConstant( "powerScale", pSet->ssao_scale );
}


//  🔆 Lens flare
//--------------------------------------------------------------------------------------------------------------
void AppGui::InitLens()
{
	MaterialPtr material =
		std::static_pointer_cast<Material>( MaterialManager::getSingleton().load(
			"LensFlare", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ) );

	mLensPass = material->getTechnique(0)->getPass(0);
	// GpuProgramParametersSharedPtr psParams = mLensPass->getFragmentProgramParameters();
}

void AppGui::UpdLens(Camera *camera)
{
	if (!mLensPass || !camera)  return;
	GpuProgramParametersSharedPtr psParams = mLensPass->getFragmentProgramParameters();

	Vector3 p = scn->sunDir * -10000.f;
	p.y = -p.y;
	// Vector4 pos{p.x, p.y, p.z, 1.f};
	// camera->projectSphere()
	// pos = pos * camera->getViewMatrix() * camera->getProjectionMatrix();

	Vector3 pos2D = camera->getProjectionMatrix() * (camera->getViewMatrix() * p);
	// Real x =  pos2D.x * 0.5f + 0.5f;
	// Real y = -pos2D.y * 0.5f + 0.5f;
	// Vector2 uv(x,y);
	Vector2 uv(pos2D.x, pos2D.y);
/*
	// pos.x /= mWindow->getWidth();
	// pos.y /= mWindow->getHeight();
*/
/*
	// get camera and light world positions
	Vector3 lightPosition_ws = p;  //lightNode()->_getDerivedPosition();
	Vector3 cameraPosition_ws = camera->getDerivedPosition();
	// compute distance
	float lc_distance_ws = cameraPosition_ws.distance(lightPosition_ws);

	// compute geometry vectors update
	Matrix4 vm = camera->getViewMatrix();
	// light position in camera space
	Vector3 lightPosition = vm * lightPosition_ws;

	Vector2 uv(lightPosition.x, lightPosition.y);
	
	// camera position in camera space
	Vector3 cameraPosition = Vector3(0,0,0);
	Vector3 cameraDirection = camera->getDirection(); // should be (0 0 -1)
	// camera to light vector
	Vector3 light_vector = lightPosition - cameraPosition;
	light_vector.normalise();
*/
/*
	Real LightDistance  = p.distance(mCamera->getPosition());
	Vector3 CameraVect  = camera->getDirection();  // normalized vector (length 1)

	CameraVect = camera->getPosition() + (LightDistance * CameraVect);

	// The LensFlare effect takes place along this vector.
	Vector3 LFvect = (CameraVect - p);

	Vector2 uv(LFvect.x, LFvect.y);
*/
	// static float ti = 0.f;  // test
	// ti += 0.016f* 0.1f;
	// Vector2 uv(cosf(ti)*cosf(ti*1.3f)*0.5f, sinf(ti)*0.5f);
	// psParams->setNamedConstant( "uvSunPos", uv );
	LogO(toStr(uv));
	psParams->setNamedConstant( "uvSunPos", uv );
}


//  🌄 Sun beams
//--------------------------------------------------------------------------------------------------------------
void AppGui::InitSunbeams()
{
	MaterialPtr material =
		std::static_pointer_cast<Material>( MaterialManager::getSingleton().load(
			"LensFlare", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ) );

	mLensPass = material->getTechnique(0)->getPass(0);
	// GpuProgramParametersSharedPtr psParams = mLensPass->getFragmentProgramParameters();
}

void AppGui::UpdSunbeams(Camera *cam)
{

}


//  All post effects
//--------------------------------------------------------------------------------------------------------------
void AppGui::InitEffects()
{
	InitSSAO();
	InitLens();
	InitSunbeams();
}

void AppGui::UpdateEffects(Camera *cam)
{
	if (pSet->g.ssao)  //- in ReflectListener::passEarlyPreExecute too
		UpdSSAO(cam);  // 🕳️
	
	if (pSet->g.lens_flare)
		UpdLens(cam);  // 🔆
	
	if (pSet->g.sunbeams)
		UpdSunbeams(cam);  // 🌄
	
	if (pSet->gi)
		UpdateGI();  // 🌇
}
