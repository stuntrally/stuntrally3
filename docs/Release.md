The process of releasing and things that need to be checked before.

### 1. Code
  * Fix crashes and critical bugs
  * Be sure the code works and there is nothing that would make it broken
  * Make sure all tools or debug logs in code are disabled
  * Try a clean rebuild _and check `make install`_?

### 2. Update
  * Content
    * For new **tracks**: make [tracks ghosts](Editor.md#steps-after),  
      add to [ini](../config/tracks.ini), [champs](../config/championships.xml) and [challs](../config/challenges.xml) xmls (all in config/)
    * Run editor tools to check tracks (errors, presets, etc): ToolSceneXml, ToolTracksWarnings  
      by starting with arguments `sr-editor3 scene` and `sr-editor3 warnings`
    * For new **cars**: check in-car cameras, boost and flare pos, perf tests, easy sim, etc  
      also either add to champs and challs or make new ones for them
  * `Readme.txt` (links, track count, etc)
  * Version numbers
    * in Gui, file `*_en.tag.xml`, tag `name="GameVersion"`
    * in `installer.nsi` script, at top
  * [Sync translations](Localization.md#translation-sync)  
    check some other language so there are no `#{MyGUI Tags}` in GUI

### 3. Test
  * Check all the game modes - Championship, Challenge, Split screen, Multiplayer, Replays
  * Check at least new features and important older ones (see [Features](Features.md))
  * Delete/rename your user dir to check default config (update *-default.cfg)
  * Try a few different Vehicles and Tracks
  * Test Graphics presets from combo
  * Try Track Editor - duplicate a track, save, and at least basic editing, of each mode

### 4. Packages
  * Create packages for supported systems and test them
    * Windows installer - use [dist/installer.nsi](../dist/installer.nsi) with [NSIS](https://nsis.sourceforge.net/Main_Page), easy if from CI build from github Actions
    * Very old: Linux binaries - need VM (was 12.04), info how to [here](https://github.com/stuntrally/stuntrally/tree/master/dist/linux-archive)dist/linux-archive/Readme.md)
  * Make sure all packages use the same version of the sources (meh)
  * Once the packages work, tag the version number to all repositories (stuntrally3 and tracks3)
  * Upload packages, download back and test (or check checksums) to make sure there wasn't corruption

### 5. Websites
  * Update websites
    * [Homepage](https://stuntrally.tuxfamily.org/), tracks count, [Downloads](https://stuntrally.tuxfamily.org/downloads) release date, links
    * [Changelog](Changelog.md), [Features](Features.md), [Statistics](Statistics.md)
  * Update Tracks Browser
    * rename tracks.ini to .txt, use dist/make_roadstats_xml.py, copy into cms
    * also cars.xml for Car Browser, remove unnecessary comments
  * For significant look changes update home **screens**, and pics on other websites
  * Notify known third-party packagers about new release
  * Announce release - on [New Forum](https://groups.f-hub.org/stunt-rally/),  
   [Ogre showcase](https://www.ogre3d.org/forums/viewtopic.php?f=11&t=58244), meh old: [VDrift dev](https://vdrift.net/Forum/showthread.php?tid=1629), [freegamer](https://freegamer.blogspot.com/), etc.
