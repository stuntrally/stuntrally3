#pragma once
#include "Threading/MessageQueueSystem.h"

class GameState;


class BaseSystem : public Mq::MessageQueueSystem
{
protected:
	GameState   *mCurrentGameState;

public:
	BaseSystem( GameState *gameState );
	virtual ~BaseSystem();

	virtual void initialize();
	virtual void deinitialize();

	virtual void createScene01();
	virtual void createScene02();

	virtual void destroyScene();

	void beginFrameParallel();
	void update( float timeSinceLast );
	
	void finishFrameParallel();
	void finishFrame();
};
