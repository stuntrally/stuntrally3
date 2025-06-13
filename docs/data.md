## Data

*Post from forum.*  
About our data / media licensing (issues).

## ♻️Removing, replacing data with non-CC licenses - to do

This is the continuation of that [old forum topic](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532).

I'd say it is a priority (over other data stuff) but of course it's up to us.

The goal is to have *all* resurces only to use licenses: CC0, CC-BY or CC-BY-SA, cleary.

And having each data (mesh, texture, sound) listed with that info in a txt file.  
(unless it was based on CC0 (like Gui icons), but I recommend for this too).

Data still needed to be replaced (updated):

- ✅Done  
🌦️**Skies**, from [_skies-old.txt](../data/textures/skies/_skies-old.txt). This is a big one. Need some more good, variations too for regular nature tracks (since we lots, there should be enough skies and variations).  
And same for other planets skies🌌. Now there is [_skies-new.txt](../data/textures/skies/_skies-new.txt).
- ✅Done  
🌳**Meshes**. Best: all from trees-old/ dir. Surely tree.07 and tree.09. Different new ones for jungle and other like oak etc for forests.  
Also for adding new trees there is a [trees todo](trees.md) with big list of models to try.  
Still *todo*: `fir*.mesh`
- `grass1` **texture** and all based on it (`grassForest`, etc)
- 🛣️**Roads**  
I based oldest roads on some asphalt **texture** from VDrift I'm not sure which. Best if we replace and BTW make it look better. Maybe mix some gravel or grass on side and center to be more real, etc. Don't do it manually in GIMP/Krita. Use our `.json` material feature: add detail maps and just create a blending pattern like our `alpha*` textures to blend between two or more.
- Likely *todo*:  
`waveNM.png, water_nh.png, water_nh.png`, I can't remember, maybe from Ogre.
- Not done  
🔊**Sounds**. All VDrift sounds I believe.  
Already used [engine-sim](https://github.com/stuntrally/stuntrally3/issues/7) to replace engines. Isn't ideal (no exhaust there) but with more work could do.  
More info for sound [tasks here](https://forum.freegamedev.net/viewtopic.php?f=81&t=18532#p110261).  
**More** *todo*: rest of VDrift: bump_*.wav, surface/*  
**More info below** in last post. This is *bad*, not CC-*.
- There were some **textures** in cars, like: underside, rear lamps (of XZ, possibly other old cars),  
maybe more, that were likely based on some unlicensed found images/pictures on intenet.  
To fix either drop whole car (we have new, better quality [in todo](vehicles.md)) or remodel that part with mesh instead.
- Check if *fonts* are okay with [Free Software licenses](https://www.gnu.org/licenses/license-list.en.html#Fonts), replace if not.


## Sounds

Few forum topic posts history below.

- CryHam · 24 Dec 2024

Finally all engine sounds replaced. 🚗🔊💥  
Used that engine-sim, then EQ in Audacity.  
Almost all cars have unique sound now. Usually 8 sounds for 1 engine, each 1000 rpm apart.  

Details in repo:  
[data/sounds/engines](../data/sounds/engines) in
[_sounds.txt](../data/sounds/_sounds.txt#L49) and [issue 7](https://github.com/stuntrally/stuntrally3/issues/7)  

- CryHam · 18 April 2025

Ugh. Okay so after talking with Speed Dreams team ([on matrix](https://speed-dreams.net/en/links)), it turns out that sounds recorded from EngineSim, are not CC-BY or such. EngineSim is FOSS, but [engine setup files](https://catalog.engine-sim.parts/) (those small text configs that make the engine in this simulator) have no license, meaning they're proprietary, and recording made from them is derivative work or such. While it is possible to comment there and if any author is still active and at all willing to release under CC-BY*, I'm pretty sure this is very unlikely and rather impossible for all engines used.

So all engine sounds in SR3 are bad then.
Only solution would be using those few, short, crappy, bad recordings from freesound from real cars driving by, with uneven rpm or volume, echo etc. Just like in Speed Dreams, so maybe just use those from SD then.

If we had *Content Manager* (info and [task here](Tasks.md#content-manager)), so downloadable content in game, we could still allow users (including me) to use those sounds, signifying this license issue.
Obviously, true Free Software with only CC content can't have it, and Content Manager would also solve any car issues. There could be a SR3 version (as default) with only hovering Sci-Fi cars, not even real cars at all.

Seems like the way to go would be:
1. Replacing all above data with anything CC*, or removing.
2. Ensuring all data has known origin and CC* license.
3. Probably doing a check once again, going through each file in data. Making sure each model from sketchfab is still there, comes from real artist, who made the model (not ripped it or got from somewhere) and made textures (not got them). Like I wrote in [Objects.md](Objects.md).
4. Contacting a-lec again.
5. If they agree moving all from github there.
6. Then possibly finding a source for downloadable content for game. Implementing Content Manager. Putting any non free stuff there/elsewhere.

But I'll end this on a sad note, I'm [not developing](status.md) SR3, so this is either not happeing or very far future.
