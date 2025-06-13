## Tracks

*Post from forum.*  
About creating tracks and their content.

🆕To be informed about **new official tracks**, see [Changelog](Changelog.md) for count and [tracks3](https://github.com/stuntrally/tracks3/commits/main) repo **commits** for new names.

----
## 🏞️Tracks

First, probably easiest, most fun, and also most welcome contribution, is making new tracks. If you want to create some, I recommend:

- Info in docs [Editor](Editor.md) with links to new **video tutorials▶️**
- (outdated) old SR Editor 2.6 on [my old website](https://cryham.tuxfamily.org/portfolio/2015-sr-track-editor/) on many pages with images, and [old video](EditorOld.md) tutorial

From my experience I need about 🕐1 hour editing per 1 km of track. So shortest track can be made in 1h, but long rally or big stunt tracks can easily take 8h of editing and testing. Very long tracks can take few days to finish. It depends on how long you want to add detail and fix smaller issues.


### 🌟Quality etc

If you want your track **included** in next SR 3.x version, you can read and follow these old [remarks](https://forum.freegamedev.net/viewtopic.php?f=80&t=5034) also this post about my 🌟 [track rating](https://forum.freegamedev.net/viewtopic.php?f=80&t=5878&p=60432#p60432).  
Basically the track needs to be of **good quality and have various elements** (especially if it is long or a stunt track), those then appear in Track menu with icons (e.g. fluids, objects, obstacles, jumps, various loops, pipes, halfpipes, onpipes, etc.). I can edit the track and improve quality or add some more things, but the more of that is done already, the less I need to do and more likely a track will meet requirements.


### 👻Track's ghost🆕

In SR3 you can now do the Track's ghost yourself. And this will be helpful, if it's done properly, same like the rest was. This and filling tracks.ini is described in new docs [chapter on Editor](Editor.md#steps-after) page.

The file .gho needs to be in data/ghosts/ so maybe just attach it on forum here (2nd _r for reversed). Or in future I could make code to check if present in track's dir IDK.


## ✅How to contribute

Tracks should be done as PR (pull requests to tracks3 repo).

I am also okay if you just pack your track and attach here, or somewhere else. If you don't know git, and really don't want the trouble.

But if you are also adding content to data/, then 2 PRs are needed and that's really much less trouble for me to add (just merge PR).

Each track should (ideally) be in its own PR. I can make an exception if you do few at once in one PR and we already knew what they are from Forum screens etc. This way we can discuss in PR any issues left, separately for a track, while others might be fine. We could still discuss this on forum first, but usually with less detail without track files.


### 🖼️Textures

If you want some custom textures or materials for your tracks e.g. for terrain, vegetation, road, sky, etc. check the page [Materials](Materials.md). It tells what to put where and what to edit. Everything new has to be added to [config/presets.xml](../config/presets.xml) (game uses it and editor lists).


### 📦Objects🏢

Static meshes, buildings etc or dynamic have their own [Objects page](Objects.md). Check it to know how to export them from Blender to Ogre .mesh v2 and into game.

Of course all needs to be CC0, CC-BY, or CC-BY-SA and its info added to appropriate _*.txt file in that folder.

ToDo: need to describe particles✨ a bit and remake all editor tutorial videos.

----
### 🔄Updating tracks

Also known as renewing. Was done few times for many tracks, likely all. Whenever something big in data changed like terrain textures, skies, rocks etc.

Making tracks look better, especially now adding horizon(s) and updating to newer content is very welcome.

If this keeps in track's style it's good. Same for light / color / brightnes / sun direction, especially for sunsets. You could break a track's ambience just by moving sun to other place, this can happen just by replacing sky. Which also needs updating fog colors etc.


### 👥Modding tracks

Since this topic came recently, I should write more. Generally modded tracks aren't that creative, yet need few MB in repo and game size, and own line in (already big) tracks list.

Modded tracks can go into SR3 but I'm not too happy about them. Especially if they don't change much, like only day time and sky.

Best modded tracks would need to seriously change whole scenery, add more objects, etc to be considered of higher rating, at which point it may be even possible to remove original track.

But to keep both, old track had to be of high good rating already, and new in completely different scenery like e.g. a jungle track modded into marble, crystal, anomaly etc. So that both would be justified to be creative on its own. Still, if they have same road and terrain then you know why I'd prefer putting that effort into making new tracks instead.

Modded tracks especially should be in own PRs or just packed and attached on forum. I recommend starting a thread for each track. This way I could post early what's my impression and if I like it enough to merge PR later (if that's your goal).


### 🌟Rating cd

A modded track gets -1 to rating at start.

Tracks with rating 2 and below are considered (by me) worse looking, not great and could be removed in future SR versions. On the other hand tracks with rating 6 are already at top (quality) and likely can't be improved.

- CryHam · 10 Mar 2024

Finally started new track editor video tutorial. Got 2 chapters now.

[Playlist here](https://www.youtube.com/playlist?list=PLU7SCFz6w40PY3L8qFNTe4DRq_W6ofs1d). Link to docs [Editor](https://github.com/stuntrally/stuntrally3/blob/main/docs/Editor.md) is the same.
