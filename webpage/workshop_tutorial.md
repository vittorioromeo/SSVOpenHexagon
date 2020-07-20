# Workshop Tutorial

This document will guide you through the process of creating a new Open Hexagon level pack and upload it on the Steam Workshop.



## Getting Started

### Prerequisites

Open Hexagon uses [**Lua**](http://www.lua.org/) as the main scripting language for custom levels, alongside [**JSON**](https://www.json.org/json-en.html) for level metadata and configuration files.

It is recommended to be somewhat familiar with both languages before diving into Open Hexagon content creation, but if you're feeling brave and have previous programming experience you can just try to "wing it" by modifying existing levels and learning on the fly. A quick and fun way of learning the syntax of the above languages is the "Learn X in Y minutes" website:

- [Learn X in Y minutes: Lua](https://learnxinyminutes.com/docs/lua/)

- [Learn X in Y minutes: JSON](https://learnxinyminutes.com/docs/json/)

### Content Structure

Open Hexagon custom content is organized in the following hierarchy:

- **Level Pack** *(folder)*
  - **`pack.json`** *(main JSON metadata file)*
  - **Levels** *(folder containing level JSON metadata files)*
    - **`mylevel.json`**
  - **Music** *(folder containing music `.ogg` and JSON metadata files)*
    - **`mysong.ogg`**
    - **`mysong.json`**
  - **Scripts** *(folder containing `.lua` scripts)*
    - **`mylevel.lua`**
  - **Styles** *(folder containing visual style JSON metadata files)*
    - **`mystyle.json`**

Therefore, the main unit of work that can be published and installed is a **pack**. Inside a pack, you can have an arbitrary number of **levels**, **musics**, **scripts**, and **styles**. A completed level requires a **level `.json` metadata file**, **music `.ogg` file**, **music `.json` metadata file**, **level `.lua` script**, and a **style `.json` file**. All the separate files are linked together by **textual identifiers**, specified in the `.json` metadata files.

## Creating your First Custom Level

The easiest way to get started is to subscribe to the ["Example Workshop Level
"](https://steamcommunity.com/sharedfiles/filedetails/?id=2157745755) on the Steam Workshop, navigate to its contents (usually `C:/Program Files (x86)/Steam/steamapps/workshop/content/1358090/2157745755`), and copy-paste that folder into your `Open Hexagon/Packs/` folder[^where_is_open_hexagon_folder].

After that, rename the pasted `2157745755` to something else (e.g. `MyFirstPack`), and start exploring its contents with your favorite text editor (personally, I use [Visual Studio Code](https://code.visualstudio.com/)).

Delete the `Example Workshop Level.workshop.json` file (as it will be generated automatically when you upload your pack to the workshop), and the preview image `.png` file, as you will provide one yourself later.

Start by taking a look at `pack.json` - there are multiple fields to change. Note that Open Hexagon expects every single level pack to have a unique id - therefore, to minimize the chance of collisions between packs made by different authors, you should specify a sufficiently unique `"disambiguator"` that is very unlikely someone else might accidentally use. Such field will not be displayed in game, so it can be completely arbitrary. The rest of the fields also participate in the calculation of the final pack id, so all of your packs can share the same `"disambiguator"`, provided that they do have different names (as they rightfully should). The `"version"` field is useful for keeping track of changes and updates of your custom level pack. You should bump it up by one whenever you make changes to your pack and re-upload it to the workshop.

After that, you should explore the other folders and see how things work. The `.json` and `.lua` files are commented, so you should have enough context to figure out how things are linked together and what role they play.

The level logic is stored in `Scripts/Levels/examplelevel.lua`. Please refer to the [Lua Reference](#lua-reference) to figure out what the used functions do, and to discover new functions to play around with. You will notice a bunch of `u_execScript` calls at the top of the Lua file - these are other scripts that the level script depends on. Dependencies should always be provided as part of your pack folder[^will_dependency_management_change].

Now it's up to you! Explore the dependencies, experiment with the code, add your own music[^music_copyright_issues], and - when you're satisfied - share your pack on the workshop and ask for feedback from other players, in order to perfect it. Keep in mind that you can join our official Discord before or during level development to ask for help and share your ideas - see [Where to get Help](#where-to-get-help) for more information.

[^will_dependency_management_change]: It is possible that in the future I will implement a solution to share dependencies between different packs, but that is not available yet and it is not a priority. For now, think of a level pack as a standalone self-contained entity.

[^music_copyright_issues]: Please ensure that the music files you upload to the Steam Workshop are not protected by copyright laws. If they are, you should get explicit permission from the copyright owner for them to be used as a custom Open Hexagon level, and always credit the original artists as part of your workshop item description.



## Uploading to the Steam Workshop

The current recommended way of uploading your level to the Steam Workshop is to use the provided `OHWorkshopUploader.exe` command-line tool available in your Open Hexagon folder[^where_is_open_hexagon_folder].

[^where_is_open_hexagon_folder]: If you are not sure where Open Hexagon is installed, you can navigate to the folder by right-clicking on Open Hexagon in your Steam library, selecting "Properties", going to the "Local Files" tab, and then clicking on the "Browse Local Files" button.

When running the tool, you'll see a few options:

```text
Enter one of the following options:
0. Create a new workshop item.
1. Upload contents of an existing workshop item.
2. Set preview image of an existing workshop item.
3. Exit
```

If you intend to upload a brand new level, then write `0` and press *enter*. On success, the program will give you back a workshop item id:

```text
Successfully created new workshop item: 2165863494.
```

Copy that id to your clipboard as you're going to need it for later. Once you have finished creating and testing your custom level, you should have a folder under `Packs/` containing all its data. To upload it, write `1` and press *enter*. You will be prompted to enter the workshop item id you want to upload data to - you should now paste the id you previously copied[^what_id_to_use].

[^what_id_to_use]: Or any existing id for a workshop item you own, if you intend to update it rather than creating a new one.

You will then be prompted to enter the path to the folder containing the data. Please insert an absolute path to the pack folder (e.g. `C:/MySteamLibrary/common/Open Hexagon/Packs/MyFirstPack`) and keep following the instructions on screen. Your data should now be uploaded to the workshop.

Finally, you can write `2` and press *enter* to add a preview image to your workshop item. A 620x620 `.png` image is recommended for this task - you will need to provide an absolute path to the image similarly to what you have done in the previous step.



# Lua Reference {#lua-reference}

Below is the full reference for all of the Lua functions that you can use in the entire game.

## Core Functions

Below are the Core Functions, which are the base functions that help the game assemble the level together and make the level playable. Some of these functions are required and they must be implemented in order for your level to work, while others are completely optional and can be implemented to help assist you in completing certain tasks.

* **`onInit()` (Required)**: A core function that is called when the level is first selected upon in the menu. The Lua function is also ran upon every attempt to reset the configuration. Use this function to initialize all of the level settings and setup some parameters for the level, such as tracking variables and setting conditions with the menu selection (such as setting difficulty multiplier thresholds).

* **`onLoad()` (Required)**: A core function that is called when the level begins. Use this function for basic event handling, such as timing messages and setting an end to the level with `u_eventKill`. However, more advanced events should be handled with other Lua measures (or with the currently WIP "Events for 2.0" module). You can also initialize some values that couldn't be initialized with `onInit`.

* **`onStep()` (Required)**: A core function that is called every time when the level timeline is empty. This is the core function where all of your pattern/wall spawning logic should be happening, as this is the intended design of the function. When a level increment is called, this function temporarily stops running until the level increments successfully. 

* **`onIncrement()` (Required)**: A core function that is called every time the level increments. It doesn't run the moment the increment is called, as it has to wait for all of the walls to be cleared before this function can run. Use this to implement any custom incrementation logic.

* **`onUnload()` (Required)**: A core function that is called every time when the player exits the level or restarts the level. This is hardly ever used in pack developing, but some uses for it can be to help run certain events only on first playthrough of the level.

* **`onUpdate(float mFrameTime)` (Required)**: A core function that is called every frame throughout the level, with the argument `mFrameTime` being a float representing the time elapsed for a single frame. This function can be very useful to solve logic that would otherwise be impossible to do with other core functions, but be careful to not put too much code into this function. Putting too much code in this function (or putting faulty code) can be a contributing factor to causing lag, so be sure to use this function wisely.

* **`onCursorSwap()`**: A core function that is called every time the player swaps.

* **`onDeath()`**: A core function that is called when the player dies. This function does not count invincibility deaths, and they must be actual deaths for this function to be called.

<!-- START GENERATED DOCS HERE -->
<!-- Generated from Open Hexagon v2.01. -->
<!-- TODO: Add callbacks. -->
## Utility Functions (u_)

Below are the utility functions, which can be identified with the "u_" prefix. These are overall functions that either help utilize the game engine, be incredibly beneficial to pack developers, or help simplify complex calculations.

* **`void u_log(string message)`**: Print out `message` to the console.

* **`void u_execScript(string scriptFilename)`**: Execute the script located at `<pack>/Scripts/scriptFilename`.

* **`void u_playSound(string soundId)`**: Play the sound with id `soundId`. The id must be registered in `assets.json`, under `"soundBuffers"`.

* **`void u_setMusic(string musicId)`**: Stop the current music and play the music with id `musicId`. The id is defined in the music `.json` file, under `"id"`.

* **`void u_setMusicSegment(string musicId, int segment)`**: Stop the current music and play the music with id `musicId`, starting at segment `segment`. Segments are defined in the music `.json` file, under `"segments"`.

* **`void u_setMusicSeconds(string musicId, float time)`**: Stop the current music and play the music with id `musicId`, starting at time `time` (in seconds).

* **`bool u_isKeyPressed(int keyCode)`**: Return `true` if the keyboard key with code `keyCode` is being pressed, `false` otherwise. The key code must match the definition of the SFML `sf::Keyboard::Key` enumeration.

* **`void u_haltTime(double duration)`**: Pause the game timer for `duration` seconds.

* **`void u_timelineWait(double duration)`**: *Add to the main timeline*: wait for `duration` frames (assuming 60 FPS framerate).

* **`void u_clearWalls()`**: Remove all existing walls.

* **`float u_getPlayerAngle()`**: Return the current angle of the player, in radians.

* **`void u_setPlayerAngle(float angle)`**: Set the current angle of the player to `angle`, in radians.

* **`bool u_isMouseButtonPressed(int buttonCode)`**: Return `true` if the mouse button with code `buttonCode` is being pressed, `false` otherwise. The button code must match the definition of the SFML `sf::Mouse::Button` enumeration.

* **`bool u_isFastSpinning()`**: Return `true` if the camera is currently "fast spinning", `false` otherwise.

* **`void u_forceIncrement()`**: Immediately force a difficulty increment, regardless of the chosen automatic increment parameters.

* **`void u_kill()`**: *Add to the main timeline*: kill the player.

* **`void u_eventKill()`**: *Add to the event timeline*: kill the player.

* **`float u_getDifficultyMult()`**: Return the current difficulty multiplier.

* **`float u_getSpeedMultDM()`**: Return the current speed multiplier, adjusted for the chosen difficulty multiplier.

* **`float u_getDelayMultDM()`**: Return the current delay multiplier, adjusted for the chosen difficulty multiplier.

* **`void u_swapPlayer(bool playSound)`**: Force-swaps (180 degrees) the player when invoked. If `playSound` is `true`, the swap sound will be played.


## Message Functions (m_)

Below are the message functions, which can be identified with the "m_" prefix. These functions are capable of displaying custom messages on the screen which can help pack developers communicate additional info or anything else useful (e.g Song Lyrics) to the players. For keeping track of statistics, please look at `l_addTracked`.

* **`void m_messageAdd(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level.

* **`void m_messageAddImportant(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will be printed during every run of the level.

* **`void m_messageAddImportantSilent(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during every run of the level, and will not produce any sound.

* **`void m_clearMessages()`**: Remove all previously scheduled messages.


## Main Timeline Functions (t_)

Below are the main timeline functions, which can be identified with the "t_" prefix. These are functions that have effects on the main timeline itself, but they mainly consist of waiting functions. Using these functions helps time out your patterns and space them in the first place.

* **`void t_wait(double duration)`**: *Add to the main timeline*: wait for `duration` frames (under the assumption of a 60 FPS frame rate).

* **`void t_waitS(double duration)`**: *Add to the main timeline*: wait for `duration` seconds.

* **`void t_waitUntilS(double duration)`**: *Add to the main timeline*: wait until the timer reaches `duration` seconds.


## Event Timeline Functions (e_)

Below are the event timeline functions, which can be identified with the "e_" prefix. These are functions that are similar to the Main Timeline functions, but they instead are for the event timeline as opposed to the main timeline. Use these functions to help set up basic events for the game. More advanced events must be done with pure Lua.

* **`void e_eventStopTime(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` frames (under the assumption of a 60 FPS frame rate).

* **`void e_eventStopTimeS(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` seconds.

* **`void e_eventWait(double duration)`**: *Add to the event timeline*: wait for `duration` frames (under the assumption of a 60 FPS frame rate).

* **`void e_eventWaitS(double duration)`**: *Add to the event timeline*: wait for `duration` seconds.

* **`void e_eventWaitUntilS(double duration)`**: *Add to the event timeline*: wait until the timer reaches `duration` seconds.


## Level Functions (l_)

Below are the level functions, which can be identified with the "l_" prefix. These are functions that have a role in altering the level mechanics themselves, including all level properties and attributes. These typically get called en masse in `onInit` to initialize properties.

* **`float l_getSpeedMult()`**: Gets the speed multiplier of the level. The speed multiplier is the current speed of the walls. Is incremented by ``SpeedInc`` every increment and caps at ``speedMax``.

* **`void l_setSpeedMult(float value)`**: Sets the speed multiplier of the level to `value`. Changes do not apply to all walls immediately, and changes apply as soon as the next wall is created.

* **`float l_getSpeedInc()`**: Gets the speed increment of the level. This is applied every level increment to the speed multiplier. Incrementation is additive.

* **`void l_setSpeedInc(float value)`**: Sets the speed increment of the level to `value`.

* **`float l_getSpeedMax()`**: Gets the maximum speed of the level. This is the highest that speed can go; speed can not get any higher than this.

* **`void l_setSpeedMax(float value)`**: Sets the maximum speed of the level to `value`. Keep in mind that speed keeps going past the speed max, so setting a higher speed max may make the speed instantly increase to the max.

* **`float l_getRotationSpeed()`**: Gets the rotation speed of the level. Is incremented by ``RotationSpeedInc`` every increment and caps at ``RotationSpeedMax``.

* **`float l_getRotationSpeedInc()`**: Gets the rotation speed increment of the level. This is applied every level increment to the rotation speed. Incrementation is additive.

* **`void l_setRotationSpeedInc(float value)`**: Sets the rotation speed increment of the level to `value`. Is effective on the next level increment.

* **`float l_getRotationSpeedMax()`**: Gets the maximum rotation speed of the level. This is the highest that rotation speed can go; rotation speed can not get any higher than this.

* **`void l_setRotationSpeedMax(float value)`**: Sets the maximum rotation speed of the level to `value`. Keep in mind that rotation speed keeps going past the max, so setting a higher rotation speed max may make the rotation speed instantly increase to the max.

* **`float l_getDelayMult()`**: Gets the delay multiplier of the level. The delay multiplier is the multiplier used to assist in spacing patterns, especially in cases of higher / lower speeds.  Is incremented by ``DelayInc`` every increment and is clamped between ``DelayMin`` and ``DelayMax``

* **`void l_setDelayMult(float value)`**: Sets the delay multiplier of the level to `value`. Changes do not apply to patterns immediately, and changes apply as soon as the next pattern is spawned.

* **`float l_getDelayInc()`**: Gets the delay increment of the level. This is applied every level increment to the delay multiplier. Incrementation is additive.

* **`void l_setDelayInc(float value)`**: Sets the delay increment of the level to `value`.

* **`float l_getDelayMin()`**: Gets the minimum delay of the level. This is the lowest that delay can go; delay can not get any lower than this.

* **`void l_setDelayMin(float value)`**: Sets the minimum delay of the level to `value`. Keep in mind that delay can go below the delay min, so setting a lower delay min may make the delay instantly decrease to the minimum.

* **`float l_getDelayMax()`**: Gets the maximum delay of the level. This is the highest that delay can go; delay can not get any higher than this.

* **`void l_setDelayMax(float value)`**: Sets the maximum delay of the level to `value`. Keep in mind that delay can go above the delay max, so setting a higher delay max may make the delay instantly increase to the maximum.

* **`float l_getFastSpin()`**: Gets the fast spin of the level. The fast spin is a brief moment that starts at level incrementation where the rotation increases speed drastically to try and throw off the player a bit. This speed quickly (or slowly, depending on the value) decelerates and fades away to the  updated rotation speed.

* **`void l_setFastSpin(float value)`**: Sets the fast spin of the level to `value`. A higher value increases intensity and duration of the fast spin.

* **`float l_getIncTime()`**: Get the incrementation time (in seconds) of a level. This is the length of a "level" in an Open Hexagon level (It's ambiguous but hopefully you understand what that means), and when this duration is reached, the level increments.

* **`void l_setIncTime(float value)`**: Set the incrementation time (in seconds) of a level to `value`.

* **`float l_getPulseMin()`**: Gets the minimum value the pulse can be. Pulse gives variety in the wall speed of the level so the wall speed doesn't feel monotone. Can also be used to help sync a level up with it's music.

* **`void l_setPulseMin(float value)`**: Sets the minimum pulse value to `value`.

* **`float l_getPulseMax()`**: Gets the maximum value the pulse can be. Pulse gives variety in the wall speed of the level so the wall speed doesn't feel monotone. Can also be used to help sync a level up with it's music.

* **`void l_setPulseMax(float value)`**: Sets the maximum pulse value to `value`.

* **`float l_getPulseSpeed()`**: Gets the speed the pulse goes from ``PulseMin`` to ``PulseMax``. Can also be used to help sync a level up with it's music.

* **`void l_setPulseSpeed(float value)`**: Gets the speed the pulse goes from ``PulseMin`` to ``PulseMax`` by `value`. Can also be used to help sync a level up with it's music.

* **`float l_getPulseSpeedR()`**: Gets the speed the pulse goes from ``PulseMax`` to ``PulseMin``.

* **`void l_setPulseSpeedR(float value)`**: Gets the speed the pulse goes from ``PulseMax`` to ``PulseMin`` by `value`. Can also be used to help sync a level up with it's music.

* **`float l_getPulseDelayMax()`**: Gets the delay the level has to wait before it begins another pulse cycle.

* **`void l_setPulseDelayMax(float value)`**: Sets the delay the level has to wait before it begins another pulse cycle with `value`.

* **`float l_getPulseDelayHalfMax()`**: Gets the delay the level has to wait before it begins pulsing from ``PulseMax`` to ``PulseMin``.

* **`void l_setPulseDelayHalfMax(float value)`**: Gets the delay the level has to wait before it begins pulsing from ``PulseMax`` to ``PulseMin`` with `value`.

* **`float l_getBeatPulseMax()`**: Gets the maximum beatpulse size of the polygon in a level. This is the highest value that the polygon will "pulse" in size. Useful for syncing the level to the music.

* **`float l_getBeatPulseDelayMax()`**: Gets the delay for how fast the beatpulse pulses in frames (assuming 60 FPS logic). This paired with ``BeatPulseMax`` will be useful to help sync a level with the music that it's playing.

* **`void l_setBeatPulseDelayMax(float value)`**: Sets the delay for how fast the beatpulse pulses in `value` frames (assuming 60 FPS Logic).

* **`float l_getBeatPulseInitialDelay()`**: Gets the initial delay before beatpulse begins pulsing. This is very useful to use at the very beginning of the level to assist syncing the beatpulse with the song.

* **`void l_setBeatPulseInitialDelay(float value)`**: Sets the initial delay before beatpulse begins pulsing to `value`. Highly discouraged to use this here. Use this in your music JSON files.

* **`float l_getBeatPulseSpeedMult()`**: Gets how fast the polygon pulses with the beatpulse. This is very useful to help keep your level in sync with the music.

* **`void l_setBeatPulseSpeedMult(float value)`**: Sets how fast the polygon pulses with beatpulse to `value`.

* **`float l_getRadiusMin()`**: Gets the minimum radius of the polygon in a level. This is used to determine the absolute size of the polygon in the level.

* **`void l_setRadiusMin(float value)`**: Sets the minimum radius of the polygon to `value`. Use this to set the size of the polygon in the level, not ``BeatPulseMax``.

* **`float l_getWallSkewLeft()`**: Gets the Y axis offset of the top left vertex in all walls.

* **`void l_setWallSkewLeft(float value)`**: Sets the Y axis offset of the top left vertex to `value` in all newly generated walls. If you would like to have more individual control of the wall vertices, please use the custom walls system under the prefix ``cw_``.

* **`float l_getWallSkewRight()`**: Gets the Y axis offset of the top right vertex in all walls.

* **`void l_setWallSkewRight(float value)`**: Sets the Y axis offset of the top right vertex to `value` in all newly generated walls. If you would like to have more individual control of the wall vertices, please use the custom walls system under the prefix ``cw_``.

* **`float l_getWallAngleLeft()`**: Gets the X axis offset of the top left vertex in all walls.

* **`void l_setWallAngleLeft(float value)`**: Sets the X axis offset of the top left vertex to `value` in all newly generated walls. If you would like to have more individual control of the wall vertices, please use the custom walls system under the prefix ``cw_``.

* **`float l_getWallAngleRight()`**: Gets the X axis offset of the top right vertex in all walls.

* **`void l_setWallAngleRight(float value)`**: Sets the X axis offset of the top right vertex to `value` in all newly generated walls. If you would like to have more individual control of the wall vertices, please use the custom walls system under the prefix ``cw_``.

* **`bool l_get3dRequired()`**: Gets whether 3D must be enabled in order to have a valid score in this level. By default, this value is ``false``.

* **`void l_set3dRequired(bool value)`**: Sets whether 3D must be enabled to `value` to have a valid score. Only set this to ``true`` if your level relies on 3D effects to work as intended.

* **`int l_getCameraShake()`**: Gets the intensity of the camera shaking in a level.

* **`void l_setCameraShake(int value)`**: Sets the intensity of the camera shaking in a level to `value`. This remains permanent until you either set this to 0 or the player dies.

* **`unsigned int l_getSides()`**: Gets the current number of sides on the polygon in a level.

* **`void l_setSides(unsigned int value)`**: Sets the current number of sides on the polygon to `value`. This change happens immediately and previously spawned walls will not adjust to the new side count.

* **`unsigned int l_getSidesMax()`**: Gets the maximum range that the number of sides can possibly be at random. ``enableRndSideChanges`` must be enabled for this property to have any use.

* **`void l_setSidesMax(unsigned int value)`**: Sets the maximum range that the number of sides can possibly be to `value`.

* **`unsigned int l_getSidesMin()`**: Gets the minimum range that the number of sides can possibly be at random. ``enableRndSideChanges`` must be enabled for this property to have any use.

* **`void l_setSidesMin(unsigned int value)`**: Sets the minimum range that the number of sides can possibly be to `value`.

* **`bool l_getSwapEnabled()`**: Gets whether the swap mechanic is enabled for a level. By default, this is set to ``false``.

* **`void l_setSwapEnabled(bool value)`**: Sets the swap mechanic's availability to `value`.

* **`bool l_getTutorialMode()`**: Gets whether tutorial mode is enabled. In tutorial mode, players are granted invincibility from dying to walls. This mode is typically enabled whenever a pack developer needs to demonstrate a new concept to the player so that way they can easily learn the new mechanic/concept. This invincibility will not count towards invalidating a score, but it's usually not important to score on a tutorial level. By default, this is set to ``false``.

* **`void l_setTutorialMode(bool value)`**: Sets tutorial mode to `value`. Remember, only enable this if you need to demonstrate a new concept for players to learn, or use it as a gimmick to a level.

* **`bool l_getIncEnabled()`**: Gets whether the level can increment or not. This is Open Hexagon's way of establishing a difficulty curve in the level and set a sense of progression throughout the level. By default, this value is set to ``true``.

* **`void l_setIncEnabled(bool value)`**: Toggles level incrementation to `value`. Only disable this if you feel like the level can not benefit from incrementing in any way.

* **`bool l_getDarkenUnevenBackgroundChunk()`**: Gets whether the ``Nth`` panel of a polygon with ``N`` sides (assuming ``N`` is odd) will be darkened to make styles look more balanced. By default, this value is set to ``true``, but there can be styles where having this darkened panel can look very unpleasing.

* **`void l_setDarkenUnevenBackgroundChunk(bool value)`**: Sets the darkened panel to `value`.

* **`size_t l_getCurrentIncrements()`**: Gets the current amount of times the level has incremented. Very useful for keeping track of levels.

* **`void l_setCurrentIncrements(size_t value)`**: Sets the current amount of times the level has incremented to `value`. This function is utterly pointless to use unless you are tracking this variable.

* **`void l_enableRndSideChanges(bool enabled)`**: Toggles random side changes to `enabled`, (not) allowing sides to change between ``SidesMin`` and ``SidesMax`` inclusively every level increment.

* **`void l_addTracked(string variable, string name)`**: Add the variable `variable` to the list of tracked variables, with name `name`. Tracked variables are displayed in game, below the game timer.

* **`void l_setRotation(float angle)`**: Set the background camera rotation to `angle` radians.

* **`float l_getRotation()`**: Return the background camera rotation, in radians.

* **`double l_getLevelTime()`**: Get the current game timer value, in seconds.

* **`bool l_getOfficial()`**: Return `true` if "official mode" is enabled, `false` otherwise.


## Style Functions (s_)

Below are the style functions, which can be identified with the "s_" prefix. These are functions that have a role in altering the attributes of the current style that is on the level. Style attributes, unlike level attributes, do not get initialized in Lua and rather are premade in a JSON file (but this is subject to change).

* **`float s_getHueMin()`**: Gets the minimum value for the hue range of a level style. The hue attribute is an important attribute that is dedicated specifically to all colors that have the ``dynamic`` property enabled.

* **`void s_setHueMin(float value)`**: Sets the minimum value for the hue range to `value`. Usually you want this value at 0 to start off at completely red.

* **`float s_getHueMax()`**: Gets the maximum value for the hue range of a level style. Only applies to all colors with the ``dynamic`` property enabled.

* **`void s_setHueMax(float value)`**: Sets the maximum value for the hue range to `value`. Usually you want this value at 360 to end off at red, to hopefully loop the colors around.

* **`float s_getHueInc()`**: Alias to ``s_getHueIncrement``. Done for backwards compatibility.

* **`void s_setHueInc(float value)`**: Alias to ``s_setHueIncrement``. Done for backwards compatibility.

* **`float s_getHueIncrement()`**: Gets how fast the hue increments from ``HueMin`` to ``HueMax``. The hue value is added by this value every 1/60th of a second.

* **`void s_setHueIncrement(float value)`**: Sets how fast the hue increments from ``HueMin`` to ``HueMax`` by `value`. Be careful with high values, as this can make your style induce epileptic seizures.

* **`float s_getPulseMin()`**: Gets the minimum range for the multiplier of the ``pulse`` attribute in style colors. By default, this value is set to 0.

* **`void s_setPulseMin(float value)`**: Sets the minimum range for the multiplier of the ``pulse`` attribute to `value`.

* **`float s_getPulseMax()`**: Gets the maximum range for the multiplier of the ``pulse`` attribute in style colors. By default, this value is set to 0, but ideally it should be set to 1.

* **`void s_setPulseMax(float value)`**: Sets the maximum range for the multiplier of the ``pulse`` attribute to `value`.

* **`float s_getPulseInc()`**: Alias to ``s_getPulseIncrement``. Done for backwards compatibility.

* **`void s_setPulseInc(float value)`**: Alias to ``s_setPulseIncrement``. Done for backwards compatibility.

* **`float s_getPulseIncrement()`**: Gets how fast the pulse increments from ``PulseMin`` to ``PulseMax``. The pulse value is added by this value every 1/60th of a second.

* **`void s_setPulseIncrement(float value)`**: Sets how fast the pulse increments from ``PulseMin`` to ``PulseMax`` by `value`. Be careful with high values, as this can make your style induce epileptic seizures.

* **`bool s_getHuePingPong()`**: Gets whether the hue should go ``Start-End-Start-End`` or ``Start-End, Start-End`` with the hue cycling.

* **`void s_setHuePingPong(bool value)`**: Toggles ping ponging in the hue cycling (``Start-End-Start-End``) with `value`.

* **`float s_getMaxSwapTime()`**: Gets the amount of time that has to pass (in 1/100th of a second) before the background color offset alternates. The background colors by default alternate between 0 and 1. By default, this happens every second.

* **`void s_setMaxSwapTime(float value)`**: Sets the amount of time that has to pass (in 1/100th of a second) to `value` before the background color alternates.

* **`float s_get3dDepth()`**: Gets the current amount of 3D layers that are present in the style.

* **`void s_set3dDepth(float value)`**: Sets the amount of 3D layers in a style to `value`.

* **`float s_get3dSkew()`**: Gets the current value of where the 3D skew is in the style. The Skew is what gives the 3D effect in the first place, showing the 3D layers and giving the illusion of 3D in the game.

* **`void s_set3dSkew(float value)`**: Sets the 3D skew at value `value`.

* **`float s_get3dSpacing()`**: Gets the spacing that is done between 3D layers. A higher number leads to more separation between layers.

* **`void s_set3dSpacing(float value)`**: Sets the spacing between 3D layers to `value`.

* **`float s_get3dDarkenMult()`**: Gets the darkening multiplier applied to the 3D layers in a style. This is taken from the ``main`` color.

* **`void s_set3dDarkenMult(float value)`**: Sets the darkening multiplier to `value` for the 3D layers.

* **`float s_get3dAlphaMult()`**: Gets the alpha (transparency) multiplier applied to the 3D layers in a style. Originally references the ``main`` color.

* **`void s_set3dAlphaMult(float value)`**: Sets the alpha multiplier to `value` for the 3D layers. A higher value makes the layers more transparent.

* **`float s_get3dAlphaFalloff()`**: Gets the alpha (transparency) multiplier applied to the 3D layers consecutively in a style. Takes reference from the ``main`` color.

* **`void s_set3dAlphaFalloff(float value)`**: Sets the alpha multiplier to `value` for for the 3D layers and applies them layer after layer. This property can get finnicky.

* **`float s_get3dPulseMax()`**: Gets the highest value that the ``3DSkew`` can go in a style.

* **`void s_set3dPulseMax(float value)`**: Sets the highest value the ``3DSkew`` can go to `value`.

* **`float s_get3dPulseMin()`**: Gets the lowest value that the ``3DSkew`` can go in a style.

* **`void s_set3dPulseMin(float value)`**: Sets the lowest value the ``3DSkew`` can go to `value`.

* **`float s_get3dPulseSpeed()`**: Gets how fast the ``3DSkew`` moves between ``3DPulseMin`` and ``3DPulseMax``.

* **`void s_set3dPulseSpeed(float value)`**: Sets how fast the ``3DSkew`` moves between ``3DPulseMin`` and ``3DPulseMax`` by `value`.

* **`float s_get3dPerspectiveMult()`**: Gets the 3D perspective multiplier of the style. Works with the attribute ``3DSpacing`` to space out layers.

* **`void s_set3dPerspectiveMult(float value)`**: Sets the 3D perspective multiplier to `value`.

* **`float s_getBGTileRadius()`**: Gets the distances of how far the background panels are drawn. By default, this is a big enough value so you do not see the border. However, feel free to shrink them if you'd like.

* **`void s_setBGTileRadius(float value)`**: Sets how far the background panels are drawn to distance `value`.

* **`unsigned int s_getBGColorOffset()`**: Gets the offset of the style by how much the colors shift. Usually this sits between 0 and 1, but can easily be customized.

* **`void s_setBGColorOffset(unsigned int value)`**: Shifts the background colors to have an offset of `value`.

* **`float s_getBGRotationOffset()`**: Gets the literal rotation offset of the background panels in degrees. This usually stays at 0, but can be messed with to make some stylish level styles.

* **`void s_setBGRotationOffset(float value)`**: Sets the rotation offset of the background panels to `value` degrees.

* **`void s_setStyle(string styleId)`**: Set the currently active style to the style with id `styleId`. Styles can be defined as `.json` files in the `<pack>/Styles/` folder.

* **`void s_setCapColorMain()`**: Set the color of the center polygon to match the main style color.

* **`void s_setCapColorMainDarkened()`**: Set the color of the center polygon to match the main style color, darkened.

* **`void s_setCapColorByIndex(int index)`**: Set the color of the center polygon to match the  style color with index `index`.


## Wall Functions (w_)

Below are the basic wall functions, which can be identified with the "w_" prefix. These are the functions sole responsible for wall creation in the levels. There are a variety of walls that can be made with different degrees of complexity, all of which can be used to construct your own patterns.

* **`void w_wall(int side, float thickness)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier.

* **`void w_wallAdj(int side, float thickness, float speedMult)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `speedMult`.

* **`void w_wallAcc(int side, float thickness, float speedMult, float acceleration, float minSpeed, float maxSpeed)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `speedMult`. The wall will have a speed acceleration value of `acceleration`. The minimum and maximum speed of the wall are bounded by `minSpeed` and `maxSpeed`, adjusted  for the current difficulty multiplier.

* **`void w_wallHModSpeedData(float hueModifier, int side, float thickness, float speedMult, float acceleration, float minSpeed, float maxSpeed, bool pingPong)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `speedMult`. The wall will have a speed acceleration value of `acceleration`. The minimum and maximum speed of the wall are bounded by `minSpeed` and `maxSpeed`, adjusted  for the current difficulty multiplier. The hue of the wall will be adjusted by `hueModifier`. If `pingPong` is enabled, the wall will accelerate back and forth between its minimum and maximum speed.

* **`void w_wallHModCurveData(float hueModifier, int side, float thickness, float curveSpeedMult, float curveAcceleration, float curveMinSpeed, float curveMaxSpeed, bool pingPong)`**: Create a new curving wall at side `side`, with thickness `thickness`. The curving speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `curveSpeedMult`. The wall will have a curving speed acceleration value of `curveAcceleration`. The minimum and maximum curving speed of the wall are bounded by `curveMinSpeed` and `curveMaxSpeed`, adjusted  for the current difficulty multiplier. The hue of the wall will be adjusted by `hueModifier`. If `pingPong` is enabled, the wall will accelerate back and forth between its minimum and maximum speed.


## Custom Wall Functions (cw_)

Below are the custom wall functions, which can be identified with the "cw_" prefix. These are 2.0 exclusive functions with foundations of [Object-oriented programming](https://en.wikipedia.org/wiki/Object-oriented_programming) to allow pack developers to customize individual walls and their properties and make the most out of them.

* **`int cw_create()`**: Create a new custom wall and return a integer handle to it.

* **`void cw_destroy(int cwHandle)`**: Destroy the custom wall represented by `cwHandle`.

* **`void cw_setVertexPos(int cwHandle, int vertexIndex, float x, float y)`**: Given the custom wall represented by `cwHandle`, set the position of its vertex with index `vertexIndex` to `{x, y}`.

* **`void cw_setVertexColor(int cwHandle, int vertexIndex, int r, int g, int b, int a)`**: Given the custom wall represented by `cwHandle`, set the color of its vertex with index `vertexIndex` to `{r, g, b, a}`.

* **`tuple<float, float> cw_getVertexPos(int cwHandle, int vertexIndex)`**: Given the custom wall represented by `cwHandle`, set the position of its vertex with index `vertexIndex` to `{x, y}`.

* **`void cw_clear()`**: Remove all existing custom walls.


## Miscellaneous Functions

Below are the miscellaneous functions, which can have a variable prefix or no prefix at all. These are other functions that are listed that cannot qualify for one of the above eight categories and achieve some other purpose, with some functions not meant to be used by pack developers at all.

* **`void steam_unlockAchievement(string achievementId)`**: Unlock the Steam achievement with id `achievementId`.


## Where to get Help {#where-to-get-help}

The Open Hexagon community is active on the [official Discord server](https://discord.me/openhexagon). The server contains many channels, including `#pack-dev` which is dedicated to level pack developers (and is the recommended place to ask for help and share your progress and ideas), and `#source-dev` which is dedicated to development on the Open Hexagon engine (which is open-source and available [on GitHub](https://github.com/SuperV1234/SSVOpenHexagon/)).

The official Discord server is also a great place to share your levels and get feedback before uploading them to the Steam Workshop.
