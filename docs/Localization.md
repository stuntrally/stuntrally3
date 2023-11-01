_How to translate Stunt Rally 3.x game/editor to a language._

We moved to Weblate. Link for general info: [**translating**](https://docs.weblate.org/en/latest/user/translating.html). _And for details [integration](
https://docs.weblate.org/en/latest/devel/integration.html)._

### Introduction

Stunt Rally 3 supports translating (nearly all) visible strings in Game and Editor.  
_Supplying translations, even small typo fixes, is a great way to contribute._

You can easily and conveniently translate on the [webpage](https://hosted.weblate.org/projects/stunt-rally-3/stunt-rally-3/). It has some more features.

_You can also download the translation from there and translate it locally using a tool like [PoEdit](https://sourceforge.net/projects/poedit/)._

### How to start

  - Get yourself an account on Weblate, if you don't have one already
  - Pick your language or add language if it's not there _(if you intend to translate at least 20% of it)_
  - Press Translate to start, pick unfinished strings or other from combo

### Order

There are lots of strings to translate, above 1200.  
This is the recommended order in which to translate.

  - First translate the strings that have 'HUD' or 'MAIN' in **Developer comment**  
    _This way even with few translations a language can already be useful_
  - Next translate only one word strings (for game and editor, Gui tab captions etc.)
  - Then short game strings, related to challenges etc.
  - Game Hints and other contents of Welcome screen
  - Editor Input texts (keys shown in editor help window, long but crucial for editor users)
  - Leave tooltips for last (they have 'TIP' in Developer comment and are quite long)

### General Hints

  * Try to keep your translated strings at equal length (or shorter) of the english strings,  
    to make sure they will fit into the GUI (for tooltips this doesn't matter).
  * Full translation requires time and patience, also good game and editor experience to catch the text meaning.

  * Check **Source string description:**
    * This is my comment, usually a group name for few strings telling what are they for.
    * Those having 'Main' are more important than others.
    * Usually if this comment has a long group name it's not that important as the shorter ones.
    * If you see 'Input' then this is a text for either mouse or keyboard action
    * Only few strings have some unusual longer remarks
  * ?_ToDo:_ See details, it shows more info, namely:
  * **Source string location**
    * for Gui .layout files shows in which, at which line and   **Widget name hierarchy** (parent widget captions)   ending with widget **type** on which this string is.
    * for source .cpp files shows path (inside source/), file name and at which line.
  * Context, this is a string id from xml   e.g. all input texts start with Input, tips start with Tip etc. (apart from that it's not very helpful)

### How to read location

On Weblate these are as links to source files (first occurrence in file at least) in our Git repo.

Example for Boost string:
```
Game.layout :1076..SingleRace.Game.Main.Text
 :1133..SingleRace.Game.Tab
 :2842..Options.View.Camera.Check
ogre/Challenges.cpp:596
ogre/Gui_Init.cpp:521
ogre/Gui_Network.cpp:132
```
So, this string Boost, can be found in Gui:
  * file Game.layout lines 1076,1133 and 2842
  * in 'Single Race' window, tab 'Game', subtab 'Main' as a Text
  * in 'Single Race' window, tab 'Game', as Tab caption
  * in 'Options' window, tab 'View', subtab 'Camera' as CheckBox caption

And in above mentioned source files with line numbers.

Line numbers are combined if in same file e.g.  
```
ogre/Gui_Network.cpp:29:345:567
```

### How to check your strings back

To do this you need to get the .po file back from webpage (download).  
Or if you translate with a program locally just save it and get the new .po file.  

Run the Python3 script locale/xml_po_parser.py on the .po file to get a *_tag.xml file back.  
e.g. inside locale/
```
xml_po_parser.py de.po core_language_de_tag.xml
```
Move the xml into data/gui. Start game or editor and test.

  
----


### Translation **Sync**

Synchronizing / updating repo translations in Github after changes in weblate.

This needs 2 steps:
- First is automatic, Weblate pushes after up to 1 hour, its modified `.po` files, directly to our Git repo.  
- Second is manual, needs starting **[2upd-xml.sh](../locale/2upd-xml.sh)** (or just `xml_po_all.py`) inside `locale/`  
  To convert `.po` to `*_tag.xml` files for MyGui in [Media/gui](../Media/gui/). And pushing to Git.  


### Technical Details

All original English strings in Stunt Rally 3 are in the [*_en_tag.xml file](../Media/gui/core_language_en_tag.xml),  
used by MyGUI's own translation system.  

> This file is edited and sorted manually. Each new string for Gui or Hud is added there.  
It also has xml comments with group and subgroups names,  
which then show up as Develeoper comment on web (as #. in `sr.pot` file).

After changes in the .xml file, run **[1upd-all.sh](../locale/1upd-all.sh)** inside `locale/`.

A small C++ program [sr_translator](../src/transl/main.cpp) generates `sr.pot` templates file from the .xml file.  
_It also searches for string references in sources and Gui layouts, also getting widgets hierarchy._

We use gettext's `.po` and `.pot` files for web translations, it is a very popular text format.


----

### Adding new language

See example commit adding Czech (cs) language: [link](https://github.com/stuntrally/stuntrally/commit/18018ecff5ddc27eea7d26f023e2ecea554d5e88)

Needs to be added in 4 places:

data/gui/core_language.xml
```
<Info name="cs">
  <Source>core_language_cs_tag.xml</Source>
</Info>
```

data/gui/core_language_en_tag.xml
```
<Tag name="LANG_CS">Čeština</Tag>
```
Language name in this language (found on e.g. Wikipedia).  
This will be visible in combobox on Gui.  
Note that it needs translation update after, to have the new tag in all languages.

src/common/AppGui_Init.cpp
```
else if (!strcmp(buf,"Czech")) loc = "cs";
```
English name of language from windows GetLocaleInfoA function.

src/common/GuiCom_Util.cpp
```
languages["cs"] = TR("#{LANG_CS}");
```
