# Contributing

_How to participate in developing Stunt Rally 3 game or editor._

Help is welcomed.  
Thank you for all your contributions.

Even smaller things count like:
- Testing and reporting a bug.
- Improving a translation to a language.
- Increasing popularity, spreading info about Stunt Rally 3 on internet (with link to our [website](https://stuntrally.tuxfamily.org/)).
- Sharing an honest opinion, or idea on Forum.
- Constructive discussion about features, etc.


Below is a list (in no order, just my favorite 2 first)
of bigger focused contributions:

------------------------
## Programmers

If you know C++, [Ogre-Next](https://forums.ogre3d.org/viewforum.php?f=25) (2 and up) or shaders,  
and preferably could handle some of the [Roadmap](Roadmap.md) tasks.  
Details for tools and sources are in [Developing](Developing.md).  
Own ideas could also go into SR, just need some discussion first.  
Most ideas are already in [Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php). Sorted by P (priority): 1 is most important, higher are less so.

Medium changes can go through [pull requests](https://github.com/stuntrally/stuntrally/pulls).  
For those with more time and effort, commits to main branch will be possible.

There is no style guide (yet), I simply recommend following my own.  
A quick info: it is not random, it's quite specifically formatted to how I want it to be (in my files).

------------------------
## Track creators

If you manage to create an interesting track, it can be included in future versions.  
More info in [this topic](https://groups.f-hub.org/d/lpntzPrD/-creating-tracks-and-their-content].  
If needed, I can finish them, may change something.  
Tracks are licensed GPLv3 by default.

------------------------
## Testers

If you found an unknown bug please report it, info [here](Troubleshooting.md).  
Even better if you're able to build from sources.  
If so, then also honest, constructive opinions or suggestions (e.g. about my latest  change or planned) are welcome, on Forum.

## Packagers

If you have experience in packaging software or games, for some OS distributions,  
or know how to improve building from sources (or make it easier).

------------------------
## Translators

If you want to help translating a language, check Wiki page [Localization](Localization.md).

------------------------
## Sound Artists

If you can record or edit sounds for games. Especially looped engine sounds.

## 3D Artists

If you know Blender and can help with importing existing models into game [Objects wiki](Objects.md).  
Or can edit vehicles ([new topic](https://groups.f-hub.org/d/adePgxzW/-cars-new-vehicles-to-do), [old topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526) and [docs page](VehicleModeling.md)) or objects for game, UV unwrap, texturing, etc.


------------------------
# Donations

Financial support is possible through [CryHam's page](https://cryham.tuxfamily.org/donate/).

------------------------
.
------------------------

# Git Workflow 

> Also new data like tracks, objects, textures, sounds etc. aren't done like this.  Just uploaded as attachments on Forum.

This page describes the procedure to get your code (fixes, patches, new features, data etc.) included into Stunt Rally.  
  This assumes you have already cloned our code and done something with it and now are ready to share your stuff.  

If you don't know what to do, but want to help, check out the repo page [Contributing](Contributing.md),  
[ToDo](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php) with programming tasks with descriptions (sort by priority P, 1 is critical, higher are less important),  
and also [Roadmap](Roadmap.md) for next version(s).

  
## Prerequisites ##

  * [Git](https://git-scm.com/) version control software
  * [GitHub](https://github.com/signup/free) account, it's free (our code is hosted there)
  * Preferably some prior Git knowledge.  There is a lot of Git tutorials on internet.

## Set up Git ##

If this is the first time you use Git, setup your name and email:

```
$ git config --global user.name "My Name"
$ git config --global user.email "myemail@example.com"
```

You also need to generate ssh keys and setup them on GitHub to push there.  
See how to on [Linux](https://help.github.com/articles/generating-ssh-keys#platform-linux) or [Windows](https://help.github.com/articles/generating-ssh-keys#platform-windows).

  
## Workflow ##

A simpler way is to make patches, but only for really small things.

Forking is made possible by GitHub and is great for first contributions.  
In the long run, we probably let you commit directly to our repo, which saves everyone's time.  
(Preferably in a branch, since CryHam doesn't like surprises, on master).

  - [Fork](https://docs.github.com/en/get-started/quickstart/fork-a-repo#forking-a-repository) our repository at GitHub (done once)
  - Clone it (done once)
  - Develop it (fix, modify or add your new stuff)
  - Test it.
  -   If you added new files use `git add`  
  - Commit your changes: `git commit -a -m "Implemented something"`   (or "Fixed a crash when..", etc.)  
  - Push the code to GitHub: `git push`
  - Create a [pull](https://help.github.com/send-pull-requests/) request on GitHub
  -   Rest.  _(or praise the Lord and pass the ammunition)_
  - Go back to 3.

> Note: it is likely that work still needs to be done before it will be accepted.  Also, it is possible that it won't be accepted by CryHam at all (if it doesn't fit SR).  Thus, it's best to discuss it before.

If you work on our repo (without fork) points 1 and 8 are gone.

>   An artificial feedback loop appears between 3-4, and sometimes between 5-7 (forgot to add),  
Point 9 appears randomly between all of others, and imaginary points 0 and 12 start to appear later.  
And none of this would happen if we had gone fishing.  

Have fun  _(usually between 3 and 4)_.  
