_Frequently Asked Questions, or answers to some questions from past._

### General 🚗

**What PC do I need to play?**  

PCs with dedicated GPUs are best (crucial), low budget is enough, details in [Hardware Requirements](Running.md#hardware-requirements).

**Can I play on a laptop?**  

Some laptops that are recent or meant for gaming can handle the game.  
But older ones can't. Especially not laptops without a dedicated GPU.

**How to change camera view?**  

The default keys are C for next, X for previous. Hold Shift for main cameras only, there are many more without Shift.

**What's the green car? What about the orange one?**  

This is nicely explained by in-game Hints which also are translated, press Ctrl-F1 if you missed them.  
The *green* is the Track's ghost (my ES drive for each track, drives well and without mistakes).  
The *orange* one is your best drive Ghost (by default). It will appear after you drive a track. And will update after you drive it faster.  
You can show/hide them on Replays tab and change their color on Paint tab.

**Are there AI cars?**  

No. Some tracks are quite difficult to drive by human and I think AI couldn't handle them well, surely not so good to compete with a human.  
It would be also difficult to code and test that (but not impossible, by someone keen with this topic).

----

### Difficulty 🚦

**Why is it so difficult to drive?**  

Quite common question. This game is not an arcade style by design (but it is not a difficult sim-racing either).  
Usually getting used to a new car simulation takes time and practice. It is a part of the game.  
It may take weeks (or more) but in the end it is more rewarding to be able to do it.  
This is especially true for people playing asphalt racing games before, and never actually driving on gravel.  
And lastly, we don't believe it is difficult. In fact [Richard Burns Rally](https://en.wikipedia.org/wiki/Richard_Burns_Rally) is still more complicated (and like any other game gets easy to play after time).  
It may be also possible that you are trying to play a game type that you don't like. But I'd say trying other game types is good experience.

**How do I make the game easier to play?**  

We recommend first driving slower cars (like e.g. Bison, LK4) on easy, flat tracks.  
I don't recommend turning on ABS or TCS since those will in fact make it harder to understand what is happening with tires (which ones spin or slide) and thus make the game experience worse, weird or simply not as it is meant to be (there was a [poll](https://forum.freegamedev.net/viewtopic.php?f=81&t=5598) on forum too).  
This may also happen when using different cars. The best cars are Hyena, Fox or similar. If starting normal simulation mode, I highly recommend using those as first. Those are easy to handle, fast and can drive on all tracks. Once you learn handling them you can try others.  
If you're not a fan of wheels, you can try the spaceships on some flat tracks. They are very easy to handle and fun. Just don't try them in pipes or other stunts, aren't drivable there (they go nuts).

**Why is it so easy to drive? How do I make the game harder to play?**  

It gets really easy once you know all tracks and get used to the simulation. It's not yet on the level of difficulty I'd like to have.  
Try other cars. Some exotic cars are big, broad and different to handle. Or latest stunt tracks, which are very difficult.  
We have a lot of challenges in game. Those are difficult to pass since a good time is required on track and that usually with no mistakes. They can be also driven with reversed tracks which double the count.

----

### Multiplayer 👥

**Does it work?**  

Yes. But it only works with Direct Connect (need to know other IP).  

Public game list server is not working since years.  
You could try running it on your own:
- need to build [this program](../src/network/master-server/main.cpp)
- start it on PC with known IP
- then use that IP in other PCs, on Gui or in `game.cfg` line with `master_server_address = `

**Why were there no servers/games in multiplayer list?**  

Check out this [page](Multiplayer.md) about multiplayer and players topic.

**Where can I find players to play multiplayer, Is there a lobby?**  

There is no lobby. Doesn't seem to be a way to find players since there aren't many.  
I recommend playing with your friends.

**I can't connect, I get some error message.**  

This isn't a question, but see other common multiplayer issues and [FAQ here](Multiplayer.md#troubleshooting).

----

### Input 🎮

**Do I need a steering wheel to play it?**  

No. I assure you that this game is completely playable with keyboard. In fact, all official game videos were (mostly from replays) driven with keyboard.  
But surely you can take advantage of it. Force feedback isn't implemented though.

**Can I play with a gamepad?**  

Sure. Check the section [Controllers](Running.md#controllers) on how to configure it. And setup it on Input tab.

**How do I reset my settings?**  

This is in our [Paths](Paths.md), just by deleting your config files.

**What's the difference between Space brake and key Down?**  

This is the same as in cars. Regular brake will act on all wheels (with higher ratio on front). And it's best at slowing down car.  
The handbrake acts on rear wheels only and can also easily lock them to make a faster car spin possible in sharp turns.

  
### OS

**Why is the game not in my Linux distribution?**  

More info in [Install](Install.md). Our provided binaries should cover all distributions.  
You can also [build](Building.md) from sources, on Linux it's a rather easy task.

**Why is there no Mac OS X package?**  

Nobody managed to make it. And then continued to update it. If you still want to play the game, I recommend trying it on Debian.  
Apple is one of the worst companies ever, and their products are overpriced, meant to be garbage, see [video](https://www.youtube.com/watch?v=AUaJ8pDlxi8) and [wiki](https://en.wikipedia.org/wiki/Planned_obsolescence).  
I won't support it myself, instead will promote [Freedom](https://www.fsf.org/) that Linux OSes and [FOSS](https://www.gnu.org/philosophy/free-sw.html) give.  

----

### Contribute

**I have found a bug, what should I do?**  

If it is a crash than it's obvious but if not make sure it is a bug. Observe if there is a pattern to have it happen, this will be useful in fixing it.  
Read this wiki about [Troubleshooting](Troubleshooting.md). It tells how to report a bug and what is needed for it.  
See in [Readme](../Readme.md#️feedback) under ️feedback on how to report.

**How can I contribute to this project?**  

See page [Contributing](Contributing.md) for details and areas in which you can contribute.  
Thank you for all contributions.

**How can I make a new track?**  

This needs some time to learn the included SR3 Track Editor but is explained in a (very OLD) series of video tutorials on [Editor](Editor.md) page with some more info.

**Can I donate or support this project financially?**  

Yes. Since few years CryHam started accepting donations. To donate use [Donate page](https://cryham.org/donate/).  
Or directly [paypal link](https://paypal.me/cryham) or [ko-fi link](https://ko-fi.com/cryham) that also allows monthly donations.  
Thank you for donating. For now, I didn't set up patreon yet.

**I have a question that is not answered here / how can I learn more about this project?**  

Firstly by reading the Documentation. It describes most things already.  
Quick questions can be on github [Issues](https://github.com/stuntrally/stuntrally3/issues) page, for longer discussions we have new Forum.  
For info about CryHam (main creator of SR), his other projects, and more, you can read on [CryHam's website](https://cryham.org/).
