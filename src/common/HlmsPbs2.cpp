#include "Def_Str.h"
#include "HlmsPbs2.h"
using namespace Ogre;


HlmsPbs2::HlmsPbs2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders )
	:HlmsPbs(dataFolder, libraryFolders )
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
	// LogO("- calc for: " + mtr);   // on every item
	
	if (mtr.find("grass") != std::string::npos)
		setProperty( "grass", 1 );

	// if (mtr.find("road") != std::string::npos)
	// 	setProperty( "road", 1 );

    //  todo:
	const auto &paramMap = rnd->getCustomParameters();
	auto itor = paramMap.find( selected_glow );
	if( itor != paramMap.end() )
	{
		setProperty( "selected_glow", (int)itor->second.x );
	}
}

void HlmsPbs2::calculateHashForPreCaster(
	Ogre::Renderable *rnd, Ogre::PiecesMap *inOut )
{
    HlmsPbs::calculateHashForPreCaster( rnd, inOut );

	const auto& mtr = rnd->getDatablockOrMaterialName();
	// LogO("- cast for: " + mtr);   // on every item

	if (mtr.find("grass") != std::string::npos)
		setProperty( "grass", 1 );
}


// createDatablockImpl


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
if( hlms->getType() == Ogre::HLMS_PBS )
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