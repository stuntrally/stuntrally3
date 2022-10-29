## Description

Very early efforts to port latest [Stunt Rally](https://github.com/stuntrally/stuntrally) 2.7 to latest [Ogre-next](https://github.com/OGRECave/ogre-next) 3.0.  

[Roadmap here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=roadmap#sr_3), Help and contributing is welcome.

Forum: [SR topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18515), [Ogre topic]().

## Building

On Windows CMake will fail, due to other paths.

Only tested on GNU/Linux, Debian 11.  

It is using Conan (optionally) to get all deps for original Stunt Rally, info on old [wiki here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=compile).

Needs to have Ogre-Next built from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).

See [CMakeLists.txt](/CMakeLists.txt) and adjust if needed, it has some fixes, for SLD2, OpenAL etc.
