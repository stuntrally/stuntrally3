
# Developing Guide

Guide for Ogre-Next (links), tools used and general info for SR3 sources (code).  
Can be useful if you want to:
- develop SR3, test or fix a bug
- learn more about SR3 code or find something in it
- learn about Ogre-Next used in SR3 or other libraries etc
- fork SR3 code and start your own project (license required: GPLv3 or newer) based on it

## IDE

Recommended, for having e.g. go to definition, find references:
### [Qt Creator](https://www.qt.io/download)
Easier to set up first, CMake is integrated. Fast, but less options.

### [VSCodium](https://github.com/VSCodium/vscodium/releases)
Used by CryHam, needs more to set up. Info on [my guide](https://cryham.tuxfamily.org/cpp-guide/#VSCodium).  
Search in all files works super fast. Many extensions available.  

Extensions needed at least: 
- clangd by llvm-vs-code-extensions
- CodeLLDB by vadimcn
- CMake by twxs
- CMake Tools by ms-vscode
- Native Debug by webfreak

SR3 has VSCodium config files in subdir `.vscode/` in repo.  
Some extensions and IDE preferences in `settings.json` and  
in `launch.json` configurations (for binaries) are set up, to run or debug.

## 🛠️ Debugging Tools

Advanced, extra, external tools, should be used when testing or developing new (bigger) things in game or editor:
- [Renderdoc](https://renderdoc.org/) - For debugging GPU, tracking rendered frame, its draw calls, shaders, textures etc.  
[documentation](https://renderdoc.org/docs/index.html), [forum post](https://forums.ogre3d.org/viewtopic.php?p=554959#p554959) with quick help on using it, [video tutorials](https://www.youtube.com/results?search_query=tutorial+%22renderdoc%22).
- [Valgrind](https://valgrind.org/) - For debugging CPU, memory leaks etc.
- [ASAN](https://clang.llvm.org/docs/AddressSanitizer.html) - same as above?

--------

# Ogre-Next docs

Is the engine used for rendering, managing scene, also loading resources, logging to `.log` files etc.

- [Manual](https://ogrecave.github.io/ogre-next/api/latest/manual.html) - need to read it when beginning.
- [Compositor](https://ogrecave.github.io/ogre-next/api/latest/compositor.html) - for effects, RTTs, reflections, etc.
- [comparison](https://www.ogre3d.org/about/what-version-to-choose) of Ogre and Ogre-Next.

----
## HLMS, materials, shaders

**HLMS** (High Level Material System) is the part in Ogre-Next responsible for generating shaders used in materials.  
There is no shader graph editor, everything is in big text files.  

- [HLMS doc](https://ogrecave.github.io/ogre-next/api/latest/hlms.html) - **long** read, explains all.
- [HLMS shaders](https://ogrecave.github.io/ogre-next/api/latest/hlms.html#HlmsCreationOfShaders).
- [Ogre wiki](https://wiki.ogre3d.org/Ogre+2.1+FAQ#I_m_writing_my_own_Hlms_implementation_or_just_want_to_know_more_about_it._Where_do_I_find_learning_material_resources_) - with links to old forum topics about common tasks.

It is very complex and extensive, it covers plenty of conditions for shader variations (in .any files).  
We have own shaders for:
- **Terra** for terrain, with layers, triplanar, based on PBS.  
Main files in `Media/Hlms/Terra/Any`.
- **PBS**, regular shaders for all materials (objects, road,  cars, vegetation etc).  
Main files in `Media/Hlms/Pbs/Any/Main`.  
I'm writing **water**/fluids also inside PBS, don't want to split.
- Unlit (not lighting). We should only use this for **Gui**.  
**Particles** must be at least: colored by sun diffuse, darker by shadows, also by terrain.
Small files in `Media/Hlms/Unlit/Any`.

Shader code `.any` files are preferred, universal and get translated to `.glsl` or `.hlsl`.

----
## ⚖️ Materials comparison

Everything with materials is different.  
In **old** SR with Ogre, we used shiny material generator with its own `.shader` syntax and `.mat` files.  
Now in SR3 with Ogre-Next we use HLMS and **new** PBS materials.  

### ⭐ Old

From old `.mat` files such parameters are changed:
- `ambient` color - gone, nothing in new
- `specular` - power exponent (4th number), use new `roughness` for this
- `env_map true` - _ToDo:_ reflection is now as `re="1"` in `presets.xml`.
- `refl_amount` - now fresnel

### 🌠 New

We use default `specular_workflow`, metallic workflow is simpler.  
New HLMS `.material` parameters:
- `roughness` 0.001 to 1.0 (replaces old specular power exponent).
- `metalness` (not to be used in specular_workflow?).
- `fresnel` is _reflection_ (mirror) amount and
- `fresnel_coeff` is its color.

For terrain layer textures these parames are in `presets.xml` as `ro - roughness, me - metalness, re - reflection`.

All in code `Ogre/ogre-next/Components/Hlms/Pbs/src/OgreHlmsPbsDatablock.cpp`.  
Datablock is basically what has all material parameters.  

Currently editing files is the only way like before.  
Main `.material` **files** are in `/Media/materials/Pbs`.  
Materials can be adjusted real-time in SR3 Track Editor: from Options, on tab **Tweak**, find material and use sliders etc.  
_ToDo:_ this should be our main tool, needs saving .material .json files.  
There is also that `.material.json` format, longer and more advanced, not too great to edit by hand.


--------

# Stunt Rally 3

## config files 📄

[user] - means file is in user `.config/stuntrally3` dir, and saved by SR3.  
Otherwise, means a static file in `config/` dir, manually edited.  
(i) - means file has info about itself and syntax, written inside.

Vehicles config
- config/cars.xml - (i) list for Vehicles tab with manually set parameters
- Media/cars/cameras.xml - (i) all follow camera views for game
- Media/carsim/[mode]/* -  
*.car - vehicle file (can edit directly in game Alt-Z),  
*_stats.xml - vehicle performance test results (after Ctrl-F5)  
.tire coeffs, suspension

Game modes
- championships.xml - (i) for Gui, tutorials too
- challenges.xml - (i) with tracks list, pass conditions, game setup etc

Game progress
- progress.xml, progress_rev.xml - [user] progress for championships, _rev for reversed
- progressL.xml, progressL_rev.xml - [user] same but for challenges

Game config
- input.xml and input_p[1..4].xml - [user] for game, common and player input bindings, Gui tab Input

- paint.cfg - current vehicles paint setups
- paints.ini - dynamic list for vehicles on Paints tab

Common config
- fluids.xml - (i) 
- presets.xml - params for editor lists and game too
- tracks.xml - [user] tracks bookmark and rating, from Gui Track tab

Below * means: nothing or 2,3,4.. etc if more.
- heightmap*.f32 - heightmap(s) for terrain(s). Format is raw 32 bit floats (4B for 1 height). Size always square: 512,1024,2048 etc.
- road*.xml - file(s) for each road/river etc. Has many params and points.
- scene.xml - 1 file for each track, with all editable params from Gui tabs in Track Editor.

## emojis

You can also get familiar with `src/emojis` file.  
It lists many emojis used for indentifying code sections, variable blocks etc in sources.  
Done for better orientation, grouping and cooler code in files, especially for big ones with many at once.  
It is possible to search for all related to some component or aspect in sources,  
by searching for an emoji, e.g. 💨 for game boost, ⛰️ for all terrain stuff, 🎯 for all ray casts made, etc.

## subdirs 📂

SR3 code is in `src/` its subdirs are:
- [lib] [btOgre2](https://github.com/Ybalrid/BtOgre2) - for visualisation of bullet debug lines in Ogre-Next (called Ogre 2.1 before)
- common - files for both editor and game. Mainly for common Gui, scene (map / track) elements.  
Has also some MyGui classes a bit extended: `MultiList2.*, Slider*.*, Gui_Popup.*` and `GraphView.*` for game Graphs (Tweak F9).
- common/data - has all data structures (mostly `.xml` config files, list above).
- editor - purely editor sources
- game - purely game sources
- network - game multiplayer code, info in `DesignDoc.txt`. Also `master-server/`, small program for server game list (not used).
- OgreCommon - [Ogre-Next] base application classes - slightly changed.
- [lib] [oics](https://sourceforge.net/projects/oics/) - for input configurations, bindings and analog key inputs emulation.
- Terra - terrain and other components, from [Ogre-Next] - quite modified (more info below).
- sound - sound engine, based on [RoR]'s, using openal-soft
- vdrift - SR simulation (based on old [VDrift] from about 2010) with a lot of changes and custom code,  
Also has simulation for spaceships, sphere etc, and for `Buoyancy.*`.


----

# Ogre-Next

Ogre-Next has few components: Terra, Atmosphere and PlanarReflections in sources form.  
Those are included in SR3, inside `src/Terra/` and modified for our needs.  
The basic application code and setup is also included from Ogre-Next, inside `src/OgreCommon`, modified.  
Many key changes are marked with `//** `, searching it will find all modified places.


## Modified

Components completely new with Ogre-Next, included in SR3, and modified already.

### Atmosphere 🌫️

Only used for **fog** and its params.  
It can render sky atmosphere with sun (and no clouds). We don't use this. We could someday have dynamic sky shaders this way.  
Modified to not change light after its update. And added more params for shaders, fog etc.  

Our fog shader code is in: `Media/Hlms/Pbs/Any/Fog_piece_ps.any`  
and is used in Pbs and Terra `.any` there `@insertpiece( applyFog )`.

### Terra ⛰️

The terrain component. Very efficient. Has triplanar already.  

#### Comparison

This **changed** completely from old Ogre. Now heightmaps are power of 2, e.g. 1024, not 1025 like old.  
Old SR `heightmap.f32` are converted on 1st load in SR3. This also needs slight pos offset.  
SR3 loads `heightmap.f32` and gets size (e.g. 1024) from file size (1024*1024*4 B).  

#### Normalized

Terra by design had _normalized float heights_ from 0.0 to 1.0 only.  
We **don't** use that, changed it, since our Hmap and SR editor use any, real height values.  
_ToDo:_  
This **broke** `src/Terra/TerraShadowMapper.cpp` shadowmap generation (mostly for heights below 0).  
and terrain shadowmap is disabled.  
BTW it took way to long, 5 sec at start, possibly due to compute shader building.  
It's this `if (!m_bGenerateShadowMap)` and `return;  //**  5 sec delay` below.  

This also **broke** terrain page visibility. Likely decreasing Fps, no idea how much.  
Made all visible in `Terra::isVisible` for `if (!bNormalized)`.  
In ctor `Terra::Terra(` setting `bNormalized` to 1 does try to normalize Hmap.  

#### Extended

Terra Extended with old SR **blendmap** RTT and its noise.  
Also with emissive layers, property: `emissive terrain`.  
Changed skirt to be relative size below, not that default big to lowest point.
_ToDo:_ Some holes can appear on horizon terrains. Should be highe for them.


### PlanarReflection 🪞

Used for water/fluids.  
_ToDo_: Modified to have more control and detail options for its camera.

## Custom

These are using Ogre-Next base components but extend them to our specific rendering purposes.

### HlmsPbs2

Main **shaders** for all materials (except terrain and particles) here:  
`Media/Hlms/Pbs/Any/Main/800.PixelShader_piece_ps.any`  (modified)  
`Media/Hlms/Pbs/Any/Main/800.VertexShader_piece_vs.any`  (not yet)  
structures with variables passed to them are in:  
`Media/Hlms/Pbs/Any/Main/500.Structs_piece_vs_piece_ps.any`  

### Compositor


## Changed from SR

These have changed when porting SR to SR3 to use Ogre-Next meshes etc.

### Road 🛣️

Original code made by CryHam for: roads, pipes, their transitions and bridge walls, columns.  
Code inside:
- Road_Prepass.cpp - needed computations before rebuild.
- Road_Rebuild.cpp - main big code, creating geometry vertices and meshes.
- Grid.* - WIP new for paging grid and adding meshes together in cells, for less batches (draw calls).  
Now used for columns to gather more together.

Creating **mesh** code was based on Ogre-Next: `/Samples/2.0/ApiUsage/DynamicGeometry/DynamicGeometryGameState.cpp`.  
[Few posts](https://forums.ogre3d.org/viewtopic.php?p=554774#p554774) with info and issues (fixed).  

**Glass pipes** rendering changed. Old Ogre had 2 passes with opposite culling.  
New does not support passes [topic](https://forums.ogre3d.org/viewtopic.php?t=96902), and instead has:  
2 nodes with same mesh, but clones datablock and sets opposite cull in it. Code in `pipe glass 2nd item` section of `Road_Mesh.cpp`.

### Vegetation 🌳🪨

Means models for trees, rocks, plants etc.  
It is not paged, does not have impostors (like it was in old SR with paged-geometry).  
Simply uses Ogre-Next Items that will be automatically HW instanced, to reduce draw calls, also works with mesh LODs.  
All models are placed during track load by code in `src/common/SceneTrees.cpp`.  

### Grass 🌿

Currently done simplest way, has mesh pages, with vertices and indices.  
Quite inefficient to render and slow to create mesh.  
Code in `Grass.h` and `Grass_Mesh.cpp`.  
_ToDo_: add RTT for density map, read terrain height map in shader..  
[old topic](https://forums.ogre3d.org/viewtopic.php?t=85626&start=25)  


## SR

Rest of code that was same in SR 2.x. Only small changes when porting to SR3.

### Simulation 🚗

Game code in `Update_Poses.cpp` gets data from VDrift simulation (on 2nd thread) calling `newPoses(`.  
Rendering update calls `updatePoses(` to set vehicles (from carModels) to new poses.  

VDrift simulation, main update code in `src/vdrift/cardynamics_update.cpp` in `void CARDYNAMICS::UpdateBody(`.  
SR own code for non-car vehicles like: 🚀 Spaceship and 🔘 Sphere, is in `SimulateSpaceship(` and `SimulateSphere(`.  

### Replay 📽️

All files in `src/game/` with `replay` in name, mainly `Replay.h and .cpp` and `Gui_Replay.cpp` for Gui events.  
Partly code in `Update_Poses.cpp` for filling replay/ghost data etc.
Mode detail in `CGame.h` sections with vars for this.

### Sound 🔉

Sound manager code in `src/sound/`. Is based on RoR's code. Completely replaced VDrift's code.


----

# 🟢 Ogre-Next - Advanced 🧰🔧

Explaining how to do advaced things, that need extending HLMS, shaders and .cpp code for it too.  
Some utility code `src/common/AppGui_Util.cpp` with few really basic things that needed methods now.

Some links (few also in [post here](https://forums.ogre3d.org/viewtopic.php?p=554575#p554575)) with good info (may be somewhat random and really old):
- [recent longer 2 replies](https://forums.ogre3d.org/viewtopic.php?p=555022#p555022)
- [wiki links](https://wiki.ogre3d.org/Ogre+2.1+FAQ#I_m_writing_my_own_Hlms_implementation_or_just_want_to_know_more_about_it._Where_do_I_find_learning_material_resources_)
- [done] [Easier way to communicate with hlms shader?](https://forums.ogre3d.org/viewtopic.php?f=25&t=83081) - old, **good** with stuff below too
- [done] [Adding HLMS customisations per datablock](https://forums.ogre3d.org/viewtopic.php?t=84775)
- [done] [Adding Wind to Grass](https://forums.ogre3d.org/viewtopic.php?t=95892)
- [done, old] [custom hlms wind and fog](https://spotcpp.com/creating-a-custom-hlms-to-add-support-for-wind-and-fog/)
- [?Compositors, HLMS and depth buffers](https://forums.ogre3d.org/viewtopic.php?p=543364#p543364)
- [done] [basic using in app](https://ogrecave.github.io/ogre-next/api/2.3/_using_ogre_in_your_app.html)

## HLMS extending 🌠

Customizing [doc link](https://ogrecave.github.io/ogre-next/api/latest/hlms.html#HlmsCreationOfShadersCustomizing).

From docs: Hlms implementation can be customized:
- Through HlmsListener.  
This allows you to have access to the buffer pass to fill extra information; or bind extra buffers to the shader.
- Overload HlmsPbs.  
Useful for overriding only specific parts, or adding new functionality that requires storing extra information in a datablock (e.g. overload HlmsPbsDatablock to add more variables, and then overload HlmsPbs::createDatablockImpl to create these custom datablocks)
- Directly modify HlmsPbs, HlmsPbsDatablock and the template (.any).

SR3 does all above, we have our `HlmsPbs2.  
Also a HLMS PBS listener, it's that default `hlmsPbs->setListener( mHlmsPbsTerraShadows );` from Terra.  
Done so objects also receive terrain shadows. Only one listener can be used.  

And we have own `HlmsPbsDatablock2` with more stuff when needed for paint or fluids.

### Adding more uniforms

Adding more params **for all shaders**. E.g. `globalTime` and our own fog stuff like `fogHparams`.  
Easiest to add new in `struct AtmoSettingsGpu` also `struct AtmoSettings` and then fill it in `Atmosphere2Npr::_update` (called each frame).  

### setProperty

Allows using different code blocks in `.any` shaders (i.e. different shading paths).  
Calling `setProperty` in `HlmsPbs2::calculateHashForPreCreate(` and same? in `calculateHashForPreCaster`.  
Our method checks in name start from `.material` and sets a few cases already (grass, sky, water etc and body for vehicles).

See `selected_glow` for blue selection glow in SR editor for objects, road segments etc.  

Code in `AppGui::UpdSelectGlow(` changes "selected" for a `Renderable` and forces `CalculateHashFor`.  
Then code in `HlmsPbs2::calculateHashForPreCreate(` does by `getCustomParameters(` get the value  
and calls `setProperty` for shaders.

_ToDo:_ It works but slow, when selecting it creates a new shader (delay 1st time). Better use some uniform for this.

_TODO: Add renderable->hasCustomParameter( 99999 ) for all to get info in debugger.._


### Add vertex color to PBS

_ToDo:_ [topic](https://forums.ogre3d.org/viewtopic.php?p=554630#p554630), add for road blending.

### Adding textures

[topic from 2015](https://forums.ogre3d.org/viewtopic.php?f=25&t=84539)


### Own DataBlock2

Inside `HlmsPbs2::createDatablockImpl(` we create `HlmsPbsDatablock2` for vehicle paint params.  
It has own `HlmsPbsDatablock2::uploadToConstBuffer`, which is where data is uploaded from C++ to GPU material shader buffers.  

Currently can't change total size, must set `mUserValue[` in cpp and get in shader `material.userValue[`.
_ToDo_: `body_paint`

When setting car material need to use `HlmsPbsDatablock2*`. Set by `CarModel::SetPaint()`.  
Changes need `scheduleConstBufferUpdate()` to apply.


### Own HLMS

The modified `HlmsPbs2` replaces default `HlmsPbs` in Ogre-Next and reads all from same dirs as Pbs.  
It gathers all above points.  

Own HLMS should inherit from Pbs, and needs its own dirs with shaders (probably?).  
Also own name in `.material` for materials e.g. `hlms water_name pbs` that last `pbs`.  

Own HLMS could control more like `MaterialSizeInGpu` (it needs same size in `uploadToConstBuffer`).  
This assert triggering, means those were different:  
`assert( (size_t)( passBufferPtr - startupPtr ) * 4u == mapSize );`


### Other 🧪

Other common assert: [mCachedTransformOutOfDate](https://ogrecave.github.io/ogre-next/api/latest/_ogre20_changes.html#AssersionCachedOutOfDate).  
At end of [post](https://forums.ogre3d.org/viewtopic.php?p=554822#p554822).

Don't change node pos or rot from inside listeners.  
Or after call `_getFullTransformUpdated(` e.g. for `ndSky`, `light->getParentNode()->_`, camera etc.

### Workspaces 🪄

Many compositor Workspaces are created from code (telling how to render stuff, also few extra for editor minimap RTT).  
In `.log` lines with: `++++ WS add:`, in cpp code any `addWorkspace`.  

`SR3.compositor` has definitions for most workspaces used. Only shadows are made in code.

### Workspace Listener

[Done] Do `workspace->addListener(` so that `PlanarReflWorkspaceListener` is updated, for every workspace that can see the reflection (e.g. the cubemap workspaces).  
In code `AddListenerRnd2Tex()`, it's `ReflectListener`, `mWsListener` in `Reflect.h`.

`ReflectListener::passEarlyPreExecute` does check which `render_pass` it is from `SR3.compositor` by `identifier` number.

## ToDo

Ogre-Next: Sky_Postprocess, ReconstructPosFromDepth, Refractions,  
later: Sample_Hdr, Sample_Tutorial_SSAO, 