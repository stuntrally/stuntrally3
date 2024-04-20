_Help on using SR3 Track Editor._


### Introduction

Stunt Rally 3 includes a Track Editor that can be used for creating your own tracks or modifying the existing ones.

It can also export track(s) to Rigs of Rods. Details [here](EditorExportRoR.md).


## NEW Video Tutorial

Just started video tutorial for latest version 3.0:  

[Full playlist](https://www.youtube.com/playlist?list=PLU7SCFz6w40PY3L8qFNTe4DRq_W6ofs1d) on youtube.

### [Chapter 1: Intro](https://youtu.be/Ckoz-GmU3qk)  

0:59 Help, input  
1:15 **Main modes** F1, Tab  
2:06 **Camera** mode, move, rotate  
2:50 Camera Top view  
3:20 Camera options: speed, intertia, view buttons  
4:03 **Edit modes** overview  
4:59 Input Shortcuts Alt-Q, tabs, subtabs etc  
8:48 **Toggle**: V Vegetation, G fog, I weather, P particles  
10:09 **Focus** \  
11:06 Preview camera F7  

### [Chapter 2: Terrain](https://youtu.be/D4_I-C3DLxo)

0:08 **New/duplicate** track, backup  
1:28 Removing not needed elements  
2:45 Terrain size and **heightmap** setup info  
3:54 New heightmap size, flat  
4:23 Change triangle size  
4:46 Approaches for making tracks  
5:27 Terrain **Generator**  
6:58 **Brushes** presests  
8:07 Terrain **edit** tools: Deform, Smooth, Filter, Height  
10:50 longer editing terrain with various brushes  
15:28 end, look around  

### [Chapter 3: Terrain Layers](https://youtu.be/NO2M1kMjR3o)

0:34 Layer scale  
0:57 Noise factors (+1 +2) intro  
2:17 Blendmap **Test** F9  
3:39 Example (Sway) - only terrain **Angle** ranges, smooth  
5:26 **Default** layer - 1st, for not covered ranges, sharp edge spots  
7:30 **Noise** factors (also -1), and parameters  
9:06 Example (Beach) - terrain **Height** ranges  
10:31 **Only Noise** layer, for blending other layers into it  
12:12 Example (MountCaro) - 1st terrain Height range, sand  
14:40 Example (MountCaro) - 2nd terrain Height range, highest  
16:23 Example (Terrains) - 5 different, other layer setups  
17:32 **Adding** layer for high slope angles  
19:19 Adding layer for top heights  


Chapter 4: Terrain Horizons

ToDo ...


----
### VERY OLD Video Tutorial

Outdated. For very old SR 2.x versions. Moved [here](EditorOld.md).


----
  

## Shortcuts

All key and mouse actions can be seen in Options tab *Input/Help*, use Ctrl-F1 to show tab for current mode.  
The text included on those pages is a complete reference to all editor actions (and gets translated to your language, if done).

Here is a quick reference to most common actions.

  

**F1 or ` - GUI Options toggle**  
**Tab - Camera / Edit mode toggle**  
current mode is shown in left bottom corner: Cam, Edit or Gui

  
### Camera

|  |  |  |  |
| --- | --- | --- | --- |
| Keys: | W,S, A,D, Q,E - move || Arrows - rotate |
| Mouse: | LMB - rotate | RMB - move (pan) | MMB - zoom |
| All moves and value changes with: ||||
| | shift - slower | ctrl - faster | alt - very fast |
| Alt-Z - toggle top view camera | | |
| \ - focus camera on current (object, anything) | | |

### Edit Modes

  * ⛰️ Terrain
    * **D** - Deform
    * E - Set Height
    * S - Smooth (flatten)
    * F - Filter (remove bumps, noise)

  * 🛣️ Road
    * **R** - Edit Road
    * Q - car Start Position and area

  * Other
    * W - Fluid areas (water🌊, mud, lava🌋, acid, etc)
    * X - Objects (dynamic📦 or static, obstacles, buildings🏢)
    * A - Particles (clouds☁️, fire 🔥, smoke🌋, water etc)

### Main

**Ctrl-F1** - ❔ Press to show help page for current mode.

**F4** - Save track (after this track is ready to be played in game)  
**F5** - Reload track (one time undo)

### Toggle

**V** - 🌳🪨🌿 Vegetation - hide it for clean editing road etc  
**G** - 🌫️ Fog - when editing from distance  
**I** - 🌧️ Weather - rain, snow, etc  
**P** - 🔥 Particles - clouds, dust, etc  

### Update

**B** - Rebuild road (after terrain under it changed, etc.)  
F8 - Update (after changes made on Terrain/Vegetation tab)

### Tools

**U** - align terrain to road tool (in Road🛣️ edit mode, for selected segments)  
**C** - toggle on/off dynamic objects simulation (in Objects📦 mode)  
**F7** - enter/leave Preview camera

   
### Shortcuts

| Jump directly to Gui tab: ||
| --- | --- |
| Alt-I - Input/help | Alt-D - Terrain Brushes |
| Alt-Q - Track | Alt-T - Terrain Layers |
| Alt-E - Settings | Alt-R - Road |
| | Alt-V - Vegetation |
| | Alt-X - Objects |

   
----

### Approaches

Basic Approaches to making a track:

  1. Make great terrain and place road in it (difficult)
  2. Make cool road and decorate terrain after (easy)
  3. Mixed mode. Edit terrain and road freely.

### Workflow

A list of short steps that describe creating a new track on a high level:

Save **frequently** (F4), after each step or change. When made a mistake hit F5 to reload last track state.

  * 1
    * load an existing track - that has the most of desired features already set up (eg. has good terrain layers, vegetation, terrain heightmap etc.).
    * enter new track name, press button: new (duplicate)
  * 2
    * on terrain tab, choose area, make new flat terrain (pick heightmap and triangle size) - _when making new terrain shape_
    * edit terrain (deform up/down or smooth)
    * it's good to place car start now and test terrain scale in game by driving
    * _optionally_ edit terrain layers
  * 3
    * draw initial road shape - inserting road points from top view
    * edit road points (make bridges,etc), adjust roll angles, _optionally_ use align terrain to road tool
  * 4
    * setup car start position and its area (in Edit mode press Q and Ctrl-LMB to place it at cursor) - after this track can be driven in game
    * choose road direction (+1 or -1), add road checkpoints (spheres, chkR), mark 1st checkpoint - after this track timing works
  * 5
    * adjust vegetation amounts (grass, trees)
    * _optionally_ edit vegetation layers and other parameters on vegetation tab
    * enter preview camera and make a track screenshot
    * drive the track, go back to editor and make changes to road / terrain to get the desired diving experience (this is quite important and best to repeat it often, starting early and after each major change)


## Steps after

### ini

Filling 📄 `config/tracks.ini` is done by CryHam to have some consistency for all tracks so far.  
Adding a line to tracks.ini at end, with your track's name (put it in original tracks/ dir) will show difficulty ratings, time, etc on Gui tab Track.  
See inside [tracks.ini](../config/tracks.ini) for more info about syntax and values.  
Each rating is manually set by feeling and should be checked with other tracks to fit rest well.  

After modifying a track (tracks3 PR was merged), you can edit [tracks.ini](../config/tracks.ini) and add yourself to authors (last `a:`)  
and set modified date `dd/mm/yy` as today or to when it was done.


### Track's Ghost

How to add Track's Ghost🏞️👻 drive to your new track.  
This will also show racing line (trail) and few more pacenotes (bumps etc).

  1. Be sure you didn't drive ES yet (well or faster) on this track. Or just delete your older ghost for it.
  2. Start game, use Rewind: Back in time.
  3. Use car: ES and Simulation: Normal.
  4. Drive smoothly, as perfectly as you can, always on road, with no hits.  
     Drive 1 lap from start to finish, not using boost unless impossible otherwise (or some underwater).
  5. Copy user's ghost (named by your track and `_ES.rpl`) from:  
     `~/.local/share/stuntrally3/ghosts/normal/` into:
     `/data/ghosts/original`.
  6. Start convert tool, run: `./stuntrally3 convert`
  7. This will result in new file in `/data/ghosts` and be visible in game now.
  8. After this, better track time will be known.  
     Adjust last value `T=` in `tracks.ini` so that it gives about 12 points for this ES ghost drive.  
     Add extra seconds 2..7 for difficult tracks for user rewinds and mistakes.
