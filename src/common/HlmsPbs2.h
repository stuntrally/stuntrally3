#pragma once
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreArchive.h>
#include <OgreHlmsPbsDatablock.h>
#include <rapidjson/document.h>

//  HLMS overrides for SR3, used instead of default Pbs and Unlit
class App;

//  our Unlit  ----------------
class HlmsUnlit2 : public Ogre::HlmsUnlit
{
public:
	HlmsUnlit2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsUnlit2() override;
	
	void calculateHashForPreCreate(
		Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;
};


//  our Pbs  ----------------
class HlmsPbs2 : public Ogre::HlmsPbs
{
public:
	App* app =0;
	HlmsPbs2( App* app1, Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsPbs2() override;
	
	constexpr static size_t selected_glow = 123;

	class FluidsXml* pFluidsXml =0;  //! set this after Load

	void CalculateHashFor( Ogre::Renderable *renderable, Ogre::uint32 &outHash, Ogre::uint32 &outCasterHash )
	{
        calculateHashFor( renderable, outHash, outCasterHash );
	}
	
	//  setProperty for our shader types
	void calculateHashForPreCreate(
		Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	void calculateHashForPreCaster(
		 Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	//  ctor  new HlmsPbsDb2
	Ogre::HlmsDatablock *createDatablockImpl(
		Ogre::IdString name,
		const Ogre::HlmsMacroblock *macro,
		const Ogre::HlmsBlendblock *blend,
		const Ogre::HlmsParamVec &params ) override;

	//  save  adds our types
	void _saveJson(const Ogre::HlmsDatablock *datablock, Ogre::String &outString,
		Ogre::HlmsJsonListener *listener, const Ogre::String &additionalTextureExtension) const override;

	void _loadJson(const rapidjson::Value &jsonValue, const Ogre::HlmsJson::NamedBlocks &blocks,
		Ogre::HlmsDatablock *datablock, const Ogre::String &resourceGroup,
		Ogre::HlmsJsonListener *listener, const Ogre::String &additionalTextureExtension ) const override;
};


//  our Datablock2  ----------------
//  replaces ALL default HlmsPbsDatablocks
enum EType
{
	DB_PBS =0,  // default normal
	DB_Paint,  // paint for vehicles
	DB_Fluid,  // water, mud etc
	DB_Tree,  // tree with wind
	DB_ALL
};

class HlmsPbsDb2 : public Ogre::HlmsPbsDatablock
{
	friend class HlmsPbs2;
public:
	App* app =0;
	EType eType = DB_PBS;

	//  body_paint
	Ogre::Vector4 paintClr[3] = {Ogre::Vector4(0,0.5,1,0), Ogre::Vector4(0,1,0,0), Ogre::Vector4(1,0.5,0,0)};
	Ogre::Vector4 paintMul = Ogre::Vector4(0.2,0.5,0.7,0);

	//  fluid params
	// Ogre::Vector4 waterClr[3] = {Ogre::Vector4(0,0.5,1,0), Ogre::Vector4(0,1,0,0), Ogre::Vector4(1,0.5,0,0)};

	//  cube reflect  object,road..
	bool reflect = false;
	
	HlmsPbsDb2(	App* app1,
		Ogre::IdString name, HlmsPbs2 *creator,
		const Ogre::HlmsMacroblock *macro,
		const Ogre::HlmsBlendblock *blend,
		const Ogre::HlmsParamVec &params );
	~HlmsPbsDb2();

    //  what we send to the GPU (to send more)
	void uploadToConstBuffer( char *dstPtr, Ogre::uint8 dirtyFlags ) override;

	void cloneImpl( HlmsDatablock *db ) const override;
};
