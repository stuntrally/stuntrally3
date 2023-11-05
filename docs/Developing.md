
# Developing Guide

Guide for Ogre-Next (links), tools used and general info for SR3 sources (code).  
Can be useful if you want to:
- develop SR3, test or fix a bug
- learn more about SR3 code or find something in it
- learn about Ogre-Next used in SR3 or other libraries etc
- fork SR3 code and start your own project (license required: GPLv3 or newer) based on it

## ToDo

Many _ToDo:_ tasks are added BTW of topics in this file.  
Shorter, fast updated list with more is on [Roadmap](Roadmap.md), also for planning.  
More in [Task list](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) with good general info (sorted by P (priority), 1 is most important).  
Lastly there are plenty of `todo:` or `fixme` places marked in sources.

## IDE

Recommended all in one tool, having e.g.: go to definition, find references, debugging etc.
### [Qt Creator](https://www.qt.io/download)
Easier to set up first, CMake is integrated. Fast, but less options.

### [VSCodium](https://github.com/VSCodium/vscodium/releases)
Used by CryHam, needs a moment to set up frist. More info on [my C++ guide](https://cryham.tuxfamily.org/cpp-guide/#VSCodium).  
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
To use run e.g. `valgrind --leak-check=full ./sr-editor3`, at end it will list details about leaks.
- [ASAN](https://clang.llvm.org/docs/AddressSanitizer.html) - close purpose to above
- Profiler. E.g.: Intel VTune, AMD uProf, Google Orbit, KDAB/hotspot. Any of them will do.  
For measuring performance, CPU time spent in methods from classes in code.  
Used [KDAB/hotspot](https://github.com/KDAB/hotspot) on Linux, quick tutorial [video](https://www.youtube.com/watch?v=6ogEkQ-vKt4), one [longer](https://www.youtube.com/watch?v=HOR4LiS4uMI). Overview of profiling [tools](https://www.youtube.com/watch?v=GL0GIdj6k2Q), Heaptrack [video](https://www.youtube.com/watch?v=OXqqVSdrSAw).

WIP Using Vulkan, it has better support for debugging tools, has more debug asserts etc, which <del>is</del> would be good for finding errors.  
Mentioned in [Post1](https://forums.ogre3d.org/viewtopic.php?p=553813#p553813), [Post2](https://forums.ogre3d.org/viewtopic.php?p=554446#p554446).  
It starts longer though, shader compilation is much slower. MyGui **doesn't** work with Vulkan.

--------

# Ogre-Next docs

Ogre-Next is the engine used for rendering, managing 3D scene, also loading resources, logging to `.log` files etc.

- [Manual](https://ogrecave.github.io/ogre-next/api/latest/manual.html) - need to read it when beginning.
- [version comparison](https://www.ogre3d.org/about/what-version-to-choose) of Ogre and Ogre-Next.
- [Compositor](https://ogrecave.github.io/ogre-next/api/latest/compositor.html) - for effects, RTTs, reflections, etc.
- [Terrain](https://ogrecave.github.io/ogre-next/api/latest/_terra_system.html) - details of new Terra system

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
Main files in `data/Hlms/Terra/Any`.
- **PBS**, regular shaders for all materials (objects, road,  cars, vegetation etc).  
Main files in `data/Hlms/Pbs/Any/Main`.  
I'm writing **water**/fluids also inside PBS, don't want to split.
- Unlit (no lighting). We should only use this for **Gui**.  
**Particles** must be at least: colored by sun diffuse, darker by shadows, also by terrain.  
Small files in `data/Hlms/Unlit/Any`.

Shader code in `.any` files are preferred, universal and these get translated to `.glsl` or `.hlsl` at end.

Continued in own page for [Materials](Materials.md).

--------

# Stunt Rally 3

## config files 📄

[👤] - means file is in user `.config/stuntrally3` dir, and saved by SR3.  
Otherwise, means a static file in `config/` dir, manually edited.  
(❔) - means file has info about itself and syntax, written inside.

Vehicles config
- config/cars.xml - (❔) - list for Vehicles tab with manually set parameters
- data/cars/cameras.xml - (❔) - all follow camera views for game
- data/carsim/[mode]/* -  
*.car - vehicle file (can edit directly in game Alt-Z),  
*_stats.xml - vehicle performance test results (after Ctrl-F5)  
.tire coeffs, suspension

Game modes
- championships.xml - (❔) - for Gui, tutorials too
- challenges.xml - (❔) - with tracks list, pass conditions, game setup etc

Game progress
- progress.xml, progress_rev.xml - [👤] - progress for championships, _rev for reversed
- progressL.xml, progressL_rev.xml - [👤] - same but for challenges

Game config
- input.xml and input_p[1..4].xml - [👤] - for game, common and player input bindings, Gui tab Input

- paint.cfg - current vehicles paint setups
- paints.ini - dynamic list for vehicles on Paints tab

Common config
- fluids.xml - (❔) - buoyancy and other fluid parameters for fluid areas
- presets.xml - (❔) - params for editor lists and game too
- tracks.xml - [👤] - tracks bookmark and rating, from Gui Track tab

Track files, in each track's dir.  
Below * means: nothing or 2,3,4.. etc if more.
- heightmap*.f32 - heightmap(s) for terrain(s). Format is raw 32 bit floats (4B for 1 height). Size always square: 512,1024,2048 etc.
- road*.xml - file(s) for each road/river etc. Has many params and points.
- scene.xml - 1 file for each track, with all editable params from Gui tabs in Track Editor.

## emojis

You can also get familiar with [emojis](../src/emojis.txt) file.  
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

## Art, tasks

[topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532&sid=b1e7ee6c60f01d5f2fd7ec5d0b4ad800) - Remove all non CC data, topic, New skies.  
More todo on [Roadmap](Roadmap.md).


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

Our fog shader code is in: `data/Hlms/Pbs/Any/Fog_piece_ps.any`  
and is used in Pbs and Terra `.any` there `@insertpiece( applyFog )`.


### Terra ⛰️

The terrain component. Very efficient. Has triplanar already.  

_ToDo:_ But `setDetailTriplanarNormalEnabled(true);` is broken still, has black-white spots. Posts: [start](https://forums.ogre3d.org/viewtopic.php?p=554304#p554304) until [end](https://forums.ogre3d.org/viewtopic.php?p=554312#p554312).

#### Comparison

This **changed** completely from old Ogre. Now heightmaps are power of 2, e.g. 1024, not 1025 like old.  
Old SR `heightmap.f32` are converted on 1st load in SR3. This also needs slight pos offset.  
SR3 loads `heightmap.f32` and gets size (e.g. 1024) from file size (`1024*1024*4 B`).  

#### Normalized

Terra by design had _normalized float heights_ from 0.0 to 1.0 only.  
We **don't** use that, changed it, since our Hmap and SR editor use any, real height values.  

**_ToDo:_**  
This **broke** `src/Terra/TerraShadowMapper.cpp` shadowmap generation (mostly for heights below 0).  
and terrain **shadowmap** is disabled.  
BTW it took way too long, 5 sec at start, possibly due to compute shader building.  
It's this `if (!m_bGenerateShadowMap)` and `return;  //**  5 sec delay` below.  
Set earlier `,bGenerateShadowMap( 0 )  //** ter par  //^^ todo: 1 in ed`.

This also **broke** terrain page **visibility**. Likely decreasing Fps, no idea how much.  
Made all visible in `Terra::isVisible` for `if (!bNormalized)`.  
In ctor `Terra::Terra(` setting `bNormalized` to 1 does try to normalize Hmap.  

**_ToDo:_** Call Terra::update when the camera changes for each of splitscreen views.

We use `tdb->setBrdf(TerraBrdf::BlinnPhongLegacyMath);` because it looks best, other are bad.

#### Extended

Terra is extended with **blendmap** RTT (from old SR) and its noise.  
Also with emissive, property: `emissive terrain`.  

Changed skirt to be relative size below, not that default big to lowest point.  
_ToDo:_ Some holes can appear on horizon terrains. Should be higher for them.

Added _own_ terrain raycast code (for cursor hit pos), since there wasn't one, [topic](https://forums.ogre3d.org/viewtopic.php?t=96602).  
_ToDo:_ For far future. Not a problem, but since we have many, [how to do multiple terras](https://forums.ogre3d.org/viewtopic.php?t=96050).


### PlanarReflection 🪞

Used for water/fluids.  
Modified to have more control and detail options for its camera.  
Shader code in `Samples/data/Hlms/Pbs/Any/PlanarReflections_piece_ps.any`, mainly:  
`planarReflUVs`, normal is in `pixelData.normal`, and distortion is added to `pointInPlane` from normal.  
More info in [post](https://forums.ogre3d.org/viewtopic.php?p=554536#p554536).

## Custom

These are using Ogre-Next base components but extend them to our specific rendering purposes.

### HlmsPbs2

Main **shaders** for all materials (except terrain and particles) here:  
[800.PixelShader_piece_ps.any](data/Hlms/Pbs/Any/Main/800.PixelShader_piece_ps.any)  (modified)  
[800.VertexShader_piece_vs.any](data/Hlms/Pbs/Any/Main/800.VertexShader_piece_vs.any)  (not yet)  
structures with variables passed to them are in:  
[500.Structs_piece_vs_piece_ps.any](data/Hlms/Pbs/Any/Main/500.Structs_piece_vs_piece_ps.any)  

Keep in mind [post](https://forums.ogre3d.org/viewtopic.php?p=554100&sid=6798838bbed3be6881aa07bf10012412#p554100),
in .any this does not comment: // @property( hlms_fog )  
Either add && 0 inside, or any letters to make non existing name, or remove @ too.

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
Road editing is **slower** now, [topic](https://forums.ogre3d.org/viewtopic.php?p=553712#p553712)  
due to going from V2 mesh to v1 computing tangents and back to v2.  
_ToDo:_ For far future. We should compute tangents/binormals ourself.  

_ToDo:_ We don't handle _device lost_ (happens in DirectX when going out of fullscreen),  
it will go bad for all created meshes (road, grass).  

**Glass pipes** rendering changed. Old Ogre had 2 passes with opposite culling.  
New does not support passes [[2.3] Dealing with multi pass rendering in ogre next](https://forums.ogre3d.org/viewtopic.php?t=96902), and instead has:  
2 nodes with same mesh, but clones datablock and sets opposite cull in it. Code in `pipe glass 2nd item` section of `Road_Mesh.cpp`.  

_ToDo:_ glass pipes are too bright (glow) in fog.

Transparent objects are sorted by Ogre-Next so they don't blink randomly like in old Ogre.  
There is though a less noticable issue with order on borders showin elipses between pipe segments [screen here](https://forums.ogre3d.org/viewtopic.php?p=553945&sid=6798838bbed3be6881aa07bf10012412#p553945).  

### Vegetation 🌳🪨

Means models for trees, rocks, plants etc.  
It is not paged, does not have impostors (like it was in old SR with paged-geometry).  
Simply uses Ogre-Next Items that will be automatically HW instanced, to reduce draw calls, also works with mesh LODs.  
All models are placed during track load by code in `src/common/SceneTrees.cpp`.  

### Objects .mesh

For Vegetation, Objects, etc.  
All models in **`.mesh`** files from old SR, need to be converted to newer version for SR3 using `OgreMeshTool`.  
Provided Python script `config/ogre-mesh.py` is helpful for this. Comments inside have more detail.  
It can process whole dir (e.g. new `objects3/` with files to convert) and has command presets, with LODs set up.  

### Grass 🌿

It has pages (auto size from terrain size) and no LoDs.  
Also no fading yet. Started, find: `grow fade away`.  
Currently done simplest way, has mesh pages, with vertices and indices.  
Quite inefficient to render and slow to create mesh.  
Code in `Grass.h` and `Grass_Mesh.cpp`.  
**_ToDo:_** add RTT for density map, read terrain height map in shader, do vertices in shader not on CPU..  
[old topic](https://forums.ogre3d.org/viewtopic.php?t=85626&start=25)  
[some info](https://forums.ogre3d.org/viewtopic.php?p=553666#p553666) (under "How should I add grass?") on how to do it better.  


## SR

Rest of code that was same in SR 2.x. Only small changes when porting to SR3.

### Simulation 🚗

Game code in `Update_Poses.cpp` gets data from VDrift simulation (on 2nd thread) calling `newPoses(`.  
Rendering update calls `updatePoses(` to set vehicles (from carModels) to new poses.  

VDrift simulation, main update code in `src/vdrift/cardynamics_update.cpp` in `void CARDYNAMICS::UpdateBody(`.  
SR own code for non-car vehicles like: 🚀 Spaceship and 🔘 Sphere, is in `SimulateSpaceship(` and `SimulateSphere(`.  

Few possible _ToDo:_ [here](https://stuntrally.tuxfamily.org/mantis/view.php?id=17).

### Replay 📽️

All files in `src/game/` with `replay` in name, mainly `Replay.h and .cpp` and `Gui_Replay.cpp` for Gui events.  
Partly code in `Update_Poses.cpp` for filling replay/ghost data etc.  
More detail in `CGame.h` sections with vars for this.  

### Sound 🔉

Sound manager code in `src/sound/`. Is based on [RoR's](https://github.com/RigsOfRods/rigs-of-rods/tree/master/source/main/audio) code. Completely replaced VDrift's code.

**_ToDo:_** [Task here](https://stuntrally.tuxfamily.org/mantis/view.php?id=1). Ambient sounds (rain, wind, forest etc). Hit sounds (barrels etc).


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
- [[Solved][2.1] Trying to create a new HLMS](https://forums.ogre3d.org/viewtopic.php?f=25&t=83763)
- [done] [basic using in app](https://ogrecave.github.io/ogre-next/api/2.3/_using_ogre_in_your_app.html)

## HLMS shaders output

Shaders are saved to `/home/ch/.cache/stuntrally3/shaders`.  
This is useful to know what finally ended in all shader's code (e.g. glsl) after HLMS did its preprocessor work.  
Can be disabled, find all 3 `setDebugOutputPath` and set 1st arg to false, 2nd is for properties.  

With 2nd arg true, properties are at top of each shader file,  
this makes all compiling errors have wrong line numbers,  
but at least one can get some idea of what kind of shader it was.

## HLMS extending 🌠

Customizing [doc link](https://ogrecave.github.io/ogre-next/api/latest/hlms.html#HlmsCreationOfShadersCustomizing).

From docs: Hlms implementation can be customized:
- Through HlmsListener.  
This allows you to have access to the buffer pass to fill extra information; or bind extra buffers to the shader.
- Overload HlmsPbs. Intro [post](https://forums.ogre3d.org/viewtopic.php?p=554026&sid=6798838bbed3be6881aa07bf10012412#p554026)  
Useful for overriding only specific parts, or adding new functionality that requires storing extra information in a datablock (e.g. overload HlmsPbsDatablock to add more variables, and then overload HlmsPbs::createDatablockImpl to create these custom datablocks)
- Directly modify HlmsPbs, HlmsPbsDatablock and the template (.any).

SR3 does all above, we have our `HlmsPbs2`.  
Also a HLMS PBS listener, it's that default `hlmsPbs->setListener( mHlmsPbsTerraShadows );`  
from Terra, this is only for Pbs objects, `hlmsTerra` has no listener (_ToDo:_ adding globalTime to both would need it).  
Done so objects also receive terrain shadows. Only one listener can be used.  

And we have own datablocks: `HlmsPbsDbCar` and `HlmsPbsDbWater` with more stuff when needed for paint or fluids.

### Adding more uniforms

Adding more params for Pbs shaders and terrain. E.g. `globalTime` and our own height fog like `fogHparams`.  
Easiest to add new in `struct AtmoSettingsGpu` also `struct AtmoSettings` and then fill it in `Atmosphere2Npr::_update` (called each frame).  
**_ToDo:_** This is half working way. Not all have `atmo` struct.  

_ToDo:_ [topic](https://forums.ogre3d.org/viewtopic.php?p=535357#p535357) done through HlmsListener using `getPassBufferSize` and `preparePassBuffer`.

This shader file `data/Hlms/Pbs/Any/Main/500.Structs_piece_vs_piece_ps.any` has definitions for:
- `MaterialStructDecl`
- `CONST_BUFFER_STRUCT_BEGIN( PassBuffer` - uniforms that change per pass
- `@piece( VStoPS_block )`
- and lots more

### setProperty

Allows using different code blocks in `.any` shaders (i.e. different shading paths).  
Calling `setProperty` in `HlmsPbs2::calculateHashForPreCreate(` and same? in `calculateHashForPreCaster`.  
Our method checks in name start from `.material` and sets a few cases already (grass, sky, water etc and body for vehicles).

See `selected_glow` for blue selection glow in SR editor for objects, road segments etc.  

Code in `AppGui::UpdSelectGlow(` changes "selected" for a `Renderable` and forces `CalculateHashFor`.  
Then code in `HlmsPbs2::calculateHashForPreCreate(` does by `getCustomParameters(` get the value  
and calls `setProperty` for shaders.

_ToDo:_ It works but slow, when selecting it creates a new shader (delay 1st time). Better use some uniform for this.

_ToDo:_ ? Add renderable->hasCustomParameter( 999.. ) for all to get info in debugger..


### Add vertex color to PBS

_ToDo:_ [topic](https://forums.ogre3d.org/viewtopic.php?p=554630#p554630), add for road blending.

### Adding textures

[topic from 2015](https://forums.ogre3d.org/viewtopic.php?f=25&t=84539)


### Own DataBlocks

Inside `HlmsPbs2::createDatablockImpl(` we create `HlmsPbsDbCar` for vehicle paint params.  
It has own `HlmsPbsDbCar::uploadToConstBuffer`, which is where data is uploaded from C++ to GPU material shader buffers.  

Currently can't change total size, must set `mUserValue[` in cpp and get in shader `material.userValue[`.  
_ToDo:_ `body_paint` property not used yet.  
**_ToDo:_** clone on our `HlmsPbsDbCar` fails? Same vehicles can't have different paints.

When setting car material need to use `HlmsPbsDbCar*`. Set by `CarModel::SetPaint()`.  
Changes need `scheduleConstBufferUpdate()` to apply.


### Own HLMS

The modified `HlmsPbs2` replaces default `HlmsPbs` in Ogre-Next and reads all from same dirs as Pbs.  
It gathers all above points.  

Fully own HLMS should inherit from Pbs, and (probably?) needs its own dirs with shaders.  
Also own name in `.material` for materials e.g. `hlms water_name pbs` that last `pbs`.  

_ToDo:_ [Trying to create a new HLMS](https://forums.ogre3d.org/viewtopic.php?f=25&t=83763)

Some example is already in Terra. Has also more and difficult code,
e.g. `Fill command Buffers` in `HlmsTerra::fillBuffersFor(`.

Own HLMS could control more, like `MaterialSizeInGpu` (it needs same size in `uploadToConstBuffer`).  
This assert triggering, means those were different:  
`assert( (size_t)( passBufferPtr - startupPtr ) * 4u == mapSize );`


### Other 🧪

Other common assert: [mCachedTransformOutOfDate](https://ogrecave.github.io/ogre-next/api/latest/_ogre20_changes.html#AssersionCachedOutOfDate).  
At end of [post](https://forums.ogre3d.org/viewtopic.php?p=554822#p554822).

Don't change node pos or rot from inside listeners.  
Or after, need to call `_getFullTransformUpdated(` e.g. for `ndSky`, `light->getParentNode()->_`, camera etc.

Also [post](https://forums.ogre3d.org/viewtopic.php?p=554822#p554822), leaking GPU RAM inactive and  
Exception: `Mapping the buffer twice within the same frame detected!`.

### Workspaces 🪄

Workspace basically is the setup for rendering one view target, only like reflection, shadow, etc and lastly screen.  
Many compositor Workspaces are created from code (telling how to render stuff, also few extra for editor minimap RTT).  
Creating is in `.log` lines with: `--++ WS add:`, and in cpp code by any `addWorkspace`.  

`SR3.compositor` has definitions for most workspaces used.  
Only shadows are made completely by code in `src/common/AppGui_Shadows.cpp`, based on `Samples/2.0/ApiUsage/ShadowMapFromCode/ShadowMapFromCode.cpp`.  
_Todo:_ only 3 PSSM splits work, no other count. ESM (Sh_Soft) is also broken.

Parts of [post](https://forums.ogre3d.org/viewtopic.php?p=553666#p553666) with info.

### Workspace Listener

We do `workspace->addListener(` so that `PlanarReflWorkspaceListener` is updated, for _every_ workspace that can see the reflection (e.g. the cubemap workspaces).  
In code `AddListenerRnd2Tex()`, it's `ReflectListener`, `mWsListener` in `FluidsReflect.h`.

`ReflectListener::passEarlyPreExecute` does check which `render_pass` it is from `SR3.compositor` by `identifier` number.

## Effects

_ToDo:_ From Ogre-Next samples to use:
- **ReconstructPosFromDepth** (water, soft particles) also [topic](https://forums.ogre3d.org/viewtopic.php?t=97096&sid=5b5762532f55a6b74f28e1404b1d54bb), [maybe](https://forums.ogre3d.org/viewtopic.php?t=97059&sid=5b5762532f55a6b74f28e1404b1d54bb)
- **Refractions** (water)
- Sky_Postprocess (no?)
- InterpolationLoop (meh, later)

Effects:
- Sample_**Hdr** (with bloom), Sample_Tutorial_**SSAO**
- Lastly GI: InstantRadiosity (main), ImageVoxelizer (VCT), LocalCubemaps refl
- meh: OpenVR, StereoRendering
- to fix: ShadowMapDebugging, ShadowMapFromCode
