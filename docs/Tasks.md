# Tasks Table

Legend:  
- Id - unique task number (from Mantis), new increments last  
- Pri - Priority: 1 (most) .. 30 (least important)  
- Size:  - how much work to do:  
  0 Tiny  1 Little  2 Small  3 Medium  4 Big  5 Large  6 Huge  7 Enormous  8 Gigantic  9 Insane  
- Component:  Game  Logic  Graphics  Shader  Editor  GUI  Simulation  Input  Sound  Data  
- Task name - short, same as section below  
- Updated - date last changed: yyyy-mm-dd  
- % Done - how much completed  
- Details - link to below, own section with more info, and possibly subtasks  

----
| Id |Pri |  Size      | Component|       Task name(s)          | Updated   |%Done|    Details               |
|----|----|------------|----------|-----------------------------|------------|----|--------------------------|
| 01 | 1  | 4 Big      | Sound    | 🔊Sounds: ambient, object   | 2024-12-28 | 55 | [link](#sounds)          |
| 02 | 2  | 4 Big      | Graphics | ✨**Particles**             | 2023-08-20 | 0  | [link](#particles)       |
|    |    |            |          |                             |            |    |                          |
| 20 | 3  | 5 Large    | Shader   | 🌊**Water waves**           | 2024-12-28 | 0  | [link](#water-waves)     |
| 15 | 4  | 5 Large    | Shader   | 🚗**Damage and dirt**       | 2015-08-21 | 0  | [link](#damage-and-dirt) |
| 33 | 3  | 6 Huge     | Data     | Data, vehicles, trees       | 2025-02-09 | 1  | [link](#data)            |
|    |    |            |          |                             |            |    |                          |
| 21 | 5  | 4 Big      | Game     | Gameplay Modes, Career      | 2022-07-07 | 0  | [link](#gameplay-modes)  |
| 07 | 6  | 4 Big      | Graphics | Rivers, waterfalls          | 2022-07-07 | 30 | [link](#rivers-waterfalls)|
| 06 | 7  | 4 Big      | Graphics | 🛣️Road sides, walls         | 2024-08-23 | 60 | [link](#road-sides-walls)|
|    |    |            |          |                             |            |    |                          |
| 26 | 8  | 3 Medium   | Graphics | 👥Splitscreen, per-viewport | 2023-10-01 | 0  | [link](#splitscreen-per-viewport)|
| 13 | 9  | 5 Large    | Logic    | 📡Multiplayer things        | 2015-08-21 | 0  | [link](#multiplayer-things)|
| 12 | 10 | 4 Big      | Game     | 🛢️Objects, dynamic          | 2015-08-21 | 0  | [link](#objects-dynamic) |
|    |    |            |          |                             |            |    |                          |
| 16 | 11 | 3 Medium   | GUI      | 🚗Garage, 3D previews       | 2022-07-07 | 0  | [link](#garage-3d-previews)|
| 25 | 12 | 4 Big      | Shader   | 🚗Car skins and vinyls      | 2015-08-21 | 0  | [link](#car-skins-and-vinyls)|
|    |    |            |          |                             |            |    |                          |
| 10 | 13 | 4 Big      | Graphics | 🎨Colormap terrain, grass🌿 | 2023-08-20 | 0  | [link](#colormap-for-terrain-grass)|
| 03 | 13 | 5 Large    | Editor   | 🎨Painting editor density   | 2022-07-07 | 0  | [link](#painting-in-editor)|
|    |    |            |          |                             |            |    |                          |
| 19 | 14 | 3 Medium   | Input    | Force feedback              | 2025-02-09 | 0  | [link](#force-feedback)  |
| 18 | 14 | 4 Big      | GUI      | Hud layouts, tracks Gui     | 2024-08-23 | 30 | [link](#hud-layouts-tracks-gui)|
|    |    |            |          |                             |            |    |                          |
| 29 | 15 | 7 Enormous | Logic    | 📦**Content manager**       | 2024-12-28 | 1  | [link](#content-manager) |
| 28 | 16 | 8 Gigantic | Game     | **AI opponents**            | 2024-12-28 | 0  | [link](#ai-opponents)    |
| 31 | 17 | 6 Huge     | Logic    | **Server** Lap stats        | 2025-02-09 | 0  | [link](#server-lap-stats)|
|    |    |            |          |                             |            |    |                          |
| 11 | 18 | 2 Small    | Graphics | Horizons                    | 2024-12-28 | 85 | [link](#horizons)        |
| 14 | 18 | 2 Small    | Logic    | Pacenotes, info             | 2023-08-20 | 60 | [link](#pacenotes)       |
| 04 | 19 | 3 Medium   | Logic    | 🛣️Road splitting, checks    | 2023-08-20 | 70 | [link](#road-splitting)  |
| 24 | 19 | 3 Medium   | Logic    | Lap stats, achievements     | 2015-08-21 | 0  | [link](#lap-stats-achievements)|
| 32 | 20 | 9 Insane   | Sim      | Simulation, RoR             | 2025-02-09 | 0  | [link](#simulation)      |


----

# Details

----

## Sounds

1. More tire-surface Sounds.

  The more the better (more realistic, and detailed), one should hear what wheels are doing.  
  Already gathered some from http://www.freesound.org/browse/ need to edit somewhat.  
  Ideally each surface should have a different sound (depending on slip,slide).  

2. Ambient background sounds on track

  like: rain, wind, shore waves, forest with birds etc.  
  For all that have rain, strong wind for D12,T5,W10, all forest,jungle,moss with some nature sounds.  
  All sounds are loaded at game start (somewhat wasted time, especially when developing).  

4. Sound in replay from multiplayer is broken

  Something is wrong when saving (car rpm, tires) from car sim or in frame,  
  causing random slide sounds, also tire trails have random alpha.  

5. Effects: [Done] Reverb.  
  Doppler fix. add Underwater lowpass filter.
  Dynamic change reverb parameters according to terrain,  
  eg. in cave, pipe, near terrain, open terrain, underwater.  


Ad 5. Moved already. Reverb works.  
Changing preset by scenery (param in scene.xml, set in ed).  

Try this seems more feature rich: https://github.com/OGRECave/ogre-audiovideo  

[Done] Make all (except electric) new engine sounds: https://github.com/stuntrally/stuntrally3/issues/7  


----

## Particles

1. Make more particle textures, for specific road surfaces:
- road: gravel, wet gravel, mud (jungle), savanna (other color), island sand  
  (should be different for each road material (each scenery)
- terrain: grass, leaves, rocks
  (depending on surface, terrain layer)

2. More particle types (more emitters, emitting conditions):
- const gravel, small dust, wet (emit always, depending on car vel)
- sliding side (slow, fade, more gravel, dust, snow, etc.)
- slip wheel spin (faster, flying mud, gravel, stones, etc.)  

3. Maybe also some impact only particles  
(like big hit, splash, tree wood, rock, concrete, car metal?, car glass),  
of course car damage would be cool with it.  


Problems:  
4. Can't vary the trail texture, can only be 1d texture stretched.  
  Also each trail uses 2 batches. So 1 car needs 8 for them (probably has to be so).  

5. Point 1. doesn't make much sense if we add more batches (4 with each new type, when they emit).  
  Unfortunately making 4 emitters in 1 particle system doesn't change it.  

-? Write own Particle System or rewrite Ogre's (Difficult) and/or  
+? Many particles in 1 texture (uv atlas) like so  
http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Random+Particle+Texture+%28HLSL%29&sort_mode=comment_desc  
But this will get complicated. Need to gather particles in 1 texture for each scenery and sort with size..  

6. Particles material add light (by diffuse and ambient).  
So they don't look the same on all tracks, and get darker when it's dark.  

I think that something simple, written by us (or me), meant just for car would be much better.  
5a. So 1 particle system with uv atlas (all particles in 1 texture)  
5b. and custom emitter in area near of car but not inside of it  
If the emitter will check size of particle (when emitting)  
and the distance from car (some planes) and also velocity/angle of car,  
we can get rid of particles inside of car.  
If it also checks terrain distance with particle size  
(or maybe even road from bullet) we wouldn't need soft particles (nearly no hard edges).

I think similar about trails, I don't like the 8 batches for each car  
(what if we someday wanted more than 4, batches would make it impossible).  
Idk but imo 2 would be ok, wouldnt have triangle strips though.  
Probably leave this, for Ogre 2.1.  


7. Particles on fluid surface, water/mud, some ripples  
(transparent diffuse and normal). Bigger and fading with time.  
Not sure how to fit colors, for all presets.  

8. Use upcoming ParticleFX2 from Ogre-Next 3.0 in SR3.  
https://github.com/OGRECave/ogre-next/issues/402  

Won't be sorted, so not for dust.  
But additive, alpha_tested (road-wheel stuff)? and weather should be better.  


----

## Gameplay Modes

@CryHam, Apr 2011  
  
1. We have *Race Against Time* currently (fastest lap counts), it will have ghost run at some point in future.  
  
2. We have somewhat discussed that we want also 2 player *Split Race* (with car collision and without).  
Could be even 3 or 4 way split if there were other input devices implemented (gamepad, wheel).  
  
3. *Collection mode*, no particular track road, just stars located in places that are not easy to get to.  
Could be against time to collect all for speed or when difficult just to collect all.  
There could be some points for making some stunts in air (loop, jump, rotation etc).  
  
4. If we had damage there could be a *Derby Arena*  
(with AI ? but there isn't and I can't imagine it in the near future).  
  

@neptunia Apr 2011  
  
I suggest to have a game mode with sharing track records on the server so you can see how do you perform among others.  
I think it is no so difficult to implement.  
  
Tracks with fixed time - either you win or you loose.  
  
Also a car damage mode with optional health packs in some places and maybe also a fixed number of "lives".  
This may also include damage not from collision by from acidic terrain, mines (visible but difficult to avoid), etc.  


@CryHam, Jul 2011  
  
Okay 1 and 2 are done. Input devices only keyboards, joysticks are implemented but have some range issues.  


@a.ismaiel Jan 2012  
  
there are different types of game modes that can be  
very useful in multiplayer like tag(virus) .forza have like 20 of those  
here is one you can check out:  
http://www.youtube.com/watch?v=oWYaGmmjUaw
  
what i would like is an infrastructure to allow plug-able game types that can control  
the scoring and receives events (when cars hit or checkpoint is reached).  


@MirceaKitsune, May 2013  
  
I'd like to see various game types!  
Currently it feels like there's no objective in StuntRally than simply driving around.  
There are many ideas that can be added to the list,  
and I think most would be easy to implement too.  
This is a list of the types I imagine can and should be done:  
  
- Classic race mode; Whoever crosses first during the last lap wins.  
If there are no opponents, this mode should reflect the current behavior  
(you roam around by yourself without any rules).  
  
- Elimination mode. The number of laps matches the number of players.  
Each lap, the last player is removed from the race, until two players are left during the last lap.  
Only possible on looping circuits (no sprint).  
  
- Checkpoint mode. Each checkpoint has a time limit, and you must get there before it runs out.  
The time left from previous checkpoints is added to the next checkpoint.  
Road length can probably be used to calculate a fair time automatically, so no map changes are needed.  
If there are multiple players, the   one who crosses the finish with the greatest remaining time wins.  
  
- Radar mode... or what recent Need For Speed games call Speedtrap.  
Each checkpoint registers your velocity at the moment when you reach it.  
The player with the highest registered speeds wins.  
  
- Drift mode. You get score the more you slide  
without leaving the road or hitting anything, highest score wins.  
  
- Since we have so many bumps and hills, we could also afford an air time mode.  
Players get score the longer their cars spend time in the air.  
After everyone crosses the finish line, the player with the longest total air time wins.  
  
- Considering performance damage and car destruction gets done ( issue 234 ),  
we could have a destruction mode.  
The objective would be to ram your opponent's cars until destroying them.  
The amount of damage you avoid taking represents your score,  
and after all functional cars cross the line the least damaged player wins.  
  
Then there's police chase mode... oh wait, that's for another chapter ;)  
But really, I think this set would be quite fun and appropriate for SR.  
Hope my suggestions are of help and something in this sense might get done.  


@tapiovierros May 2013  
  
Personally I don't much like e.g. pure drifting, but a general score based game mode sounds good,  
i.e. combining drift and air mode and spicing it up with  
e.g. additional score for hitting dynamic objects.  
Other car games also usually award score for breaching car's top speed and slipstreaming/chasing a car just ahead.  
  
Elimination doesn't sound much fun, it forces players early out into spectator mode which no one likes.  
  
Apr 2014  
Jamboree: editor setting for tracks. When enabled, checkpoints can be passed in any order,  
as long as all of them are passed, then pass the last checkpoint (car start).  
If reverse, the car start is rotated 180 degrees.  
Good for off-road tracks and tracks where orientation is the key to success.  
The checkpoint beam would probably show the closest checkpoint that is in the   field of view.  
  
@dimproject  
  
Carrer mode. (planed, on roadmap)  
http://forum.freegamedev.net/viewtopic.php?f=79&t=5211
  
  
----  
  
## Rivers, waterfalls  
  
Use road spline and its points, to place them in editor.  
Need a nice material, like our water, but with repeated flow on one direction.  
Rivers will change height (we need those even if there are some bugs in look).  
  
The hardest thing here is probably, that rivers need to deform terrain below  
(can't do it manually every time you move river point).  
And this should be other Hmap, that is added to the original.  
So river has spline points sticked to original terrain, but result will be deformed below it.  
  
And vertical setup of river points should create waterfals.  
  
@scrawl:  
Refraction is possible, but no screen reflection, only sky.  
  
Added in 2.7, done as Roads, with water materials.  
Actually we can just deform terrain manually for them.  
  
Now they don't do anything, so:  
Add physics - for particles and buoyancy with drag.  
  
  
----  
  
## Water waves  
  
1. Better underwater (distortion, rays, etc).  
Also while camera is underwater reduce sound volume (or even some low pass filter effect).  
  
3. Foam waves, near shore - just some animation *in material*.  
  Could use the spline road (no), (auto) placed by editor.  
  
  Or extend water shader and generate a map for shore waves and normals ?  
  Mesh could be easier but many more batches.  

4. Water **waves**.  
  Close (to camera) area made with a grid of vertices, using 2nd material which  
  reads heigh textures in vertex shader for waves animation.  
  Also normal computation to get proper vertex normals (or maybe just sum of normal textures).  

  **Waves** should be possible to do now in SR3 from Terra.  
  Ocean was made from Terrain component once.  
  Difficult, shader stuff. Need to have params in Gui.  

5. Foam texture, used randomly on water, or on higher parts.  
  
7. ?Caustics on terrain, just near/clear water.  


----  
  
## Road sides, walls  
  
New mode in editor where you could put another splines that would be:  
wall, fence, rail etc. maybe also cliffs or terrain slope if look good?  
Would be good for gravel too, some variation at least.  
Or very difficult: added cliffs in road geometry (like on rbr screen).  
  
If this works well, should be also used for simple objects, splines, metal parts on other tracks,  
which you could partly drive on, or just for decoration instead of objects.  
  
  
Same mode as Road.  
Many roads and Rivers possible now in 2.7  
  
New todo:  
- Allow only walls to be made, also for fences, platforms etc.  
- Add new wall Geometry types.  
- 
- Grid segmentation with size param, not road merge length.  
- Group all roads by materials, to reduce drawn batches.  
  
  
Walls now possible.  
Examples on TestC13-Rivers.  
Could use an even own type for fences (less tris).  
  
  
----  
  
## Damage and dirt  
  
@scrawl, Oct 14, 2011  

Visual damage, when hitting walls etc.  
Easy to change glass diff,normal map (to make it look broken).  
  
Other aspects, like loose parts of chassis, are more difficult,  
each car needs to be rigged and animated, so they are unrealistic for the future.  
  
Dirt on car would be possible too.  
Blending the diffuse texture with a dirt texture, based on a dynamic blend map.  
A bit dirty already at start.  
  
I'm thinking of a dynamic cube map (6 textures, eg. 512 or smaller) mapped to whole car just like a box.  
  
This cube map would be painted during drive. Use xyz in car space when hitting a wall etc.  
For dirt/dust/mud just in some places near wheels, over time, depending on car velocity.  
And more intensively while spinning in mud, or even cleaning it after, in water.  
  
The 1st, damage cube map would have the blend weight between fresh car / destroyed car textures.  
The 2nd cubemap would have blend factor for the dirt texture that would be added (alpha blended) onto the 1st.  
Later could be more factors like blend to dirt/dust/mud/snow, but only those which can be on current track.  
  
Finally if we have the 1st cubemap and painting it working,  
maybe its possible to use it in vertex shader to have mesh deformations.  
Or do we need to edit the manual object's verices on cpu for that, would be more difficult.  


----  
  
## Splitscreen, per-viewport  
  
- ?old- Ok it looks like the funny sky position moving  
was caused by something else (HUD viewport broke it, fixed now)  
But still, everything in preViewportUpdate (except HUD)  
seems to be 1 frame delayed, and is not correct for  
multiple viewports (i.e. multiple local players)  
  
- Create weather particle systems for each player, and update during drive.  
Need to show only 1 player's weather particles in each viewport.  
  
Too difficult to even touch and too Fps consuming:  
- Water reflection (is from 1st player)  
- Impostors (will probably be fixed with HW instancing in Ogre 2.1)  
http://forum.freegamedev.net/viewtopic.php?f=78&t=4633
  
- Grass deform  
  
- Arrow and pacenotes  
  
Arrow and Pacenotes are now hidden in splitscreen, replay play or when no road.  
Beam works in splitscreen, hidden in replay.  
  
also racing line (trail).  
  
In SR3 there are no Impostors and it is fixed. Also Water reflections work fine.  
But there is more stuff missing and seriously broken from HUD: minimap, gauges.  
Terrains and roads need updating per split view too.  
  
  
----  
  
## Multiplayer things  
  
  
!!1. Car Sounds seem to be CT or very high pitch (rpm,gear?).  
!2. Tire Sounds are garbage, in replay.  
  
!3. Disable cars collision when rewinding (multiplayer, splitscreen)  
just with the rewinding ones  
need to send bool rewinding over network for car  
  chassis->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);  
or chassis->getBroadphaseProxy()->m_collisionFilterMask = 0; //setCollisionFilterMask();  
  
4. Spectate - switch between other players camera during drive (K,L) or after end.  
  
5. Quick chat key: press a key (T?) to chat in multiplayer at any time,  
disable backspace and enter in this mode.  
  
?6. Confirm Port Forwarding success or failure in chat.  
  
--- Not likely ---  
?7. Show Nick Changes in chat, to avoid impersonation.  
  
-8. Damage view of other players near their name tags, using a bar or percentage.  
  
-9. Multiplayer lobby, IRC or sth else with global chat for all inside game.  
https://github.com/MegaGlest/megaglest-source/blob/develop/source/glest_game/menu/menu_state_masterserver.cpp

new  
10. Restore **master (game) server** for multiplayer games lists,  
[forum topic](https://groups.f-hub.org/d/2Hu9znrb/new-masterserver-for-multiplayer-)  
*Possible* on a-lec.org if/once we get there (only CC data required 1st).  
  
  
----  
  
## Objects, dynamic  
  
Would be good if they had *buoyancy*.  
Need a simple box shape for it and use that code which cardynamics uses.  
  
Maybe even own *sparks*, e.g. barrel hitting wall or other barrel.  
  
Could have some more color variations (*materials*).  
Best if would reuse mesh and textures.  
  
  
----  
  
## Garage, 3D previews  
  
3D preview of vehicles in Gui (not image).  
Allow rotate with mouse.  
Preview chosen car color.  
  
And/or a garage model (like in VDrift) when the menu is open.  
Car in center and mouse controlls own camera (rotate, zoom).  
But Gui might require adapting.   
  
  
----  
  
## Car skins and vinyls  
  
@MirceaKitsune, May 2013  
  
Adding a system for skins/vinyls. All graphics located in a common folder.  
The player has a car customization screen to **edit**:  
pick an image, drag it to any spot on their car, rotate it and scale it.  
I also suggest allowing skins to be colorized. Some images might contain their own colors though.   
Here's a video showing this method: https://www.youtube.com/watch?v=rKNxAoBRjk0  
  
  
----  
  
## Colormap for terrain, grass  
  
2 new RTTs that will generate:  
  
1. colormap for Terrain  
Should make it look more interesting. Not sure how many noise layers or how many colors.  
And if we add the colors, or probably better use noise layers inside HSV.  
Needs a new subtab in Terrain, Colormap after Blendmap and Noise.  
  
2. colormap for Grass  
We use some predefined images now, with rtt it would allow noise patterns  
and other colors for whole grass (not channels). Probably not too fancy.  
But having a RTT for it one could precisely adjust grass color  
for any scene, and images for that wouldn't be needed.  
  
3. also use noise for grass channels, same from blendmap RTT   
  
  
----  
  
## Painting in editor  
  
A mode in editor where you can paint textures (on terrain).  
Those textures could then be used for:  
- vegetation density (global, eg. make oasis with trees on desert)  
- vegetation layers density (for invidual models, custom placing)  
- terrain layers density (custom blendmap painting)  
- grass density (add or deny grass in places)  
- grass layers density (variations)  
- terrain color (RGB)  
Density is 1 byte, RGB is 3.  
  
This should be universal as much as possible, so you could attach  
any painted texture to any of the density slots.  
  
  
----  
  
## Force feedback  
  
Force feedback on master is gone. It did sometimes work, but only on Linux,  
and it was never adjusted to make some sense:  
(from tires - the 3rd graph - Mz, aligning torque).  

Needs code for SDL2 haptic support and update from sim.  
I don't have a wheel, so unable to test.  
Mentioned here: forum.freegamedev.net/viewtopic.php?f=81&t=6447  

Someone with FF steering wheel - needed for testing.  
Add GUI option for strength, minimum/deadzone,  
autocenter?, low speed align?, etc.  
Like in Speed Dreams, during game menu.  

  
----  
  
## Hud layouts, tracks Gui  
  
Hud layouts. Current will be 1st. Also possible:  
  1. the old layout (rpm left), minimap top right  
  2. both gauges in center, bottom  
  3. only rpm gauge and speed as text, etc.  
  4. other needles, needle color, gauges that fill up solid  
Best already make a hud.xml, so its editable.  
  
Move rest of overlay to Gui (not needed).  
From CarDbg.overlay and a lot in Editor.overlay.  
  
Tracks list, user.ini, was started.  
  1. Smaller previews, for faster load, so that the big view,road,terrain load later.  
    But mainly would be useful for new view 2c. and showing champ/chall small track icons for stages.  
  2. New Tracks views  
   2a. A new view with stats in columns (from user.ini):  
     user rating, score, last driven on (date), driven laps, score set on (date) etc.  
   2b. Make track stats (on right) collapsible: 2 tabs, 2nd would hide minimap and show stats from 2a.  
   2c. View with thumbnail images (like in image browsers).  
     Grid with small images with text name below (needs 1. done).  
  3? Loading previews on thread, so non blocking up/dn keys.  
  
Moved all to Gui, overlay not used in SR3.  
Tracks  
2. Done user rating and bookmark in new 3rd Tracks view.  
Rest not very needed.  
  
New: Hud gauges drawn in pixels shaders code.  
Could be very fancy easier, no textures needed.  
  
Done 1 and 2c.  
New Gallery Tracks view, with smaller images and track name.  
  
  
----  
  
## Content manager  
  
@CryHam, Jun 2013  
  
A lot of code to have resources downloadable.  
Just like in: VDrift (using svn), RoR, and last Speed Dreams.  
  
Benefits:  
- Since we have users contributing tracks (rarely), it would be cooler if all users  
could download new tracks in game (not search forum and extract to user dir)  
or wait for next release.  
  
Note: some (many?) championships/challenges will become  
unavailable because of *missing tracks*.  
Also needs checking if track/vehicle exists before starting multiplayer.  
  
- We could make a **smaller package** eg. a demo, or with only best tracks, or no IRL vehicles  
  
- Could also do it for vehicles.  
Needs implementing of vehicles reading from user data.  
  
But since this is a gigantic issue (lots of coding, testing)  
I'm not seeing this done. Also needs versioning, hash checking, etc.  
  
Probably we could use git+github for it, tracks are aready there, would be best to use that.  
Maybe raw:// could work with http here ?  
OFC only for downloading.  
  
RoR has it already implemented, check if we could adapt and use it too  
https://github.com/RigsOfRods/rigs-of-rods/pull/2850
  
  
----  
  
## AI opponents  
  
@MirceaKitsune, May 2013  
  
As fun as it is to drive around the world, it gets boring without someone to race against.  
There are currently no full uptime servers and a constant number of players online,  
while some like myself prefer playing offline most of the time.  
  
It would be really awesome if bots could be added, consisting of AI racers  
the player can configure (name and customize their car).
This could be an addition to the existing player slots, where instead of players 2 / 3 / 4  
being local (split-screen) or network, each can be set to an AI of a certain difficulty.  
  
I personally don't mind if the AI is weak or can't navigate tracks with complex jumps or pipes,  
as long as there are cars that follow along on basic tracks.  
  
  
@CryHam, May 2013  
  
I will not start this issue. It is just for me impossible to write any AI  
that could drive all the tracks - and drive it well  
(so that I could drive with it and not just laugh at AI mistakes).  
  
But of course some tracks are easier and that perfect AI isn't needed.  
Still it is a gigantic issue.  
We have 116 tracks, so drawing a line, or adjusting anything by hand is wasting time.  
  
I would like an implemetation of a learning approach.  
So: I start a track in game and start an AI learning mode.  
This simulates driving for AI (as fast as CPU can simulate) and  
produces some data that AI will use later  
(e.g. an ideal drive spline with car velocities and places where to brake etc.)  
And this goes several iterations, so that the lap time gets minimized.  
  
Lastly need to keep in mind that you could collide with AI,  
and also AI will get stuck. So it needs to probably rewind sometimes  
(since trying to get on good way inside pipes is sometimes difficult even for people, rewind is a lot easier).  
Or just reset and place on road.  
  
I can help somebody but definitely won't start this issue.  
  
  
@MirceaKitsune, May 2013  
  
Yeah, some tracks would be impossible for an AI to ever navigate.  
Namely those that have loops and coiled pipes (bots should be disabled on them IMO).  
Maps that have simple roads and bridges though should work nicely from my understanding.  
  
I agree that having to edit tracks for AI support would be a bad approach.  
The way I envisioned this, roads would be automatically detected and bots would   
simply follow the path by noticing turns ahead.  
The learning approach would be an interesting idea too and  
could yield more realism, but I'm not sure if going that far is needed.  
  
As for the AI getting stuck, the ideal solution would be knowing to reverse the car  
and find their way back to the road.  
That might be trickier than teaching them to simply follow the road however.  
An easy method for starters would be teleporting to the last checkpoint (IIRC F12 does that for players).  
They lose more time but better than nothing till they get smarter.  
  

@CryHam, Jul 2013  
  
Since the track's ghosts are now present, AI is even less needed.  
  
But if somebody wanted to implement it, then the track's ghost info could be  
used to guide the AI's car (position,rotation,steer,braking is there, more could added if needed).  
  
  
----  
  
## Server Lap stats

Just like in [Speed Dreams](https://speed-dreams.net/masterserver/user/cryham)  
or in old [RoR]()

Needs server side code (php?) and DB to hold user track records.

*Possible only* on a-lec.org, if/once we get there (only CC data required 1st).


----  
  
## Lap stats, achievements  
  
Dec 2012  
  
Statistics shown after each lap, e.g.:  
- top speed  
- average speed  
- % pressing gas pedal  
- % breaking  
- longest jump m  
- total seconds on air  
- % sliding  
- etc.  
  
Show them for e.g. 20s after a lap in a list similar to opponent list  
(make gui option to disable)  
Hitting enter/esc to hide.  
  
Button on track tab to show for best lap from each track.  
(could use ghost.rpl to get that data ? or save in .txt file)  
  
May 2013  
  
Not needed (stupid) game achievements  
E.g.:  
- total in air for (1min, 10,..)
- been upside down for (1min, 10,..)
- used boost for (1min, 10,..)
- 
- driven 10,100,1000 km
- driven (1st, half, all) tutorials, championships
- driven 10,50,100,(all from each scenery?),all? tracks
- 
- slided for 2,3,5 seconds
- experienced accel of 1,2,3 G's
- had 10,100 crashes/tree hits
  
Need a cool effect (at end or during race?) that shows the image and some particles with text of what you achieved.  
Some gui page with all of them and cool icon images too.  
  
I guess some are easy to implement but don't sound too fancy. Others would be cool but are difficult to implement.  
Would be good to have a .cfg file where you could add/edit those.  
And the range should be high, starting from simple stuff that will happen for new players,  
and endnig in achievements for me (or very commited players) like drove all champs etc.  
  
  
----  
  
## Horizons  
  
Maybe some tracks don't need it (eg. dense jungle), but to make tracks not to "end so suddenly" with terrain.  

Done 3.  
No discard (yet). Patching is manually edited so both terrains cross.  
Todo: tool in editor to **auto fit** both on border.  
Editing uses same tools, just changing which terrain to edit.  
Brush size is same but bigger as it depends on terrain triangle size.  
  
4. Also more terrains possible in SR3, now need *fixing*:  
move terrain and editing *cursor position*.  
  
  
----  
  
## Road splitting  
  
@MirceaKitsune, May 2013  
  
It would be possible to make some interesting tracks if roads could be separated and combined.  
Players could be allowed to choose different paths each lap, or given less obvious **shortcuts** to discover.  
Tracks could also contain complex road hubs through which a player must find their own way.  
From my understanding, there would be three different parts to such a change:  
  
- **[Done]** Allowing roads themselves to be split and merged in the editor.  
I didn't test it recently, but last I heard roads couldn't be divided or combined.  
  
- Allowing multiple **checkpoints** to represent one location (act as one).  
So when splitting a road you can add checkpoints to each **fork**, and reaching either will let you advance further.  
  
- optional: *derivative* roads could allow for *multiple start and end locations*.  
If a track has multiple starting points, a player will randomly spawn at one of them.  
If it has multiple ending points, any player can get to either in order to finish (aiming to find the closest one).  
But this would only be possible for sprint tracks since loops require a single finish line.  

@cryham, 2015-08-20
  
1. Could be done with making many roads possible, so the split would be just a 2nd road.  
This way you would create a small road part just for the 2nd way of split, main road stayed on 1st.  
  
2. Is probably most difficult part of this.  
Implement split for checkpoint (i.e. the next checkpoint can be 1 of 2 after split,  
then next are normal and after split 1 checkpoint is common for both ways.  
Your idea I think won't work, i.e. allowing any checks from both ways  
(which should arrow point to, and what if there are more in 1 of split ways).  
  
3. Sounds cool for a city track or a road system. Or some fancy track with many roads too.  
  
@tapiovierros  
  
I find **hidden shortcuts** fun, so +1.  
However, I don't think we necessarily need split checkpoints -  
shortcuts tend to be short by definition and the roads   
need to go into the same general direction anyway, so just place the checkpoints at the split points  
(or maybe in some cases add a big checkpoint that covers both roads).  

@cryham, 2022-07-07

Done  
Just by adding more roads, not looped.  
  
4. Make *alpha* transition, not hard edge.  
  
  
----  
  
## Pacenotes  
  
1a. Maybe a stack with next 1 (or 2) shown (instead of arrow).  
1b. meh- voice announcer talk, when added (like IRL WRC copilot).  
  
2. ?meh, manual added:  
Obstacle,  Narrow,  Road Split,  
Slow, Stop, Danger,  
Ice, Mud, Water, etc.  

3. **GUI** info window with all signs, add some text info  


----
## Simulation

Simulation tasks.  

1. In VDrift wheels cast just 1 ray down.  
Should be possible to cast more at angle interval (e.g. 10 deg).  
For big outside wheels, probably all around?  
Also more times at tire width interval (e.g. left, center, right side).  

Tried in code, fails, maybe normals?  
Added in CARDYNAMICS::UpdateWheelContacts()  
// todo .. more wheel rays  

Example, debug shows rays in https://github.com/WimbleSoft/VehiclePhysics

Probably still bad as it is 1 contact only.

2. *(bad idea)* Try add RoR sim to SR,  
at least for big wheeled vehicles and suspension.  
(imgui, scripts, later too?)  
RoR problems: tire inflation, bad params: pressure for wheels,  
multiplayer, no tire sim (Pacejka's Magic Formula),  
new collisions for all: veget, road, obj mesh etc.  
deformations, no normals? update mesh,  
vehicle editor?  


----

## Data

- Top priority:  ♻️ Removing, replacing data with non-CC licenses  
https://groups.f-hub.org/d/pIoLYCaO/-removing-replacing-data-with-non-cc-licenses-to-do

- Medium priority:  
🚗Cars, new vehicles  
collection of models to do:  https://skfb.ly/oDrzD  
https://groups.f-hub.org/d/adePgxzW/-cars-new-vehicles-done-and-to-do

- High priority:  
🏞️Creating tracks and their content  
https://groups.f-hub.org/d/lpntzPrD/-creating-tracks-and-their-content

- Low priority:  
🌳Trees to do  
https://groups.f-hub.org/d/4n0bTwmC/-trees-to-do

- Lowest priority:  
🎵Music  
I don't see this happening. Still, was already few times brought up.  
There is not much quality, fitting music that I'd like in SR.  
Has to be 5 min or more, CC-BY-SA (like all data).  
Style could be: rock, metal, trance, chiptune.  
Something that I wouldn't mind hearing, and would not get bored or annoyed by it.  
For reference, my favorite music in playlists: https://cryham.org/entertainment/playlists/
