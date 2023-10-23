![](/Media/hud/stuntrally-logo.jpg)

[![Translation status](https://hosted.weblate.org/widget/stunt-rally-3/stunt-rally-3/svg-badge.svg)](https://hosted.weblate.org/engage/stunt-rally-3/)
![Last commit date](https://flat.badgen.net/github/last-commit/stuntrally/stuntrally3)
![Commits count](https://flat.badgen.net/github/commits/stuntrally/stuntrally3)
![License](https://flat.badgen.net/github/license/stuntrally/stuntrally3)

## Links

[Stunt Rally Homepage](https://stuntrally.tuxfamily.org/) - Download links, old documentation wiki, track & vehicle browsers etc.  
[Forums](https://forum.freegamedev.net/viewforum.php?f=77) - Discussions, tracks, Issues, etc.  
[Sources](https://github.com/stuntrally/stuntrally3/) - also for bugs & Issues, pull requests, etc.  
[Documentation](https://github.com/stuntrally/stuntrally3/blob/main/docs/Features.md) - new, is inside docs/  
Old [Screenshots](https://stuntrally.tuxfamily.org/gallery) - Galleries from 2.x versions and development  
[Videos](https://www.youtube.com/user/TheCrystalHammer) - from game and editor (old)  
[Donations](https://cryham.tuxfamily.org/donate/) - financial support

------------------------------------------------------------------------------

## Description

Port and continuation of latest [Stunt Rally](https://github.com/stuntrally/stuntrally) 2.x using latest [Ogre-Next](https://github.com/OGRECave/ogre-next) 3.0 for rendering.  

Already playable game in all modes. Working Track Editor, almost fully restored.  
Still WIP, not yet released and beta (possible bugs).

**Full features** [list here](docs/Features.md).  

**Changes** and new features listed in [changelog](docs/Changelog.md).  
For details see [Roadmap](docs/Roadmap.md) with missing features and todo.  

**Documentation** is now inside [docs/](docs/) dir. It's partly WIP.

Progress posts in [SR forum topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18515). Dev questions and answers in Ogre-Next [topic](https://forums.ogre3d.org/viewtopic.php?t=96576).

------------------------------------------------------------------------------

## Building

How to compile project from sources is described in [Building](docs/Building.md).  
Requires building Ogre-Next and MyGui fork from sources first.

------------------------------------------------------------------------------

## Contributing

Help and contributing is welcome.  
If you'd like to contribute, please check [Contributing](docs/Contributing.md).  
You can help with [translating](docs/Localization.md), testing, adding or improving content, fixing, developing code etc.

## Developing

General info with some details is in [Developing](docs/Developing.md).  
Sources have emojis, [this file](/src/emojis.txt) has all, with quick components guide etc.

------------------------------------------------------------------------------

## License

    Stunt Rally 3 - 3D racing game, with own Track Editor
                    based on Ogre-Next rendering and VDrift simulation
    Copyright (C) 2010-2023  Crystal Hammer and contributors


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
    
	For Media (art, data) licenses, see various _*.txt files in Media/ subdirs.

------------------------------------------------------------------------------
