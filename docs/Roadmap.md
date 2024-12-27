_List of things **to do**, plann`ed` for next releases._  

This is the latest, fast updated list.  
More in [Tasks tracker](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php?page_number=1) with older tasks too and some more detail.

Priorities marked: !, `crucial`, **Next or Big**, _Important_.


### 3.3 !

📽️ new game **video**  

grh opt chk  
*new* tracks add to challs 🏆  

🌧️ Ambient [Sounds](https://stuntrally.tuxfamily.org/mantis/view.php?id=1): rain, wind, forest etc  
📦 object *hit* sounds🔊  

fix *tracks*:  
MogFoss waterfals up, Preyth bld up-  
Tiny, Mudflats, Knotted - ssao bad alpha bridge  
MiningCity, KapabaRiv, Overgrowth, Glitchy - road segs white blend transp/invisible  

### 3.4

*editor videos* all new  

! auto road/ter **Surface**: Particles, trail, fix clr, sounds, from `presets.xml` not per track  
new road particles  

🛣️! new **Road** textures, from ground 4k CC0, rem old road**  
detail mix in .json, 2,3 variations  
traces alpha texture, *blending* with color per vertex: in turns, *noise*  

finish remove all non **CC** data, [new topic](https://groups.f-hub.org/d/pIoLYCaO/-removing-replacing-data-with-non-cc-lincenses-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532&sid=b1e7ee6c60f01d5f2fd7ec5d0b4ad800)  
- replace fir*
- grass1, grassA* texture
- full chk, wave tex?
- redo sounds/vdrift:  bump, gravel,grass,squeal


### 🏗️ Editor

fix ed btn: ApplyRefl, ApplyShadows

! **ed key** for obj random rot, yaw, set up/dn y +-90  
ed add underwater  
ed add arrow for emit dir  

! ed *manual* update minimap **RTTs**, upd skip slower, or create on save?  

ed fields rot, focus, teleport `offset`, no cam ray cut  

objects pick `errors`  
new terrain issues [here](https://groups.f-hub.org/d/SW0mnXNV/track-rework-horizons-skies-and-updates/14)  
edit `moved` terrains, error in ray pos-  

_tools_: fix **scale ter h**  
copy horizon which ter, copy roads which combo,all  


----
### 🪄 Effects

lens flare, ed add real pos on sky, other for sun dir?  
sunbeams dither, par hq, [shaders](https://www.shadertoy.com/results?query=tag%3Dgodrays)  

**HDR**, bloom, [fix fireflies](https://catlikecoding.com/unity/tutorials/custom-srp/hdr/), adjust bright,sky *all* tracks  
**Soft** Particles, Pbs: fog fade, lit by diffuse  
old motion blur-  


### 🔉 Sound

**Track sounds**: waterfall, volcano, river, shore waves..  ed mode, pos, radius  
**Track lights** ed mode, spot angle, dist, -easy add: bridges, pipe lights?  

*low pass* filter for: underwater, no throttle,  
reverb change in water, caves, pipes-  
in-car or back exhaust filter?-  

meh, sound engine from [RoR PR](https://github.com/RigsOfRods/rigs-of-rods/pull/3182), fix branch *sound*..  
merge *replace* with [ogre-audiovideo](https://github.com/OGRECave/ogre-audiovideo)  
or add *new* code for:
- limit? dynamic hit [sounds pool](https://github.com/kcat/openal-soft/issues/972#issuecomment-1934265230) (e.g. 10)
- more tire sounds, load only used (few, pool e.g. 2)

----
### Tracks

New **tracks**, also with **fields**  
walls repeat mul uv, more mtr, wall *types*, [link](https://stuntrally.tuxfamily.org/mantis/view.php?id=6)  

💎Add gems to tracks, more types 3d  
xml par continuous?, logic btn continue, hid collected gems on start?  

### New

translucency - lighter tree leaves, outside, facing sun  

underwater: darken fog from car depth, *fix* issues [at end](https://catlikecoding.com/unity/tutorials/flow/looking-through-water/)  
less poly, own collision *_p.mesh for veget/obj/bld, e.g. shrooms on Cloud etc

hud new best time show, win particles-  

----
### Fix Ogre

move rest? from SR3.compositor to AppGui_Compositor.cpp  

own *rqg*/vis for veget on horiz? to skip in wtr refl etc

**specular** on terrain  

move *trails* before particles/clouds  
car *glass*, pipes, beam etc not visible in fluid if seen from above  
glass pipes not fogged?

**rivers** wrong way  
rqg rivers after water  
river alpha fade last segs  

! 🌊refract, depth: pass projmatrix `NOT` **inverse**  
waterfalls refract `cuts` off above cam y  
underwater top has no surface, refract  

shader **params**: refract clr, *depth* clr, emissive fluids?  

! **wind** anim `shadow caster` too, ed prv obj no wind-  
🌪️ wind *scale* params in json, for all trees, bushes, grass sway own-  

----
### 👥 Split screen

! fix no **fog** in fluids, no refract **depth** with fsaa  
win place twice 1st on Oc, 3plr?  

car glass reflect cube **sky pos** bad on Test1-Flat?  
-each car own reflect cube  

fix **gauges** etc  

fix .car thrusters lights pos  
**hide** flares for in car camera  

old [Splitscreen](https://stuntrally.tuxfamily.org/mantis/view.php?id=26) issues  


----
### New game stuff✨

🏆 new [game Modes](https://stuntrally.tuxfamily.org/mantis/view.php?id=21): [**Career**](https://forum.freegamedev.net/viewtopic.php?f=79&t=5211)  
Score for: drift (sliding on road), air time (jumps etc), hitting dynamic objects  

🕹️ New **Game** elements:  
🎳 moving & rotating obstacles, elevators  

**entities**.xml (sum of object, animation, light, 🔥particles, sound, code script, etc)  

🤪 Mods: rainbow roads vertex clrs, rgb anim cars/obj
barrel stacks on track?, funny sounds replace,  
🌦️ weather change? to: heavy, light rain, cloudy, time: aft/noon, night, sunset, dawn

test, big, move to bullet3?


----
### 🟢 Ogre new

🌊! Terra `waves`, vehicle deform tex/compute-, fluid *particles*  

💦 Vehicle, wheels **dirt**,mud and **damage** *shader*, by dynamic paint, detail texture  

🕳️ Terrain new **ssao map** added to shadowmap: normals, ssao pass, from top view  
GI for terrain [issue](https://github.com/OGRECave/ogre-next/issues/475)?  

🌫️ *inside* height fog?  
_volumetric_ clouds,dust, noise 3d, for depth, add cur fog, particles-  

⏲️ Gauges with shader? [demo](https://www.shadertoy.com/view/7t3fzs)  

🌀 Add [FastNoise](https://github.com/Auburn/FastNoiseLite) for brushes and ter gen  


----
### Ogre cd, restore, optimize

**parallelize** vegetation, add test grass, veget on more threads  

- ⛰️ Terrain  
  ! **shadowmap** fix < 0  
  fix restore **pages** visibility, [bug video](https://www.youtube.com/watch?v=4PCupZ6aGqk&list=PLU7SCFz6w40OJsw3ci6Hbwz2mqL2mqC6c&index=2)  
  triplanar **only** for chosen layers  
  add emissive, reflect *par*  
  read blendmap, for ter wheel **surface** ids, many ters  
  fix *triplanar* normalmaps-  [bug video](https://www.youtube.com/watch?v=SI7zPOfAxOI&list=PLU7SCFz6w40OJsw3ci6Hbwz2mqL2mqC6c&index=8)  
- 🛣️ Road  
  **Grid** for walls, road, pipes, ed update cell  
  1 *mesh* with 4 LODs, shader fix specular stretch  
- 🌿 Grass  
  lights on `grass`, set tbn?  
  grass density as **RTT**, noise, color  
  grass shader get hmap?  
  grass far fade-  
  no index, vertex buffers? texture atlas-  

----
### old restore cd

_Minimap_ circle, terrain add, fix pos tris  
tire trails clr, no uv tex-  

🖥️ finish render system rest options  
🖱️ Input *mouse* bind  
👀 VR mode meh-  
DX- .hlsl or .any, from only .glsl: blendmap? effects
btnRplDelete for ghost should also remove its track record from .txt  

auto Add SR *icons* to: game & editor for Windows & Linux, and installer  
upmerge MyGui ?CMake .lib, Conan  
broken MyGui set scrollview size, gallery list  

.mesh LODs test adjust-  
water refl vis dist? low gets cut bad  
Fog, 2 colors cam dir fix-  

gpu mem leaks or renders inactive  
slow track load, shaders cache big  

----
### Data

🚗! New **Vehicles**: [new topic](https://groups.f-hub.org/d/adePgxzW/-cars-new-vehicles-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526), models **collection** [link](https://sketchfab.com/cryham/collections/vehicles-todo-for-stunt-rally-327a2dd7593f47c7b97af6b806a60bb8)  

🌳new [trees](https://groups.f-hub.org/d/4n0bTwmC/-trees-to-do), plants  

📦 New static Objects, *tall* city buildings, dynamic hay  
redo or replace lowest quality meshes: , skyscraper* 🏢  

🏢 finish rest of 0AD han buildings, maur, others?  

🌟 Sceneries: Spring blooming🌸 trees, Fractal, Organic?, Candy?  

[Horizons](https://stuntrally.tuxfamily.org/mantis/view.php?id=11), add to rest of tracks  


----
### old Tasks

Basically all [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) with priority 1 to 12.

#### Graphics

🏞️ [Rivers/waterfalls](https://stuntrally.tuxfamily.org/mantis/view.php?id=7)  
✨ [Particles](https://stuntrally.tuxfamily.org/mantis/view.php?id=2)  
⚡ lightning weather: mesh line, thunder sound, light, hdr glow-  

#### Meh
ed roads: [split](https://stuntrally.tuxfamily.org/mantis/view.php?id=4), checkpoints, merge?  
sim new poses _interpolation_?  
[Pace notes](https://stuntrally.tuxfamily.org/mantis/view.php?id=14) - game Info window for all signs  
