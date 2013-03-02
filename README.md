# [Open Hexagon](http://www.facebook.com/OpenHexagon) - version 1.7 #
## [vee software](http://veegamedev.wordpress.com/) ##

A free, open source clone of **[Super Hexagon](https://itunes.apple.com/us/app/super-hexagon/id549027629?mt=8)** (by Terry Cavanagh) </br>
Programmed by **[Vittorio Romeo](http://vittorioromeo.info)</br>**
Music by **[BOSSFIGHT](https://soundcloud.com/bossfightswe)</br>**

----------

##EPILESPSY WARNING##
*The game may contain flashing light patterns that could trigger a seizure. Play at your own risk.*

----------

## Links and additional credits ##


**Official page:** [http://www.facebook.com/OpenHexagon](http://www.facebook.com/OpenHexagon) </br>
**Source code page:** [https://github.com/SuperV1234](https://github.com/SuperV1234)</br>
**Vittorio's website**: [http://vittorioromeo.info](http://vittorioromeo.info)</br>

Special thanks:</br> 
to **BubblegumBalloon** for the announcer sounds </br>
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

Version 1.7

* Added: **3D effects** (can be customized in style JSON files) (can be enabled/disabled/tuned in config.json file)
* Added: **antialiasing**
* Added: new default level, **PI**
* Added: invincibility JSON config variable for debugging
* Added: **camera shake effect** on death
* Added: **new main menu**
* Fixed: "renderTexture is too big" bug
* Fixed: spinning bug in Aperoigon
* Fixed: unable to use '0' character in profile names
* Fixed: a lot of minor bugs and a game crash
* Changed: custom sounds now require extension to play via LUA/JSON scripts (previous levels will need to be updated!)

Version 1.6

* Fixed: impossible patterns with extreme difficulty multipliers (should be fixed) - **getPerfectDelayDM** is a new function that takes difficulty multiplier into account
* Fixed: outdated level documentation file
* Added: flash effect on death
* Added: pulse effect option is saved when closing the game
* Added: **mouse-button control** (use left/right buttons to spin)
* Added: **custom sounds** for level packs (they work differently from normal sounds, please refer to the sounds documentation)

Version 1.52

* Fixed: non-folders in Packs/ directory caused crash
* Fixed: impossible wall bug with low difficulty multipliers

Version 1.51

* Fixed: crash on tutorial level
* Added: Space and Enter keys now restart the game (only if you're already dead)

Version 1.5

* Fixed: player death position now displays more accurately
* Added: **pulsing effect** (can be disabled right in the main menu)
* Added: **level packs** - sharing and installing levels is now really easy, just place the level folder in the Packs directory (unfortunately, scores are reset)
* Added: additional shortcuts for menu options (requested by people having troubles with the function keys)
* Changed: toned down difficulty multiplier's effect
* Changed: difficulty multipliers has now a wider range
* Changed: balanced default levels
* Added: new hard endurance level, **labyrinth**
* Changed: delay multiplier now has an effect
* Fixed: minor **LUA** bugs

Version 1.4

* Changed: **LUA** file execution errors do not crash the game anymore - they display an error in the console and kill the player
* Changed: **LUA** runtime execution errors do not crash the game anymore - they display an error in the console and try continuing the game
* Removed: scripted events from default levels (the flow isn't interrupted anymore)
* Removed: experimental pseudo-3D effects
* Fixed: index calculation bug in default pattern **LUA** files (thanks Sean Pek!)
* Fixed: level rotation always in the same direction
* Fixed: `getPerfectDelay()` not returning the correct values
* Added: **LUA** **REQUIRED** level function onUpdate(mFrameTime), which is called every frame
* Added: **LUA** `isKeyPressed(mKey)` command, as requested - it returns true if `mKey` is pressed
* Changed: logging is only enabled in debug mode, which can be set in config.json or by using the `debug.bat` file, which loads the debug config override

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
