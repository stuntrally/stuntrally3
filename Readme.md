## Description

WIP Port and continuation of latest [Stunt Rally](https://github.com/stuntrally/stuntrally) 2.7 using latest [Ogre-Next](https://github.com/OGRECave/ogre-next) 3.0.  

Already playable game in Single Player modes. WIP SR3 Track Editor.  

Changes and new features listed on top of [changelog](https://stuntrally.tuxfamily.org/wiki/doku.php?id=changelog).  
For details see [Roadmap](https://stuntrally.tuxfamily.org/wiki/doku.php?id=roadmap#sr_3) with missing features and todo. Help and contributing is welcome.

Progress posts on [SR forum](https://forum.freegamedev.net/viewtopic.php?f=81&t=18515). Dev questions in [Ogre topic](https://forums.ogre3d.org/viewtopic.php?t=96576).

Note: sources have emojis, [this file](/src/emojis) has all, with quick components guide etc.


----

## Building

On Windows CMake will fail, due to other paths (try `Dependencies/Ogre`).

Only tested on GNU/Linux, Debian 11. But should also work on Debian-based.  
_First steps here are same as in my [ogre3ter-demo](https://github.com/cryham/ogre3ter-demo/) it could be easier to build that for start._
Guide below has setup steps for empty Debian 11:  

1. Basic setup for building C++ etc:  
`sudo apt-get install g++ binutils gdb git make cmake ninja-build`

2. First install Ogre dependencies, as in [here](https://github.com/OGRECave/ogre-next#dependencies-linux)  
`sudo apt-get install libfreetype6-dev libfreeimage-dev libzzip-dev libxrandr-dev libxcb-randr0-dev libxaw7-dev freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libx11-xcb-dev libxcb-keysyms1-dev doxygen graphviz python-clang libsdl2-dev`

3. Build **Ogre-Next** from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  

- Save the file [build_ogre_linux_c++latest.sh](https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_linux_c%2B%2Blatest.sh) and put it inside our root folder, called here e.g. `dev/`

- Go into `dev/` and start it:  
```
cd dev/
chmod +x ./build_ogre_linux_c++latest.sh
./build_ogre_linux_c++latest.sh
```

- This should succeed after a longer while and build Ogre-Next with its dependencies.

- If so you can start and check Ogre demos and samples inside:  
`dev/Ogre/ogre-next/build/Release/bin/`  
Good to check if they work before continuing.


4. Build *MyGui-Next* (my fork of latest MyGui on branch `ogre3`) from [sources](https://github.com/cryham/mygui-next/tree/ogre3), using above Ogre-Next for its deps.  
I use `../mygui-next` for MyGui.  
Needs to have set:  
    - MYGUI_RENDERSYSTEM: 8 - Ogre 3.x
    - MYGUI_USE_FREETYPE: yes
    - all MYGUI_BUILD*: no, CMAKE_BUILD_TYPE: RelWithDebInfo
    - MYGUI_STATIC: no, MYGUI_DONT_USE_OBSOLETE: no

5. We use [Conan](https://conan.io/) to get all deps (same as for original Stunt Rally), info on old [wiki here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=compile).  
This is optional but recommended, especially on Windows.  
See [CMakeLists.txt](/CMakeLists.txt) and adjust if needed, it has some of my setup fixes, for SDL2, OpenAL etc.  

To get Conan: (needs Python3, is already present)  
```
sudo apt-get install python3-pip
pip install conan
```
6. Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `Media/tracks`:  
```
cd dev/
git clone https://github.com/stuntrally/stuntrally3.git sr3
cd sr3/Media

git clone https://github.com/stuntrally/tracks3.git tracks
cd ..
```

7. Build SR3. Inside `dev/sr3`:
```
mkdir build
cd build
cmake ../
(or: cmake ../ -DCMAKE_BUILD_TYPE="Debug" - for better debugging of issues)
make -j5
(or: ninja - if not using makefile)
```


----
### My folder tree
```
dev - root folder
   sr3 - has this repo
      bin - output binaries
         Debug
         Release - has StuntRally3, plugins.cfg, resources2.cfg
         RelWithDebInfo
      Media
         tracks - has Test1-Flat, Test3-Bumps - cloned tracks3 repo (or old tracks)
   mygui-next
      build
         lib - has libMyGUI.Ogre2Platform.a, libMyGUIEngine.so.3.2.3
   Ogre
      ogre-next
         build/Release/bin/ - has Ogre binaries, samples and tutorials
         build/Release/lib/ - has libOgreMain.so.3.0, RenderSystem_GL3Plus.so.3.0
         Dependencies/lib/ - has libFreeImage.a, libfreetype.a
         OgreMain/include/ - has Ogre*.h
      ogre-next-deps
   build_ogre_linux_c++latest.sh - script used to get and build

/home/user/.config/stuntrally3 - has current game.cfg, ogre.cfg, Ogre.log, MyGUI.log

```
Note: I use `../Ogre/ogre-next` dir with Ogre. But CMake default is `Dependencies/Ogre`, it can be as link.
