_How to start the game, configure controller, settings etc._


### Hardware requirements

The recommended minimum hardware is, at least:

* CPU: 2 cores, 2.4 GHz
* GPU: dedicated, with 2GB GPU RAM and Open GL 4 supported:  
  - Low budget (e.g. Radeon RX 570, GeForce GTX 970, GeForce RTX 3050)  
  - or High (e.g. Radeon RX 6600 XT, GeForce RTX 3070) etc.
  - Link to [GPU benchmark](https://www.videocardbenchmark.net/compare/2954vs3558vs3521vs4495vs4444/) for comparing.
* RAM: 4GB
* Disk space: 4GB

Integrated GPUs and old laptops don't handle the game well or at all.  
In any case, if you get graphic errors, try updating your graphic driver, and making sure game uses dedicated GPU.  

Old or slow CPUs could make the game not handle simulation correctly and slow down.  
Too low GPUs (benchmark score or cores count) will slower rendering,  
to less than ideal 60 Fps (from display VSync), especially on big, complex, demanding tracks.

The game _was_ developed on a PC with Radeon RX 570, which will likely be optimal (unless you have a better GPU).  
Using default settings (Higher), it achieved around 60 Fps on all tracks (bit less in complex ones).  

It is possible to run some tracks (Test* should) on lower hardware, but with low Fps (could be unplayable, below 30).  

Pressing F11 will cycle Fps bar (see [Tweak](Tweak.md) page for more info).  
In game F9 shows (default) time Graph of Fps.

  
### Running
  - Esc/Tab key shows/hides GUI (F1 or ` in Editor).
  - Ogre dialog (before start) has screen resolution, VSync, antialiasing etc.
  - In Options, on Screen tab, pick Graphics preset according to your GPU.
  - Quit game/editor to save settings and start again.
  - Press 🏁 New Game button to start driving.
  - For new users, you can enter *How to play* to watch few lessons, and read Hints there.
  - Help page in game has Quick help and other pages (this Readme, Contributing, and all gathered Credits).
  - Have Fun 😀

   
### Input

Keys used in game can be seen in Options on tab `Input`. Also quickly on Welcome screen (Ctr-F1).

If you want to reassign keys, or have a **game controller** 🎮 go to tab `Player1` (or other) to bind it and test range.

For keyboard input, there is a **sensitivity** setting. You can change it for each control, or pick for all from combo (preset).  
There are also speed sensitive steering and steering range settings on Setup tab in Single Race window.

By default, in game you can change **cameras** 🎥 with C/X (with shift for main cameras only).  
Cameras can be adjusted in game, by mouse - move mouse to see actions.

   
### Controllers

Click button to start binding, then move axis or press button or key to bind it.  
Click on Edit > to change Inverse for a control (for analog axis).  
Press RMB (right mouse) on button to unbind.

Check your axes range on those bars displayed.  
If moving an axis from start to end also moves the bar from left to right completely  
(and without moving over the range) then configuration is done properly.

_Binding both controller axis and keyboard keys is possible._  
_To check if your device was detected search for `<Joystick>` in ogre.log_

After your configuration is complete you can restart game to have the settings saved.

You may want to backup your input_p0.xml (for Player1) in user settings dir (see [Paths](Paths.md) page) or create presets this way.

----

### Graphics Options

Using preset combobox (which changes all settings) should be enough. Remember to quit and restart after change.

You may want to change some options individually.  
Many have marks at end (** * ^ # etc) see legend for meaning (right, bottom on Gui).  
Those without marks are updated instantly. Some, like Vegetation can be updated with button.

There are few `Graphics` options that are more important and have more impact on performance than the rest.  
Generally first tabs have higher impact on Fps: Reflection, Water, Shadows.

_Fps - Frames per second. The first value shown in upper left corner of the screen.  
Higher values mean smoother play. It is recommended to play with at least 30 Fps or to aim at monitor's refresh rate._

  * Reflections and Water - very demanding, lowering Fps.  
    Size (of textures for them) is important. Also _frame skip_ and _faces at once_ for dynamic cube reflections can help.
    
  * Shadows - quite demanding too. If you have very low Fps turn them off (None).  
    Could do same for split screen, since Fps drops with each new player viewport.  
    Increasing Size and Filter will make them better quality.  
    Shadows _do_ need restart to change (Todo fix).

  * Vegetation - Reduce Trees and Grass multipliers for more Fps.  
    For newer GPUs you can set them higher to have denser vegetation.

  * Materials - If you have an old GPU, you should lower Anisotropy (0-16, 4 is enough).  
    Turning off Triplanar can reduce Fps much, especially on demanding tracks (big and with high mountains).

  * Detail - has options for geometry Level Of Detail.  
    Reducing it will decrease drawn triangles count and make geometry less smooth.

For minimum settings just pick Lowest from preset combobox and restart.  
For values suggested, check other Graphics options on Gui, or in [sources](../src/common/GuiCom_Presets.cpp) for that.

   
### Configs

All settings are saved in `game.cfg` file or in `editor.cfg`.

**For help on how to locate these files, see [Paths](Paths.md) page.**

Deleting this file(s) will force using default settings at next start of game or editor.

If problems occur check `ogre.log` for errors (or `ogre_ed.log` for editor).

   
###  Feedback

If you have problems running, and suspect a bug, see [Troubleshooting](Troubleshooting.md).  
Testing and reporting bugs is welcome.  
Especially if you can build the game from [Sources](Building.md).
