_Tips for finding out and reporting bugs or crashes._

### Introduction

This page describes how to properly report bugs etc. as well as self help tips.

### Where to report problems

  * [Github Issues](https://github.com/stuntrally/stuntrally3/issues) or
  * [Bugs & Help forum](https://forum.freegamedev.net/viewforum.php?f=78)

### What we will ask from you

So how about saving your time as well as ours and providing the answers before we get a chance to ask them? You might also find the cause by yourself.

  * **Do you have an integrated Intel graphics card (e.g. a non-gaming laptop)?**
    * There is your cause for the crash - Stunt Rally 3 needs a decent GPU, nothing we can do  
    apart from recommending you to upgrade your hardware (or visiting a friend with better hardware, or borrowing it 😉
  * **What operating system are you using?**
  * What version of Stunt Rally3?
  * From where did you install SR3?
    * If you are not using the latest release version of SR3, we'll probably ask you to upgrade and test that.
    * If you aren't using our binaries, we will probably recommend trying them (unless you are already building from sources).
  * **When did the crash or bug occur?**
    * Did you get any graphics?
    * Did you get to the menu?
    * After loading a track?
    * What triggered it?
    * Does it repeat?
  * **Please post ogre.log file (or ogre_ed.log for editor)**
    * Their location is given on the [Paths](Paths.md) page. Also in Game, bottom of Help tab.
    * Use attachments if on forum
  * **Please follow up**
    * Fixing bugs might take days or longer  
    We are not always able to reproduce the problem (e.g. due to missing hardware).  
    If we can't replicate it, we rely on your testing and confirmation of fixes.

### Debugging yourself (advanced)

If you wish to be extra helpful (and are using Linux), you can provide a stack trace of the crash.

  - Install ''gdb'', the GNU Debugger
  - <del>Install ''stuntrally3-dbg'' or similar, i.e. Stunt Rally 3 with debugging symbols (if it exists).</del>
  - You could *compile the game* [(info here)](Building.md) yourself in Debug configuration, with debugging info.
  - Run SR3 through the debugger:
    * `gdb stuntrally3`
    * _you are taken to gdb prompt_
    * `run`
    * _wait for the crash_
    * `bt`
    * _stands for backtrace_
    * Copy-paste the resulting trace to use as with the log files
  