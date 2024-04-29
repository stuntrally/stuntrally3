_General info about the project._

### Main ###

See [Building](Building.md) page for compiling from sources.  

Translations on [Weblate](https://hosted.weblate.org/projects/stunt-rally-3/stunt-rally-3/).

Page for [Donations](https://cryham.tuxfamily.org/donate/) if you'd like to support me and SR financially.

If you'd like to contribute please read [Contributing](Contributing.md).  
For latest updates in repo and info on progress check [Changelog](Changelog.md).  
For future plans see [Roadmap](Roadmap.md) and developer [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) - These pages also have _known issues_.  

### Media

**[Videos](https://www.youtube.com/user/TheCrystalHammer)** from gameplay and _old_ editor tutorials.  
[Screenshots gallery](https://stuntrally.tuxfamily.org/gallery) (from all versions) and [Development gallery](https://stuntrally.tuxfamily.org/gallery-dev) (lots of funny screenshots).  

Creator and main developer [CryHam's website](https://cryham.tuxfamily.org/).  

It has project pages for [new game](https://cryham.tuxfamily.org/portfolio/stuntrally3/) and [old game](https://cryham.tuxfamily.org/portfolio/stuntrally/) and longer with images for [old editor](https://cryham.tuxfamily.org/portfolio/2015-sr-track-editor/).  
_Old_ presentation document of the project's past (aka SR 2.6 book, 196 pages), topic with [info here](https://forum.freegamedev.net/viewtopic.php?f=81&t=7411) and [repo here](https://github.com/stuntrally/presentation).  

### Feedback

Be sure to [Read before posting](https://groups.f-hub.org/d/2ftpShKs/-how-to-post-sr3-info-help-links-about-etc-) first.  
You can create new Issue on [github](https://github.com/stuntrally/stuntrally3/issues) - If not already present in _known issues_ (on [Roadmap](Roadmap.md), Tasks).  
Or post on [New Forums](https://groups.f-hub.org/stunt-rally/) if already invited there.  
There is no chat or easier way. This is to prevent spam and distraction.

----
### Technologies and Libraries used

Code is written in [C++](https://en.wikipedia.org/wiki/C%2B%2B). Uses [CMake](https://cmake.org/) and possibly [Conan](https://conan.io/) to build.  
[Git](https://git-scm.com/) for repositories. Located on [Github here](https://github.com/stuntrally/). Licensed under [GPL v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).  

Car simulation is done by [VDrift](https://vdrift.net/) (older version) also using [bullet physics](https://bulletphysics.org/).  
Sound with [OpenAL Soft](https://openal-soft.org/).  

Rendering by [Ogre-Next](https://www.ogre3d.org/) 3.0 with PBR materials using [HLMS](https://ogrecave.github.io/ogre-next/api/latest/hlms.html). Gui by [MyGUI](https://github.com/MyGUI/mygui).  

Road is based on a 3D [spline](https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Catmull.E2.80.93Rom_spline) and it's fully customizable in editor.  
Both road and SR Track Editor are fully implemented our own code by CryHam.

#### Website

Website is made with [CMS Made Simple](https://www.cmsmadesimple.org/), [DokuWiki](https://www.dokuwiki.org/dokuwiki) and [Mantis Bug Tracker](https://www.mantisbt.org/).  
Hosted on [TuxFamily](https://www.tuxfamily.org/en/about).
