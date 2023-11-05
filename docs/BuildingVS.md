## About

This page describes how to compile Stunt Rally 3:
- on **Windows** (10 only tested)
- buiding first **all dependencies from sources**
- using **VS** 2019  
  Could be older but seems that newer versions aren't yet tested for Ogre-Next at least.  
  (_Visual Studio is a big commercial solution though_)  

Surely this is a **long** and somewhat **tedious** process, it can take **2 hours** or more to complete ⚠️.  

_SR3 depends on Ogre-Next 3.0, and MyGui fork for it which aren't released or packaged at all._  
_Other alternative could be getting Conan to work, or vcpkg. This is not done for SR3 ([WIP PR here](https://github.com/stuntrally/stuntrally3/pull/1))._  

_An easier project to build is my [ogre3ter-demo](https://github.com/cryham/ogre3ter-demo/) it only uses Ogre-Next._  

## 1. Getting Dependencies

We need tools: [Git](https://git-scm.com/downloads) and [CMake](https://cmake.org/download/) installed.

This table lists all needed dependencies (libraries) for SR3:

| **Library** | Version used | Website                                    | Downloads                                                                 |
|-------------|--------------|--------------------------------------------|---------------------------------------------------------------------------|
| tinyxml2    | 9.0.0        | <https://github.com/leethomason/tinyxml2/> | [Downloads](https://github.com/leethomason/tinyxml2/tags)                 |
| Bullet      | 3.25         | <https://pybullet.org/wordpress/>          | [Downloads](https://github.com/bulletphysics/bullet3/releases)            |
| Boost       | 1.81         | <https://www.boost.org>                    | [Downloads](https://sourceforge.net/projects/boost/files/boost-binaries/) |
| Enet        | 1.3.17       | <https://enet.bespin.org>                  | [Downloads](https://github.com/lsalzman/enet/tags)                        |
| Ogg         | 1.3.5        | <https://xiph.org>                         | [Downloads](https://xiph.org/downloads/)                                  |
| Vorbis      | 1.3.7        | <https://xiph.org>                         | [Downloads](https://xiph.org/downloads/)                                  |
| OpenAL Soft | 1.23.1       | <https://github.com/kcat/openal-soft>      | [tags](https://github.com/kcat/openal-soft/tags)                          |
|             |              |                                            |                                                                           |
| Ogre-Next, SDL2  | 3.0     | <https://www.ogre3d.org>                   | [git repo](https://github.com/OGRECave/ogre-next) not to Download, script |
| MyGui-next  | 3.?          | <http://mygui.info/>                       | [git repo](https://github.com/cryham/mygui-next/)                         |

Newer, latest versions can be used.  
Only Ogre-Next and MyGui-next are special cases, info in own, later sections.

## 2. Building dependencies

### 2.1

**Ogre-Next** soruces are downloaded (using Git) by a script, which will also get its dependencies like SDL2 and build them and itself.  
**MyGui-next** is my fork, it needs downloaded .zip with sources or cloned by Git.  
Both are explained later, for now no need to do anything.

### 2.2 

For **Boost**, since I used VS 2019 14.2 (upgrade 2, not 3 yet) I downloaded boost_1_81_0-msvc-14.2-64.exe to match it.  
For me this e.g. 14.2 is shown in *VS Installer* app on tab *Individual components* as: C++ Modules for v142 build tools (x64/x86)  
and are present and picked in Project properties, in General tab, Platform Toolset.  
This is an `.exe` that will extract Boost files to a typed folder.  

### 2.3

**Bullet** is different, it has a file [`build_visual_studio_vr_pybullet_double_dynamic.bat`](https://github.com/bulletphysics/bullet3/blob/master/build_visual_studio_vr_pybullet_double_dynamic.bat).  
Important line in this `.bat` is starting with: `premake4  --dynamic-runtime --double  `  
Meaning bullet will use double types and dynamic DLL for runtime.  
Start this `.bat` file, it that will generate a folder bullet3\vs2010\ with `0_Bullet3Solution.sln` file,  
open it and build with your VS (first convert dialog will show).  
You can build whole solution, there is more stuff.  
We definitely need to build: BulletCollision, BulletDynamics, BulletFileLoader, BulletWorldImporter, LinearMath.  
_Those are older bullet 2 libs (bt* names), not the bullet 3 new (b3* names)._

### 2.4

The rest: **tinyxml2, Enet, Ogg, Vorbis, OpenAL Soft**, for each:  
download `.zip` file with sources for latest stable release version and extract in root folder e.g. `c:\dev`  
Resulting in e.g. these dirs:
```
c:\dev\
  bullet3-3.25
  enet-1.3.17
  libogg-1.3.5
  libvorbis-1.3.7
  openal-soft-1.23.1
  tinyxml2-9.0.0
```

These libs have `CMakeLists.txt` file inside, which means we need to:  
**for each lib**: start CMake-Gui, pick sources folder to where you extracted it  
e.g. `c:\dev\enet-1.3.17`, and then new `build` folder inside e.g. to `c:\dev\enet-1.3.17\build`  
After that set, press Configure, twice, and lastly Generate.  
Now there should be a `.sln` file inside build\ dir.  

Open it (with your VS) and build solution.  
We need to build `Release x64` (64 bit compiler, Release configuration), _later if needed Debug for any issue finding._

Many of these libraries will build just as _Static library .lib_. That's okay, _these will link with our exe, not be in own DLL_.  

For **enet** also these system libs were needed to add in Properties - Linker input:  
winmm.lib;Ws2_32.lib;

Very **important** thing is that all should use in:  
Project Properties - C++ - Code Generation - Runtime Library: **Multi Threaded DLL**, and for Debug same but with Debug DLL.  
This was set so in all but best to be sure. _Failing that would lead to many linker issues much later when linking SR3._  


## 3. Building Ogre-Next

Building Ogre-Next from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  

Save the file [build_ogre_Visual_Studio_16_2019_x64.bat](https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_Visual_Studio_16_2019_x64.bat)  
and put it inside our root folder `c:\dev` then open cmd and start the `.bat`.

This should succeed after a longer while and build Ogre-Next with its dependencies.

If so you can start and check Ogre demos and samples inside:  
  `c:\dev\Ogre\ogre-next\build\bin\Release\`  
  It's good to check if they work before continuing.

### 3.1 Ogre components, build
- Important step, we need to have below 2 components built with Ogre.
  
  In `c:\dev\Ogre\ogre-next\build\`  
  edit the file `CMakeCache.txt` and be sure the lines:
  ```
  OGRE_BUILD_COMPONENT_ATMOSPHERE:BOOL=ON
  ...
  OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS:BOOL=ON
  ```
  have ON at end. _At least PLANAR_REFLECTIONS is not by default._

- Now **build Ogre** again. So, in all needed configurations, by opening solution  
  `OGRE-Next.sln` from above `build\` dir, and building in VS.  
  This will take less time than first in cmd.  

- _Note:_ I'm not sure how to do this quicker (set both ON before 1st build).  
  It would need setting TRUE in `Ogre/ogre-next/CMakeLists.txt` at end of line with:
  ```
  option( OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS "Component to use planar reflections, can be used by both HlmsPbs & HlmsUnlit" TRUE )
  ```
  and option( OGRE_BUILD_COMPONENT_ATMOSPHERE has TRUE already.

- _Optional note_.  
  This (rebuild Ogre) step is also needed, after updating Ogre sources to latest (`git pull` in `Ogre/ogre-next/`).  
  Caution: if this was done later, after building MyGui and/or StuntRally3,  
  then you need to rebuild also MyGui and then StuntRally3 too.
 
## 4. Building MyGui-Next

MyGui-Next is my fork of MyGui on branch `ogre3`.  
I follow its build guide [here](https://github.com/cryham/mygui-next/tree/ogre3).  

It needs to have set (should be by default):  
   - MYGUI_RENDERSYSTEM: 8 - Ogre 3.x
   - MYGUI_USE_FREETYPE: yes
   - all MYGUI_BUILD*: no
   - MYGUI_STATIC: no, MYGUI_DONT_USE_OBSOLETE: no

You could donwload .zip with sources, but `git clone` is better,  
since you can later do `git pull` to get any updates.

Inside `c:\dev\`
```
git clone https://github.com/cryham/mygui-next --branch ogre3 --single-branch
```
Open CMake-Gui, pick sources `c:\dev\mygui-next`, build to `c:\dev\mygui-next\build`.  
Press Configure twice, then Generate.  

We need Release build, _and possibly Debug too._  

**WIP**: There are probably wrong paths there, that need editing and fixing in CMakeLists.txt.

## 6. Clone SR3

Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `data/tracks`:  
In `c:\dev\`
```
git clone https://github.com/stuntrally/stuntrally3.git sr3
cd sr3/data

git clone https://github.com/stuntrally/tracks3.git tracks
```

## 7. Build SR3

_ToDo:_ Rename the file CMakeLists-VS.txt to CMakeLists.txt.  

Open CMake-Gui, pick sources `c:\dev\sr3` and build as `c:\dev\sr3\build`.  


## 8. Start StuntRally3

If it succeeds, go into one of:  
`sr3/bin/RelWithDebInfo` - if used `cmake ../`  
`sr3/bin/Release` - if used `cmake ../ -DCMAKE_BUILD_TYPE="Release"`  
`sr3/bin/Debug` - if used `cmake ../ -DCMAKE_BUILD_TYPE="Debug"`  
and start the executable:  
`./sr-editor3` - for SR3 Track Editor  
`./stuntrally3` - for SR3 game  
