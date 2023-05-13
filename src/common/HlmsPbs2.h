#pragma once
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreArchive.h>
#include <OgreHlmsPbsDatablock.h>

//  HLMS overrides for SR3, used instead of default Pbs and Unlit


//  our Unlit  ----------------
class HlmsUnlit2 : public Ogre::HlmsUnlit
{
public:
	HlmsUnlit2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsUnlit2() override;
	
	void calculateHashForPreCreate(
		Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	// void calculateHashForPreCaster(
	// 	 Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;
/*
	Ogre::HlmsDatablock *createDatablockImpl(
		Ogre::IdString datablockName,
		const Ogre::HlmsMacroblock *macroblock,
		const Ogre::HlmsBlendblock *blendblock,
		const Ogre::HlmsParamVec   &paramVec ) override;
*/
};

//  our Pbs  ----------------
class HlmsPbs2 : public Ogre::HlmsPbs
{
public:
	HlmsPbs2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsPbs2() override;
	
	constexpr static size_t selected_glow = 123;

	void CalculateHashFor( Ogre::Renderable *renderable, Ogre::uint32 &outHash, Ogre::uint32 &outCasterHash )
	{
        calculateHashFor( renderable, outHash, outCasterHash );
	}

	void calculateHashForPreCreate(
		Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	void calculateHashForPreCaster(
		 Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;
/*
	Ogre::HlmsDatablock *createDatablockImpl(
		Ogre::IdString datablockName,
		const Ogre::HlmsMacroblock *macroblock,
		const Ogre::HlmsBlendblock *blendblock,
		const Ogre::HlmsParamVec   &paramVec ) override;
*/
};

//  not used - our datablock  ----------------
class HlmsPbsDatablock2 : public Ogre::HlmsPbsDatablock
{
	friend class HlmsPbs2;

public:
	HlmsPbsDatablock2(
		Ogre::IdString name, HlmsPbs2 *creator,
		const Ogre::HlmsMacroblock *macroblock,
		const Ogre::HlmsBlendblock *blendblock,
		const Ogre::HlmsParamVec &params );

    //  what we send to the GPU (to send more)
	void uploadToConstBuffer( char *dstPtr, Ogre::uint8 dirtyFlags ) override;
};
