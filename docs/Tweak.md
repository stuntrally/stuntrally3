_Various tools and visualizations to tweak game._


## Tools list
This Wiki describes most tools available in game (found on Tweak tab).   
How to use them and what are they for.   
Mainly they are helpful in visualizing the result of edited .car sections.

They are listed here for quick guide:

| Shortcut | Tool name          | Description |
|----------|--------------------|-------------|
| F11      | Fps bar            | Cycle Fps bar modes (left top). More info for values on Gui. First is always _Frames per second_ |
| F10      | Wireframe          | Changes rendering to wireframe lines. To see geometry detail and LOD swtiching   |
| Alt-F    | Material Editor    | Allows editing PBS material properties directly with sliders. Can save as json  |
|          |                    |             |
| Ctrl-F10 | Bullet Debug Lines | show collision shapes as lines |
| F9       | Graphs             | many types, show detailed change of some values, life over time period |
| Shift-F9 | Car debug Text     | shows text values from simulation |
| Alt-Z    | Car file Editor    | allows editing .car file in game, Alt-Shift-Z will save changes and restart  |
| Ctrl-F5  | Perf Test          | Starts car performance test, results show eg. 0-100 kmh time etc. |

   
#### Common
In game and editor.

### Fps bar
Cycle Fps bar mode with F11.  

This small left top bar shows values (that change, especially when moving), in default mode:
  - Rendered Frames per second (Fps), it's best to play with at least 30, 60 being optimal for most monitors.
  - Triangle count, e.g. 1.20 means there are 1 200 000 triangles drawn each frame (all render targets).
  - Batches count, e.g. 254 means there were 254 draw calls to render geometry on GPU.
  - GPU Memory use. e.g. 521M means 521 MB are occupied (by textures, geometry etc).

This is better explained on Gui for each mode.

### Wireframe
Toggle 🌐 wireframe mode with F10.  
It is useful to check how dense are triangles in car/wheel model (or also for whole track).

   
#### Game tools

   
### Bullet Debug Lines
This is useful to check and adjust the shape of car body that collides.  
To use bullet lines first check the global (Startup) option, and restart game.

{{http://stuntrally.tuxfamily.org/uploads/images/wiki/1.jpg}}

   
If game was started with it enabled, you can toggle bullet debug lines with Ctrl-F10 or the next checkbox.

{{http://stuntrally.tuxfamily.org/uploads/images/wiki/2.jpg}}

If you just need to edit car collision, don't use official tracks.   
_They have a lot of vegetation and it will be horribly slow to draw all lines (and this stays until you quit and restart game)._   
Use test tracks which are usually quite empty (and also reload fast).

   
### Developer keys

If you mark the checkbox 'Developer keys..', also shown on previous screen (Tweak tab), you can:

1. Use Alt-Shift-digit/letter to quickly start a test track, without using menu.  
So e.g. Alt-Shift-A will load Test1-Flat, Alt-Shift-F loads Test3-Bumps, etc.  
This can be set in in `game.cfg` under `tweak_tracks` for each digit/letter key.

2. Press Ctrl-F at any time to show Gui and focus cursor in track search edit.


   
### Tree collisions

As shown on previous screen, you can see yellow capsule shapes for palms.   
We use such simple shapes for trees. No need for full trimesh and should be faster.

Editing those tree collisions, is done in game, file collisions.xml.

Open Tweak window and switch to collisions tab, to edit that file.   
Shift-Alt-Z will save your modifications, and restart game.   
See the top of this file for more info (e.g. how to disable collision or use full trimesh for model).


   
### Car file Editor
Editing car settings (.car file) is done in game.
```
Alt-Z - toggles car editor in game.
Alt-Shift-Z - saves changes and restarts game.
```
Use tracks Test1-Flat or Test2-Asphalt etc, to reduce reloading time and concentrate on editing car.

_You can use your Text Editor (e.g. to have syntax coloring) and press F5 to reload game after saving .car file._

If the .car file for current car wasn't modified, Editor will show cyan "Original" text and file path.   
After saving changes, yellow "User" path will appear.

Edited .car file is located in user dir path (see [Paths](Paths.md)) in `data/carsim/mode/cars/` subdir,  
where mode is current simulation mode (easy or normal).

   
### Graphs
Graphs in game can be used to test more advanced car behaviour and simulation.

Press F9 to show/hide, F2,F3 will cycle through various graph types. Or use Gui to pick from combo.

E.g. Car engine torque curve can be seen, all gear ratios for car speed.   
Those are explained more in [VehicleEditing](VehicleEditing.md).


   
### Car debug Text
This was the first simulation visualisation and it is still useful if you want to check the values (if they don't change too fast).

Use this checkbox to toggle it or Shift-F9.

On Gui you can also change how many text sections are displayed and change text color to black or white.

{{http://stuntrally.tuxfamily.org/uploads/images/wiki/3.jpg}}

   
### Performance test

Car performance test is automatic. It's used to get car performance info, which is shown on Gui, tab Car.

Also useful to check this when editing car (especially engine torque).

Press Ctrl-F5 to run test for current car (it will load track ''Test10-FlatPerf'').

The car will accelerate to top speed and then brake. Simulation is speed up for this (to not wait long for results, factor in game.cfg section sim, perf_speed, use 1 for normal).

After it finishes it will show up results, and save them in user dir. This is in user path ''/data/cars/ES/ES_stats.xml''

Perf test stats contain info like max engine torque and power, top speed, acceleration times to 60, 100, 160, 200 km/h (with downforce and drag at those speeds) and brake time from 100, 60 to 0 km/h.

Most of those stats are then shown in info panel on Gui when when picking car in game.

_Run it for each sim mode (provided you made it different as game requires). See bottom of page for differences list._

{{http://stuntrally.tuxfamily.org/uploads/images/wiki/4.jpg}}


   
## Game (advanced)

   
### Surfaces

{{http://stuntrally.tuxfamily.org/uploads/images/wiki/12.jpg}}

   
### Tires
{{http://stuntrally.tuxfamily.org/uploads/images/wiki/10.jpg}}

{{http://stuntrally.tuxfamily.org/uploads/images/wiki/11.jpg}}

Tire editing is meant only for people with good knowledge of simulation, who know Pacejka Magic Formula.

It can be very time consuming/wasting, and produce no better results.

Other and simpler parameters are in surfaces.cfg.