_Info on how to add new textures and materials for terrain, road, grass, objects, vehicles etc._

## General

### Introduction ###

This page explains what to do when adding new content.  
I.e. where to put your new data and which files need to be edited so it can appear and look good in SR3.

The file [config/presets.xml](../config/presets.xml) is used by editor and game. It is edited manually (in original install folder).  
Nothing new will appear in editor's pick lists, unless you add a line for it in that file.  
It is quite simple, just copy a similar line and add it (keep it alphabetically sorted and space aligned).  
Change the 2nd param (or 1st for skies) to your new resource name.

See comments inside presets.xml `inside <!-- -->` for (short) info on each param.  
Later you can (and should) setup other values for parameters, e.g. layer scaling or model size etc. that are set (by default) when picking in editor (less manual setup needed).  
The `r - rating` param is subjective and high values are meant for best, most popular resources used. If you add a unique thing, used on couple tracks, rate it low, unless it's meant to be more.



### Skies ###

Sky textures are in data/skies.  
Those are 360x90 degree spherical textures. One texture for whole skydome, size 4096x1024.  
Materials are in data/materials/scene/sky.mat. Have to add new there, just copy last and replace texture for yours.


### Terrain ###

Textures for terrain layers are in data/terrain. See about.txt for sources.  
They are all .jpg saved at about 97%. Size is 1024x1024.

Name endings mean:  
_d - diffuse texture (main)  
_s - specular amount (not color)   if not present, will be black (no specular)  
_n - normal map (needed, if not provided can be made with some tool)   You can use flat_n.png before real normalmap for quicker test.  
*_h - old not used, was for broken parallax*

Unlike other things, terrain has its own material so only adding to presets.xml is needed.


### Road ###

Road textures are in `data/road`.  
Materials in `data/materials/Pbs/road.material`.

When adding a new road material you need to add two materials e.g. `roadJungle_ter` and `roadJungle`.  
The one with `_ter` is for road on terrain, it has more bumps, and alpha border/texture.  
The other (without `_ter`) is for bridged roads and is more flat, it can also use other textures.

Other road materials are in `data/materials/Pbs/pipe.material`: road wall, pipe, pipe glass, pipe wall, column, universal.

_ToDo:_ .material files are to be replaced with `.material.json`.


### Grass ###

Grass textures are in data/grass. They are transparent .png and mostly 512x512.  
Materials in data/materials/scene/grass.mat. Just copy a line and change texture name (and color if needed).  
Lastly adding a line in presets.xml is needed.


### Vegetation ###

Models (meshes) are in data/trees.  
Materials in data/materials/scene/trees.mat.


### Objects ###

Objects have their own wiki page, it also has more info on material editing (.mat files).


----

## ⚖️ Materials

Almost everything with materials is different now in SR3.  
In **old** SR with Ogre, we used _shiny_ material generator with its own `.shader` syntax and `.mat` files.  
Now in SR3 with Ogre-Next we use _HLMS_ and **new** PBS materials.  

### ⭐ Old

Old `.mat` example for new material `ES_glass` inheriting from `car_glass`:  
```
material ES_glass
{
	parent car_glass
```

From old `.mat` files main parameters changed are:
- `ambient` color - **gone**, nothing in new
- `specular` - power exponent (4th number), use new `roughness` for this
- `env_map true` - _ToDo:_ reflection is now as `re="1"` in `presets.xml`.
- `refl_amount` - now fresnel
- `twoside_diffuse true` - now `two_sided true` - for tree leaves, glass etc
- `terrain_light_map true` - gone, auto in new
- `bump_scale 0.5` - now 

### 🌠 New

New `.material` example of new material `ES_glass` **inheriting** from `car_glass`:  
```
hlms ES_glass pbs : car_glass
{
```

New HLMS `.material` parameters, using [PBS](https://duckduckgo.com/?q=physically+based+shading&t=newext&atb=v321-1&ia=web) - Physically Based Shading:
- `roughness` 0.001 to 1.0 (replaces old specular power exponent).
- `metalness` (not to be used in specular_workflow?).
- `fresnel` is _reflection_ (mirror) amount and
- `fresnel_coeff` is its color.

We use default `specular_workflow`, metallic workflow is simpler.  

Also texture keywords changed, all are optional:  
`diffuse_map`, `normal_map`, `specular_map`, `roughness_map`, `emissive_map`.

For terrain layer textures these parames are in `presets.xml` as:  
`ro - roughness, me - metalness, re - reflection`.

### New continued

All in code `Ogre/ogre-next/Components/Hlms/Pbs/src/OgreHlmsPbsDatablock.cpp`.  
Datablock is basically what has all material parameters.  

Editing files by hand is one way, the better is using **Material Editor** (Alt-F) GUI,  
saving and then applying / merging changes back to original files.  

Main `.material` and `.material.json` **files** are in `/data/materials/Pbs`.  
Materials can be adjusted real-time in Material Editor (Alt-F): or in Options, button on tab **Tweak**. find material and use sliders etc.  
_ToDo:_ this is the main tool, needs saving _all_ as `.material.json` files.  

We now also use `.material.json` format, longer and more advanced.  
A bit worse to edit by hand. Can be saved from Material Editor GUI into `.json` files in:  
`/home/user/.cache/stuntrally3/materials`

In `.material.json` you can't inherit materials (confirmed [here](https://forums.ogre3d.org/viewtopic.php?p=553712#p553712)).  
Need to duplicate whole e.g. for rivers.  
Also if present, need to remove secion `"reflection"` with `DynamicCubemap`, will crash otherwise, it is added by code.  

In `.material` scripts you can't set textures to wrap or other sampler configs but inheriting works.  

