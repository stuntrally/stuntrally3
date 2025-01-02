## About

How to build on Windows is on other [page](BuildingVS.md).

It should work on Debian-based (like Ubuntu etc).  
Tested on GNU/Linux: Debian 12 and Kubuntu 20.04.  

## 1. Basic setup

Basic setup for building C++ etc. Start in terminal:  
`sudo apt-get -y install g++ binutils gdb git make cmake ninja-build`

Also install [Conan](https://conan.io/downloads)

## 2. Clone SR3

First create a new folder `stuntrally3` e.g. in your home dir (`mkdir stuntrally3`), and go into it (`cd stuntrally3`).

Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `data/tracks`:  
```
git clone https://github.com/stuntrally/stuntrally3.git .
git clone https://github.com/stuntrally/tracks3.git data/tracks
```

## 3. Add the conan remote

Open up a terminal and run the following command:
`conan remote add rigs-of-rods-deps https://conan.cloudsmith.io/rigs-of-rods/deps/`

## 4. Running CMake

Run the following command in the `stuntrally3` folder to generate the ninja build files:

```bash
cmake . -Bbuild \
  -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=CMake/conan_provider.cmake
```

## 5. Compiling

cd into the build directory and run the following command:
`ninja`

## 6. Start StuntRally3

If build succeeded, go into `stuntrally3/bin/Release`  
and start the executable:  
`./sr-editor3` - for SR3 Track Editor  
`./stuntrally3` - for SR3 game  

## 7. Running

For any crashes or issues, check logs inside:  
`/home/user/.config/stuntrally3`  
it has current logs:
```
Ogre.log - log from game
Ogre_ed.log - log from editor
```
More info and my folder tree with all key paths and files is at end of [Building-old](Building-old.md#9-running).
