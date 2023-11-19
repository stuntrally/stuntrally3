_Info on how to add new textures and materials for terrain, road, grass, objects, vehicles etc._

## General

### Introduction

This page explains what to do when adding new content.  
I.e. where to put your new data and which files need to be edited so it can appear and look good in SR3.

The file [config/presets.xml](../config/presets.xml) is used by editor and game. It is edited manually (in original install folder).  
Nothing new will appear in editor's pick lists, unless you add a line for it in that file.  
It is quite simple, just copy a similar line and add it (keep it alphabetically sorted and space aligned).  
Change the 2nd param (or 1st for skies) to your new resource name.

See comments inside presets.xml `inside <!-- -->` for (short) info on each param.  
Later you can (and should) setup other values for parameters, e.g. layer scaling or model size etc,  
that are set (by default) when picking in editor (less manual setup needed).  
The `r - rating` param is subjective and high values are meant for best, most popular resources used.  
If you add a unique thing, used on couple tracks, rate it low, unless it's meant to be more.

###

### Skies

Sky textures are in [data/skies](../data/skies). More info in [_skies-new.txt](../data/skies/_skies-new.txt) and [_skies-old.txt](../data/skies/_skies-old.txt).  
Those are 360x90 degree spherical textures. One texture for whole skydome, size 4k x 1k or 8k x 2k (1k is 1024).  
Materials are in [skies.material](../data/materials/Pbs/skies.material). Have to add new there, just copy last and replace texture for yours.


### Terrain

Textures for terrain layers are in [data/terrain](../data/terrain). More info in [_terrain.txt](../data/terrain/_terrain.txt).  
They are all .jpg saved at about 97%. Size is square 1k (1024) or 2k.

Name endings mean:  
_d - diffuse texture (main)  
_s - specular amount (not color)   if not present, will be black (no specular)  
_n - normal map (needed, if not provided can be made with some tool)   You can use flat_n.png before real normalmap for quicker test.  
*_h - old not used now, was for broken parallax height*

Unlike other things, terrain has its own material and shader, so only adding to presets.xml is needed.


### Road

Road textures are in [data/road](../data/road). Materials in [road.material](../data/materials/Pbs/road.material).

When adding a new road material you need to add two materials e.g. `roadJungle_ter` and `roadJungle`.  
The one with `_ter` is for road on terrain, it has more bumps, and alpha border/texture.  
The other (without `_ter`) is for bridged roads and is more flat, it can use own textures.

Other road materials are in [pipe.material](../data/materials/Pbs/pipe.material): road wall, pipe, pipe glass, pipe wall, column, universal.

_ToDo:_ .material files are to be replaced with `.material.json`.


### Grass

Grass textures are in data/grass. They are transparent .png and mostly 512x512.  
Materials in data/materials/scene/grass.mat. Just copy a line and change texture name (and color if needed).  
Lastly adding a line in presets.xml is needed.


### Vegetation

Models (meshes) are in data/trees.  
Materials in data/materials/scene/trees.mat.


### Objects

Objects have their own wiki page, it also has more info on material editing (.mat files).


----

## ⚖️ Materials

Almost everything with materials is different now in SR3.  
In **old** SR with Ogre, we used _shiny_ material generator with its own `.shader` syntax and `.mat` files.  

Now in SR3 with Ogre-Next we use _HLMS_ and **new** PBS materials.  
Main `.material` and `.material.json` **files** are in [data/materials/Pbs](../data/materials/Pbs).  

### ⭐ Old

Old `.mat` example for new material `ES_glass` inheriting from `car_glass`:  
```
material ES_glass
{
	parent car_glass
```

From old `.mat` files main parameters changed are:
- `ambient` color - **gone**, nothing in new
- `specular` - now only 3 values, have to remove **4th**  
   instead of power exponent (4th number), use new `roughness`
- `envMap ReflectionCube` - not needed, auto
- `env_map true` - **reflection** for objects/all materials as `reflect yes`, for terrain as `re="1"` in `presets.xml`
- `refl_amount` - now `fresnel_coeff` as color
- `twoside_diffuse true` - now `two_sided true` - for tree leaves, glass (not pipe) etc
- `terrain_light_map true` - gone, auto in new

### 🌠 New

New `.material` example of new material `ES_glass` **inheriting** from `car_glass`:  
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
- *`metalness` - not for our `specular_workflow`.*
- *`fresnel` - IOR factors, fresnel_coeff used instead*

We use default `specular_workflow`, metallic workflow is simpler, not to be used.  

Also texture keywords changed, all are optional:  
`diffuse_map`, `normal_map`, `specular_map` (same as `metalness_map`), `roughness_map`, `emissive_map`.  
There is no reflMap or `reflect_map`, we can use either:
- `specular_map` (1.0 is shinest) or
- new `roughness_map` (0 to 0.2 is shinest)
- or both for big/complex models.

Need to add `reflect yes` to .material or .json to get that `env_refl` (environment reflection cubemap) as before.

Quick examples, top of [PbsExample.material-](../data/materials/Pbs/PbsExample.material-).

For terrain layer textures these parames are in `presets.xml` as:  
`ro - roughness, me - metalness, re - reflection`.

Details are as comments in code [OgreHlmsPbsDatablock.h](https://github.com/OGRECave/ogre-next/blob/master/Components/Hlms/Pbs/include/OgreHlmsPbsDatablock.h).  
Datablock is basically a structure with all material parameters.  

### Material Editor

Editing files by hand is one way, the better is using our **Material Editor** (Alt-F) GUI (both in game and editor),  
saving and then applying / merging changes back to original files.  

Materials can be adjusted real-time in Material Editor (Alt-F): or in Options, button on tab **Tweak**.  
Find material (search by name above) and then use sliders to tweak. There are few tabs, water has own set.  

### material.json

We now also use `.material.json` format, longer and more advanced.  
A bit worse to edit by hand. Can be **saved** from Material Editor GUI into `.json` files in:  
`/home/user/.cache/stuntrally3/materials`  
either one material or all.  

A couple of _freaking hopeless_ issues to keep in mind:

In `.material` scripts you can't set textures to **wrap** (and they _don't_ by default) or other sampler configs, but inheriting works.  

`.material` is now just meant as a faster way of porting old SR `.mat`,  
then best to save as .json from Material Editor (still a lot manual work to do).

In `.material.json` you **can't inherit** materials (confirmed [here](https://forums.ogre3d.org/viewtopic.php?p=553712#p553712)).  
Need to duplicate whole e.g. for rivers.  
Also if present, need to remove section `"reflection"` with `DynamicCubemap`, will crash otherwise, it is added by code.  

_ToDo:_ try without River* materials needed, road setProperty("river")..?  
_ToDo:_ make this easier, reflection for objects..