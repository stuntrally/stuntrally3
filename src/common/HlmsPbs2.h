#pragma once
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreArchive.h>
// #include <OgreHlmsPbsDatablock.h>


class HlmsPbs2 : public Ogre::HlmsPbs
{
public:
	HlmsPbs2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsPbs2() override;
	
	const static size_t selected_glow = 123;

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

/*
class HlmsPbsDatablock2 : public Ogre::HlmsPbsDatablock
{
	friend class HlmsPbs2;

public:
	HlmsPbsDatablock2(
		Ogre::IdString name, HlmsPbs2 *creator,
		const Ogre::HlmsMacroblock *macroblock,
		const Ogre::HlmsBlendblock *blendblock,
		const Ogre::HlmsParamVec &params );
};
*/