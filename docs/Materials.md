_Info on how to add new textures and materials for terrain, road, grass, objects, vehicles etc._

# General

## Introduction

This page explains what to do when adding new content.  
I.e. where to put your new data and which files need to be edited so it can appear and look good in SR3.

### 📄 Presets

The file [config/presets.xml](../config/presets.xml) is used by editor and game.  
It is edited manually (in original install folder).  
Nothing new will appear in editor's pick lists, unless you add a line for it in that file.  
It is quite simple, just copy a similar line and add it (keep it alphabetically sorted and spaces aligned).  
Change the 2nd param (or 1st for skies) to your new resource name.

See comments inside presets.xml `inside <!-- -->` for (short) info on each parameter.  
Later you can (and should) setup other values for parameters, e.g. layer scaling or model size etc,  
that are set (by default) when picking in editor (less manual setup needed).  
The `r - rating` param is subjective and high values are meant for best, most popular resources used.  
If you add a unique thing, used on couple tracks, rate it low, unless it's meant to be more.

### 🔮 Materials

**All materials** are now in [all.material.json](../data/materials/Pbs/all.material.json).  
Only particles aren't yet.  
More info on this `.json` format at bottom.

### 📝 Text

There are files `_*.txt` in each key data subdir, with info for:  
- original data name, author, license, url etc.  

These are gathered on **Credits** tab in game.  
These `txt` files need to be filled any time we add something new.  
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

Road textures are in [data/road](../data/road) with also some general for water etc.  
Some info in [_readme.txt](../data/road/_readme.txt). _ToDo: rest_

When adding a new road material you need to add two materials e.g. `roadJungle_ter` and `roadJungle`.  
The one with `_ter` is for road **on terrain**, it has more bumps, and alpha border/texture.  
The other (without `_ter`) is for **bridged** roads and is more flat, it can use own textures.

**Other** materials include: road wall, pipe, pipe glass, pipe wall, column, and universal.  
Since every material is in `all.material.json`, the only good list for orientation is in `presets.xml` and same in editor.

**Pipe glass** materials are twice in _all.material.json_ e.g. `pipeGlass` and `pipeGlass2`.  
So 2nd has `2` at end and only difference is in Macroblock, `cull_mode`.  
This 2nd is auto cloned by SR3 code, but since we save(d) all materials at once, they're just in `.json` too.

**Rivers** have a similar situation, a `River_` material is basically a copy of a `Water_` material.  
With `"reflect": true` added and speed is higher (it's in `detail_diffuse3` - `offset` 1st value).  
Since we can't inherit in `.json`, we'd need to copy each new fluid material into a river material, change name and speed etc.


## 🌿 Grass

Grass textures are in data/grass. They are transparent .png and mostly 512x512.  


## 🌳🪨 Vegetation

Models (meshes) are in `data/trees*`.  
The `trees-old` should to be replaced eventually with better.  
Rocks, caves, crystals etc are in `data/rocks` and `data/rockshex`.  
More info in their `_*.txt` files.


## 📦🏢 Objects

Objects have their own [Objects page](Objects.md) for meshes, exporting etc.  
Meshes, textures etc are in `data/`, subdirs:  
- `objects` - for dynamic objects with `.bullet` files and their 2nd `*_static.mesh` if needed (just a copy of `.mesh`).  
- `objects0` - for 0.A.D. models (mainly buildings) only, converted (from .dae).  
- `objects2` - for static objects with just `.mesh` and textures.
- `objectsC` - for new City buildings, Alien etc. Also has various, universal `metal*` textures.
- `obstacles` - recently split, only for obstacles.

Each subdir has a `_*.txt` file with info on all from this folder. Any new model added needs also a new entry there in `_*.txt`.  
This is important for Credits in game, for credits to authors (if CC-BY, CC-BY-SA, but CC0 too) and tracking licenses, models, urls, etc.  

More info on materials for objects, in next chapter here.


----

# 🔮 Materials

Almost everything with materials is different now in SR3.  
In **old** SR with Ogre, we used _shiny_ material generator with its own `.shader` syntax and `.mat` files.  

Now in SR3 with Ogre-Next we use _HLMS_ and **new** PBS materials.  
Main `.material` and `.material.json` **files** are in [data/materials/Pbs](../data/materials/Pbs).  

## ⏳ Old SR .mat

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

## 🆕 New .material (temporary)
  
> Note: New Ogre-Next `.material` format is worse than `.json` (info below)  
and should be used just temporarily.

> A couple of _freaking hopeless_ issues to keep in mind:  
> In `.material` scripts you can't set textures to **wrap**  
> (and they _don't_ by default) or other sampler configs, but inheriting works.  
> The `.material` is just meant as a faster way of porting old SR `.mat`,  
> then best to save as .json from Material Editor (still a lot manual work to do).  
> And still for few exceptions, like the unlit particles and hud stuff.

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


## 🛠️ Material Editor

Editing files by hand is one way, the better is using our **Material Editor** (Alt-F) GUI (both in game and editor),  
saving and then applying / merging changes back to original files.  

Materials can be adjusted real-time in Material Editor (Alt-F): or in Options, button on tab **Tweak**.  
Find material (search by name above) and then use sliders to tweak. There are few tabs, water has own set.  


----

## 🌠 New .material.json

We now use `.material.json` format (for almost all materials). It is more advanced, but longer.  
A bit worse to edit by hand. Especially having all in one file.  
There are no comments in json, sadly. And the file has over 50000 lines already.

Material Editor GUI can **save** into `.json` files in:  
`/home/user/.cache/stuntrally3/materials`  
either one material or all.  
It won't be exactly the same as in [all.material.json](../data/materials/Pbs/all.material.json).

In `.material.json` you **can't inherit** materials (confirmed [here](https://forums.ogre3d.org/viewtopic.php?p=553712#p553712)).  
Need to **duplicate** them whole e.g. for rivers, pipe glass (2 at end, other cull) and vehicles (painted) body (6 times + ghost 😩).  

Also important is to not break **.json syntax**. E.g. don't leave a comma `,` before `}`.  
It also needs more symbols like `: " [ ] { }` to work.  
I highly recommend using an IDE that does check syntax  
and underlines in red right when a syntax error happens during editing.  
I use VSCodium, but possilby smaller editors could too.  


### Top blocks

Top section of (each) `.material.json` has now: samplers, macroblocks and blendblocks.  
This is a detail on Ogre-Next implementation for more performance on rendering (in-depth info [here](https://ogrecave.github.io/ogre-next/api/latest/hlms.html)).  

Shortly put, many parameters (like e.g. cull (e.g. leaves), depth_write (e.g. glass), texture wrap/clamp etc.)  
have been grouped to blocks and these are reused. Not all parameters set for each material like before.  
In old SR `.mat` we had everything possible to use, easy to set, and short. Now it's more complex and long.  

For example, if you're making a new material (for pipe glass, tree leaves, grass, etc),  
then check other similar material and copy its top section, like:
```
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
```
to your material. It should render properly then.

----

# 📃 Examples

These sections below can also be used as starting themplate for your material(s).  
Few frist chapters below have special cases.  
These materials can be applied to objects, roads, vehicles, etc.  
Not particles yet. Terrain layers won't be there.  


## 💧 Fluids

Fluids like `Water_*` and `River_*` aren't meant to be edited by hand in `.json`.  
Parameters stored don't have names, hard to say which value does what (code in HlmsPbsDb2::HlmsPbsDb2).  
Simply use Material Editor Gui to tweak them. Save, single material .json.

Then manually merge with `all.material.json`.  
You could overwrite whole 'pbs' section, leaving top blocks.
But have to replace samplers etc to our.  
For testing you can save in Material Editor and put new files with all json.
But names can't duplicate.

## ⛅ Skies

Quite easy, yet a lot of lines.  
The only thing that matters is emissive texture. Rest is the same.  


```
		"sky/light_blue" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"shadow_const_bias" : 0.01,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [0, 0, 0],
				"background" : [1, 1, 1, 1]
			},
			"specular" :
			{
				"value" : [0, 0, 0]
			},
			"fresnel" :
			{
				"value" : [0.01, 0.01, 0.01],
				"mode" : "coloured"
			},
			"roughness" :
			{
				"value" : 0.01
			},
			"emissive" :
			{
				"value" : [1, 1, 1],
				"texture" : "sky_lightblue.jpg",
				"sampler" : "Sampler_Sky"
			}
		},
```
----


## Example 1 basic

Very basic. Only **diffuse** texture `balloon2cl.png` .

```
		"balloon2cl" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"shadow_const_bias" : 0.01,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [0.9, 0.9, 0.9],
				"background" : [1, 1, 1, 1],
				"texture" : "balloon2cl.png",
				"sampler" : "Sampler_10"
			},
			"specular" :
			{
				"value" : [0.2, 0.2, 0.2]
			},
			"fresnel" :
			{
				"value" : 0.1,
				"mode" : "coeff"
			},
			"roughness" :
			{
				"value" : 0.6
			}
		},
```


## Example 2

**Diffuse and normal** textures.

```
		"ptol_struct" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"shadow_const_bias" : 0.01,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [0.744, 0.744, 0.744],
				"background" : [1, 1, 1, 1],
				"texture" : "ptol_struct.jpg",
				"sampler" : "Sampler_10"
			},
			"specular" :
			{
				"value" : [0.2, 0.2, 0.2]
			},
			"fresnel" :
			{
				"value" : 0.1,
				"mode" : "coeff"
			},
			"normal" :
			{
				"value" : 1,
				"texture" : "ptol_struct_norm.jpg",
				"sampler" : "Sampler_10"
			},
			"roughness" :
			{
				"value" : 0.6
			}
		},
```


## Example 3, common

With 3 textures:
- diffuse
- normalmap (with `_norm` or `_n` at end)
- specular (`_spec` or `_s` at end)

Here also bump_scale is set as 0.6 under "normal", "value".  
fresnel - is as 1 value coeff (not 3)  
roughness - as 1 value

```
		"hexrock_grey" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"shadow_const_bias" : 0.01,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [0.8, 0.8, 0.8],
				"background" : [1, 1, 1, 1],
				"texture" : "hexrock_grey.jpg",
				"sampler" : "Sampler_10"
			},
			"specular" :
			{
				"value" : [0.4, 0.3, 0.35],
				"texture" : "hexrock_spec.jpg",
				"sampler" : "Sampler_10"
			},
			"fresnel" :
			{
				"value" : 0.1,
				"mode" : "coeff"
			},
			"normal" :
			{
				"value" : 0.6,
				"texture" : "hexrock_norm.jpg",
				"sampler" : "Sampler_10"
			},
			"roughness" :
			{
				"value" : 0.5
			}
		},
```


## Example 4, reflect

Also with **reflection** as `"reflect" : true,` for metal / glossy look.  
Here `fresnel` is `coloured` and has 3 values.  
It is the reflection color (and amount) for dynamic environment cube reflection.

```
		"hexrock_metal" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"shadow_const_bias" : 0.01,
			"reflect" : true,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [0.8, 0.8, 0.8],
				"background" : [1, 1, 1, 1],
				"texture" : "hexrock_metal.jpg",
				"sampler" : "Sampler_10"
			},
			"specular" :
			{
				"value" : [0.3, 0.35, 0.35],
				"texture" : "hexrock_metal_spec.jpg",
				"sampler" : "Sampler_10"
			},
			"fresnel" :
			{
				"value" : [0.3, 0.37, 0.4],
				"mode" : "coloured"
			},
			"normal" :
			{
				"value" : 1,
				"texture" : "hexrock_metal_norm.jpg",
				"sampler" : "Sampler_10"
			},
			"roughness" :
			{
				"value" : 0.3
			}
		},
```


## Example 5, roughness map

**Advanced** PBS, with reflect and also `roughness` map.  
Intensity on roughness map texture (black-white) give unique per texel roughness, applied same as other textures.  
The `"value" : 0.3` under `roughness` does scale texture values 0 to 1 now as 0 to 0.3.  

```
		"garbage_bin_blue" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"reflect" : true,
			"shadow_const_bias" : 0.01,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [1, 1, 1],
				"background" : [1, 1, 1, 1],
				"texture" : "garbage_bin_blue.jpg",
				"sampler" : "Sampler_10"
			},
			"specular" :
			{
				"value" : [0.6, 0.6, 0.6],
				"texture" : "garbage_bin_green_spec.jpg",
				"sampler" : "Sampler_10"
			},
			"fresnel" :
			{
				"value" : [1, 1, 1],
				"mode" : "coloured"
			},
			"normal" :
			{
				"value" : 1,
				"texture" : "garbage_bin_norm.jpg",
				"sampler" : "Sampler_10"
			},
			"roughness" :
			{
				"value" : 0.3,
				"texture" : "garbage_bin_rough.jpg",
				"sampler" : "Sampler_10"
			}
		},
```

Here all **possible textures** are present:  
diffuse, specular, normal, roughness.  

- diffuse - is base non metal color, shouldn't have shadows  
  and colors shouldn't be too dark or too bright, more in sRGB scale.
- normal - normalized perpendicular vector, scaled to 0..1 from -1..1.
- specular - tells where surface will shine from light reflection.
- roughness - tells how much it will shine. 0 is mirror, 0.2 most shine, 0.4 gets blurry, 0.9 is just dull.

There is no *metalness* texture, it's meant for a simpler metal workflow,
we use specular workflow which has own colors (on texture) for specular shine.


## Example 6, detail maps

Extra textures (layers) can be added to modulate original diffuse and/or normal textures.

```
		"obstaclesGrey" :
		{
			"macroblock" : "Macroblock_0",
			"blendblock" : "Blendblock_0",
			"shadow_const_bias" : 0.01,
			"workflow" : "specular_ogre",
			"diffuse" :
			{
				"value" : [0.8, 0.8, 0.8],
				"background" : [1, 1, 1, 1],
				"texture" : "obstaclesGrey_d.jpg",
				"sampler" : "Sampler_10"
			},
			"specular" :
			{
				"value" : [1, 1, 1],
				"texture" : "obstaclesGrey_s.jpg",
				"sampler" : "Sampler_10"
			},
			"fresnel" :
			{
				"value" : [0.6, 0.6, 0.6],
				"mode" : "coloured"
			},
			"normal" :
			{
				"value" : 1,
				"texture" : "obstaclesGrey_n.jpg",
				"sampler" : "Sampler_10"
			},
			"roughness" :
			{
				"value" : 0.7,
				"texture" : "obstaclesGrey_n.jpg"
			},
			"detail_diffuse0" :
			{
				"mode" : "Multiply",
				"scale" : [12, 12],
				"value" : 0.8,
				"texture" : "metal_212_wh.jpg",
				"sampler" : "Sampler_10"
			},
			"detail_normal0" :
			{
				"mode" : "Multiply",
				"scale" : [12, 12],
				"value" : 0.9,
				"texture" : "metal_212_spec.jpg",
				"sampler" : "Sampler_10"
			}
},
```

In this quite advanced material, 2 extra maps are added.  
First `detail_diffuse0` map multiplying regular `diffuse` map.  
Other variants possible, all are inside `@piece` in [this shader file](../data/Hlms/Pbs/Any/Main/200.BlendModes_piece_ps.any).
The "scale" is UV coords multiplier, it means that detail map with repeat 12 times more.

Great for adding smaller scale detail like scratches, etc.
Mostly detail_diffuse0 should be white though to not change regular color.

Another map is `detail_normal0` to modulate `normal` map.
The `value` is a multiplier for how much.

Note that `detail_diffuse0` and `detail_normal0` aren't required both, only diffuse or normal is okay too.
Also value, scale and offset can have other values set.

There can be more detail maps (4 total, 0..3). But let's not over do this, it affects Fps (performance).


_ToDo:_ we could even do material animations this way,  
by adding e.g. `sin(atmo.globalTime.x)` in some `@piece` cases..
