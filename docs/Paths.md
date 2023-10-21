_Where is your user data located, saved logs and configs._


### Introduction

This page describes where the game searches for its data and configuration files.

The information is useful if you are e.g.:
  * submitting a bug/crash - you should include your `Ogre.log`, `game.cfg` and `ogre.cfg`
  * need to reset or backup settigs
  * doing manual configuration (editing .cfg without the GUI)
  * packaging Stunt Rally 3

### Location of User's dir

This path is visible directly in game, **in Gui**, on Help page, under User Path. You can mark it and copy.

On Windows (10 and up), the user's directory is located in:  
`C:\Users\USERNAME\AppData\Roaming\stuntrally3`

On GNU/Linux the user's dir is mostly:  
`~/.config/stuntrally3`

(where tilde means the user's home directory, i.e. /home/USERNAME/). USERNAME is to be replaced by yours.

However, `$XDG_CONFIG_HOME/stuntrally3` takes precedence if the environment variable is set. (**?**)

  
### User Configs and Logs

The **default** configuration files are located in the data path, under `config` subdirectory. The game creates dedicated configuration files for each users and they take precedence over the defaults. If you want to reset your configuration to defaults, just delete your user's dir game.cfg, input*.xml, editor.cfg.

Files in user's dir are:
  * **Configs**
    * editor.cfg - settings for editor
    * game.cfg - settings for game
    * input.xml - game input bindings from General page only
    * input_p0.xml .. input_p3.xml - input bindings for players 1..4 (you can copy them for backup, and replace files to have different setups)
    * progress.xml - championships progress
    * progress_rev.xml - same as above but for reverse direction
    * progressL.xml, progressL_rev.xml - challenges progress
  * **Logs**
    * ogre.log - rendering and game log (most important)
    * ogre_ed.log - editor log
    * ogre.err, ogre_ed.err - same as .log but only with errors and warnings, also from std::cerr
    * MyGUI.log - not important, stuff from GUI

#### User data

As the game's installation directory might very well be write protected from non-administrators, your user generated data is saved in user's dir. This data might be critical to the game or editor, but they can start without it with default settings.

Subdirs are: _([mode] is easy or normal, both simulation modes)_
  * cache - see more below
  * data/carsim/[mode]/cars - if you edit car (Alt-Z) and save it, then it will be here
  * data/carsim/[mode]/tires - if you edit tires (one of graphs mode F9), and then save them (Alt-Z, Tires tab), they will appear here
  * ghosts/mode - best laps for each car on each track are saved here
  * records/mode - .txt files for best times on tracks
  * replays - if you save replays they land here
  * screenshots - filled when making screens in game or editor (by default F12)
  * tracks - user made tracks are located here. To share a track just pack the track's dir. Then unpacking here makes the track available.

If you want to reset record time for a track just go inside records/ and delete the .txt file with same track name (or edit it and remove section for a car name).

To completely remove the game you need to delete your user's dir. Ghosts and replays may become big (e.g. 1GB).


### Game Data

As a rule of thumb, Stunt Rally doesn't care what its working directory is (i.e. from where you run the game). It will detect the actual executable location and search the data in relation to that. 

There are few different possibilities here, making it possible to run the game from the Git source tree as well as from `make install`ed prefix. On Linux, common system data installation paths as well as `XDG_DATA_DIRS` are checked too.

If you have your data in a really obscure path, you can always define `STUNTRALLY_DATA_ROOT` environment variable.

### Cache

Cache is used for automatically generated files. Deleting cache files is safe - the application will re-create them when they are next needed, although this might result in a temporary slow-down.

On Windows, this is currently the same as user's config directory.

On Linux (and other similar systems), the cache dir is most likely `~/.cache/stuntrally3` (where tilde means the user's home directory). However, `$XDG_CACHE_HOME/stuntrally3` takes precedence if the environment variable is set.

  
### Config XMLs

These files are listed in [Developing here](Developing.md#config-files-📄)
