## Description

Early efforts to port latest [Stunt Rally](https://github.com/stuntrally/stuntrally) 2.7 to latest [Ogre-next](https://github.com/OGRECave/ogre-next) 3.0.  

[Roadmap here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=roadmap#sr_3), Help and contributing is welcome.

Forum: [SR topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18515), [Ogre topic](https://forums.ogre3d.org/viewtopic.php?t=96576).

## Building

On Windows CMake will fail, due to other paths.

Only tested on GNU/Linux, Debian 11.  

It is using Conan (optionally) to get all deps for original Stunt Rally, info on old [wiki here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=compile).

Needs to have Ogre-Next built from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  
I use `../Ogre/ogre-next` dir with Ogre. But CMake default is `Dependencies/Ogre`, it can be as link.

See [CMakeLists.txt](/CMakeLists.txt) and adjust if needed, it has some of my setup fixes, for SLD2, OpenAL etc.  

Also needs [SR tracks](https://github.com/stuntrally/tracks) repo inside `Media/tracks` cloned or as link.
