## Description

WIP Port and continuation of latest [Stunt Rally](https://github.com/stuntrally/stuntrally) 2.7 using latest [Ogre-Next](https://github.com/OGRECave/ogre-next) 3.0.  

Already playable game in Single Player modes.  
WIP SR3 Track Editor.

For details see [Roadmap](https://stuntrally.tuxfamily.org/wiki/doku.php?id=roadmap#sr_3) with missing features and todo. Help and contributing is welcome.

Progress posts on [SR forum](https://forum.freegamedev.net/viewtopic.php?f=81&t=18515). Dev questions in [Ogre topic](https://forums.ogre3d.org/viewtopic.php?t=96576).

Note: sources have emojis, [this file](/src/emojis) has all, with quick components guide etc.

----

## Building

On Windows CMake will fail, due to other paths (try `Dependencies/Ogre`).

Only tested on GNU/Linux, Debian 11.  

To build we first need to:

1. Build *Ogre-Next* from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output), works well.  
I use `../Ogre/ogre-next` dir with Ogre. But CMake default is `Dependencies/Ogre`, it can be as link.

2. Build *MyGui-Next* (my fork of latest MyGui on branch `ogre3`) from [sources](https://github.com/cryham/mygui-next/tree/ogre3), using above Ogre-Next for its deps.  
I use `../mygui-next` for MyGui.  
Needs to have set:  
    - MYGUI_RENDERSYSTEM: 8 - Ogre 3.x
    - MYGUI_USE_FREETYPE: yes
    - all MYGUI_BUILD*: no, CMAKE_BUILD_TYPE: RelWithDebInfo
	- MYGUI_STATIC: no, MYGUI_DONT_USE_OBSOLETE: no

3. Clone this repo and  
Use Conan to get all deps (same as for original Stunt Rally), info on old [wiki here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=compile).  
This is optional but recommended, especially on Windows.  
See [CMakeLists.txt](/CMakeLists.txt) and adjust if needed, it has some of my setup fixes, for SDL2, OpenAL etc.  

4. Have [SR tracks](https://github.com/stuntrally/tracks) repo inside `Media/tracks`, cloned or as link.


### My folder tree
```
sr3 - with this repo
   bin - output binaries
      Debug
      Release - has StuntRally3, plugins.cfg, resources2.cfg
      RelWithDebInfo
   Media
      tracks - has Test1-Flat, Test3-Bumps
mygui-next
   build
      lib - has libMyGUI.Ogre2Platform.a, libMyGUIEngine.so.3.2.3
Ogre
   ogre-next
      build/Release/lib/ - has libOgreMain.so.3.0, RenderSystem_GL3Plus.so.3.0
      Dependencies/lib/ - has libFreeImage.a, libfreetype.a
      OgreMain/include/ - has Ogre*.h
   ogre-next-deps

/home/user/.config/stuntrally3 - has current game.cfg, ogre.cfg, Ogre.log, MyGUI.log
```