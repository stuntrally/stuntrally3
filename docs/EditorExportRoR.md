_Help on using SR3 Track Editor to export track(s) to Rigs of Rods._


### Introduction

Stunt Rally 3 Track Editor (ed) can now export track(s) for [Rigs of Rods](https://www.rigsofrods.org/) (RoR).  
This has few benefits:
- More Tracks for RoR, with road, in good quality, from SR 2.x
  - also possible to edit them and export again
- Good Assets (CC licensed) from SR 2.x, for reuse on future RoR tracks
- **New** possiblity to create and edit new tracks for RoR using SR3 Track Editor
  - If you also share SR3 track data with it (recommended), others could edit or modify it later too, and even drive in SR3 and RoR.
  - Path to SR3 user tracks is `/home/username/.local/share/stuntrally3/tracks`


### Setup

These are all steps needed:

#### RoR mods path

*Main RoR* path for *mods* (or content).  
It should end with / or \\.  
On Linux: `/home/username/.rigsofrods/mods/`  
Here will tracks be exported into.  
Also other subdirs will be needed or created.

#### Old SR

This the path to SR 2.x `data/` dir, where are subdirs like: materials, objects*, terrain, trees* etc.  
It should end with / or \\.

#### SR packs

Download all SR packs (from RoR repository, or you could already have them in *mods*)  
and extract them in RoR mods path.  

Extract `sr-*` *packs* inside RoR *mods* dir.

To have subdirs like, e.g.:
- sr-materials-v1
  - with *.material files
  - sr-materials-v1.assetpack
- sr-trees2-v1
- and **all** other available

This is needed for auto determining which packs are needed for exported tracks.

_Alternatively SR3 Track Editor can export also other way, not using packs._  
_Simply by copying (almost) all needed resources into RoR track dir._


### How to export

Steps:

1. You will need **both**: **SR3** binaries (how to in [Install](Install.md)) to use editor and export,  
 **and SR2**. For SR2 data you can [install](https://github.com/stuntrally/stuntrally/blob/master/docs/install.md) or download from [github](https://github.com/stuntrally/stuntrally/)

    Why both?  
    _SR3 data (meshes) is incompatible with RoR, while SR2 is close as it uses same Ogre version._  
    _But SR2 is not developed now. Thus SR3 editor is more advanced._

2. Start SR3 Track Editor (info in [here](Running.md)), and load any track.

3. Go to (last) tab Export in editor.

4. Go through all points of previous chapter Setup.  

5. Then setup paths in Export, subtab Settings.

6. Go to first tab (Warnings). Press Export.  
   It can take few seconds. At end there will be log of what was done.  
   Check it for warnings (orange or red).
   If it went well (and sr packs are present if used) then you can start this track in RoR.


----

### Tools

Some more tools are present in ed on Export tab.

These are used for content of sr-packs.

Buttons:

#### .materials

Will convert from SR2 `data/materials/scene` all needed `.mat` files into `.material` for RoR.  
Resulting files is in RoR mods path in subdir `materials`.

#### terrain textures

Will convert/combine SR2 terrain textures (files *_d, *_s, *_n .jpg) into RoR (needed _ds.png and _nh.png).  
_d - diffuse, s - specular (ignored, bug), n - normal, h - height (now just made simply from n)_  
Resulting files are put into RoR mods path in subdir `terrain`.


#### .odef for .mesh

Old, not used now.

Creates .odef file for each .mesh inside RoR mods path, in subdirs:  
objects, objects2, objectsC, objects0, obstacles,	trees, trees2, trees-old, rocks, rockshex


### Details

When making packs, from SR data

renamed:
- all grass* to sr-grass*
- palm.mesh to palm11.mesh and set material inside to Plm1
- palm2.mesh to palm22.mesh

not including any:
- dynamic objects (all having .bullet file near)
- obstacle* hexrock* hexcave* anomaly*
- tree_dead* post_apo_building* etc from sceneries not in RoR
