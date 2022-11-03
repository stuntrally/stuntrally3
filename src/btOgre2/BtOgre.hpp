#pragma once

#include <Ogre.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef BTOGRE_VERBOSE_CHECK
#pragma message ("You are using OGRE " STR(OGRE_VERSION_MAJOR) "." STR(OGRE_VERSION_MINOR) " " OGRE_VERSION_NAME)
#endif

// #if (OGRE_VERSION_MAJOR >= 2 && OGRE_VERSION_MINOR >= 1)
// #ifdef BTOGRE_VERBOSE_CHECK
// #pragma message ("Your version of Ogre is compatible")
// #undef BTOGRE_VERBOSE_CHECK
// #endif
// #else
// #error "Ogre 2.1 or later required!"
// #endif

#include "BtOgreGP.h"
#include "BtOgrePG.h"
#include "BtOgreExtras.h"
