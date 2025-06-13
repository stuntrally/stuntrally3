# Contributing

_How to participate in developing Stunt Rally 3._

Help is welcomed.  
Thank you for all your contributions.

Even smaller things count like:
- 👥Increasing popularity, spreading info about Stunt Rally 3 on internet (with link to SR [website](https://cryham.org/stuntrally/)).
- 🪲Testing and reporting bugs ([Issues on github](https://github.com/stuntrally/stuntrally3/issues)).
- 🌏Improving a translation to a language (on [Weblate](https://hosted.weblate.org/projects/stunt-rally-3/stunt-rally-3/), [Localization](Localization.md)).
- 💭Constructive discussions and honest opinions about features, etc.
- 💡Lastly new ideas or features (since there are already *plenty* in [Tasks](Tasks.md) and [Roadmap](Roadmap.md)).


Below is a list of **bigger** focused contributions:  
(in no order, just my favorite 2 first)

------------------------
## 🖥️Programmers

If you know C++, [Ogre-Next](https://forums.ogre3d.org/viewforum.php?f=25) (3.0 and up) or shaders,  
and preferably could handle some of the [Roadmap](Roadmap.md) tasks.  
Details for tools and sources are in [Developing](Developing.md).  
Own ideas could also go into SR3, just need some discussion first.  
Most ideas are already in [Tasks](Tasks.md). Sorted by priority: 1 is most important, higher are less so.

Medium changes can go through [pull requests](https://github.com/stuntrally/stuntrally3/pulls).  
For those with more time and effort, commits to main branch will be possible.

There is no style guide (yet), I simply recommend following my own.  
A quick info: it is not random, it's quite specifically formatted to how I want it to be (in my files).

------------------------
## 🏞️Track creators

If you manage to create an interesting track, it can be included in future versions.  
More info in [this topic](tracks.md).  
If needed, I can finish them, may change something.  
Tracks are licensed GPLv3 by default.

------------------------
## 🎮Testers

If you found an unknown bug please report it, info [here](Troubleshooting.md).  
Even better if you're able to build from sources.  
If so, then also honest, constructive opinions or suggestions (e.g. about my latest  change or planned) are welcome.

## 📦Packagers

If you have experience in packaging software or games, for some OS distributions,  
or know how to improve building from sources (or make it easier).

------------------------
## 🌏Translators

If you want to help translating a language, check Wiki page [Localization](Localization.md).

------------------------
## 🔉Sound Artists

If you can record or edit sounds for games. Especially looped engine sounds.

## 🚗3D Artists

If you know Blender and can help with importing existing models into game [Objects wiki](Objects.md).  
Or can edit vehicles ([new topic](vehicles.md), [old topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526) and [docs page](VehicleModeling.md)) or objects for game, UV unwrap, texturing, etc.


------------------------
# 💜Donations

Financial support is possible through [CryHam's page](https://cryham.org/donate/) or directly on [Ko-Fi](https://ko-fi.com/cryham).

------------------------
.
------------------------

# Git Workflow 

## Intro ##

> New data like tracks, objects, textures, sounds, particles, etc. can alteratively be uploaded as attachments on Forum or else.

This section describes the procedure to get your code (fixes, patches, new features, tracks, data etc.) included into Stunt Rally.  
  This assumes you have already cloned our code and done something with it and now are ready to share your stuff.  

If you don't know what to do, but want to help, check out the repo page [Contributing](Contributing.md),  
[Tasks](Tasks.md) with programming tasks with descriptions, and also [Roadmap](Roadmap.md) for next version(s).

  
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
