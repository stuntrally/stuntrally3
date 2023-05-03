#include "Def_Str.h"
#include "HlmsPbs2.h"
using namespace Ogre;


//  our Unlit
//----------------------------------------------------------------
HlmsUnlit2::HlmsUnlit2( Archive *dataFolder, ArchiveVec *libraryFolders )
	:HlmsUnlit( dataFolder, libraryFolders )
{
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
	
	if (mtr.substr(0,5) == "grass")
		setProperty( "grass", 1 );

	// if (mtr.find("road") != std::string::npos)
	// if (mtr.substr(0,4) == "road")
	// 	setProperty( "road", 1 );

    //  todo: road segs sel
	/*const auto &paramMap = rnd->getCustomParameters();
	auto itor = paramMap.find( selected_glow );
	if( itor != paramMap.end() )
	{
		setProperty( "selected_glow", (int)itor->second.x );
	}*/
}

void HlmsPbs2::calculateHashForPreCaster(
	Renderable *rnd, PiecesMap *inOut )
{
    HlmsPbs::calculateHashForPreCaster( rnd, inOut );

	const auto& mtr = rnd->getDatablockOrMaterialName();
	// LogO("- cast for: " + mtr);   // on every item

	if (mtr.substr(0,5) == "grass")
		setProperty( "grass", 1 );
}


//  createDatablockImpl
//----------------------------------------------------------------
/*
HlmsDatablock* HlmsPbs2::createDatablockImpl(
	IdString datablockName,
	const HlmsMacroblock *macroblock,
	const HlmsBlendblock *blendblock,
	const HlmsParamVec &paramVec )
{
    return OGRE_NEW HlmsPbsDatablock2( datablockName, this, macroblock, blendblock, paramVec );
}
*/

void HlmsPbsDatablock2::uploadToConstBuffer( char *dstPtr, uint8 dirtyFlags )
{
	
}


// todo: Notes
//----------------------------------------------------------------
/*
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
	assert( dynamic_cast<MyHlmsPbs*>( hlms ) );
	MyHlmsPbs *myHlmsPbs = static_cast<MyHlmsPbs*>( hlms );
	uint32 hash, casterHash;
	myHlmsPbs->calculateHashFor( renderableA, hash, casterHash );
	renderableA->_setHlmsHashes( hash, casterHash );
}
*/


//  * * *  todo: try? ..
// pbs->setOptimizationStrategy( ConstBufferPool::LowerGpuOverhead );
// Hlms::setPrecisionMode with the following options:
// PrecisionFull32

// HlmsTextureManager::dumpMemoryUsage

//hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );
/*
@insertpiece( custom_vs_attributes )
@insertpiece( custom_vs_uniformDeclaration )
@insertpiece( PassStructDecl )
@insertpiece( custom_vs_uniformStructDeclaration )
@insertpiece( custom_vs_posMaterialLoad )
@insertpiece( custom_vs_preTransform )  //+
@insertpiece( custom_vs_preExecution )

@insertpiece( custom_ps_uniformDeclaration )
@insertpiece( custom_ps_functions )
@insertpiece( custom_ps_preExecution )
@insertpiece( custom_ps_posMaterialLoad )
@piece( custom_ps_preLights )
*/
