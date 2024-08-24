/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
modified by CryHam for SR3
*/
#include "pch.h"
#include "App.h"
#include <iostream>

#include "System/MainEntryPoints.h"
#include "System/Desktop/UnitTesting.h"

#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GameState.h"
#include "SdlInputHandler.h"

#include <OgrePrerequisites.h>
#include <OgreWindow.h>
#include <OgreTimer.h>
#include <memory>

#include "Threading/OgreThreads.h"
#include "../network/enet-wrapper.hpp"
using namespace std;


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI MainEntryPoints::mainAppSingleThreaded(
	HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int MainEntryPoints::mainAppSingleThreaded( int argc, const char *argv[] )
#endif
{
	UnitTest unitTest;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	unitTest.parseCmdLine( __argc, __argv );
#else
	unitTest.parseCmdLine( argc, argv );
#endif

	if( unitTest.getParams().isPlayback() )
	{
		return unitTest.loadFromJson( unitTest.getParams().recordPath.c_str(),
									  unitTest.getParams().outputPath );
	}

	GameState *graphicsGameState = 0;
	GraphicsSystem *graphicsSystem = 0;
	GameState *logicGameState = 0;
	LogicSystem *logicSystem = 0;

	//**
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	args.all = vector<string>( __argv, __argv + __argc );
#else
	args.all = vector<string>( argv, argv + argc );
#endif
	if (args.Help())
		return 0;

	cout << "Init :: new App\n";
	unique_ptr<App> app = make_unique<App>();
	
	cout << "Init :: LoadSettings\n";
	app->LoadSettings();

#ifndef SR_EDITOR
	//  Initialize networking
	net::ENetContainer enet;
#endif
	//**

	cout << "Init :: createSystems\n";
	MainEntryPoints::createSystems( app.get(),
		&graphicsGameState, &graphicsSystem,
		&logicGameState, &logicSystem );

	try
	{
		graphicsSystem->initialize( getWindowTitle() );
		if( logicSystem )
			logicSystem->initialize();

		if( graphicsSystem->getQuit() )
		{
			if( logicSystem )
				logicSystem->deinitialize();
			graphicsSystem->deinitialize();

			MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem,
											 logicGameState, logicSystem );
			return 0; // User cancelled config
		}

		if( unitTest.getParams().isRecording() )
			unitTest.startRecording( graphicsSystem );

		Ogre::Window *renderWindow = graphicsSystem->getRenderWindow();

		graphicsSystem->createScene01();
		if( logicSystem )
			logicSystem->createScene01();

		graphicsSystem->createScene02();
		if( logicSystem )
			logicSystem->createScene02();

	#if OGRE_USE_SDL2
		// Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
		SdlInputHandler *inputHandler = graphicsSystem->getInputHandler();
	   	if (graphicsSystem->mGrabMouse)
		{
	 		inputHandler->setGrabMousePointer( true );
	 		inputHandler->setMouseRelative( true );
		}
 		inputHandler->setMouseVisible( false );
	#endif

		Ogre::Timer timer;
		Ogre::uint64 startTime = timer.getMicroseconds();
		double accumulator = MainEntryPoints::Frametime;

		double timeSinceLast = 1.0 / 60.0;

		while( !graphicsSystem->getQuit() )
		{
			while( accumulator >= MainEntryPoints::Frametime && logicSystem )
			{
				logicSystem->beginFrameParallel();
				logicSystem->update( static_cast<float>( MainEntryPoints::Frametime ) );
				logicSystem->finishFrameParallel();

				logicSystem->finishFrame();
				graphicsSystem->finishFrame();

				accumulator -= MainEntryPoints::Frametime;
			}

			graphicsSystem->beginFrameParallel();
			graphicsSystem->update( static_cast<float>( timeSinceLast ) );
			graphicsSystem->finishFrameParallel();
			if( !logicSystem )
				graphicsSystem->finishFrame();

			if( unitTest.getParams().isRecording() )
				unitTest.notifyRecordingNewFrame( graphicsSystem );

			if( !renderWindow->isVisible() )
			{
				// Don't burn CPU cycles unnecessary when we're minimized.
				Ogre::Threads::Sleep( 500 );
			}

			Ogre::uint64 endTime = timer.getMicroseconds();
			timeSinceLast = (endTime - startTime) / 1000000.0;
			timeSinceLast = min( 1.0, timeSinceLast ); // Prevent from going haywire.
			accumulator += timeSinceLast;
			startTime = endTime;
		}

		if( unitTest.getParams().isRecording() )
		{
			unitTest.saveToJson( unitTest.getParams().recordPath.c_str(),
								 unitTest.getParams().bCompressDuration );
		}

		graphicsSystem->destroyScene();
		if( logicSystem )
		{
			logicSystem->destroyScene();
			logicSystem->deinitialize();
		}
		graphicsSystem->deinitialize();

		MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem,
										 logicGameState, logicSystem );
	}
	catch( Ogre::Exception &e )
	{
		MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem,
										 logicGameState, logicSystem );
		throw e;
	}
	catch( ... )
	{
		MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem,
										 logicGameState, logicSystem );
	}
	return 0;
}
