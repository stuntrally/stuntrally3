## About

This page describes how to build Stunt Rally 3 on **Linux** (Debian-based).  
_How to build on Windows is on other [page](BuildingVS.md)._  

It is **detailed** and has all steps done **manually**.  
_All these steps are done quicker in provided .py script as written in [Building](Building.md)._  

Tested on GNU/Linux, Debian 11 and 12. It should work on Debian-based (like Ubuntu etc).  
_First steps here are same as in my [ogre3ter-demo](https://github.com/cryham/ogre3ter-demo/) it could be easier to build that for start._  
Guide below has setup steps for empty Debian 12:  

## 1. Basic

Basic setup for building C++ etc:  
`sudo apt-get install g++ binutils gdb git make cmake ninja-build`

## 2. Setup dependencies

First install Ogre dependencies, as in [here](https://github.com/OGRECave/ogre-next#dependencies-linux)  
`sudo apt-get install libfreetype6-dev libfreeimage-dev libzzip-dev libxrandr-dev libxcb-randr0-dev libxaw7-dev freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libx11-xcb-dev libxcb-keysyms1-dev doxygen graphviz python-clang libsdl2-dev`

## 3.1 Build Ogre-Next

- Build Ogre-Next from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  

- Save the file [build_ogre_linux_c++latest.sh](https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_linux_c%2B%2Blatest.sh) and put it inside our root folder, called here `dev/`

- We need Ogre-Next branch v3.0 now. So edit `build_ogre_linux_c++latest.sh` and replace in this top line:  
  `OGRE_BRANCH_NAME="master"` to `OGRE_BRANCH_NAME="v3-0"` and save file.

- Go into `dev/` and start it:  
  ```
  cd dev/
  chmod +x ./build_ogre_linux_c++latest.sh
  ./build_ogre_linux_c++latest.sh
  ```

- This should succeed after a longer while and build Ogre-Next with its dependencies.

- If so you can start and check Ogre demos and samples inside:  
  `dev/Ogre/ogre-next/build/Release/bin/`  
  It's good to check if they work before continuing.

## 3.2 Ogre components, build
- Important step, we need to have below 2 components built with Ogre.
  
  In all existing / needed configurations (all 3 below):  
  _(Release is enough if all goes well, if not then Debug too)_
  ```
  cd dev/
  cd Ogre/ogre-next/build/Debug
  cd Ogre/ogre-next/build/Release
  cd Ogre/ogre-next/build/RelWithDebInfo
  ```
  edit the file `CMakeCache.txt` and be sure the lines:
  ```
  OGRE_BUILD_COMPONENT_ATMOSPHERE:BOOL=ON
  ...
  OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS:BOOL=ON
  ```
  have ON at end. _At least PLANAR_REFLECTIONS is not by default._

- Now **build Ogre** again. So, in all existing / needed configurations (3 folders above),  
  start `make -j6` or `ninja`, depends on what was used.  
  This will take less time than first time.  
  _If you want a full rebuild, do `make clean` or `ninja clean` before._

- _Note:_ I'm not sure how to do this quicker (set both ON before 1st build).  
  It would need setting TRUE in `Ogre/ogre-next/CMakeLists.txt` at end of line with:
  ```
  option( OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS "Component to use planar reflections, can be used by both HlmsPbs & HlmsUnlit" TRUE )
  ```
  and option( OGRE_BUILD_COMPONENT_ATMOSPHERE has TRUE already.

- _Optional note_.  
  This (rebuild Ogre) step is also needed, after updating Ogre sources to latest (`git pull` in `Ogre/ogre-next/`).  
  Caution: if this was done later, after building MyGui and/or StuntRally3,  
  then you need to rebuild also MyGui and then StuntRally3.
 
## 4. Build MyGui-Next

Get MyGui-Next, my fork of MyGui on branch `ogre3`.  
I follow its build guide [here](https://github.com/cryham/mygui-next/tree/ogre3).  
It needs to have set (should be by default):  
   - MYGUI_RENDERSYSTEM: 8 - Ogre 3.x
   - MYGUI_USE_FREETYPE: yes
   - all MYGUI_BUILD*: no
   - MYGUI_STATIC: no, MYGUI_DONT_USE_OBSOLETE: no

We need Release build, and possibly Debug too.  
```
git clone https://github.com/cryham/mygui-next --branch ogre3 --single-branch
cd mygui-next
mkdir build
cd build
mkdir Debug
cd Debug
cmake ./../.. -DCMAKE_BUILD_TYPE="Debug"
make -j6

cd ..
mkdir Release
cd Release
cmake ./../.. -DCMAKE_BUILD_TYPE="Release"
make -j6
```

## 5. Get deps for SR3

We use [Conan](https://conan.io/) to get all deps (same as for original Stunt Rally), info on old [wiki here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=compile) for CI builds.  
This is optional but recommended, especially on Windows.  
See [CMakeLists.txt](/CMakeLists.txt) and adjust if needed, it has some of my setup fixes, for SDL2, OpenAL etc.  

To get Conan: (needs Python3, is already present)  
```
sudo apt-get install python3-pip
pip install conan
```

## 6. Clone SR3

Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `data/tracks`:  
```
cd dev/
git clone https://github.com/stuntrally/stuntrally3.git sr3
cd sr3/data

git clone https://github.com/stuntrally/tracks3.git tracks
cd ..
```

## 7. Build SR3

At first we need to:
- rename `CMake/` dir to e.g. `CMakeCI/`
- rename `CMakeManual/` dir to `CMake/`
- rename `CMakeLists.txt` to e.g. `CMakeListsCI.txt`
- rename `CMakeLists-Debian.txt` to `CMakeLists.txt`

Then Inside `dev/sr3`:
```
mkdir build
cd build
cmake ../
(or: cmake ../ -DCMAKE_BUILD_TYPE="Debug" - for better debugging of issues)
make -j5
(or: ninja - if not using makefile)
```

## 8. Start StuntRally3

If it succeeds, go into one of:  
`sr3/bin/RelWithDebInfo` - if used `cmake ../`  
`sr3/bin/Release` - if used `cmake ../ -DCMAKE_BUILD_TYPE="Release"`  
`sr3/bin/Debug` - if used `cmake ../ -DCMAKE_BUILD_TYPE="Debug"`  
and start the executable:  
`./sr-editor3` - for SR3 Track Editor  
`./stuntrally3` - for SR3 game  

## 9. Running

For any crashes or issues, check logs inside:  
`/home/user/.config/stuntrally3`  
it has current logs and configs (_ed for SR3 Track Editor, without for SR3 game):
```
editor.cfg - editor options
game.cfg - game options

Ogre.log - log from game
Ogre_ed.log - log from editor

ogre.cfg - Ogre start window, Render Systems setup
ogre_ed.cfg - same, for editor

MyGUI.log - MyGui logs from game and editor
MyGUI_ed.log
```

----
## My folder tree

For reference here are all key paths and files:
```
dev - root folder
   sr3 - has this repo
      bin - output binaries
         Debug
         Release - has stuntrally3, sr3-editor, plugins.cfg, resources2.cfg
         RelWithDebInfo
      data
         tracks - has Test1-Flat, Test3-Bumps - cloned tracks3 repo (or old tracks)
   mygui-next
      build/Release/lib - has libMyGUI.Ogre2Platform.a, libMyGUIEngine.so.3.2.3
      build/Debug/lib - has libMyGUI.Ogre2Platform_d.a, libMyGUIEngine_d.so.3.2.3
   Ogre
      ogre-next
         build/Release/bin/ - has Ogre binaries, samples and tutorials
         build/Release/lib/ - has libOgreMain.so.3.0, RenderSystem_GL3Plus.so.3.0
         build/Debug/bin/ - has Ogre binaries, samples and tutorials
         build/Debug/lib/ - has libOgreMain_d.so.3.0, RenderSystem_GL3Plus_d.so.3.0
         Dependencies/lib/ - has libFreeImage.a, libfreetype.a
         OgreMain/include/ - has Ogre*.h
      ogre-next-deps
   build_ogre_linux_c++latest.sh - script used to get and build Ogre

```
Note: I use `../Ogre/ogre-next` dir with Ogre. But CMake default is `Dependencies/Ogre`, it can be as link.
