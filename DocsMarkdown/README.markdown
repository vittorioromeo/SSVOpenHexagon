# [Open Hexagon](http://www.facebook.com/OpenHexagon) - version 1.4 #
## [vee software](http://veegamedev.wordpress.com/) ##

**A free, open source clone of "[Super Hexagon](https://itunes.apple.com/us/app/super-hexagon/id549027629?mt=8)" (by Terry Cavanagh). </br>**
**Programmed by Vittorio Romeo. </br>**
**Music by BOSSFIGHT.** 

----------

##EPILESPSY WARNING##
*The game may contain flashing light patterns that could trigger a seizure. Play at your own risk.*

----------

## Links and additional credits ##


**Official page:** [http://www.facebook.com/OpenHexagon](http://www.facebook.com/OpenHexagon) </br>
**Source code page:** [https://github.com/SuperV1234](https://github.com/SuperV1234)</br>
**Vittorio's blog**: [http://veegamedev.wordpress.com](http://veegamedev.wordpress.com)</br>

**Terry Cavanagh's page**: [http://distractionware.com](http://distractionware.com/) </br>
**BOSSFIGHT's page:** [http://soundcloud.com/bossfightswe](http://soundcloud.com/bossfightswe) </br>

Special thanks:</br> 
to **BubblegumBalloon** for the announcer sounds </br>
to **Dajoh** for hosting </br>
to **Tomaka17** for creating [LuaWrapper](http://code.google.com/p/luawrapper/)</br>
to **Ethan Lee** for the [Unix port](https://github.com/flibitijibibo/OpenHexagon-Unix)</br>
to **Jookia** for his help with the CMake files</br>
to **Giuseppe Vinci**, **Jordo Matt**, **Pascal Schuster**, **Sean Pek** for beta-testing

----------

## Description ##

> "You are a triangle." </br>
Fast paced, challenging, free to play, open source game.

> You control a little triangle, which can be rotated clockwise or counterclockwise. Your goal is to suvive as long as possible by dodging polygons that move towards the center. Things get more difficult as time passes!

> Features a lot of variety and full customization: users can create/modify/share levels, patterns, music, sounds, scripts, and more.

>Clone of the game "Super Hexagon" by Terry Cavanagh.</br>
Please check it out and consider buying it if you like Open Hexagon!


----------

## Requirements ##

**Visual C++ Redistributable:**
[http://www.microsoft.com/en-us/download/details.aspx?id=30679](http://www.microsoft.com/en-us/download/details.aspx?id=30679)


----------

## Installation ##

Simply extract the contents of the compressed file in a folder.</br>
Open **SSVOpenHexagon.exe** or any of the batch files to play.

*If you need further assistance post on the official page:*
[http://www.facebook.com/OpenHexagon](http://www.facebook.com/OpenHexagon)

----------

## How to play ##

In the menu, press the **Left** and **Right** arrow keys to choose levels.</br>
Press **Enter** to play.</br>
Keep **ESC** pressed to quit the game.

In game, press the **Left** and **Right** arrow keys to rotate.</br>
Press **SHIFT** to rotate more slowly.</br>
Avoid the walls! Press **R** to restart.</br> Press **ESC** to go back to the menu.

----------

## Customization ##

Customizing Open Hexagon can be very easy or very complex depending on what you're trying to do. </br>Almost everything is written in simple to edit [JSON](http://www.json.org/).

Patterns, however, must be created using [LUA](http://www.lua.org/) (which can also be used for other cool stuff).

Please refer to the files in the **documentation** folder.

*Keep in mind that things will change from version to version. Existing files may not work in the next version of Open Hexagon or may work differently.*

*If you need further assistance post on the official page:*
[http://www.facebook.com/OpenHexagon](http://www.facebook.com/OpenHexagon)

----------


## Changelog ##

Version 1.4

* Removed: scripted events from default levels (the flow isn't interrupted anymore)
* Changed: pseudo-3D effects are now disabled by default

Version 1.3a *(experimental version)*

* Added: **automatic difficulty variants** (select in menu with up/down arrow key) - scores are not shared between difficulty variants!
* Changed: balanced default levels to feel more like a natural progression
* Fixed: random side changing now happens as soon as possible
* Added: **LUA** hardcoded functions for levels (`onLoad`, `onStep`, `onUnload`, `onIncrement`) - **these are REQUIRED in level script files**
* Added: **LUA** `log(mLog)` function, which sends a message to the console
* Fixed: **LUA** context now gets reset every time you start/restart a level
* Added: **3D effects** (customizable in the level file) - they can be disabled from config.json
* Added: `"rotation_speed_max"` level parameter
* Changed: messages now show only the first time you play the level (not on restart)
* Added: `"message_important_add"` event - it shows even if you restart the level
* Added: `playSound(mId)` **LUA** command
* Added: `forceIncrement()` **LUA** command
* Added: `messageAdd(mMessage, mDuration)` **LUA** command
* Added: `messageImportantAdd(mMessage, mDuration)` **LUA** command
* Added: `getDifficultyMult()` **LUA** command
* Added: new level and **Commando Steve** song
* Fixed: impossible wall bug (?)

Version 1.2

* Added: completely new **JSON event scripting system**
* Added: completely new **LUA scripting** for patterns
* Added: **new announcer sounds** by **BubblegumBalloon**! (thank you)
* Added: **save profiles** for scores
* Added: new event scripting commands (`"play_sound"`, LUA-related)
* Added: new song, **Captain Cool**
* Added: new level, **Golden Ratio**

Version 1.11

* Added: readme files for customization 
* Added: credits in menu screen
* Added: JSON file for sound customization

Version 1.1

* Fixed: input being registered when the game was not in focus
* Added: config overrides (JSON files that override certain config parameters)
* Added: .bat files for WINDOWED and FULLSCREEN modes
* Changed: JSON name members for windowed/fullscreen config parameters
* Fixed: sudden side number changing bug (now it waits until all the obstacles are removed)
* Fixed: impossible pattern bug
* Added: another tutorial level
* Added: new style (zen2)
