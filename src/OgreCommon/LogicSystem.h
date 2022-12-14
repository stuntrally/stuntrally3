#pragma once
#include "BaseSystem.h"
#include <OgrePrerequisites.h>
#include <deque>

class GameEntityManager;


class LogicSystem : public BaseSystem
{
protected:
	BaseSystem          *mGraphicsSystem;
	GameEntityManager   *mGameEntityManager;

	Ogre::uint32                mCurrentTransformIdx;
	std::deque<Ogre::uint32>    mAvailableTransformIdx;

	/// @see MessageQueueSystem::processIncomingMessage
	virtual void processIncomingMessage( Mq::MessageId messageId, const void *data );

public:
	LogicSystem( GameState *gameState );
	virtual ~LogicSystem();

	void _notifyGraphicsSystem( BaseSystem *graphicsSystem )    { mGraphicsSystem = graphicsSystem; }
	void _notifyGameEntityManager( GameEntityManager *mgr )     { mGameEntityManager = mgr; }

	void finishFrameParallel();

	GameEntityManager* getGameEntityManager()               { return mGameEntityManager; }
	Ogre::uint32 getCurrentTransformIdx() const             { return mCurrentTransformIdx; }
};
