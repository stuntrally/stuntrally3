_List of things **to do**, planned for next releases._  

This is the latest, fast updated list.  
More in [Tasks tracker](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php?page_number=1) with older tasks too and some more detail.

Git [sr3 repo](https://github.com/stuntrally/stuntrally3) and [tracks3](https://github.com/stuntrally/tracks3), older simple [terrain demo](https://github.com/cryham/ogre3ter-demo)  

Marked: <u>critical</u>, `crucial`, **Next or Big**, _Important_.

### 3.0

New tracks add to `champs` 🏆  
SR 3.0 📽️ `editor Videos`, all new  

### 3.1 cd restore

- 🌊 Water tweak par, **refract, depth** for color, border alpha  
- 🌪️ **Wind**, for trees  
  Grass **deform** spheres, wind params, far fade-  
- Lights on `grass`, sun lit particles-  
- ⛰️ Terrain  
  **shadowmap** fix < 0, save/load-  
  add emissive, reflect par, fix triplanar normalmaps-  
  blendmap also .hlsl or .any for DX  
  read blendmap, for ter wheel **surface** ids, many ters  
  pages visibility fix restore  
- 🛣️ Road  
  **Grid** for walls, road, pipes, ed update cell  
  1 mesh with 4 LODs, shader fix specular stretch  

- 🏗️ Editor, fix  
  objects pick `errors`  
  minimap **RTTs** upd skip (slow), or create on save  
  new terrain issues [here](https://groups.f-hub.org/d/SW0mnXNV/track-rework-horizons-skies-and-updates/14)
  edit moved terrains, error in ray pos-  
  _tools_: fix **scale ter h**, copy horizon, which ter, road  

- ⏲️ `HUD`  
  fix splitscreen gauges etc  
  _Minimap_ circle, terrain add, fix pos tris  
  btnRplDelete for ghost should also remove its track record from .txt  

- Rest  
  **Linux** release, appimage or flatpak
  MyGui CMake .lib, _Conan_  
  .mesh LODs test adjust-  
  water vis dist?  
  tire **trails clr**, no uv tex-  
  Fog, 2 colors cam dir fix-  
  gpu mem leaks or renders inactive  

----
### 3.2 new, data

🔊 **Ambient** [Sounds](https://stuntrally.tuxfamily.org/mantis/view.php?id=1): rain, wind, forest etc  
🚗🔊 More car **Sounds**.. todo [task](https://stuntrally.tuxfamily.org/mantis/view.php?id=1), use [engine-sim](https://github.com/stuntrally/stuntrally3/issues/7)  
📦🔊 object hit sounds, reverb change in water, caves, pipes-  

[Horizons](https://stuntrally.tuxfamily.org/mantis/view.php?id=11), add to rest of tracks  
🪄 **Effects**: Soft Particles, SSAO, HDR, bloom, rays?  
🖥️ Gui render system, all **options**?  
🌀 Add [FastNoise](https://github.com/Auburn/FastNoiseLite) for brushes and ter gen  

🌳 new trees, Bushes, more vegetation, tiny close, grasses, rock packs  
redo or replace lowest quality meshes: shroom* 🍄, skyscraper* 🏢  
normalize .mesh scales fir, etc.  

⛅🌳 Remove all non **CC** data, New skies: [new topic](https://groups.f-hub.org/d/pIoLYCaO/-removing-replacing-data-with-non-cc-lincenses-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532&sid=b1e7ee6c60f01d5f2fd7ec5d0b4ad800)  
🛣️ Road: wall types, spline walls [link](https://stuntrally.tuxfamily.org/mantis/view.php?id=6)  

----
### 3.2 restore

auto road/ter **Surface**: Particles, trail, sounds, from `presets.xml` not per track  
Road noise blend, color per vertex  
grass density as **RTT**, noise, color  
inside height fog?, noise 3d? _volumetric_ clouds  

ed _tools_: copy roads which combo,all  
Grass read Hmap - no index, vertex buffers? texture atlas-  
slow track load, shaders, ?ter get h, norm, do parallel  
🖱️ Input mouse bind, 👀 VR mode meh-  

----
### 3.3 new game stuff✨

_Basically all [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) with priority 1 to 12._

🕹️ New **Game** elements:  
✨Teleporters, moving & rotating Obstacles, elevators  
Force fields, speed up, lift, anti-gravity  
Track 💡lights, `entities.xml` (sum of object, animation, light, 🔥particles etc)

🏆✨ new [game Modes](https://stuntrally.tuxfamily.org/mantis/view.php?id=21): [Career](https://forum.freegamedev.net/viewtopic.php?f=79&t=5211), Collection, 3d cup models  
👥 [Splitscreen](https://stuntrally.tuxfamily.org/mantis/view.php?id=26) issues  

🌊 Ocean: Terra waves, vehicle deform, particles  
⏲️ Gauges with shader [demo](https://www.shadertoy.com/view/7t3fzs)  

----
#### Graphics

🏞️ [Rivers/waterfalls](https://stuntrally.tuxfamily.org/mantis/view.php?id=7)  
✨ [Particles](https://stuntrally.tuxfamily.org/mantis/view.php?id=2)  
🌞 [Sun flare](https://stuntrally.tuxfamily.org/mantis/view.php?id=9), sun rays..  
⚡ lightning weather: line, thunder sound, light or glow  

#### Data

🛣️ **Road** textures, redo, 2,3 variations, blending few at once?, traces alpha texture  
🏢 finish rest of 0AD han buildings, maur, others?  
📦 New static Objects, dynamic hay  
Sceneries: PostApo, Spring trees  

🚗 New Vehicles: [new topic](https://groups.f-hub.org/d/adePgxzW/-cars-new-vehicles-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526), models **collection** [link](https://sketchfab.com/cryham/collections/vehicles-todo-for-stunt-rally-327a2dd7593f47c7b97af6b806a60bb8)  

#### Meh
ed roads: [split](https://stuntrally.tuxfamily.org/mantis/view.php?id=4), checkpoints, merge?  
sim new poses _interpolation_?  
[Pace notes](https://stuntrally.tuxfamily.org/mantis/view.php?id=14) - game Info window for all signs  

----

### 2.x old

new deps with latest Ogre fix for terrain visibility, resulting in editor vegetation issue  
meh, fix shadow check [here](https://github.com/OGRECave/ogre/blob/master/Media/RTShaderLib/GLSL/SGXLib_IntegratedPSSM.glsl#L86), screens [here](https://forums.ogre3d.org/viewtopic.php?t=96888&sid=bf9864a27e57f24f55950ccb5d273dea)  

