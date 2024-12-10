_List of things **to do**, planned for next releases._  

This is the latest, fast updated list.  
More in [Tasks tracker](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php?page_number=1) with older tasks too and some more detail.

Priorities marked: !, `crucial`, **Next or Big**, _Important_.


### 3.3 !

📽️ new game **video**  
*editor videos* all new  

`ed` apply shadows crash  
ed fix *brushes* png load

add **sounds**🔊  
🌧️ Ambient [Sounds](https://stuntrally.tuxfamily.org/mantis/view.php?id=1): rain, wind, forest etc  
📦 object *hit* sounds  

fix *tracks*:  
MogFoss waterfals up  Preyth bld up-  
Tiny, Mudflats, Knotted - ssao bad alpha bridge  
MiningCity, KapabaRiv, Overgrowth, Glitchy - road segs white blend transp/invisible  

new tracks ghosts, add to `champs`,challs 🏆  

----
### 🪄 Effects

lens flare, ed add real pos on sky, other for sun dir?  
sunbeams dither, par hq, [shaders](https://www.shadertoy.com/results?query=tag%3Dgodrays)  

**HDR**, bloom, [fix fireflies](https://catlikecoding.com/unity/tutorials/custom-srp/hdr/), adjust bright,sky *all* tracks  
**Soft** Particles, Pbs: fog fade, lit by diffuse  
old motion blur  


### 🔉 Sound

**Track sounds**: waterfall, volcano, river, shore waves..  ed mode, pos, radius  
**Track lights** ed mode, spot angle, dist, -easy add: bridges, pipe lights?  

🚗🔊 More `car Sounds`, todo [task](https://stuntrally.tuxfamily.org/mantis/view.php?id=1), use [engine-sim](https://github.com/stuntrally/stuntrally3/issues/7), exhaust [muffler spectrum](https://duckduckgo.com/?t=ftsa&q=muffler+spectrum&atb=v395-1&ia=web)?  
underwater low pass filter, in-car or back exhaust filters-  
reverb change in water, caves, pipes-  

meh, sound engine from [RoR PR](https://github.com/RigsOfRods/rigs-of-rods/pull/3182), fix branch *sound*..  
merge **replace** with [ogre-audiovideo](https://github.com/OGRECave/ogre-audiovideo)  
or add **new** code for:
- limit? dynamic hit [sounds pool](https://github.com/kcat/openal-soft/issues/972#issuecomment-1934265230) (e.g. 10)
- more tire sounds, load only used (few, pool e.g. 2)


### Tracks

New **tracks**, also with **fields**  
walls repeat mul uv, more mtr, wall *types*, [link](https://stuntrally.tuxfamily.org/mantis/view.php?id=6)  

💎Add gems to tracks, more types 3d  
xml par continuous?, logic btn continue, hid collected gems on start?  

test, big, move to bullet3?

### New

auto road/ter **Surface**: Particles, trail, sounds, from `presets.xml` not per track  

translucency - lighter tree leaves, outside, facing sun  
Terra **water waves**  

underwater: darken fog from car depth, *fix* issues [at end](https://catlikecoding.com/unity/tutorials/flow/looking-through-water/)  
less poly, own collision *_p.mesh for veget/obj/bld, e.g. shrooms on Cloud etc

new road particles  
new **road** textures, detail mix, vert clr blend

hud new best time show, win particles-  

### Editor

! **ed key** for obj random rot, yaw, set up/dn y +-90  
ed add underwater  
ed add arrow for emit dir  

! ed *manual* update minimap **RTTs**, upd skip slower, or create on save?  

ed fields rot, focus, telep `offset`, no cam ray cut  

----
## Fix Ogre

move rest? from SR3.compositor to AppGui_Compositor.cpp  

own *rqg*/vis for veget on horiz? to skip in wtr refl etc

**specular** on terrain  

move *trails* before particles/clouds  
car *glass*, pipes, beam etc not visible in fluid if seen from above  

**rivers** wrong way  
rqg rivers after water  
river alpha fade last segs  

! 🌊refract, depth: pass projmatrix `NOT` **inverse**  
waterfalls refract `cuts` off above cam y  
underwater top has no surface, refract  

shader **params**: refract clr, *depth* clr, emissive fluids?  

! **wind** anim `shadow caster` too, ed prv obj no wind-  
🌪️ wind *scale* params in json, for all trees, bushes, grass sway own-  


### Split screen

! fix no **fog** in fluids, no refract **depth** with fsaa  
win place twice 1st on Oc, 3plr?  

car glass reflect cube **sky pos** bad on Test1-Flat?  
-each car own reflect cube  

fix **splitscreen** gauges etc  

fix .car thrusters lights pos  
**hide** flares with in car camera  


----
### New game stuff✨

🏆 new [game Modes](https://stuntrally.tuxfamily.org/mantis/view.php?id=21): [**Career**](https://forum.freegamedev.net/viewtopic.php?f=79&t=5211)  
Score for: drift (sliding on road), air time (jumps etc), hitting dynamic objects  

🕹️ New **Game** elements:  
🎳 moving & rotating obstacles, elevators  

**entities**.xml (sum of object, animation, light, 🔥particles, sound, code script, etc)  


----
### Data

Remove all non **CC** data, [new topic](https://groups.f-hub.org/d/pIoLYCaO/-removing-replacing-data-with-non-cc-lincenses-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532&sid=b1e7ee6c60f01d5f2fd7ec5d0b4ad800)  
🌳new [trees](https://groups.f-hub.org/d/4n0bTwmC/-trees-to-do), replace fir*  
redo or replace lowest quality meshes: , skyscraper* 🏢  

[Horizons](https://stuntrally.tuxfamily.org/mantis/view.php?id=11), add to rest of tracks  


----
### Ogre🟢 new

🌊! Terra `waves`, vehicle deform tex/compute-, fluid *particles*  

Vehicle, wheels **dirt**,mud and **damage** *shader*, by dynamic paint, detail texture  

Terrain new **ssao map** added to shadowmap: normals, ssao pass, from top view  
GI for terrain [issue](https://github.com/OGRECave/ogre-next/issues/475)?  

⏲️ Gauges with shader? [demo](https://www.shadertoy.com/view/7t3fzs)  

🌀 Add [FastNoise](https://github.com/Auburn/FastNoiseLite) for brushes and ter gen  


----
### Ogre cd, restore, optimize

lights on `grass` tbn?, grass shader get hmap?, grass far fade-  
**parallelize** vegetation, add test grass, veget on more threads  

- ⛰️ Terrain  
  ! **shadowmap** fix < 0  
  fix restore **pages** visibility, [bug video](https://www.youtube.com/watch?v=4PCupZ6aGqk&list=PLU7SCFz6w40OJsw3ci6Hbwz2mqL2mqC6c&index=2)  
  triplanar **only** for chosen layers  
  add emissive, reflect *par*  
  read blendmap, for ter wheel **surface** ids, many ters  
  fix *triplanar* normalmaps-  [bug video](https://www.youtube.com/watch?v=SI7zPOfAxOI&list=PLU7SCFz6w40OJsw3ci6Hbwz2mqL2mqC6c&index=8)  
  blendmap also .hlsl or .any for DX  
- 🛣️ Road  
  **Grid** for walls, road, pipes, ed update cell  
  1 mesh with 4 LODs, shader fix specular stretch  

- 🏗️ Editor, fix  
  objects pick `errors`  
  new terrain issues [here](https://groups.f-hub.org/d/SW0mnXNV/track-rework-horizons-skies-and-updates/14)  
  edit `moved` terrains, error in ray pos-  
  _tools_: fix **scale ter h**, copy horizon, which ter, road  

- ⏲️ `Hud`  
  _Minimap_ circle, terrain add, fix pos tris  
  btnRplDelete for ghost should also remove its track record from .txt  


----
### old restore cd

tire **trails clr**, no uv tex-  

*Road* noise blend, color per vertex  
grass density as **RTT**, noise, color  

*inside* height fog?  
noise 3d? _volumetric_ clouds  

ed _tools_: copy roads which combo,all  
Grass read Hmap - no index, vertex buffers? texture atlas-  

👥 [Splitscreen](https://stuntrally.tuxfamily.org/mantis/view.php?id=26) issues  
🖥️ finish render system rest options  
🖱️ Input *mouse* bind  
👀 VR mode meh-  

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
⚡ lightning weather: mesh line, thunder sound, light, hdr glow-  

#### Data

🚗! New **Vehicles**: [new topic](https://groups.f-hub.org/d/adePgxzW/-cars-new-vehicles-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526), models **collection** [link](https://sketchfab.com/cryham/collections/vehicles-todo-for-stunt-rally-327a2dd7593f47c7b97af6b806a60bb8)  

🛣️! **Road** textures, from ground 4k?, redo, 2,3 variations, blending few at once?, traces alpha texture  

📦 New static Objects, *tall* city buildings, dynamic hay  
🏢 finish rest of 0AD han buildings, maur, others?  

🌟 Sceneries: Spring blooming🌸 trees, Fractal, Organic?, Candy?  

#### Meh
ed roads: [split](https://stuntrally.tuxfamily.org/mantis/view.php?id=4), checkpoints, merge?  
sim new poses _interpolation_?  
[Pace notes](https://stuntrally.tuxfamily.org/mantis/view.php?id=14) - game Info window for all signs  
