#include "pch.h"
#include "Def_Str.h"
#include "HlmsPbs2.h"
#include "App.h"
#include "CScene.h"
#include "FluidsXml.h"

#include "OgreHlmsPbsTerraShadows.h"
#include <OgreHlmsJsonPbs.h>
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
HlmsPbs2::HlmsPbs2( App* app1, Archive *dataFolder, ArchiveVec *libraryFolders )
	:app(app1), HlmsPbs( dataFolder, libraryFolders )
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
	
	bool fluid = false;
	if (pFluidsXml)
		fluid = pFluidsXml->MatInMap(mtr);
	else
		LogO("HlmsPbs2 pFluidsXml not set!");

	if (mtr.substr(0,5) == "River")
	{	fluid = 1;
		setProperty( "river", 1 );
	}else
	if (fluid)
		setProperty( "water", 1 );
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
	return OGRE_NEW HlmsPbsDb2( app, name, this, macro, blend, params );
}


//  💥 dtor
HlmsPbsDb2::~HlmsPbsDb2()
{
	auto it = app->vDbRefl.find(this);
	if (it != app->vDbRefl.end())
		app->vDbRefl.erase(it);
}

//  🌟 ctor
HlmsPbsDb2::HlmsPbsDb2( App* app1,
		Ogre::IdString name, HlmsPbs2 *creator,
		const Ogre::HlmsMacroblock *macro,
		const Ogre::HlmsBlendblock *blend,
		const Ogre::HlmsParamVec &p )
	: HlmsPbsDatablock( name, creator, macro, blend, p )
	, app(app1)
{
	String val;
	//  extra params in .material or .json
	if( Hlms::findParamInVec( p, "bump_scale", val ) )
	{
		Real sc = s2r(val);
		setNormalMapWeight(sc);
		// LogO("db2 bump_scale2: " + fToStr(getNormalMapWeight()));
	}
	if( Hlms::findParamInVec( p, "reflect", val ) )
	{
		creator->app->vDbRefl.insert(this);  // save for later, mCubeReflTex doesn't exist yet
		// setTexture( PBSM_REFLECTION, creator->app->mCubeReflTex );
	}

	//  🌊 water fluid params
	//----------------------------------------------------------------
	if( Hlms::findParamInVec( p, "paint", val ) )
	{
		eType = DB_Paint;
		// LogO("db2 ctor paint");
		//Vector3 v = StringConverter::parseVector3( val, Vector3::UNIT_SCALE );
		// setDiffuse( v );
	}
	else
	if (Hlms::findParamInVec(p, "fluid", val) || Hlms::findParamInVec(p, "choppyness_scale", val))
	{
		eType = DB_Fluid;
		// LogO("db2 ctor fluid");

		Vector2 choppyness_scale{0,0};
		Vector4 smallWaves_midWaves{0,0,0,0};  Vector2 bigWaves{0,0};
		Vector3 bump_fresn_refra{0,0,0};  // norm-sc  fresnel  refract
		// fresnelMultiplier 1.0
		Vector4 colour{0,0,0,0}, reflectColour{0,0,0,0}, refractColour{0,0,0,0}, specClrPow{0,0,0,0};
		Vector3 bump2SpecPowerMul{0,0,0};
		Vector2 speed{0,0};

		#define S2V(dim, val, def)  StringConverter::parseVector##dim(val, def)
		if (Hlms::findParamInVec(p, "choppyness_scale", val))     choppyness_scale    = S2V(2, val, Vector2(0.15, 1.0));
		if (Hlms::findParamInVec(p, "smallWaves_midWaves", val))  smallWaves_midWaves = S2V(4, val, Vector4(0.15, 0.1, 0.3, 0.15));
		if (Hlms::findParamInVec(p, "bigWaves", val))             bigWaves            = S2V(2, val, Vector2(0.3, 0.3));
		if (Hlms::findParamInVec(p, "bump", val))                 bump_fresn_refra    = S2V(3, val, Vector3(1.5, 0.08, 0.12));

		if (Hlms::findParamInVec(p, "colour", val))               colour              = S2V(4, val, Vector4(0.49, 0.69, 0.78, 0.0));
		if (Hlms::findParamInVec(p, "reflectColour", val))        reflectColour       = S2V(4, val, Vector4(0.91, 0.93, 0.94, 0.9));
		if (Hlms::findParamInVec(p, "refractColour", val))        refractColour       = S2V(4, val, Vector4(0.76, 0.89, 0.99, 0.76));
		if (Hlms::findParamInVec(p, "specColourAndPower", val))   specClrPow          = S2V(4, val, Vector4(0.98, 1.0, 1.0, 0.15));

		if (Hlms::findParamInVec(p, "bump2SpecPowerMul", val))    bump2SpecPowerMul   = S2V(3, val, Vector3(6.0, 16.0, 0.8));
		if (Hlms::findParamInVec(p, "speed", val))                speed               = S2V(2, val, Vector2(1.0, 1.0));

		mDetailsOffsetScale[0][0] = smallWaves_midWaves.x;
		mDetailsOffsetScale[0][1] = smallWaves_midWaves.y;
		mDetailsOffsetScale[0][2] = smallWaves_midWaves.z;
		mDetailsOffsetScale[0][3] = smallWaves_midWaves.w;
		mDetailsOffsetScale[1][0] = bigWaves.x;
		mDetailsOffsetScale[1][1] = bigWaves.y;
	//  mDetailsOffsetScale[1][2] [3] ..
		mDetailsOffsetScale[2][0] = speed.x;
		mDetailsOffsetScale[2][1] = choppyness_scale.x;
		mDetailsOffsetScale[2][2] = choppyness_scale.y;
		
		mDetailsOffsetScale[2][3] = bump_fresn_refra.x;
		mDetailsOffsetScale[3][0] = bump2SpecPowerMul.x;  // bump2
		mDetailsOffsetScale[3][1] = bump2SpecPowerMul.z;  // mul


		// setDiffuse( Vector3(colour.x, colour.y, colour.z) * 0.6f );  // par..88
		setDiffuse( Vector3(refractColour.x, refractColour.y, refractColour.z) * 0.6f );  // par..
		setSpecular( Vector3(specClrPow.x, specClrPow.y, specClrPow.z) * 0.4f );
		setFresnel( Vector3(reflectColour.x, reflectColour.y, reflectColour.z) * 0.3f, 1 );

		// float roughness = specClrPow.z;
		// setRoughness(roughness);  // todo.. mul, bump_fresn_refra ..
		setTransparency( colour.w == 0.0 ? refractColour.w : 1.0 );
		setRefractionStrength( bump_fresn_refra.z );
		//bump_fresn_refra

	}
	scheduleConstBufferUpdate();
}

//  💫 upload ConstBuffer
void HlmsPbsDb2::uploadToConstBuffer( char *dstPtr, uint8 dirtyFlags )
{
	if (eType == DB_Paint)
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
	}

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
	if (eType == DB_Paint)
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

//  clone  // todo fixme
void HlmsPbsDb2::cloneImpl( HlmsDatablock *db ) const
{
	HlmsPbsDatablock::cloneImpl( db );
	// HlmsPbsDbCar *db2 = static_cast<HlmsPbsDbCar*>( db );

	// db2->paint = paint;
	// for (int i=0; i < 3; ++i)
	// 	db2->paintClr[i] = paintClr[i];
	// db2->paintMul = paintMul;
}


//  save json
class HlmsJsonPbs2 : public HlmsJsonPbs
{
public:
	HlmsJsonPbs2( HlmsManager *hlmsManager, TextureGpuManager *textureManager,
		HlmsJsonListener *listener, const String &additionalExtension )
		: HlmsJsonPbs( hlmsManager, textureManager, listener, additionalExtension )
	{
	}

    void saveMaterial( const HlmsDatablock *datablock, String &out )
    {
		HlmsPbsDb2* db2 = (HlmsPbsDb2*)datablock;
		if (db2->eType == DB_Fluid)
	        out += ",\n\t\t\t\"fluid\" : true";
		if (db2->eType == DB_Paint)
	        out += ",\n\t\t\t\"paint\" : true";

		TextureGpu *tex = db2->getTexture( PBSM_REFLECTION ), *no = 0;
		if (tex)
		{	db2->setTexture( PBSM_REFLECTION, no );
	        out += ",\n\t\t\t\"reflect\" : true";
		}
		// todo: env "reflect", auto add DynamicCubemap ..

        HlmsJsonPbs::saveMaterial( datablock, out );

		if (tex)
			db2->setTexture( PBSM_REFLECTION, tex );
    }
};


void HlmsPbs2::_saveJson(const Ogre::HlmsDatablock *datablock, Ogre::String &outString,
	Ogre::HlmsJsonListener *listener, const Ogre::String &additionalTextureExtension) const
{
	HlmsJsonPbs2 jsonPbs( mHlmsManager, mRenderSystem->getTextureGpuManager(), listener,
							additionalTextureExtension );
	jsonPbs.saveMaterial( datablock, outString );
}



#if 0
//----------------------------------------------------------------
///  todo: Notes  **Bookmarks**
//----------------------------------------------------------------


//  * * *  todo: try? ..
pbs->setOptimizationStrategy( ConstBufferPool::LowerGpuOverhead );
Hlms::setPrecisionMode with the following options:
PrecisionFull32

HlmsTextureManager::dumpMemoryUsage

hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );


HlmsPbsTerraShadows::preparePassBuffer  +

Overload Hlms::preparePassHash or HlmsListener::preparePassHash to define a custom property that follows an entirely different shader path


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
