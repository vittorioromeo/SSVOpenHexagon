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



## Lua Reference {#lua-reference}

<!-- Generated from Open Hexagon v2.01. -->

* **`void u_log(string message)`**: Print out `message` to the console.

* **`void u_execScript(string scriptFilename)`**: Execute the script located at `<pack>/Scripts/scriptFilename`.

* **`void u_playSound(string soundId)`**: Play the sound with id `soundId`. The id must be registered in `assets.json`, under `"soundBuffers"`.

* **`void u_setMusic(string musicId)`**: Stop the current music and play the music with id `musicId`. The id is defined in the music `.json` file, under `"id"`.

* **`void u_setMusicSegment(string musicId, int segment)`**: Stop the current music and play the music with id `musicId`, starting at segment `segment`. Segments are defined in the music `.json` file, under `"segments"`.

* **`void u_setMusicSeconds(string musicId, float time)`**: Stop the current music and play the music with id `musicId`, starting at time `time` (in seconds).

* **`bool u_isKeyPressed(int keyCode)`**: Return `true` if the keyboard key with code `keyCode` is being pressed, `false` otherwise. The key code must match the definition of the SFML `sf::Keyboard::Key` enumeration.

* **`void u_haltTime(double duration)`**: Pause the game timer for `duration` seconds.

* **`void u_timelineWait(double duration)`**: *Add to the main timeline*: wait for `duration` sixths of a second.

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

* **`void m_messageAdd(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level.

* **`void m_messageAddImportant(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will be printed during every run of the level.

* **`void m_messageAddImportantSilent(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level, and will not produce any sound.

* **`void m_clearMessages()`**: Remove all previously scheduled messages.

* **`void t_wait(double duration)`**: *Add to the main timeline*: wait for `duration` sixths of a second.

* **`void t_waitS(double duration)`**: *Add to the main timeline*: wait for `duration` seconds.

* **`void t_waitUntilS(double duration)`**: *Add to the main timeline*: wait until the timer reaches `duration` seconds.

* **`void e_eventStopTime(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` sixths of a second.

* **`void e_eventStopTimeS(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` seconds.

* **`void e_eventWait(double duration)`**: *Add to the event timeline*: wait for `duration` sixths of a second.

* **`void e_eventWaitS(double duration)`**: *Add to the event timeline*: wait for `duration` seconds.

* **`void e_eventWaitUntilS(double duration)`**: *Add to the event timeline*: wait until the timer reaches `duration` seconds.

* **`float l_getSpeedMult()`**: Return the `SpeedMult` field of the level status.

* **`void l_setSpeedMult(float value)`**: Set the `SpeedMult` field of the level status to `value`.

* **`float l_getSpeedInc()`**: Return the `SpeedInc` field of the level status.

* **`void l_setSpeedInc(float value)`**: Set the `SpeedInc` field of the level status to `value`.

* **`float l_getSpeedMax()`**: Return the `SpeedMax` field of the level status.

* **`void l_setSpeedMax(float value)`**: Set the `SpeedMax` field of the level status to `value`.

* **`float l_getRotationSpeed()`**: Return the `RotationSpeed` field of the level status.

* **`void l_setRotationSpeed(float value)`**: Set the `RotationSpeed` field of the level status to `value`.

* **`float l_getRotationSpeedInc()`**: Return the `RotationSpeedInc` field of the level status.

* **`void l_setRotationSpeedInc(float value)`**: Set the `RotationSpeedInc` field of the level status to `value`.

* **`float l_getRotationSpeedMax()`**: Return the `RotationSpeedMax` field of the level status.

* **`void l_setRotationSpeedMax(float value)`**: Set the `RotationSpeedMax` field of the level status to `value`.

* **`float l_getDelayMult()`**: Return the `DelayMult` field of the level status.

* **`void l_setDelayMult(float value)`**: Set the `DelayMult` field of the level status to `value`.

* **`float l_getDelayInc()`**: Return the `DelayInc` field of the level status.

* **`void l_setDelayInc(float value)`**: Set the `DelayInc` field of the level status to `value`.

* **`float l_getDelayMin()`**: Return the `DelayMin` field of the level status.

* **`void l_setDelayMin(float value)`**: Set the `DelayMin` field of the level status to `value`.

* **`float l_getDelayMax()`**: Return the `DelayMax` field of the level status.

* **`void l_setDelayMax(float value)`**: Set the `DelayMax` field of the level status to `value`.

* **`float l_getFastSpin()`**: Return the `FastSpin` field of the level status.

* **`void l_setFastSpin(float value)`**: Set the `FastSpin` field of the level status to `value`.

* **`float l_getIncTime()`**: Return the `IncTime` field of the level status.

* **`void l_setIncTime(float value)`**: Set the `IncTime` field of the level status to `value`.

* **`float l_getPulseMin()`**: Return the `PulseMin` field of the level status.

* **`void l_setPulseMin(float value)`**: Set the `PulseMin` field of the level status to `value`.

* **`float l_getPulseMax()`**: Return the `PulseMax` field of the level status.

* **`void l_setPulseMax(float value)`**: Set the `PulseMax` field of the level status to `value`.

* **`float l_getPulseSpeed()`**: Return the `PulseSpeed` field of the level status.

* **`void l_setPulseSpeed(float value)`**: Set the `PulseSpeed` field of the level status to `value`.

* **`float l_getPulseSpeedR()`**: Return the `PulseSpeedR` field of the level status.

* **`void l_setPulseSpeedR(float value)`**: Set the `PulseSpeedR` field of the level status to `value`.

* **`float l_getPulseDelayMax()`**: Return the `PulseDelayMax` field of the level status.

* **`void l_setPulseDelayMax(float value)`**: Set the `PulseDelayMax` field of the level status to `value`.

* **`float l_getPulseDelayHalfMax()`**: Return the `PulseDelayHalfMax` field of the level status.

* **`void l_setPulseDelayHalfMax(float value)`**: Set the `PulseDelayHalfMax` field of the level status to `value`.

* **`float l_getBeatPulseMax()`**: Return the `BeatPulseMax` field of the level status.

* **`void l_setBeatPulseMax(float value)`**: Set the `BeatPulseMax` field of the level status to `value`.

* **`float l_getBeatPulseDelayMax()`**: Return the `BeatPulseDelayMax` field of the level status.

* **`void l_setBeatPulseDelayMax(float value)`**: Set the `BeatPulseDelayMax` field of the level status to `value`.

* **`float l_getRadiusMin()`**: Return the `RadiusMin` field of the level status.

* **`void l_setRadiusMin(float value)`**: Set the `RadiusMin` field of the level status to `value`.

* **`float l_getWallSkewLeft()`**: Return the `WallSkewLeft` field of the level status.

* **`void l_setWallSkewLeft(float value)`**: Set the `WallSkewLeft` field of the level status to `value`.

* **`float l_getWallSkewRight()`**: Return the `WallSkewRight` field of the level status.

* **`void l_setWallSkewRight(float value)`**: Set the `WallSkewRight` field of the level status to `value`.

* **`float l_getWallAngleLeft()`**: Return the `WallAngleLeft` field of the level status.

* **`void l_setWallAngleLeft(float value)`**: Set the `WallAngleLeft` field of the level status to `value`.

* **`float l_getWallAngleRight()`**: Return the `WallAngleRight` field of the level status.

* **`void l_setWallAngleRight(float value)`**: Set the `WallAngleRight` field of the level status to `value`.

* **`float l_get3dEffectMultiplier()`**: Return the `3dEffectMultiplier` field of the level status.

* **`void l_set3dEffectMultiplier(float value)`**: Set the `3dEffectMultiplier` field of the level status to `value`.

* **`int l_getCameraShake()`**: Return the `CameraShake` field of the level status.

* **`void l_setCameraShake(int value)`**: Set the `CameraShake` field of the level status to `value`.

* **`unsigned int l_getSides()`**: Return the `Sides` field of the level status.

* **`void l_setSides(unsigned int value)`**: Set the `Sides` field of the level status to `value`.

* **`unsigned int l_getSidesMax()`**: Return the `SidesMax` field of the level status.

* **`void l_setSidesMax(unsigned int value)`**: Set the `SidesMax` field of the level status to `value`.

* **`unsigned int l_getSidesMin()`**: Return the `SidesMin` field of the level status.

* **`void l_setSidesMin(unsigned int value)`**: Set the `SidesMin` field of the level status to `value`.

* **`bool l_getSwapEnabled()`**: Return the `SwapEnabled` field of the level status.

* **`void l_setSwapEnabled(bool value)`**: Set the `SwapEnabled` field of the level status to `value`.

* **`bool l_getTutorialMode()`**: Return the `TutorialMode` field of the level status.

* **`void l_setTutorialMode(bool value)`**: Set the `TutorialMode` field of the level status to `value`.

* **`bool l_getIncEnabled()`**: Return the `IncEnabled` field of the level status.

* **`void l_setIncEnabled(bool value)`**: Set the `IncEnabled` field of the level status to `value`.

* **`bool l_getDarkenUnevenBackgroundChunk()`**: Return the `DarkenUnevenBackgroundChunk` field of the level status.

* **`void l_setDarkenUnevenBackgroundChunk(bool value)`**: Set the `DarkenUnevenBackgroundChunk` field of the level status to `value`.

* **`size_t l_getCurrentIncrements()`**: Return the `CurrentIncrements` field of the level status.

* **`void l_setCurrentIncrements(size_t value)`**: Set the `CurrentIncrements` field of the level status to `value`.

* **`void l_enableRndSideChanges(bool enabled)`**: Set random side changes to `enabled`.

* **`void l_darkenUnevenBackgroundChunk(bool enabled)`**: If `enabled` is true, one of the background's chunks will be darkened in case there is an uneven number of sides.

* **`void l_addTracked(string variable, string name)`**: Add the variable `variable` to the list of tracked variables, with name `name`. Tracked variables are displayed in game, below the game timer.

* **`void l_setRotation(float angle)`**: Set the background camera rotation to `angle` radians.

* **`float l_getRotation()`**: Return the background camera rotation, in radians.

* **`double l_getLevelTime()`**: Get the current game timer value, in seconds.

* **`bool l_getOfficial()`**: Return `true` if "official mode" is enabled, `false` otherwise.

* **`float s_getHueMin()`**: Return the `HueMin` field of the style data.

* **`void s_setHueMin(float value)`**: Set the `HueMin` field of the style data to `value`.

* **`float s_getHueMax()`**: Return the `HueMax` field of the style data.

* **`void s_setHueMax(float value)`**: Set the `HueMax` field of the style data to `value`.

* **`float s_getHueInc()`**: Return the `HueInc` field of the style data.

* **`void s_setHueInc(float value)`**: Set the `HueInc` field of the style data to `value`.

* **`float s_getHueIncrement()`**: Return the `HueIncrement` field of the style data.

* **`void s_setHueIncrement(float value)`**: Set the `HueIncrement` field of the style data to `value`.

* **`float s_getPulseMin()`**: Return the `PulseMin` field of the style data.

* **`void s_setPulseMin(float value)`**: Set the `PulseMin` field of the style data to `value`.

* **`float s_getPulseMax()`**: Return the `PulseMax` field of the style data.

* **`void s_setPulseMax(float value)`**: Set the `PulseMax` field of the style data to `value`.

* **`float s_getPulseInc()`**: Return the `PulseInc` field of the style data.

* **`void s_setPulseInc(float value)`**: Set the `PulseInc` field of the style data to `value`.

* **`float s_getPulseIncrement()`**: Return the `PulseIncrement` field of the style data.

* **`void s_setPulseIncrement(float value)`**: Set the `PulseIncrement` field of the style data to `value`.

* **`bool s_getHuePingPong()`**: Return the `HuePingPong` field of the style data.

* **`void s_setHuePingPong(bool value)`**: Set the `HuePingPong` field of the style data to `value`.

* **`float s_getMaxSwapTime()`**: Return the `MaxSwapTime` field of the style data.

* **`void s_setMaxSwapTime(float value)`**: Set the `MaxSwapTime` field of the style data to `value`.

* **`float s_get3dDepth()`**: Return the `3dDepth` field of the style data.

* **`void s_set3dDepth(float value)`**: Set the `3dDepth` field of the style data to `value`.

* **`float s_get3dSkew()`**: Return the `3dSkew` field of the style data.

* **`void s_set3dSkew(float value)`**: Set the `3dSkew` field of the style data to `value`.

* **`float s_get3dSpacing()`**: Return the `3dSpacing` field of the style data.

* **`void s_set3dSpacing(float value)`**: Set the `3dSpacing` field of the style data to `value`.

* **`float s_get3dDarkenMult()`**: Return the `3dDarkenMult` field of the style data.

* **`void s_set3dDarkenMult(float value)`**: Set the `3dDarkenMult` field of the style data to `value`.

* **`float s_get3dAlphaMult()`**: Return the `3dAlphaMult` field of the style data.

* **`void s_set3dAlphaMult(float value)`**: Set the `3dAlphaMult` field of the style data to `value`.

* **`float s_get3dAlphaFalloff()`**: Return the `3dAlphaFalloff` field of the style data.

* **`void s_set3dAlphaFalloff(float value)`**: Set the `3dAlphaFalloff` field of the style data to `value`.

* **`float s_get3dPulseMax()`**: Return the `3dPulseMax` field of the style data.

* **`void s_set3dPulseMax(float value)`**: Set the `3dPulseMax` field of the style data to `value`.

* **`float s_get3dPulseMin()`**: Return the `3dPulseMin` field of the style data.

* **`void s_set3dPulseMin(float value)`**: Set the `3dPulseMin` field of the style data to `value`.

* **`float s_get3dPulseSpeed()`**: Return the `3dPulseSpeed` field of the style data.

* **`void s_set3dPulseSpeed(float value)`**: Set the `3dPulseSpeed` field of the style data to `value`.

* **`float s_get3dPerspectiveMult()`**: Return the `3dPerspectiveMult` field of the style data.

* **`void s_set3dPerspectiveMult(float value)`**: Set the `3dPerspectiveMult` field of the style data to `value`.

* **`void s_setStyle(string styleId)`**: Set the currently active style to the style with id `styleId`. Styles can be defined as `.json` files in the `<pack>/Styles/` folder.

* **`void s_setCameraShake(int value)`**: Start a camera shake with intensity `value`.

* **`int s_getCameraShake()`**: Return the current camera shake intensity.

* **`void s_setCapColorMain()`**: Set the color of the center polygon to match the main style color.

* **`void s_setCapColorMainDarkened()`**: Set the color of the center polygon to match the main style color, darkened.

* **`void s_setCapColorByIndex(int index)`**: Set the color of the center polygon to match the  style color with index `index`.

* **`void w_wall(int side, float thickness)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier.

* **`void w_wallAdj(int side, float thickness, float speedMult)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `speedMult`.

* **`void w_wallAcc(int side, float thickness, float speedMult, float acceleration, float minSpeed, float maxSpeed)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `speedMult`. The wall will have a speed acceleration value of `acceleration`. The minimum and maximum speed of the wall are bounded by `minSpeed` and `maxSpeed`, adjusted  for the current difficulty multiplier.

* **`void w_wallHModSpeedData(float hueModifier, int side, float thickness, float speedMult, float acceleration, float minSpeed, float maxSpeed, bool pingPong)`**: Create a new wall at side `side`, with thickness `thickness`. The speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `speedMult`. The wall will have a speed acceleration value of `acceleration`. The minimum and maximum speed of the wall are bounded by `minSpeed` and `maxSpeed`, adjusted  for the current difficulty multiplier. The hue of the wall will be adjusted by `hueModifier`. If `pingPong` is enabled, the wall will accelerate back and forth between its minimum and maximum speed.

* **`void w_wallHModCurveData(float hueModifier, int side, float thickness, float curveSpeedMult, float curveAcceleration, float curveMinSpeed, float curveMaxSpeed, bool pingPong)`**: Create a new curving wall at side `side`, with thickness `thickness`. The curving speed of the wall will be calculated by using the speed multiplier, adjusted for the current difficulty multiplier, and finally multiplied by `curveSpeedMult`. The wall will have a curving speed acceleration value of `curveAcceleration`. The minimum and maximum curving speed of the wall are bounded by `curveMinSpeed` and `curveMaxSpeed`, adjusted  for the current difficulty multiplier. The hue of the wall will be adjusted by `hueModifier`. If `pingPong` is enabled, the wall will accelerate back and forth between its minimum and maximum speed.

* **`void steam_unlockAchievement(string achievementId)`**: Unlock the Steam achievement with id `achievementId`.

* **`int cw_create()`**: Create a new custom wall and return a integer handle to it.

* **`void cw_destroy(int cwHandle)`**: Destroy the custom wall represented by `cwHandle`.

* **`void cw_setVertexPos(int cwHandle, int vertexIndex, float x, float y)`**: Given the custom wall represented by `cwHandle`, set the position of its vertex with index `vertexIndex` to `{x, y}`.

* **`void cw_setVertexColor(int cwHandle, int vertexIndex, int r, int g, int b, int a)`**: Given the custom wall represented by `cwHandle`, set the color of its vertex with index `vertexIndex` to `{r, g, b, a}`.

* **`tuple<float, float> cw_getVertexPos(int cwHandle, int vertexIndex)`**: Given the custom wall represented by `cwHandle`, set the position of its vertex with index `vertexIndex` to `{x, y}`.

* **`void cw_clear()`**: Remove all existing custom walls.



## Where to get Help {#where-to-get-help}

The Open Hexagon community is active on the [official Discord server](https://discord.me/openhexagon). The server contains many channels, including `#pack-dev` which is dedicated to level pack developers (and is the recommended place to ask for help and share your progress and ideas), and `#source-dev` which is dedicated to development on the Open Hexagon engine (which is open-source and available [on GitHub](https://github.com/SuperV1234/SSVOpenHexagon/)).

The official Discord server is also a great place to share your levels and get feedback before uploading them to the Steam Workshop.
