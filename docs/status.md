# Status

*Post from forum.*  
Status of the project etc.


## ⚠️Development stopped, project status, tasks, links etc.

Well just FYI, and in case it wasn't obvious. I'm not developing Stunt Rally (SR3) since last release 3.3 (over 3 months now).  
I did improve docs only. There is a master/game server running, not sure.

### Tasks

A brief, to the point, bird's eye view of **project status and tasks** is such:

- top priority goal for the project itself: was/is becoming fully Free Software with only CC-* content.  
Details in [this topic](data.md). It's a years taking process that has very slow progress and got even set back as engine sounds are likely still unfit.  
This would also allow moving away from github and moving to [forge.a-lec.org](https://forge.a-lec.org/explore/repos).  
Where possibly php server code could be developed for player track records etc.  
Still a very long way and far, now *unlikely* future.  
Would be good to have [Content Manager](Tasks.md#content-manager) in game BTW here.
- there are many graphics, OgreNext related issues, and bugs, that I summarized in [forum post](https://forums.ogre3d.org/viewtopic.php?p=557266#p557266).
- there are probably now hundred(s) of things to do or fix, gathered in [Roadmap](Roadmap.md)
- and about 25 bigger [Tasks](Tasks.md) with more info.

There is also [Contributing](Contributing.md) info with areas and skills needed.  
There weren't really any more contributors besides 2 artists:  
EP and rubberduck, who did make many very awesome tracks, also rubberduck helped with skies, data, tracks etc.

----

### Project (characteristics)

Either way, I don't aim to develop SR3 now. I aim not to develop it.  
Frankly said, and IMO, SR3 is a **project that is**:

- quite big and complex
- not easy to build (unless on Debian-based) or to package
- likely too "my style" of code,  
surely not good for [O.O.](https://en.wikipedia.org/wiki/Object-oriented_programming) or modern C++ learning etc.  
or even not very readable due to my [shortcuts](../src/emojis.txt) (my way of achieving more, with less key presses)
- generally not good for learning  
Small projects are better for this. And I'd say OgreNext isn't popular, Ogre is old,  
I see many newer rendering engines and game [engines](https://gamefromscratch.com/25-game-engines-in-25-minutes/), worth learning more (OFC not the big commercial ones).
- not popular, or with players  
(I'm not on social media, posting, doing PR, asking popular websites to write about it, etc.)
- not having a community of people playing or being active, on chat/forum  
- my own original project  
not a game clone (even if [alternativeto.net](https://alternativeto.net/software/stunt-rally/) says so)  
not a rewrite requiring copyrighted data  
but also not a fully FOSS project with only CC-BY data  
(even if wikipedia entry says that) and being in official Linux repos.
- not having any new programmer since over a decade (likely because of (all?) above)
- not having any new artist in years, last were also over a year ago here

I'm not saying it should or shouldn't be all of that. Just trying to summarize.  
But there are a lot of not-s here. Some of these are related.  
For me it's *unrealistic* to think someone would join, or any of above would change.

----
### 3D engine

As for 3D, like I wrote in [forum post](https://forums.ogre3d.org/viewtopic.php?p=557266#p557266), I got interested in [Wicked Engine](https://github.com/turanszkij/WickedEngine).  
Instead of OgreNext, which is IMHO not developed and since year(s) didn't improve that much (as I'd like).  
Wicked Engine is based on Vulkan and DX12 while OgreNext on older GL3+ and DX11.  
It is also a game engine with editor. I'm interested in checking out and learning this engine in practice.  

Possibly this would also mean better graphics for SR4.  
If that ever happened, since I already spent 2 years to move to OgreNext. I'm not in the mood of moving to another, and spending more years.  
But I see there is *no future* with OgreNext.  

Wicked has already more features, is bigger, seems actively developed, more advanced and popular.  
IDK about performance or how much could be adjusted for low end hardware (not my top priority). There are surely many unknown factors.  
OgreNext *support* was good at start and then there wasn't any. No idea how it is with Wicked, could be similar.  
I dislike learning from engine's code and finding/fixing bugs in there, means for me developing an engine not the game.  
But yeah could be just my Wicked *hype*, until I do lots of stuff in it, and test, it's just a fantasy to move SR3 to SR4.  

SR ain't really a game for laptops or low end GPUs, Raspbery Pi-s etc.  
This is because I have a medium or high end GPU (AMD RX 6600 XT) and I simply want my/our game to give best graphics on it and enjoy that.  
Also to put something more recent into my skills set.  

### Lastly

Lastly it is a black hole, meaning it consumes all my time. An awesome black hole to be in, but a life/time sucking one for sure.  
Instead, now I can and did developed few other [projects](https://cryham.org/projects/), can have time for keyboards and electronics too.  
Lastly there are also IRL things to take care.  
I'm not saying I don't like developing SR3, it's just that I'd like to do many other projects instead and learn more.  
If I do all, and got time left, I could get back.

So I have been recently developing [CaveExpress](https://cryham.org/portfolio/2025-caveexpress-fork/), and also made my own fork of [Mars ridiculous shooter](https://cryham.org/portfolio/2025-mars-fork/),  
having fun in 2D instead of 3D. Frankly I learned more doing this than I would in SR3 now.  
Previously also moved my website and [stuntrally](https://cryham.org/stuntrally/)'s from [tuxfamily](https://forum.tuxfamily.org/post/3383/) to my own [cryham.org](http://cryham.org/) website (paid),  
then renewed my hammer [logo](https://cryham.org/about-me/my-nick-and-logo/) after so many years (over 20).  
I moved my projects to [codeberg](https://codeberg.org/cryham?tab=repositories) (because of [reasons](https://drewdevault.com/2022/03/29/free-software-free-infrastructure.html)). I will maybe post more on my blog, etc.
