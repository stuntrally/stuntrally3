_List of things **to do**, planned for next releases._  

This is the latest, fast updated list.  
More in [Tasks tracker](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php?page_number=1) with older tasks too and some more detail.

Marked: `crucial`, **Next or Big**, _Important_.


### 3.3

📽️ new game *video*  
*editor videos*? all new  

----
### 🪄 Effects

- HDR, bloom, [fix fireflies](https://catlikecoding.com/unity/tutorials/custom-srp/hdr/)  
- lens flare, [shaders](https://www.shadertoy.com/results?query=lens+flare)  
- sunrays, [shaders](https://www.shadertoy.com/results?query=tag%3Dgodrays)  
- **Soft** Particles  
- old motion blur  


### 🔉 Sound

sound engine from [RoR PR](https://github.com/RigsOfRods/rigs-of-rods/pull/3182), branch sound fix..  
or replace with [ogre-audiovideo](https://github.com/OGRECave/ogre-audiovideo))  
or add **new** code for:
- dynamic hit [sounds pool](https://github.com/kcat/openal-soft/issues/972#issuecomment-1934265230) (e.g. 10), auto removed after play
- ambient sound, reload buffer for track
- more tire sounds, load only used (few, pool e.g. 2), or:
- init more sources, gui opt, [ref 1](https://github.com/kcat/openal-soft/issues/736), [ref 2](https://github.com/kcat/openal-soft/issues/972)
- ed mode, track sounds: waterfall, volcano, river..

🌧️ **Ambient** [Sounds](https://stuntrally.tuxfamily.org/mantis/view.php?id=1): rain, wind, forest etc  
🚗🔊 More `car Sounds`, todo [task](https://stuntrally.tuxfamily.org/mantis/view.php?id=1), use [engine-sim](https://github.com/stuntrally/stuntrally3/issues/7), exhaust [muffler spectrum](https://duckduckgo.com/?t=ftsa&q=muffler+spectrum&atb=v395-1&ia=web)?  
📦 object *hit* sounds  
underwater low pass filter, in-car or back exhaust filters-  
reverb change in water, caves, pipes-  

### Big

New **tracks**, also with **fields**, add walls  
walls repeat mul uv, more mtr  

💎Add gems to tracks, more types 3d  
xml par continuous?, logic btn continue, hid collected gems on start?  

test, move to bullet3 ?

### New

underwater: darken fog from car depth, *fix* issues [at end](https://catlikecoding.com/unity/tutorials/flow/looking-through-water/)  
less poly, own collision *_p.mesh for veget/obj/bld, e.g. shrooms on Cloud etc

new road particles  
new **road** textures, detail mix, vert clr blend

hud new best time show, win particles-  

**ed key** for obj random rot, yaw, set up/dn y +-90  
add arrow for emit dir  


----
## Fix Ogre

`fix` ed RTTs **no** fluids  
move **ed**, all from SR3.compositor to AppGui_Compositor.cpp  
manual update ed RTTs^  

split screen: no **fog** in fluids, no refract **depth** with fsaa  
win place twice 1st on Oc, 3plr?  
car glass reflect cube sky pos bad on Test1-Flat?  
-each car own reflect cube  

own rqg for veget on horiz? to skip in wtr refl etc

specular on terrain  

rivers wrong way  
waterfalls refract cuts off above cam y  

Pbs particles, fade from fog, soft  

🌊refract, depth: pass projmatrix `not` **inverse**  
underwater top has no surface, refract  

shader **params**: refract clr, *depth* clr, emissive fluids?  

🌪️**wind scale** params in json, for all trees, bushes, grass sway own-  
wind anim shadow **caster** too, ed prv obj no wind-  


### Fix

**ed** fields rot, focus, telep `offset`, no cam ray cut  

`rays casts?` **H2,H1** going through big waters e.g. Islands,  
wheels jumping on big ice e.g. FreshBreeze,  
*cam tilt* jumps on Shore, Treausure, etc?  

fix .car thrusters lights pos  
**hide** flares with in car camera  


----
### New game stuff✨

🏆 new [game Modes](https://stuntrally.tuxfamily.org/mantis/view.php?id=21): [**Career**](https://forum.freegamedev.net/viewtopic.php?f=79&t=5211)  
Score for: drift (sliding on road), air time (jumps etc), hitting dynamic objects  

🕹️ New **Game** elements:  
🎳 moving & rotating obstacles, elevators  

Track 💡*lights* ed mode, car glow (as rect area) lights, bridges, pipe easy add lights?  
**entities**.xml (sum of object, animation, light, 🔥particles etc)  
Road wall types, [link](https://stuntrally.tuxfamily.org/mantis/view.php?id=6)  


----
### data

Remove all non **CC** data, [new topic](https://groups.f-hub.org/d/pIoLYCaO/-removing-replacing-data-with-non-cc-lincenses-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532&sid=b1e7ee6c60f01d5f2fd7ec5d0b4ad800)  
🌳new [trees](https://groups.f-hub.org/d/4n0bTwmC/-trees-to-do), normalize .mesh scales fir*, etc  
redo or replace lowest quality meshes: , skyscraper* 🏢  

[Horizons](https://stuntrally.tuxfamily.org/mantis/view.php?id=11), add to rest of tracks  


----
### Ogre new

Terrain new **ssao map** added to shadowmap: normals, ssao pass, from top view  
🌊 Ocean from Terra, **waves**, vehicle deform, fluid *particles*  
GI for terrain [issue](https://github.com/OGRECave/ogre-next/issues/475)?  

⏲️ Gauges with shader? [demo](https://www.shadertoy.com/view/7t3fzs)  

🌀 Add [FastNoise](https://github.com/Auburn/FastNoiseLite) for brushes and ter gen  


----
### Ogre cd, restore, optimize

*lit* particles, fog  
lights on `grass` tbn?, grass shader get hmap?, grass far fade-  
**parallelize** vegetation, add test grass, veget on more threads  

- ⛰️ Terrain  
  **shadowmap** fix < 0  
  fix restore **pages** visibility  
  triplanar **only** for chosen layers  
  add emissive, reflect *par*  
  read blendmap, for ter wheel **surface** ids, many ters  
  fix *triplanar* normalmaps-  
  blendmap also .hlsl or .any for DX  
- 🛣️ Road  
  **Grid** for walls, road, pipes, ed update cell  
  1 mesh with 4 LODs, shader fix specular stretch  

- 🏗️ Editor, fix  
  objects pick `errors`  
  minimap **RTTs** upd skip (slow), or create on save  
  new terrain issues [here](https://groups.f-hub.org/d/SW0mnXNV/track-rework-horizons-skies-and-updates/14)  
  edit `moved` terrains, error in ray pos-  
  _tools_: fix **scale ter h**, copy horizon, which ter, road  

- ⏲️ `HUD`  
  fix **splitscreen** gauges etc  
  _Minimap_ circle, terrain add, fix pos tris  
  btnRplDelete for ghost should also remove its track record from .txt  


----
### old restore cd

add new tracks to `champs` 🏆  

auto road/ter **Surface**: Particles, trail, sounds, from `presets.xml` not per track  
tire **trails clr**, no uv tex-  

Road noise blend, color per vertex  
grass density as **RTT**, noise, color  

*inside* height fog?  
noise 3d? _volumetric_ clouds  

ed _tools_: copy roads which combo,all  
Grass read Hmap - no index, vertex buffers? texture atlas-  

👥 [Splitscreen](https://stuntrally.tuxfamily.org/mantis/view.php?id=26) issues  
🖥️ finish render system rest options  
🖱️ Input mouse bind, 👀 VR mode meh-  

*auto* Add SR icons to: game & editor for Windows & Linux, and installer  
upmerge MyGui ?CMake .lib, Conan  
broken mygui set scrollview size, gallery list  

.mesh LODs test adjust-  
water refl vis dist? low gets cut bad  
Fog, 2 colors cam dir fix-  

gpu mem leaks or renders inactive  
slow track load, shaders cache big  


----
### Tasks

Basically all [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) with priority 1 to 12.

#### Graphics

🏞️ [Rivers/waterfalls](https://stuntrally.tuxfamily.org/mantis/view.php?id=7)  
✨ [Particles](https://stuntrally.tuxfamily.org/mantis/view.php?id=2)  
🌞 [Sun flare](https://stuntrally.tuxfamily.org/mantis/view.php?id=9), sun rays..  
⚡ lightning weather: line, thunder sound, light or glow  

#### Data

🌟 Sceneries: Spring blooming trees, Fractal, Organic?, Candy?  
🚗 New Vehicles: [new topic](https://groups.f-hub.org/d/adePgxzW/-cars-new-vehicles-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526), models **collection** [link](https://sketchfab.com/cryham/collections/vehicles-todo-for-stunt-rally-327a2dd7593f47c7b97af6b806a60bb8)  

🛣️ **Road** textures, from ground 4k?, redo, 2,3 variations, blending few at once?, traces alpha texture  

🏢 finish rest of 0AD han buildings, maur, others?  
📦 New static Objects, tall city buildings?, dynamic hay  

#### Meh
ed roads: [split](https://stuntrally.tuxfamily.org/mantis/view.php?id=4), checkpoints, merge?  
sim new poses _interpolation_?  
[Pace notes](https://stuntrally.tuxfamily.org/mantis/view.php?id=14) - game Info window for all signs  
