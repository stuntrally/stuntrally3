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

	Ogre::HlmsDatablock *createDatablockImpl(
		Ogre::IdString name,
		const Ogre::HlmsMacroblock *macro,
		const Ogre::HlmsBlendblock *blend,
		const Ogre::HlmsParamVec &params ) override;
};


//  our datablock  ----------------
class HlmsPbsDatablock2 : public Ogre::HlmsPbsDatablock
{
	friend class HlmsPbs2;

public:

	bool paint =0;  // body_paint
	Ogre::Vector4 paintClr[3] = {Ogre::Vector4(0,0.5,1,0), Ogre::Vector4(0,1,0,0), Ogre::Vector4(1,0.5,0,0)};
	Ogre::Vector4 paintMul = Ogre::Vector4(0.2,0.5,0.7,0);
	
	HlmsPbsDatablock2(
		Ogre::IdString name, HlmsPbs2 *creator,
		const Ogre::HlmsMacroblock *macro,
		const Ogre::HlmsBlendblock *blend,
		const Ogre::HlmsParamVec &params );

    //  what we send to the GPU (to send more)
	void uploadToConstBuffer( char *dstPtr, Ogre::uint8 dirtyFlags ) override;
};
