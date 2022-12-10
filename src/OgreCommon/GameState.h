#pragma once
#include "InputListeners.h"


class GameState : public MouseListener, public KeyboardListener, public JoystickListener
{
public:
	virtual ~GameState() {}

	virtual void initialize() {}
	virtual void deinitialize() {}

	virtual void createScene01() {}
	virtual void createScene02() {}

	virtual void destroyScene() {}

	virtual void update( float timeSinceLast ) {}
	virtual void finishFrameParallel() {}
	virtual void finishFrame() {}
};
