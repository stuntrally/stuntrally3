
# Developing Guide

Guide for SR3 source code, Ogre-Next (links) and tools used.  
Can be useful if you want to:
- develop SR3
- learn more about SR3 code
- learn about Ogre-Next used in SR3 or other libraries etc
- fork SR3 code and start your own project (required GPLv3 or newer) based on it

## IDE

Recommended:
- [Qt Creator](https://www.qt.io/download) - easier to set up first, CMake is integrated
- [VSCodium](https://github.com/VSCodium/vscodium/releases) - used by CryHam, needs more to set up. Info on [my guide](https://cryham.tuxfamily.org/cpp-guide/#VSCodium).  
Has its files in repo, in subdir `.vscode/`.  
Some extensions and IDE preferences in `settings.json` and  
in `launch.json` configurations (for binaries) are set up, to run or debug.

--------

# Ogre-Next

Is the engine used for rendering, managing scene, also loading resources, logging to `.log` files etc.

- [Manual](https://ogrecave.github.io/ogre-next/api/latest/manual.html) - need to read it when beginning.
- [Compositor](https://ogrecave.github.io/ogre-next/api/latest/compositor.html) - for effects, RTTs, reflections, etc.
- [comparison](https://www.ogre3d.org/about/what-version-to-choose) of Ogre and Ogre-Next.

----
## HLMS, materials, shaders

**HLMS** is the part in Ogre-Next responsible for generating shaders used in materials.  
There is no shader graph editor, everything is in big text files.  

- [HLMS doc](https://ogrecave.github.io/ogre-next/api/latest/hlms.html) - long read, explains all.
- [Ogre wiki](https://wiki.ogre3d.org/Ogre+2.1+FAQ#I_m_writing_my_own_Hlms_implementation_or_just_want_to_know_more_about_it._Where_do_I_find_learning_material_resources_) - with links to old forum topics about common tasks.

It is very complex and extensive, it covers plenty of conditions for shader variations (in .any files).  
We have own shaders for:
- **Terra** for terrain, with layers, triplanar, based on PBS.  
Main files in `/Media/Hlms/Terra/Any`.
- **PBS**, regular shaders for all materials (objects, road,  cars, vegetation etc).  
Main files in `/Media/Hlms/Pbs/Any/Main`.  
I'm writing **water**/fluids also inside PBS, don't want to split.
- Unlit (not lighting). We should only use this for **Gui**.  
**Particles** must be at least: colored by sun diffuse, darker by shadows, also by terrain.
Small files in `/home/ch/_sr/og3/_sr`/Media/Hlms/Unlit/Any`.

----
## ⚖️ Old Ogre comparison

Everything with materials is different. In **old** SR with Ogre, we used shiny material generator with its own `.shader` syntax and `.mat` files.  
Now in SR3 with Ogre-Next we use HLMS and **new** PBS materials.

### ⭐ Old

From old `.mat` files such parameters are gone:
- `ambient` color (nothing in new), `specular` power exponent (4th number, use new `roughness` for this)
- `env_map true`, `refl_amount`

### 🌠 New

We use default `specular_workflow`, metallic workflow is simpler.  
New HLMS `.material` parameters:
- roughness 0.001 to 1.0 (replaces old specular power exponent).
- metalness (not to be used in specular_workflow?).
- fresnel is amount aand fresnel_coeff is color of _reflection_ (mirror)
- _ToDo:_ reflection (old `env_map`), is now as re="1" in `presets.xml`.
- for terrain layer textures these parames are in `presets.xml` as `ro - roughness, me - metalness, re - reflection`.

Currently editing files is the only way like before.  
Main `.material` **files** are in `/Media/materials/Pbs`.  
Materials can be adjusted real-time in SR3 Track Editor: from Options, on tab **Tweak**, find material and use sliders etc.  
_ToDo:_ this should be our main tool, needs saving .material .json files.  
There is also that `.material.json` format, longer and more advanced, not too great to edit by hand.

----
## 🛠️ Debugging Tools

Advanced extra tools, should be used when testing or developing new (bigger) things in game or editor:
- [Renderdoc](https://renderdoc.org/) - For debugging GPU, tracking rendered frame, its draw calls, shaders, textures etc.
- [Valgrind](https://valgrind.org/) - For debugging CPU, memory leaks etc.
- [ASAN](https://clang.llvm.org/docs/AddressSanitizer.html) - same as above?


--------

# Stunt Rally 3

This section is about our code, mostly written by CryHam.

## New

Components completely new with Ogre-Next.

### HlmsPbs2

### Compositor

### Atmosphere

## Old

Same in SR 2.x

### Road

### Simulation

Update

### Replay
