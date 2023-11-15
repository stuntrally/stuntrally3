_General info about the project._

### Main ###

See [Building](Building.md) page for compiling from sources.  

If you'd like to contribute please read [Contributing](Contributing.md).  
For latest updates in repo and info on progress check [Changelog](Changelog.md).  
For future plans see [Roadmap](Roadmap.md) and developer [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) - These pages also have _known issues_.  

Page for [Donations](https://cryham.tuxfamily.org/donate/) if you'd like to support me and SR financially.

Translations on [Weblate](https://hosted.weblate.org/projects/stunt-rally-3/stunt-rally-3/).

### Media

**[Videos](https://www.youtube.com/user/TheCrystalHammer)** from gameplay and _old_ editor tutorials.  
_Old:_ [Screenshots gallery](https://stuntrally.tuxfamily.org/gallery) (from all older versions) and [Development gallery](https://stuntrally.tuxfamily.org/gallery-dev) (lots of funny screenshots).  
_Old_ presentation document of the project (aka SR 2.6 book, 196 pages), topic with [info here](https://forum.freegamedev.net/viewtopic.php?f=81&t=7411) and [repo here](https://github.com/stuntrally/presentation).  

Creator and main developer [CryHam's website](https://cryham.tuxfamily.org/).  

It has text pages for [new game](https://cryham.tuxfamily.org/portfolio/stuntrally3/) and [old game](https://cryham.tuxfamily.org/portfolio/stuntrally/) and longer with images [old editor](https://cryham.tuxfamily.org/portfolio/2015-sr-track-editor/).  

### Feedback

Be sure to [Read before posting](https://forum.freegamedev.net/viewtopic.php?f=78&t=3814) first.  
You can post on [Forum](https://forum.freegamedev.net/viewforum.php?f=77) or create new Issue on [github](https://github.com/stuntrally/stuntrally3/issues) - If not already present in _known issues_.  
There is no chat. Since years, IRC is not used, and was team's development channel.  

----
### Technologies and Libraries used

Code is written in [C++](https://en.wikipedia.org/wiki/C%2B%2B). Uses [CMake](https://cmake.org/) and possibly [Conan](https://conan.io/) to build.  
[Git](https://git-scm.com/) for repository. Located on [Github here](https://github.com/stuntrally/). Licensed under [GPL v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).  

Car simulation is done by [VDrift](https://vdrift.net/) (older version) also using [bullet physics](https://bulletphysics.org/).  
Sound with [OpenAL Soft](https://openal-soft.org/).  

Rendering by [Ogre-Next](https://www.ogre3d.org/) with PBR materials using [HLMS](https://ogrecave.github.io/ogre-next/api/latest/hlms.html). Gui by [MyGUI](https://github.com/MyGUI/mygui).  

Road is based on a 3D [spline](https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Catmull.E2.80.93Rom_spline) and it's fully customizable in editor.  
Both road and SR Track Editor are fully implemented our own code by CryHam.

#### Website

Website is made with [CMS Made Simple](https://www.cmsmadesimple.org/), [DokuWiki](https://www.dokuwiki.org/dokuwiki) and [Mantis Bug Tracker](https://www.mantisbt.org/).  
Hosted on [TuxFamily](https://www.tuxfamily.org/en/about).
