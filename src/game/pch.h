#pragma once
// include file for project specific include files that are used frequently, but are changed infrequently

///  std
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <set>
#include <cassert>

#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <time.h>
#include <math.h>

#include "half.hpp"
#include <tinyxml2.h>

///  SDL, Sound
#include <SDL.h>
#include <vorbis/vorbisfile.h>
///  Ogre
#include <Ogre.h>  //!
//#include <OgrePlatform.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include <btBulletDynamicsCommon.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>

// fix warning
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
