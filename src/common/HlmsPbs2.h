#pragma once
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
#include <OgreArchive.h>


class HlmsPbs2 : public Ogre::HlmsPbs
{
public:
	HlmsPbs2( Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders );
	~HlmsPbs2() override;
	
	void calculateHashForPreCreate( Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;
	// void calculateHashForPreCaster( Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces ) override;

	// HlmsDatablock *createDatablockImpl( IdString datablockName, const HlmsMacroblock *macroblock,
	// 									const HlmsBlendblock *blendblock,
	// 									const HlmsParamVec   &paramVec ) override;

	std::vector<Ogre::String> logs;
};
