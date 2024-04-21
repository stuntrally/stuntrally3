## About

How to build on Windows is on other [page](BuildingVS.md).

This page describes how to *quickly* build Stunt Rally 3 on **Linux** (Debian-based), using provided .py script.  

_There is a detailed page [Building-old](Building-old.md) with all steps done manually without .py script._  

Tested on GNU/Linux, Debian 12. It should work on Debian-based (like Ubuntu etc).  

_By default CMakeLists.txt is using Conan for our [CI builds](https://github.com/stuntrally/stuntrally3/actions) (and its CMake setup from this [PR here](https://github.com/stuntrally/stuntrally3/pull/1))._  

## 1. Basic setup

Basic setup for building C++ etc:  
`sudo apt-get -y install g++ binutils gdb git make cmake ninja-build`

## 2. Ogre dependencies

Setup for Ogre dependencies (as in [here](https://github.com/OGRECave/ogre-next#dependencies-linux)):  
`sudo apt-get -y install libfreetype6-dev libfreeimage-dev libzzip-dev libxrandr-dev libxcb-randr0-dev libxaw7-dev freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libx11-xcb-dev libxcb-keysyms1-dev doxygen graphviz python-clang libsdl2-dev rapidjson-dev`

## 3. SR3 dependencies

Needed to build SR3 itself:  
`sudo apt-get -y install libbullet-dev libbullet-extras-dev libtinyxml2-dev libenet-dev libogg-dev libvorbis-dev libopenal-dev libboost-system-dev libboost-thread-dev`

## 4. Build all

First create a new folder `dev/` e.g. in your home dir (`mkdir dev`), and go into it (`cd dev/`).

See remarks on top of [build-sr3-Linux.py](../build-sr3-Linux.py) file.

Download just the [build-sr3-Linux.py](../build-sr3-Linux.py) file, and put it inside `dev/`.

Start this Python script, e.g. in terminal by:  
`python3 ./build-sr3-Linux.py`

## 5. Start StuntRally3

If build succeeded, go into `sr3/bin/Release`  
and start the executable:  
`./sr-editor3` - for SR3 Track Editor  
`./stuntrally3` - for SR3 game  

## 6. Running

For any crashes or issues, check logs inside:  
`/home/user/.config/stuntrally3`  
it has current logs:
```
Ogre.log - log from game
Ogre_ed.log - log from editor
```
More info and my folder tree with all key paths and files is at end of [Building-old](Building-old.md#9-running).
