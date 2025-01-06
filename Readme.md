![](/data/hud/stuntrally-logo.jpg)

[![Build game](https://github.com/stuntrally/stuntrally3/actions/workflows/build-game.yml/badge.svg)](https://github.com/stuntrally/stuntrally3/actions/workflows/build-game.yml)
[![Translation status](https://hosted.weblate.org/widget/stunt-rally-3/stunt-rally-3/svg-badge.svg)](https://hosted.weblate.org/engage/stunt-rally-3/)  
![Last commit date](https://flat.badgen.net/github/last-commit/stuntrally/stuntrally3)
![Commits count](https://flat.badgen.net/github/commits/stuntrally/stuntrally3)
![License](https://flat.badgen.net/github/license/stuntrally/stuntrally3)  
[![Discord](https://img.shields.io/discord/1239125025395703858?label=Discord&logo=Discord&logoColor=white)](https://discord.gg/TywnXxAtR6)
![](https://img.shields.io/github/downloads/stuntrally/stuntrally3/total.svg)

## Links

### Main
🌎[Stunt Rally Homepage](https://stuntrally.tuxfamily.org/) - Download links, track & vehicle browsers etc.  
📚[Documentation](https://github.com/stuntrally/stuntrally3/blob/main/docs/_menu.md) - inside docs/, Information on many pages  
⚙️[Sources](https://github.com/stuntrally/stuntrally3/) - also for bugs & Issues, pull requests, etc.  

### Media
🖼️[Screenshots](https://stuntrally.tuxfamily.org/gallery) - Galleries from all versions and development, [latest Big](https://photos.app.goo.gl/P4ZoiGwjPxJUN6oe6) from SR 3.3  
▶️[Videos](https://www.youtube.com/user/TheCrystalHammer) - from gameplay and editor  
💜[Donations](https://cryham.tuxfamily.org/donate/) - financial support, on [Ko-Fi](https://ko-fi.com/cryham)

### Feedback
💬[Discord](https://discord.gg/TywnXxAtR6) - chat, community, quick help, etc - follow #rules - (could be removed in future)  
🗨️[Matrix](https://matrix.org/) - chat room: #stuntrally:matrix.org -  (e.g. with [element app](https://element.io/download), or other [clients](https://matrix.org/ecosystem/clients/)) - (*preferred*)

🪲New Issue, on [github](https://github.com/stuntrally/stuntrally3/issues) - for bugs etc - needs account  

🏛️[Forum](https://groups.f-hub.org/stunt-rally/) - discussions, tracks, content - [how to join](https://groups.f-hub.org/d/2ftpShKs/) info on top  
📜[Old Forum](https://forum.freegamedev.net/viewforum.php?f=77) - old archive, SR 2.x  
🪧[Reddit](https://www.reddit.com/r/stuntrally/) - (not used)  

------------------------------------------------------------------------------

## 📄Description

Stunt Rally is a 3D racing game, including Sci-Fi elements and own Track Editor.  
Works on GNU/Linux and Windows.  

For cars the game has a **rally** style of driving and sliding, mostly on gravel and loose surfaces.  
It has possible **stunt** elements (like: jumps, loops, pipes). Roads are made from 3D spline.  
It includes many Sci-Fi vehicles and different planets🌌, even surreal locations.  
All [Tracks](https://stuntrally.tuxfamily.org/tracks3) and [Vehicles](https://stuntrally.tuxfamily.org/cars) can be browsed on website.

SR 3.x continues old [SR](https://github.com/stuntrally/stuntrally) 2.x, now using [Ogre-Next](https://github.com/OGRECave/ogre-next) 3.0 for rendering and [VDrift](https://github.com/VDrift/vdrift) for simulation.

**Documentation** is [inside docs/](docs/_menu.md) dir and has many pages with more information.

## 📊Features

Stunt Rally 3.3 features 232 tracks in 40 sceneries and 33 vehicles.  
Game modes include:
* ⏱️Single Race (with your Ghost drive, track car guide), Replays,
* 🏆Challenges, Championships, Tutorials, (series of tracks), 💎Collections
* 👥Multiplayer (info [here](docs/multiplayer.md), no official server) and Split Screen, up to 6 players.  

The Track Editor allows creating and modifying tracks.  

**Full features** [list here](docs/Features.md).  
**Changes** and new features in [changelog](docs/Changelog.md).  

------------------------------------------------------------------------------

## 🚀Quick Start

[Hardware requirements](docs/Running.md#hardware-requirements).

### 🚗Game

Esc or Tab - shows/hides GUI.

Quick setup help is on the Welcome screen, shown at game start, or by Ctrl-F1.  
- Use Ogre config dialog before start to adjust Screen resolution, etc (start with `cfg` argument will show it).  
  OpenGL 3+ Rendering Subsystem is recommended now,  
  but Vulkan seems better on Windows with integrated GPUs or laptops.
- Open Options to pick graphics preset according to your GPU and *do* restart.  
- Open Options tab Input, to see or reassign keys, or configure a game controller, info [here](docs/Running.md#input).  

Game related Hints are available in menu: How to drive, with few driving lessons.  
Have fun 😀

#### Troubleshooting

If you have problems at start, check page [Running](docs/Running.md).  
All settings and logs are saved to user folder, see [Paths](docs/Paths.md). It is also shown on first Help page.

------------------------------------------------------------------------------

### 🏗️Track Editor

F1 (or tilde) - shows/hides GUI,  
Tab - switches between Camera and Edit modes.  
Press Ctrl-F1 to read what can be edited and how.  

There is no undo, so use F5 to reload last track state, and F4 to save it often.  
After each save, track can be tested in game.  
If needed do manual backup copies of track folder.

[Tutorial](docs/Editor.md) page has more info and few new videos.  


------------------------------------------------------------------------------

## ⚙️Building

How to compile project from sources:  
- On **Linux** is described in [Building](docs/Building.md) (Debian based). Provided script should do it.  
- On **Windows** in [BuildingVS](docs/BuildingVS.md) (difficult), manually building all dependecies from sources first.

------------------------------------------------------------------------------

## 🗨️Feedback

Before reporting bugs or issues, be sure to [Read before posting](https://groups.f-hub.org/d/2ftpShKs/-how-to-post-sr3-info-help-links-about-etc-) (or 
[the old one](https://forum.freegamedev.net/viewtopic.php?f=78&t=3814)) topic first.

Then create a [new issue on github](https://github.com/stuntrally/stuntrally3/issues/new/choose) (if not present already)  
or a topic on [New Forum](https://groups.f-hub.org/stunt-rally/) (if already joined, [how to](https://groups.f-hub.org/d/2ftpShKs/)).

For chat we use: [Discord](https://discord.gg/TywnXxAtR6) and Matrix #stuntrally:matrix.org (preferred)

## 🤝Contributing

Help and contributing is welcome.  
If you'd like to contribute, please check [Contributing](docs/Contributing.md) for details.  

## 🛠️Developing

Details in [Roadmap](docs/Roadmap.md) with few missing features, known issues, and future **ToDo** plans.  

General info with some details for SR3, its sources and using Ogre-Next is in [Developing](docs/Developing.md).  
Sources have emojis, [this file](/src/emojis.txt) has all, with quick components guide, shortcuts used, etc.

------------------------------------------------------------------------------

## ⚖️License

    Stunt Rally 3 - 3D racing game, with Sci-Fi elements and own Track Editor
                    based on Ogre-Next rendering and VDrift simulation
    Copyright (C) 2010-2025  Crystal Hammer and contributors


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see https://www.gnu.org/licenses/

------------------------------------------------------------------------------

    The license GNU GPL 3.0 applies to code written by us,
    which is in src/ dir, subdirs:
	- common, editor, game, network, road, transl
	and modified, subdirs:
	- Terra, sound, vdrift

    Libraries used have their own licenses, included in:
	- btOgre2, oics (modified to tinyxml2)
    - OgreCommon (modified slightly)
    - in src/vdrift/: half.hpp, quickprof.h, Buoyancy.h, unittest.h
    - src/common/MersenneTwister.h  MultiList2.h  /MessageBox/*
    
	For Media (art, data) licenses, see various _*.txt files in data/ subdirs.

------------------------------------------------------------------------------
