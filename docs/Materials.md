_Info on how to add new textures and materials for terrain, road, grass, objects, vehicles etc._

## General

### Introduction

This page explains what to do when adding new content.  
I.e. where to put your new data and which files need to be edited so it can appear and look good in SR3.

### 📄 Presets

The file [config/presets.xml](../config/presets.xml) is used by editor and game.  
It is edited manually (in original install folder).  
Nothing new will appear in editor's pick lists, unless you add a line for it in that file.  
It is quite simple, just copy a similar line and add it (keep it alphabetically sorted and space aligned).  
Change the 2nd param (or 1st for skies) to your new resource name.

See comments inside presets.xml `inside <!-- -->` for (short) info on each parameter.  
Later you can (and should) setup other values for parameters, e.g. layer scaling or model size etc,  
that are set (by default) when picking in editor (less manual setup needed).  
The `r - rating` param is subjective and high values are meant for best, most popular resources used.  
If you add a unique thing, used on couple tracks, rate it low, unless it's meant to be more.

### Materials

**All materials** are now in [all.material.json](../data/materials/Pbs/all.material.json).  
Only particles aren't yet.  
More info on this `.json` format at bottom.

### 📝 Text

There are files _*.txt in each key data subdir, with info for:  
- original data name, author, license, url etc.  
These are gathered on Credits tab in game.  
These files need to be filled any time we add something new.  
Whether it's your creation or others, we should note it in there.

## ⛅ Skies

Sky textures are in [data/skies](../data/skies). More info in [_skies-new.txt](../data/skies/_skies-new.txt) and [_skies-old.txt](../data/skies/_skies-old.txt).  
Those are 360x90 degree spherical textures. One texture for whole skydome, size 4k x 1k (old) or new 8k x 2k (1k is 1024).  
Material names start with `sky/`. To add new, just copy last and replace "emissive" texture for yours.


## ⛰️ Terrain

Textures for terrain layers are in [data/terrain](../data/terrain). More info in [_terrain.txt](../data/terrain/_terrain.txt).  
They are all .jpg saved at about 97%. Size is square 1k (1024) or 2k.

Name endings mean:  
_d - diffuse texture (main)  
_s - specular amount (not color)   if not present, will be black (no specular)  
_n - normal map (needed, if not provided can be made with some tool)   You can use flat_n.png before real normalmap for quicker test.  
*_h - old not used now, was for broken parallax height*

Unlike other things, terrain has its own material and shader, so only adding to presets.xml is needed.


## 🛣️ Road etc

Road textures are in [data/road](../data/road). Some info in [_readme.txt](../data/road/_readme.txt). _ToDo: rest_

When adding a new road material you need to add two materials e.g. `roadJungle_ter` and `roadJungle`.  
The one with `_ter` is for road **on terrain**, it has more bumps, and alpha border/texture.  
The other (without `_ter`) is for **bridged** roads and is more flat, it can use own textures.

**Other** materials include: road wall, pipe, pipe glass, pipe wall, column, and universal.  
Since every material is in `all.material.json`, the only good list for orientation is in `presets.xml` and same in editor.

**Pipe glass** materials are twice in _all.material.json_ e.g. `pipeGlass` and `pipeGlass2`.  
So 2nd has `2` at end and only difference is in Macroblock, `cull_mode`.  
This 2nd is auto cloned by SR3 code, but since we save(d) all materials at once, they're just in `.json` too.

**Rivers** have a similar situation, a `River_` material is basically a copy of a `Water_` material.  
But since we can't inherit in `.json`, we'd need to copy fluid materials into a river materials if needed, change name and speed etc.


## 🌿 Grass

Grass textures are in data/grass. They are transparent .png and mostly 512x512.  
Materials in data/materials/scene/grass.mat. Just copy a line and change texture name (and color if needed).  
Lastly adding a line in presets.xml is needed.


## 🌳🪨 Vegetation

Models (meshes) are in `data/trees*`.  
Rocks, caves, crystals etc are in `data/rocks` and `data/rockshex`.  
Materials were in data/materials/scene/trees.mat.


## 📦🏢 Objects

Objects have their own [Objects page](Objects.md) for meshes.  
Object (meshes, textures etc) are in `data/`, subdirs:  
- `objects` - for dynamic objects with `.bullet` files and their 2nd `*_static.mesh` if needed (just a copy of `.mesh`).  
- `objects0` - for 0.A.D. buildings, converted.  
- `objects2` - for static objects with just `.mesh` and textures.
- `objects3` - _ToDo:, future split newest (obstacle_ ?).._
- `objectsC` - for dynamic objects with `.bullet` files and their 2nd `*_static.mesh` if needed (just a copy of `.mesh`).
- `objects` - for dynamic objects with `.bullet` files and their 2nd `*_static.mesh` if needed (just a copy of `.mesh`).
Each subdir has a `_*.txt` file with info on all from this folder. Any new model added needs also a new entry there in `_*.txt`.  
This is important for Credits in game, for credits to authors (if CC-BY, CC-BY-SA, but CC0 too) and tracking licenses, models, urls, etc.

More info on materials for objects, in next chapter here.


----

# ⚖️ Materials

Almost everything with materials is different now in SR3.  
In **old** SR with Ogre, we used _shiny_ material generator with its own `.shader` syntax and `.mat` files.  

Now in SR3 with Ogre-Next we use _HLMS_ and **new** PBS materials.  
Main `.material` and `.material.json` **files** are in [data/materials/Pbs](../data/materials/Pbs).  

### ⭐ Old SR .mat

Old SR `.mat` example for a material `ES_glass` inheriting from `car_glass`:  
```
material ES_glass
{
	parent car_glass
```

From old `.mat` files main parameters changed are:
- `ambient` color - **gone**, nothing in new
- `specular` - now only 3 values, have to remove **4th**  
   instead of power exponent (4th number), use new `roughness`
- `envMap ReflectionCube` - not needed, automatic
- `env_map true` - **reflection** for objects/all materials
   now as `"reflect" : true` in .json, and for terrain layers as `re="1"` in `presets.xml`
- `refl_amount` - now `fresnel_coeff` as color
- `twoside_diffuse true` - now `"two_sided" : true` - for tree leaves, glass (not pipe) etc
- `terrain_light_map true` - gone, now automatic

### 🌠 New .material (temporary)
  
> Note: New Ogre-Next `.material` format is worse than `.json` (info below) and  
> should be used just temporarily, e.g. when converting old SR `.mat` to new SR3 (especially fluids).

New `.material` example of a material `ES_glass` **inheriting** from `car_glass`:  
```
hlms ES_glass pbs : car_glass
{
```

New HLMS `.material` parameters, using [PBS](https://duckduckgo.com/?q=physically+based+shading&t=newext&atb=v321-1&ia=web) - Physically Based Shading:
- `roughness` - 0.001 to 1.0 replaces old specular power exponent.  
  lowest values 0.01 are for glass, mirror (highest exponents e.g. 96),  
  about 0.2 is best for most specular shine (high exponents e.g. 32),  
  over 0.4 will get blurred and gray (low exponents e.g. 12)
- `fresnel_coeff` is **reflection** (mirror) color.  
_do **not** use:_
- *`metalness` - not for `specular_*` workflows.*
- *`fresnel` - IOR factors, fresnel_coeff used instead*

We use default `specular_ogre` workflow, metallic workflow is simpler, not to be used.  

Also texture keywords changed, all are optional:  
`diffuse_map`, `normal_map`, `specular_map` (same as `metalness_map`), `roughness_map`, `emissive_map`.  
There is no reflMap or `reflect_map`, we can use either:
- `specular_map` (1.0 is shinest) or
- new `roughness_map` (0 to 0.2 is shinest)
- or both for big/complex models.

Need to add `reflect yes` to .material to get that `env_refl` (environment reflection cubemap) as before.

**Quick examples**, top of [PbsExample.material-](../data/materials/Pbs/PbsExample.material-).

For terrain layer textures these parames are in `presets.xml` as:  
`ro - roughness, me - metalness, re - reflection`.

Details are as comments in code [OgreHlmsPbsDatablock.h](https://github.com/OGRECave/ogre-next/blob/master/Components/Hlms/Pbs/include/OgreHlmsPbsDatablock.h).  
Datablock is basically a structure with all material parameters.  


### 🛠️ Material Editor

Editing files by hand is one way, the better is using our **Material Editor** (Alt-F) GUI (both in game and editor),  
saving and then applying / merging changes back to original files.  

Materials can be adjusted real-time in Material Editor (Alt-F): or in Options, button on tab **Tweak**.  
Find material (search by name above) and then use sliders to tweak. There are few tabs, water has own set.  


### 🌠 New .material.json

We now use `.material.json` format, it is more advanced, but longer.  
A bit worse to edit by hand. Especially having all in one file.  

It can be **saved** from Material Editor GUI into `.json` files in:  
`/home/user/.cache/stuntrally3/materials`  
either one material or all.  
It won't be exactly the same as in [all.material.json](../data/materials/Pbs/all.material.json).

A couple of _freaking hopeless_ issues to keep in mind:

In `.material` scripts you can't set textures to **wrap** (and they _don't_ by default) or other sampler configs, but inheriting works.  

`.material` is now just meant as a faster way of porting old SR `.mat`,  
then best to save as .json from Material Editor (still a lot manual work to do).

In `.material.json` you **can't inherit** materials (confirmed [here](https://forums.ogre3d.org/viewtopic.php?p=553712#p553712)).  
Need to duplicate whole e.g. for rivers.  
Also if present, need to remove section `"reflection"` with `DynamicCubemap`, will crash otherwise, it is added by code.  

_ToDo:_ try without River* materials needed, road setProperty("river")..?  
