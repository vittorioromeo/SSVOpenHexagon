# [Open Hexagon](http://www.facebook.com/OpenHexagon) - version 2.0 RC - readme WIP #
## [by Vittorio Romeo](http://vittorioromeo.info) ##

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
to **Giuseppe Vinci**, **Jordo Matt**, **Pascal Schuster**, **Sean Pek** for beta-testing</br>
to the **SFML** [IRC channel](http://en.sfml-dev.org/forums/index.php?topic=2997.0) community for all the help

----------

## Description ##

> "You are a triangle." </br>
Fast paced, challenging, free to play, open source game.

> You control a little triangle, which can be rotated clockwise or counterclockwise. Your goal is to suvive as long as possible by dodging polygons that move towards the center. Things get more difficult as time passes!

> Features a lot of variety and full customization: users can create/modify/share levels, patterns, music, sounds, scripts, and more.

> Play in Official Mode and submit your highscores to the server. Be the first in the leaderboards!

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
Press **F2** to enter profile selection. Press **F1** to create a new profile.</br>
Press **F3** to enter the options menu.</br>
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

Version 1.92

* Added: simple **friend list** - track player names and get their position on the leaderboard
* Added: **key customization** (only via JSON currently)
* Changed: possible minor **performance optimization** with 3D enabled

Version 1.9

* Added: **VeeNext**, a new level pack with scripted levels
* Added: walls with **custom acceleration**
* Added: game can be restarted with **up arrow key**
* Fixed: minor bugs, minor text drawing performance issues
* Fixed: 3D opacity bug with black and white enabled
* Fixed: Profiles folder is created if it's missing

Version 1.81

* Added: screenshots in the menu (press F12)
* Added: server messages are now shown in the main menu
* Changed: if FPS gets below 20 in **official mode**, now the score is invalidated instead of ejecting the player to the main menu (this also happens if you take a screenshot!)
* Fixed: severe score security issue - **scores will be wiped, sorry!**

Version 1.8 

* Added: **new documentation file**: **online** - **PLEASE READ IT**
* Added: **options menu** - open it by pressing **F3** on the main menu screen
* Added: **official mode** - play the game as it was meant to be (locks some options, makes you eligible for online scoring)
* Added: **online highscores** - your scores in official mode will be automatically submitted to the server
* Added: **leaderboards** - in the main menu you'll be able to see the top 5 online scores in official mode
* Added: **online version checking** - you will be notified when a new version of Open Hexagon is available
* Added: **auto-restart** option - automatically restarts when you die (toggle it in options menu)
* Added: **screenshot** feature - press **F12** during gameplay to save "screenshot.png" in the game folder
* Fixed: **crash** on PI, Aperoigon with 3D effects enabled


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
