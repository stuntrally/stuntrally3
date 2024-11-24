### What's coming next

This section lists changes not yet released, but committed to repositories stuntrally3 and tracks3.  
WIP means work in progress, unfinished, still being developed.

- 3 [New tracks](https://groups.f-hub.org/d/ZXVHk6du/new-tracks-in-sr-3-3)
- Compositor rewrite, created in code
  - **SSAO** (ambient occlusion), needs Refractions
  - SplitScreen uses RTTs, smaller Hud fonts
    - Refractions work, SSAO too
  - Apply buttons work for Shadows and Reflections
  - WIP try, *not useful*, no effect on terrain [issue](https://github.com/OGRECave/ogre-next/issues/475)  
    Global Illumination ([GI methods](https://ogrecave.github.io/ogre-next/api/latest/_gi_methods.html): IFD+VCT),
    on new Gui tab Advanced after Effects, has also debug visuals.
- Game
  - Setup - Paints tab has filter, to show paints only from filter rating and above.  
    But editing paints list must be done w/o filter, on highest value.
    Pressing Random (or Space) sets random from visible list.
  - *Statistics* window with list of completed game tutorials, championships, challenges, collections
- Gui comboboxes open on click (whole, not just on down arrow)

### Version 3.2 - 23.09.2024

- Game
  - fixed Multiplayer on Windows, bug was present since SR 3.0
  - **Collection** - new game mode, in menu
    - Collect all gems located on track, for completion or quicker time
    - Hud shows 3D arrows on vehicle, pointing to closest 3 gems
  - More **Lights** - with toggable Options
    - for vehicle: brakes, boost, reverse, *under glow* and for collection gems
- Both
  - New **Vegetation** - over half of tracks renewed
    - **New Bushes**, separate multiplier and distance options
    - **New** models: Palms, Jungle trees, mushrooms
    - Vegetation on Horizon, new tab in options
    - Limit (total count) option
    - Tree wind animation, WIP, not yet varied by model
    - grass and bushes deform under vehicle
  - **Water Refraction***, depth color, soft border, WIP  
    *does not work in Split Screen - will disable, and won't restore  
    *antialiasing / FSAA does not work with refraction  
  - New **horizons** on more tracks, almost all have it
- Gui
  - New tab on Setup - Paints - Adjust - Under glow for color H,S,V
    - under glow color is auto set by picking paint
  - Main menu 3rd level for Game Types. Separate for Splitscreen and Multiplayer
  - 3D win **cups** shown on game end
  - Gui shows small track previews for game track / series
  - Gui Tracks view, new mode: **gallery**
  - Added Changes tab in game Help with this Changelog
- Editor
  - Road: wall type, for decoration walls, rectangular
    - pipe parameter changes it from wide at 0, to square at 0.5 to tall at 1.0
  - New tab Sun - **Wind**
    - unified params, affecting: trees, grass, water, weather particles
  - Vegetation materials, possible in presets
  - Edit mode for Collection, key Y
    - Insert/remove gems, move, scale
    - change type (for look, at end of presets.xml)
    - change group (for *more sets* on 1 track)
  - Edit mode for **Fields**
    - key H toggles, examples on TestC14-Fields
    - Field is a box area, which will act as force fields on vehicle  types:  
      acceleration, anti-gravity, damping - 
    - have factor value, acceleration has also direction
    - *teleporter* - instantly places vehicle in destination, Enter - toggle edit
  - Particles have direction yaw, pitch angles
- Other
  - split all.material.json to many files
  - in data dir, added subdirs: models, textures etc

### Version 3.1 - 25.04.2024

- Game
  - 8 New vehicles (Sci-Fi)
    - 3 new types: **hover car, hovercraft, drone**
    - 3 hovering cars: [R1](../data/cars/_previews/R1.jpg), [R2](../data/cars/_previews/R2.jpg), [R3](../data/cars/_previews/R3.jpg). [R1 video](https://www.youtube.com/watch?v=9pE8JPV2M8w)
    - 2 antigravity drones: [Q1](../data/cars/_previews/Q1.jpg) slower, [Q3](../data/cars/_previews/Q3.jpg) incredibly fast and agile
    - 2 racing hovercrafts with propeller: [H1](../data/cars/_previews/H1.jpg), [H2](../data/cars/_previews/H2.jpg)
    - fast car [YG](../data/cars/_previews/YG.jpg)
  - New speed sensitive steering tab and settings for Hover type vehicles on tab Setup - Steering
    - Steering range multiplier(s) for Surface affect graph and whole group
- Tracks
  - 1 New: Wnt19-Downhill
  - **New skies**. Replaced all old 4k with new 8k ones on all tracks
  - Many updated with new **horizons**, few with objects etc
- Editor
  - WIP New video tutorials, on [Editor page](Editor.md), in [playlist](https://www.youtube.com/playlist?list=PLU7SCFz6w40PY3L8qFNTe4DRq_W6ofs1d)
  - Changing object material, sets defined in `presets.xml`
  - Toggle static for dynamic object, no `*_static.mesh`
  - Fix alt-J crash, shortcuts
  - Fix **New/duplicate** track. Was having garbage for horizons, no other roads. Delete track too
  - **Export** track to Rigs of Rods added, using SR 2.x meshes
    - New tabs, settings, converters, [info here](EditorExportRoR.md)
- Both
  - New tab Options - Screen - GUI with scales for font size and windows margin size, all need restart
  - Screen and Video tabs with render system options, WIP few not done  
    - Vsync works immediately, but Resolution, Fullscreen, Antialiasing need restart  
  - Added Japanese font and quick, basic translation (could be wrong)  
  - Linux binary release

### Version 3.0 beta - 3.12.2023

- Stunt Rally 3 is continuing last SR 2.x.  
- Graphics - moved to Ogre-Next 3.0
  - A **lot smoother and higher Fps**, also in split screen
    - No vegetation lags. Auto HW instancing. Good Fps with many objects/buildings, and road points in editor
  - Vegetation models - no impostors, now using meshes with LODs
    - Graphics Detail tab has new setting - for far Vegetation/Objects geometry reduction, aka LOD bias
  - **Horizons** - far terrains on many tracks, great landscape views
  - PBS materials - with roughness and fresnel/metalness. WIP, can be improved
    - New vehicle materials - with more parameters and new look
    - Color changing **paints**, 3 colors (on Gui) at once depending on view angle
    - New paints - Big set, over **450** total
    - [Materials](Materials.md) in new .json format with more posibilities e.g. detail maps
    - New syntax and new (more complicated) **HLMS** shaders, comparison and details in [Developing](Developing.md)
  - Better glass pipes - sorted, no random blinking
  - Vehicle front **lights** - new tab Lights in Graphics options, toggle key, WIP grass is not lit
- Game
  - Up to 6 players in Splitscreen or Multiplayer
  - Splitscreen 3D Hud available for each player: checkpoint arrow, beam, pacenotes, racing line
- Gui
  - Game Setup - Paints tab with more sliders and
    - Dynamic list with groups - Add,Delete,Load,Save buttons - saved in paints.ini
  - Tracks list: 3rd view
    - New columns with **user**'s track rating (slider) and bookmark (checkbox) for better orientation
  - Game window title showing mode and players count, changing color
  - More info on Fps bar (F11), colors from values (orange and red are very high), info on Gui
  - Settings for using cache and shaders debug, mouse capture and ogre dialog show.
- New **Material Editor**
  - Alt-F both in game and editor. Can save json, needs adjusting after, [info](Materials.md)
- Moved [**translations**](Localization.md) to [Weblate]((https://hosted.weblate.org/projects/stunt-rally-3/stunt-rally-3/))
- Editor
  - Many **terrains** - new Gui buttons: < Previous, > Next, + Add, X Delete, top right (same for Roads)
    - Enter - goes to next terrain, while editing with brushes (outside Gui)
  - Terrain **Brushes** - new set, 169 total
    - Dynamic list with groups - Add,Delete,Load,Save buttons - saved in brushes.ini and png (but slow)
    - Brush parameters - New tab with sliders and **Random** buttons. Colored values
    - Grid brushes, 3×3, 5×5 etc - can repeat N-gon type, repeat in OfsY parameter
  - Bigger terrain heightmaps - available, with good Fps. File size shown (2k is 16MB, 4k is 67MB!)
  - New tabs Fluids and Particles with lists (Alt-X) for easier picking, colored names
  - **Focus camera** on current (object, fluid, road point, anything) - key \
  - Fluids quality (diffuse, refract (not done yet), reflect) keys 1,2 - to reduce rendering when many on track
  - Update now saves terrain heightmaps, so F5 won't undo after
  - Particles scale - new parameter for size e.g. for bigger clouds in distance (keys K,L)
  - Terrain circle cursor - same size far/near, animated
  - Grass layer color - can pick any, no ColorMap combo
  - Sky preview map shown on Sky tab
  - Hud hide and Trail show checkboxes. New tab View, split from Settings
- Sources
  - Require building latest Ogre-Next and MyGui-Next fork from sources. Info in git [Building](Building.md) file
  - Reworked base class and new AppGui class, common for game and editor
  - Added **emojis**, for better code orientation in key sections and methods. Quick guide in [emojis file](../src/emojis.txt)
  - Only tinyxml2 used now, no tinyxml
  - Game and Editor tools can be started passing command line **argument**(s)
    - **cfg or config** forces showing Ogre dialog with screen and rendering options
    - Starting with ? shows help for all. In code search for _Tool_ or references to MainEntryPoints::args
- **Bugs** and notable **missing** features from SR 2.8, not yet present in SR 3.0:
  - broken multiplayer on Windows
  - Video options: Resolution, Vsync, Antialiasing, etc are not on Gui, but in Ogre dialog before start. No Limit Fps
    - Can't change them without quit. Also possibly alt-tab could require track reload on Windows.
  - Terrain shadowmap, Fluids: depth color, Refraction, smooth border. Grass deform, Trees wind.
  - HUD: minimap terrain or border, bad other position markers, bad in splitscreen: gauges, minimaps, digits etc
  - Editor
    - Slower road editing (~20 Fps not 60), added skip. Full Rebuild (B) takes quite _long_, columns need it
    - Bad reflection in editor F7 camera, less Fps (minimap RTTs don't skip, no terrain+road mode)
    - Object pick errors,  moved Terrains bad edit cursor pos
  - On older PCs: slower track loading (e.g. 5 sec not 1) and few lags due to long shaders compilation etc
  - Generally lighting changed due to sRGB colors (also using gamma to fix 1.15)
    - Every color in e.g. sun, particles, materials etc needs adjusting when converting from SR 2.x (make it darker, more saturated)
  - Grass not lit by vehicle lights, particles are unlit
  
  ----
  Old SR 2.x changelog [here](https://github.com/stuntrally/stuntrally/blob/master/docs/changelog.md)
