#include "pch.h"
#include "Def_Str.h"
#include "CGame.h"
// #include "CHud.h"
#include "CGui.h"
#include "masterclient.hpp"
#include "gameclient.hpp"

#include "CarModel.h"
#include "GraphicsSystem.h"
#include "settings.h"
#include "car.h"
#include "game.h"
// #include <MyGUI.h>
#include "LinearMath/btDefaultMotionState.h"
#include <thread>
using namespace Ogre;
using namespace std;


//  💫 Simulation  2nd thread
//----------------------------------------------------------------------------------
void App::UpdThr()
{
	Ogre::Timer gtim;
	//#ifdef _WIN32
	//DWORD af = 2;
	//gtim.setOption("QueryAffinityMask", &af);
	//#endif
	gtim.reset();

	while (!mGraphicsSystem->getQuit())
	{
		///  step Game  **

		double dt = double(gtim.getMicroseconds()) * 0.000001;
		gtim.reset();
		
		if (!bLoading && !mGraphicsSystem->getQuit() && pGame)
		{
			bSimulating = true;
			bool ret = pGame->OneLoop(dt);
			if (!ret)
				mGraphicsSystem->setQuit();

			DoNetworking();
			bSimulating = false;
		}
		this_thread::sleep_for(chrono::milliseconds(
			pSet ? pSet->thread_sleep : 100));
	}
}

	
bool App::isTweakTab()
{
	int tt = !gui->tabTweak ? 0 : gui->tabTweak->getIndexSelected();
	return isTweak() && tt != 1 && tt != 2;
}


///  💫📡 Newtork update  . . . . . . . . . . . . . . . . . . . . . . . .
void App::DoNetworking()
{
	bool doNetworking = mClient && mClient->getState() == P2PGameClient::GAME;
	
	//  no pause in networked game
	int tt = !gui->tabTweak ? 0 : gui->tabTweak->getIndexSelected();
	bool gui = isFocGui || isTweakTab();
	pGame->pause = bRplPlay ? (bRplPause || gui) : (gui && !doNetworking);

	//  handle networking stuff
	if (doNetworking)
	{
		// PROFILER.beginBlock("-network");

		//  update the local car's state to the client
		protocol::CarStatePackage cs;  // note: Handles only one local car
		for (auto car : carModels)
		{
			if (car->cType == CarModel::CT_LOCAL)
			{
				cs = car->pCar->GetCarStatePackage();
				cs.trackPercent = uint8_t( car->trackPercent / 100.f * 255.f);  // pack to uint8
				break;
			}
		}
		mClient->setLocalCarState(cs);

		//  check for new car states
		protocol::CarStates states = mClient->getReceivedCarStates();
		for (auto it : states)
		{
			int8_t id = it.first;  // Car number  // note: Various places assume carModels[0] is local
			if (id == 0)  id = mClient->getId();
			
			CarModel* cm = carModels[id];
			if (cm && cm->pCar)
			{
				cm->pCar->UpdateCarState(it.second);
				cm->trackPercent = cm->pCar->trackPercentCopy;  // got from client
			}
		}
		// PROFILER.endBlock("-network");
	}
}
