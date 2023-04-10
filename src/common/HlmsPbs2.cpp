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


// const size_t magic1234 = 1234;

void HlmsPbs2::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces )
{
	HlmsPbs::calculateHashForPreCreate( renderable, inOutPieces );
	
	LogO("calc for: " + renderable->getDatablockOrMaterialName());
	// if (renderable && renderable->getMaterial())
	// 	LogO("calc for: "+renderable->getMaterial()->getName());
		// logs.push_back("calc for: "+renderable->getMaterial()->getName());
	
	const auto &paramMap = renderable->getCustomParameters();
	/*auto itor = paramMap.find( magic1234 );
	if( itor != paramMap.end() )
	{
		setProperty( "BlaBlah", (int)itor->second.x );
	}*/
}

// void HlmsPbs2::calculateHashForPreCaster  //too?


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
*/

/*
@property( geometry_shader_method == 0 )
...
@end
@property( geometry_shader_method == 1 )
...
@end
*/
// Just make sure the setCustomParameter gets called before assigning the datablock,
// otherwise you'll have to do this to refresh the Hlms hashes:
/*
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


//  * * * 
// Hlms::setPrecisionMode with the following options:
// PrecisionFull32

// HlmsTextureManager::dumpMemoryUsage

//hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );
