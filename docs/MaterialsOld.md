_Info about old material file formats .mat and .material. Not to be used._

# 🔮📜 Materials, history 

*This is now just for comparison with old SR2.*

Almost everything with materials is different now in SR3.  
In **old** SR with Ogre, we used _shiny_ material generator with its own `.shader` syntax and `.mat` files.  

Now in SR3 with Ogre-Next we use _HLMS_ and **new** PBS materials.  
Main `.material` and `.material.json` **files** are in [data/materials/Pbs](../data/materials/Pbs).  


## ⏳ Old SR .mat

Old SR 2.x `.mat` example for a material `ES_glass` *inheriting* from `car_glass`:  
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


## 🆕 Old, Temporary .material
  
> Note: New Ogre-Next `.material` format is worse than `.json` (info below)  
and is used just temporarily.

> A couple of issues to keep in mind:  
> In `.material` scripts you can't set textures to **wrap**  
> (and they _don't_ by default) or other sampler configs,  
> but inheriting works and format is much shorter.  

> The `.material` is just meant as a faster way of porting old SR `.mat`,  
> then best to save as .json from Material Editor (still a lot manual work to do).  
> And for few exceptions, like the unlit particles and Hud stuff.

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

**Quick examples**, top of [_Example.material-](../data/materials/Pbs/_Example.material-).

For 🏔️**terrain layer** textures these parames are in `presets.xml` as:  
`ro - roughness, me - metalness, re - reflection`.

Details are as comments in code [OgreHlmsPbsDatablock.h](https://github.com/OGRECave/ogre-next/blob/master/Components/Hlms/Pbs/include/OgreHlmsPbsDatablock.h).  
Datablock is basically a structure with all material parameters.  
