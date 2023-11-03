_List of things **to do**, planned for next releases._  

This is the latest, fast updated list.  
More in [Tasks tracker](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php?page_number=1) with older tasks too and some more detail.

#### About

SR 3.0 moved to Ogre-Next, 3.0 or higher.  
[Forum topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18515) (progress),
[Ogre topic](https://forums.ogre3d.org/viewtopic.php?t=96576&sid=15b9610ca52af886515b49b4733ef546) (dev Q&A)  
Git [repo](https://github.com/stuntrally/stuntrally3) and [tracks](https://github.com/stuntrally/tracks3), older simple [terrain demo](https://github.com/cryham/ogre3ter-demo)  

<u>Marked</u>: `crucial` needed before release. Also **Next or Big** and _Important_.

### 3.0

- Water, fluids  
  tweak par, **refract, depth** for color, border alpha
- Fog, 2 colors cam dir fix-
- Terrain  
  **shadowmap** fix < 0, save/load-  
  add emissive, reflect par, fix triplanar normalmaps-  
  blendmap also .hlsl or .any  
  read blendmap, for ter wheel `surface` ids, many ters  
- Road  
  **Grid** for walls, road, pipes, ed update cell  
  1 mesh with 4 LODs, shader fix specular stretch  
- Grass, add Wind params  
  **deform** spheres, alpha or grow fade far  
- **Wind** for trees  

- **Editor**, fix  
  `save` Hmaps before lay upd  
  **rtt** upd skip fix, or create on save  
  water **reflect** in `F7 cam prv`  
  terrains move, ray, pos edit  
  objs pick errors  
  _tools_: fix scale ter h, copy horizon, which ter, road  

- **HUD**  
  fix **Splitscreen** crash, gauges  
  _Minimap_ circle, terrain add, fix pos tris  
  HudRend map issues-  

- Game, meh fix  
  water vis dist?  
  tire trails no uv tex-  
  cd veget mesh, LODs adjust  
  gpu mem leaks or renders inactive  

New tracks **ghosts**, add to champs and challs 🏆  
[Horizons](https://stuntrally.tuxfamily.org/mantis/view.php?id=11), add to rest of tracks  

fix and build on `Windows`: blendmap hlsl or any, MyGui CMake .lib, `conan`  
`install` docs/*.md files  

----
### 3.1

📽️ **editor Videos**, all new  
🪄 **Effects**: Soft Particles, SSAO, HDR, bloom, rays?  
🖥️ Gui render system, all **options**?  

Road noise blend, color per vertex  
grass density as rtt, noise, color  
inside height fog?, noise 3d? volumetric clouds  
auto road, ter surface **Particles**,trail, from `presets.xml` not per track  

🌀 Add [FastNoise](https://github.com/Auburn/FastNoiseLite) for brushes and ter gen  
🌊 Ocean: Terra waves, vehicle deform, particles  
⏲️ Gauges with shader [demo](https://www.shadertoy.com/view/7t3fzs)  

🌳 new trees, Bushes, more vegetation, tiny close, grasses, rock packs  
redo or replace lowest quality meshes: shroom* 🍄, skyscraper* 🏢  
normalize .mesh scales fir, etc.  

🖱️ Input mouse bind, 👀 VR mode meh-  
slow track load, shaders, ?ter get h, norm, do parallel  
Grass - no index, vertex buffers? read Hmap, texture atlas-  

----

### 2.8

new deps with latest Ogre fix for terrain visbility, resulting in editor vegetation issue  
meh, fix shadow check [here](https://github.com/OGRECave/ogre/blob/master/Media/RTShaderLib/GLSL/SGXLib_IntegratedPSSM.glsl#L86), screens [here](https://forums.ogre3d.org/viewtopic.php?t=96888&sid=bf9864a27e57f24f55950ccb5d273dea)  
add new tracks to champs and challs  

----
### 3.1 or later

_Basically all [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) with priority 1 to 12._

all json materials, custom params? wet?  
ed _tools_: copy roads which combo,all  
ed roads: split, merge?  
sim new poses _interpolation_?  

🔊 **Ambient** [Sounds](https://stuntrally.tuxfamily.org/mantis/view.php?id=1): rain, wind, forest etc  
🚗🔊 More car **sounds**.. todo [task](https://stuntrally.tuxfamily.org/mantis/view.php?id=1), use [engine-sim](https://github.com/stuntrally/stuntrally3/issues/7)  
⛅🌳 Remove all non **CC** data, [topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532&sid=b1e7ee6c60f01d5f2fd7ec5d0b4ad800), New skies  
🛣️ Road: wall types, spline walls [link](https://stuntrally.tuxfamily.org/mantis/view.php?id=6)  

🕹️ new **Game** elements:  
teleporters, moving & rotating obstacles, elevators  
force fields, speed up, lift, anti-gravity

🏆✨ new [game Modes](https://stuntrally.tuxfamily.org/mantis/view.php?id=21): [Career](https://forum.freegamedev.net/viewtopic.php?f=79&t=5211), Collection  

🔊 reverb change in water, caves, pipes-  
⚡ lightning weather: line, thunder sound, light or glow  
new dynamic Objects  
🏢 finish rest of 0AD han buildings, maur, others?  

----
### Graphics

🏞️ [Rivers/waterfalls](https://stuntrally.tuxfamily.org/mantis/view.php?id=7)  
✨ [Particles](https://stuntrally.tuxfamily.org/mantis/view.php?id=2)  
🌞 [Sun flare](https://stuntrally.tuxfamily.org/mantis/view.php?id=9)  

### Data

🛣️ **Road** textures, adapt, 2,3 variations, blending few at once?, traces alpha texture  
New static Objects  
Sceneries: PostApo, WIP: Anomaly, Spring  

🚗 Vehicles: [topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526), **collection** [link](https://sketchfab.com/cryham/collections/vehicles-todo-for-stunt-rally-327a2dd7593f47c7b97af6b806a60bb8)  

#### Meh
👥 [Splitscreen](https://stuntrally.tuxfamily.org/mantis/view.php?id=26): arrow, pacenotes, beam  
[Pace notes](https://stuntrally.tuxfamily.org/mantis/view.php?id=14) - game Info window for all signs  
[Road split](https://stuntrally.tuxfamily.org/mantis/view.php?id=4) checkpoints  
