## 1. Getting Dependencies

We need the following tools for building SR3: 

- [Visual Studio 2019 Community](https://visualstudio.microsoft.com/downloads/) (In the "Workloads" tab enable **Desktop development with C++** and in "Individual components" enable "Windows Universal CRT SDK")

- [CMake](https://cmake.org/download/)

- [Conan](https://conan.io/downloads)

- [Git](https://git-scm.com/) (Leave all options to their defaults)


## 2. Clone SR3

Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `data/tracks`:  
In `c:\dev\`
```
git clone https://github.com/stuntrally/stuntrally3.git sr3
cd sr3/data
git clone https://github.com/stuntrally/tracks3.git tracks
```

## 3. Add the conan remote

Open up a terminal and run the following command:
`conan remote add rigs-of-rods-deps https://conan.cloudsmith.io/rigs-of-rods/deps/`

## 4. Build SR3

Open CMake-Gui, pick sources `c:\dev\sr3` and build as `c:\dev\sr3\build`.

Click the *Add Entry* and add the following values:  
Name: `CMAKE_PROJECT_TOP_LEVEL_INCLUDES`  
Type: `filepath`  
Value: `cmake/conan_provider.cmake`  

Press Configure twice.  
If it fails, then probably need to adjust some paths in CMake files.  
If it succeeded then press Generate.  
Open `c:\dev\sr3\build\StuntRally3.sln` with VS (2019),  
pick `Release x64` configuration and build it.  

This should produce valid *.exe files

## 5. Start StuntRally3

If build succeeded, start:  
`c:\dev\sr3\bin\Release\StuntRally3.exe` - for SR3 game  
`c:\dev\sr3\bin\Release\SR-Editor3.exe` - for SR3 Track Editor  
and if for Debug then inside:  
`c:\dev\sr3\bin\Debug`  

Starting any `.exe` with `cfg` argument will show Ogre config dialog.

If Ogre dialogs shows empty, replace `plugins.cfg` with `plugins_Windows.cfg` again.  

Only `Open GL 3+ Rendering Subsystem` should be used (DX11 fails).  

Logs will be in:  
`C:\Users\USERNAME\AppData\Roaming\stuntrally3\`
