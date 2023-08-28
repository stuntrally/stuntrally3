#include "Def_Str.h"
#include "HlmsPbs2.h"
#include "App.h"
#include "CScene.h"
#include "OgreHlmsPbsTerraShadows.h"
#include <OgreShaderPrimitives.h>
using namespace Ogre;


//  our Unlit
//----------------------------------------------------------------
HlmsUnlit2::HlmsUnlit2( Archive *dataFolder, ArchiveVec *libraryFolders )
	:HlmsUnlit( dataFolder, libraryFolders )
{
	// mCurrentConstBufferSize
}
HlmsUnlit2::~HlmsUnlit2()
{
}

void HlmsUnlit2::calculateHashForPreCreate(
	Renderable *rnd, PiecesMap *inOut )
{
	HlmsUnlit::calculateHashForPreCreate( rnd, inOut );
	
	const auto& mtr = rnd->getDatablockOrMaterialName();
	// LogO("- calc unlit: " + mtr);   // on every item
	
	if (mtr == "ed_RoadDens")
		setProperty( "ed_RoadDens", 1 );
	else
	if (mtr == "ed_RoadPreview")
		setProperty( "ed_RoadPreview", 1 );
}


//  our Pbs
//----------------------------------------------------------------
HlmsPbs2::HlmsPbs2( Archive *dataFolder, ArchiveVec *libraryFolders )
	:HlmsPbs( dataFolder, libraryFolders )
{
}
HlmsPbs2::~HlmsPbs2()
{
}


void HlmsPbs2::calculateHashForPreCreate(
	Renderable *rnd, PiecesMap *inOut )
{
	HlmsPbs::calculateHashForPreCreate( rnd, inOut );
	
	const auto& mtr = rnd->getDatablockOrMaterialName();
	// LogO("- calc pbs: " + mtr);   // on every item
	
	if (mtr.substr(0,5) == "Water")
		setProperty( "water", 1 );
	else
	if (mtr.substr(0,5) == "River")
		setProperty( "river", 1 );
	else
	if (mtr.substr(0,5) == "grass")
		setProperty( "grass", 1 );
	else 
	if (mtr.substr(0,4) == "sky/")
		setProperty( "sky", 1 );

	/*if (mtr.find("body") != String::npos)
	{
		// LogO("body_paint");
		setProperty( "body_paint", 1 );
	}*/
	// if (mtr.find("road") != std::string::npos)
	// if (mtr.substr(0,4) == "road")
	// 	setProperty( "road", 1 );

#ifdef SR_EDITOR
    //  selected glow  for ed road segs, objects
	const auto &paramMap = rnd->getCustomParameters();
	auto itor = paramMap.find( selected_glow );
	if( itor != paramMap.end() )
		setProperty( "selected_glow", (int)itor->second.x );
#endif
}

void HlmsPbs2::calculateHashForPreCaster(
	Renderable *rnd, PiecesMap *inOut )
{
    HlmsPbs::calculateHashForPreCaster( rnd, inOut );

	const auto& mtr = rnd->getDatablockOrMaterialName();
	// LogO("- cast for: " + mtr);   // on every item

	if (mtr.substr(0,5) == "grass")
		setProperty( "grass", 1 );

	/*if (mtr.find("body") != String::npos)
	{
		// LogO("body_paint");
		setProperty( "body_paint", 1 );
	}*/
}


//  🆕 create Datablock2
//----------------------------------------------------------------
HlmsDatablock* HlmsPbs2::createDatablockImpl(
	IdString name,
	const HlmsMacroblock *macro,
	const HlmsBlendblock *blend,
	const HlmsParamVec &params )
{
#if 1
	String val;
	if( Hlms::findParamInVec( params, "fluid", val ) )
	{
		// LogO("paint db2 fluid");
		return OGRE_NEW HlmsPbsDbWater( name, this, macro, blend, params );
	}
	else if( Hlms::findParamInVec( params, "paint", val ) )
	{
		// LogO("paint db2 new");
		return OGRE_NEW HlmsPbsDbCar( name, this, macro, blend, params );
	}else
#endif
		return OGRE_NEW HlmsPbsDatablock( name, this, macro, blend, params );
}


//  💫 upload ConstBuffer
void HlmsPbsDbCar::uploadToConstBuffer( char *dstPtr, uint8 dirtyFlags )
{
	for (int i=0; i < 3; ++i)
	{
		mUserValue[i][0] = paintClr[i].x;
		mUserValue[i][1] = paintClr[i].y;
		mUserValue[i][2] = paintClr[i].z;
	}
	mUserValue[0][3] = paintMul.x;
	mUserValue[1][3] = paintMul.y;
	mUserValue[2][3] = paintMul.z;

	// LogO("paint db2 upload");
	HlmsPbsDatablock::uploadToConstBuffer( dstPtr, dirtyFlags );
return;  // !!

	// todo .. send paint par
	char* orgPtr = dstPtr;
	HlmsPbsDatablock::uploadToConstBuffer( dstPtr, dirtyFlags );
	dstPtr = orgPtr + HlmsPbsDatablock::MaterialSizeInGpu;  // can't change this ...
	// memcpy( dstPtr, &mCustomParameter, sizeof( mCustomParameter) );

	//  game copy paint params  ----
#ifndef SR_EDITOR
	if (paint)
	{
		// LogO("paint db2 upload");
		size_t sf4 = sizeof( float4 );
		
		for (int i=0; i < 3; ++i)
		{
			float4 par4 = paintClr[i];
			memcpy( dstPtr, &par4, sf4 );  dstPtr += sf4;
		}
		float4 par4 = paintMul;
		memcpy( dstPtr, &par4, sf4 );  dstPtr += sf4;
	}
#endif
}

//  🌟 ctor
HlmsPbsDbCar::HlmsPbsDbCar(
	IdString name, HlmsPbs2 *creator,
	const HlmsMacroblock *macro, const HlmsBlendblock *blend,
	const HlmsParamVec &params )
	: HlmsPbsDatablock( name, creator, macro, blend, params )
{
	String val;
	if( Hlms::findParamInVec( params, "paint", val ) )
	{
		// LogO("paint db2 ctor, paint 1");
		paint = 1;
		//Vector3 v = StringConverter::parseVector3( val, Vector3::UNIT_SCALE );
		// setDiffuse( v );
		scheduleConstBufferUpdate();
	}
}

//  clone
void HlmsPbsDbCar::cloneImpl( HlmsDatablock *db ) const
{
	HlmsPbsDatablock::cloneImpl( db );
	HlmsPbsDbCar *db2 = static_cast<HlmsPbsDbCar*>( db );

	db2->paint = paint;
	for (int i=0; i < 3; ++i)
		db2->paintClr[i] = paintClr[i];
	db2->paintMul = paintMul;
}


// Ogre::Vector4 waterClr[3] = {Ogre::Vector4(0,0.5,1,0), Ogre::Vector4(0,1,0,0), Ogre::Vector4(1,0.5,0,0)};

//  🌊 water fluid params
//----------------------------------------------------------------
HlmsPbsDbWater::HlmsPbsDbWater(
		Ogre::IdString name, HlmsPbs2 *creator,
		const Ogre::HlmsMacroblock *macro,
		const Ogre::HlmsBlendblock *blend,
		const Ogre::HlmsParamVec &p )
	: HlmsPbsDatablock( name, creator, macro, blend, p )
{
	String val;
	if( Hlms::findParamInVec( p, "fluid", val ) )
	{
		// LogO("fluid db2 ctor 1");

		Vector2 choppyness_scale; // 0.15 25
		Vector4 smallWaves_midWaves; // 0.15 0.1 0.3 0.15
		Vector2 bigWaves; // 0.3 0.3
		Vector3 bump; // 1.5 0.08 0.12  // norm-sc  fresnel  refract
		// fresnelMultiplier 1.0

		Vector4 colour; // 0.49 0.69 0.78  0.0
		Vector4 reflectColour; // 0.91 0.93 0.94  0.9
		Vector4 refractColour; // 0.76 0.89 0.99  0.76
		Vector4 specColourAndPower; // 0.98 1.0 1.0  64

		Vector3 bump2SpecPowerMul; // 6.0 16 0.8
		Vector2 speed; // 1.0 1.0

		#define S2V(d,v,a)  StringConverter::parseVector##d(v,a)
		if (Hlms::findParamInVec(p, "choppyness_scale", val))     choppyness_scale    = S2V(2, val, Vector2(0.15, 1.0));
		if (Hlms::findParamInVec(p, "smallWaves_midWaves", val))  smallWaves_midWaves = S2V(4, val, Vector4(0.15, 0.1, 0.3, 0.15));
		if (Hlms::findParamInVec(p, "bigWaves", val))             bigWaves            = S2V(2, val, Vector2(0.3, 0.3));

		// LogO(fToStr(v.x)+" "+fToStr(v.y)+" "+fToStr(v.z)+" "+fToStr(v.w));
		mDetailsOffsetScale[0][0] = choppyness_scale.x;
		mDetailsOffsetScale[0][1] = choppyness_scale.y;
		mDetailsOffsetScale[0][2] = bigWaves.x;
		mDetailsOffsetScale[0][3] = bigWaves.y;

		mDetailsOffsetScale[1][0] = smallWaves_midWaves.x;
		mDetailsOffsetScale[1][1] = smallWaves_midWaves.y;
		mDetailsOffsetScale[1][2] = smallWaves_midWaves.z;
		mDetailsOffsetScale[1][3] = smallWaves_midWaves.w;
		// setDiffuse( v );

		scheduleConstBufferUpdate();
	}
}

//  💫 upload ConstBuffer
void HlmsPbsDbWater::uploadToConstBuffer( char *dstPtr, uint8 dirtyFlags )
{
	// LogO("water db2 upload");
	HlmsPbsDatablock::uploadToConstBuffer( dstPtr, dirtyFlags );
}















#if 0
//----------------------------------------------------------------
///  todo: Notes  **Bookmarks**
//----------------------------------------------------------------

const size_t c_geometryShaderMagicValue = 123456;

renderableA->setCustomParameter( c_geometryShaderMagicValue, Vector4( 0.0f ) ); // Use GS Method A
renderableB->setCustomParameter( c_geometryShaderMagicValue, Vector4( 1.0f ) ); // Use GS Method B

void MyHlmsPbs::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces )
{
	HlmsPbs::calculateHashForPreCreate( renderable, inOutPieces );
	
	const Renderable::CustomParameterMap &customParams = renderable->getCustomParameters();
	Renderable::CustomParameterMap::const_iterator it =
			customParams.find( c_geometryShaderMagicValue );
	
	if( it != customParams.end() )
	{
		//Geometry Shader will be used. Set which method
		setProperty( "geometry_shader_method", (int32)it->second );
	}
}

@property( geometry_shader_method == 0 )
...
@end
@property( geometry_shader_method == 1 )
...
@end

// Just make sure the setCustomParameter gets called before assigning the datablock,
// otherwise you'll have to do this to refresh the Hlms hashes:

HlmsDatablock *datablock = renderableA->getDatablock();
Hlms *hlms = datablock->getCreator();
if( hlms->getType() == HLMS_PBS )
{
	assert( dynamic_cast<HlmsPbs2*>( hlms ) );
	MyHlmsPbs *myHlmsPbs = static_cast<HlmsPbs2*>( hlms );
	uint32 hash, casterHash;
	myHlmsPbs->calculateHashFor( renderableA, hash, casterHash );
	renderableA->_setHlmsHashes( hash, casterHash );
}


//  * * *  todo: try? ..
pbs->setOptimizationStrategy( ConstBufferPool::LowerGpuOverhead );
Hlms::setPrecisionMode with the following options:
PrecisionFull32

HlmsTextureManager::dumpMemoryUsage

hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );

HlmsPbsTerraShadows::preparePassBuffer  +

Overload Hlms::preparePassHash or HlmsListener::preparePassHash to define a custom property that follows an entirely different shader path



Hlms implementation can be customized:
2  through HlmsListener. This allows you to have access to the buffer pass to fill extra information; or bind extra buffers to the shader.
3  Overload HlmsPbs. Useful for overriding only specific parts, or adding new functionality that requires storing extra information in a datablock (e.g. overload HlmsPbsDatablock to add more variables, and then overload HlmsPbs::createDatablockImpl to create these custom datablocks)
4  Directly modify HlmsPbs, HlmsPbsDatablock and the template.


custom_passBuffer 	can add extra info for pass buffer (only useful if the user is using HlmsListener or overloaded HlmsPbs).
custom_materialBuffer  ..
custom_VStoPS 	Piece where users can add more interpolants for passing data from the vertex to the pixel shader.
|
custom_vs_attributes 	Custom vertex shader attributes in the Vertex Shader (i.e. a special texcoord, etc).
custom_vs_uniformDeclaration 	Data declaration (textures, texture buffers, uniform buffers) in the Vertex Shader.
custom_vs_uniformStructDeclaration  ..
custom_vs_posMaterialLoad  ..
custom_vs_preTransform  ..
custom_vs_preExecution 	Executed before Ogre code from the Vertex Shader.
custom_vs_posExecution 	Executed after all code from the Vertex Shader has been performed.
|
custom_ps_uniformDeclaration 	Same as custom_vs_uniformDeclaration, but for the Pixel Shader
custom_ps_uniformStructDeclaration  ..
custom_ps_preExecution 	Executed before Ogre code from the Pixel Shader.
custom_ps_posMaterialLoad 	Executed right after loading material data; and before anything else. May not get executed if there is no relevant material data (i.e. doesnt have normals or QTangents for lighting calculation)
custom_ps_posSampleNormal  ..
custom_ps_preLights 	Executed right before any light (i.e. to perform your own ambient / global illumination pass). All relevant texture data should be loaded by now.
custom_ps_posExecution 	Executed after all code from the Pixel Shader has been performed.
custom_ps_uv_modifier_macros 	PBS specific. Allows you to override the macros defined in Samples/Media/Hlms/Pbs/Any/UvModifierMacros_piece_ps.any so you can apply custom transformations to each UV. e.g. #undef UV_DIFFUSE #define UV_DIFFUSE( x ) ((x) * 2.0)
custom_ps_functions 	Used to declare functions outside the main body of the shader
custom_ps_pixelData 	Declare additional data in struct PixelData from Pixel Shader 


PassStructDecl

#endif
