_Help on using SR3 Track Editor to export track(s) to Rigs of Rods._

Page with help on using SR3 Track Editor [here](Editor.md).

### Introduction

Stunt Rally 3 Track Editor (ed) can now export track(s) for [Rigs of Rods](https://www.rigsofrods.org/) (RoR).  
This has few benefits:
- More Tracks for RoR, with road, in good quality, from SR 2.x
  - also possible to edit them and export again
- Good Assets (CC licensed) from SR 2.x, for reuse on future RoR tracks
- **New** possibility to create and edit new tracks for RoR using SR3 Track Editor
  - If you also share SR3 track data with it (recommended), others could edit or modify it later too, and even drive in SR3 and RoR.
  - Path to SR3 user tracks is `/home/username/.local/share/stuntrally3/tracks`

### Forum

Topics on RoR Forum:
- [Stunt Rally tracks converted for RoR](https://forum.rigsofrods.org/threads/stunt-rally-tracks-converted-for-ror.4205/)
- [Stunt Rally asset packs](https://forum.rigsofrods.org/threads/stunt-rally-asset-packs.4204/), needed for these tracks
- [Stunt Rally tracks exporter](https://forum.rigsofrods.org/threads/porting-content-stunt-rally-tracks-exporter.4011/), first, discussion, bugs etc.

### Tracks

About 55 % of all SR3 tracks are available and drivable in RoR.

SR repo *tracks3* has a special [branch ror](https://github.com/stuntrally/tracks3/commits/ror) with changes done for the tracks version going for RoR.  
Some have reduced roads, closed jumps, and other possible changes.  
This link shows [comparison](https://github.com/stuntrally/tracks3/compare/main...ror), changes visible on road.png.  

For making new tracks this branch isn't important.  
But *sr-tracks* pack was made from this branch and has needed changes.  
Modifying any track to fit RoR, would need updates to this branch too.  
It is also easy to contribute any tracks or changes by forking [*tracks3*](https://github.com/stuntrally/tracks3/) repo.  

#### Features

Track features available in SR3 that are ignored for RoR and simply won't be there:
- Road pipes, half-pipes, on-pipes  
The pipe parameter is irrelevant and such roads just turn to bridges, no need to change.  
But some need adjust for roll angles.
- Loops and any high roll angles. Will go bad in RoR, so need to flatten, reduce.
- Road bridge columns. All ignored now, to not appear on other roads.
- Sky textures.  
Using few params from `presets.xml` for generation of Caelum sky setup.
- Particles, clouds etc.
- Height fog
- Dynamic objects
- Caves, hexrocks, etc. Some SR3 meshes aren't in old SR data or were renamed. Especially those with material variations.
- Fluids, mud etc. Only one big water can be in RoR.

----
### Setup

These are all steps needed:

#### RoR mods path

Set *Main RoR* path for *mods* (or content) on Gui, Export tab, subtab Settings.  
It should end with / or \\.  
On Linux: `/home/username/.rigsofrods/mods/`  
Here will tracks be exported into.  
Also other subdirs will be needed or created here.

#### Old SR

This the path to SR 2.x `data/` dir, where are subdirs like: materials, objects*, terrain, trees* etc.  
It should end with / or \\.

#### SR packs

Download all SR packs (from [topic](https://github.com/stuntrally/tracks3/compare/main...ror) _or in future RoR repository, or you could already have them in *mods*_)  
and **extract** them in RoR *mods* path.  

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
    _But SR2 is not developed now. Thus SR3 editor is more advanced and it can also load old SR2 tracks._

2. Start SR3 Track Editor (info in [here](Running.md)), and load any track.

3. Go to (last) tab Export in editor.

4. Go through all points of previous chapter Setup.  

5. Then setup paths in Export, subtab Settings.

6. Go to first tab (Warnings). Press **Export**.  
   It can take few seconds. At end there will be log of what was done.  
   Check it for warnings (orange or red).  
   If it went well (and sr packs are present if used) then you can start this track in RoR.

7. Test track in RoR. Things could look or be wrong.  

8. If so, go back to SR3 editor Gui, on Export tab, and **Track** or Road subtabs.  
   Adjust if needed. Press Save there. Export, test and repeat if needed.  
   For example, often adjusted:  
   - Road terrain layer, check look on Layers tab (alt-T).
     - You can specify own RoR road texture on Layers tab, picking it in unused (e.g. last 6th) slot, not enabled.
   - Road height offset, moves all bridges up/down. To adjust bridge entries and avoid step.
   - Fluids, if Auto fails, set to either None or Manual and set height.
   - Trees or grass density. E.g. reduce if too much or lags in RoR.


----

### Tools

Some more tools are present in ed on Export tab.

These are used for content of sr-packs.

Buttons:

#### Materials

Will convert from SR2 `data/materials/scene` all needed `.mat` files into `.material` for RoR.  
Resulting files is in RoR mods path in subdir `materials`.  
Had manual adjusts made after, like removing not needed etc.

#### Terrain

Will convert/combine SR2 terrain textures (files *_d, *_s, *_n .jpg) into RoR (needed _ds.png and _nh.png).  
_d - diffuse, s - specular (ignored, bug), n - normal, h - height (now just made simply from n)_  
Resulting files are put into RoR mods path in subdir `terrain`.

#### Surfaces

Will convert SR3 surfaces.cfg to RoR groundmodel.  
All friction coeffs get added a factor, value from slider All Surface Friction.  
Result file `sr-surfaces.cfg` is put into RoR mods path in `sr-checkpoint-v1` subdir.


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
