## What's coming next

This section lists changes not yet released, but committed to new repositories stuntrally3 and tracks3.  
Stunt Rally 3 is continuing last SR 2.x, using latest Ogre-Next 3.0.  

## Version 3.0 beta

- Missing things and bugs listed on [Roadmap](Roadmap.md)
- Graphics - moved to Ogre-Next
  - A **lot smoother and higher Fps**, also in split screen
    - No vegetation lags. Auto HW instancing. Good Fps with many objects/buildings, and road points in editor
  - Vegetation models - no impostors, now using meshes with LODs
    - Graphics Detail tab has new setting - for far Vegetation/Objects geometry reduction, aka LOD bias
  - **Horizons** - far terrains on many tracks, great landscape views
  - PBS materials - with roughness and fresnel/metalness. WIP, can be improved
    - New vehicle materials - with more parameters and new look
    - Color changing **paints**, 3 colors (on Gui) at once depending on view angle
    - New paints - Big set, over **450** total
    - Materials in new .json format with more posibilities e.g. detail maps
    - New .material syntax and new (more complicated) **HLMS** shaders, comparison and details in [Developing](Developing.md) file
  - Better glass pipes - sorted, no random blinking
  - Vehicle front **lights** - new tab Lights in Graphics options, toggle key, _Todo:_ WIP grass not lit
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
- Editor
  - Many **terrains** - new Gui buttons: < Previous, > Next, + Add, X Delete, top right (same for Roads)
    - Enter - goes to next terrain, while editing with brushes (outside Gui)
  - Terrain **Brushes** - new set, 169 total
    - Dynamic list with groups - Add,Delete,Load,Save buttons - saved in brushes.ini and png (but slow)
    - Brush parameters - New tab with sliders and **Random** buttons. Colored values
    - Grid brushes, 3×3, 5×5 etc - can repeat N-gon type, repeat in OfsY parameter
  - Particles scale - new parameter for size e.g. for bigger clouds in distance (keys K,L)
  - Bigger terrain heightmaps - available, with good Fps. File size shown (2k is 16MB, 4k is 67MB!)
  - Terrain circle cursor - same size far/near, animated
  - Grass layer color - can pick any, no ColorMap combo
  - Sky preview map shown on Sky tab
- New **Material Editor** - Alt-F both in game and editor. Can save json, needs adjusting after
- Moved [**translations**](Localization.md) to Weblate
- What is different, worse, compared to old SR 2.x
  - Video options: Resolution, Vsync, Antialiasing, etc are not on Gui, but in Ogre dialog before start. No LimitFps
    - Can't change them without quit. Also possibly alt-tab could require track reload on Windows
  - On older PCs
    - Slower road editing (~20 Fps not 60). Full Rebuild (B) takes quite long, columns need it
    - Slower track loading (e.g. 5 sec not 1) and few lags due to long shaders compilation etc
  - Generally lighting changed due to sRGB colors, also using gamma to fix 1.15
    - Every color in e.g. sun, particles, materials etc needs adjusting when converting from SR 2.x (darker, more saturated)
- Sources
  - Require building latest Ogre-Next and MyGui-Next fork from sources. Info in git [Building](Building.md) file
  - Reworked base class and new AppGui class, common for game and editor
  - Added **emojis**, for better code orientation in key sections and methods. Quick guide in [emojis file](../src/emojis.txt)
  - Game and Editor tools can be started passing command line **argument**(s)
    - **cfg or config** forces showing Ogre dialog with screen and rendering options
    - Starting with ? shows help for all. In code search for _Tool_ or references to MainEntryPoints::args
  - Only tinyxml2 used now, no tinyxml
