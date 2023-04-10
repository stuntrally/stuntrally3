
On Windows CMake will fail, due to other paths (try `Dependencies/Ogre` maybe).

Only tested on GNU/Linux, Debian 11. But should also work on Debian-based.  
_First steps here are same as in my [ogre3ter-demo](https://github.com/cryham/ogre3ter-demo/) it could be easier to build that for start._  
Guide below has setup steps for empty Debian 11:  

1. Basic setup for building C++ etc:  
`sudo apt-get install g++ binutils gdb git make cmake ninja-build`

2. First install Ogre dependencies, as in [here](https://github.com/OGRECave/ogre-next#dependencies-linux)  
`sudo apt-get install libfreetype6-dev libfreeimage-dev libzzip-dev libxrandr-dev libxcb-randr0-dev libxaw7-dev freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libx11-xcb-dev libxcb-keysyms1-dev doxygen graphviz python-clang libsdl2-dev`

3. Build **Ogre-Next** from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  

- Save the file [build_ogre_linux_c++latest.sh](https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_linux_c%2B%2Blatest.sh) and put it inside our root folder, called here `dev/`

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


4. Get **MyGui-Next**, my fork of MyGui on branch `ogre3`.  
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

6. We use [Conan](https://conan.io/) to get all deps (same as for original Stunt Rally), info on old [wiki here](https://stuntrally.tuxfamily.org/wiki/doku.php?id=compile).  
This is optional but recommended, especially on Windows.  
See [CMakeLists.txt](/CMakeLists.txt) and adjust if needed, it has some of my setup fixes, for SDL2, OpenAL etc.  

To get Conan: (needs Python3, is already present)  
```
sudo apt-get install python3-pip
pip install conan
```
7. Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `Media/tracks`:  
```
cd dev/
git clone https://github.com/stuntrally/stuntrally3.git sr3
cd sr3/Media

git clone https://github.com/stuntrally/tracks3.git tracks
cd ..
```

8. Build SR3. Inside `dev/sr3`:
```
mkdir build
cd build
cmake ../
(or: cmake ../ -DCMAKE_BUILD_TYPE="Debug" - for better debugging of issues)
make -j5
(or: ninja - if not using makefile)
```

9. If it succeeds, go into one of:  
`sr3/bin/RelWithDebInfo` - if used `cmake ../`  
`sr3/bin/Release` - if used `cmake ../ -DCMAKE_BUILD_TYPE="Release"`  
`sr3/bin/Debug` - if used `cmake ../ -DCMAKE_BUILD_TYPE="Debug"`  
and start the executable:  
`./sr-editor3` - for SR3 Track Editor  
`./stuntrally3` - for SR3 game  

10. For any crashes or issues, check logs inside:  
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
### My folder tree
```
dev - root folder
   sr3 - has this repo
      bin - output binaries
         Debug
         Release - has stuntrally3, sr3-editor, plugins.cfg, resources2.cfg
         RelWithDebInfo
      Media
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
