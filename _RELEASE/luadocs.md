## Utility Functions (u_)

Below are the utility functions, which can be identified with the "u_" prefix. These are overall functions that either help utilize the game engine, be incredibly beneficial to pack developers, or help simplify complex calculations.

* **`float u_rndReal()`**: Return a random real number in the [0; 1] range.

* **`float u_rndIntUpper(int upper)`**: Return a random integer number in the [1; `upper`] range.

* **`float u_rndInt(int lower, int upper)`**: Return a random integer number in the [`lower`; `upper`] range.

* **`float u_rndSwitch(int mode, int lower, int upper)`**: Internal replacement for `math.random`. Calls `u_rndReal()` with `mode == 0`, `u_rndUpper(upper)` with `mode == 1`, and `u_rndInt(lower, upper)` with `mode == 2`.

* **`unsigned long long u_getAttemptRandomSeed()`**: Obtain the current random seed, automatically generated at the beginning of the level. `math.randomseed` is automatically initialized with the result of this function at the beginning of a level.

* **`bool u_inMenu()`**: Returns `true` if the script is being executed in the menu, `false` otherwise.

* **`int u_getVersionMajor()`**: Returns the major of the current version of the game

* **`int u_getVersionMinor()`**: Returns the minor of the current version of the game

* **`int u_getVersionMicro()`**: Returns the micro of the current version of the game

* **`string u_getVersionString()`**: Returns the string representing the current version of the game

* **`void u_setFlashEffect(float value)`**: Flash the screen with `value` intensity (from 0 to 255).

* **`void u_log(string message)`**: Print out `message` to the console.

* **`void u_execScript(string scriptFilename)`**: Execute the script located at `<pack>/Scripts/scriptFilename`.

* **`void u_execDependencyScript(string packDisambiguator, string packName, string packAuthor, string scriptFilename)`**: Execute the script provided by the dependee pack with disambiguator `packDisambiguator`, name `packName`, author `packAuthor`, located at `<dependeePack>/Scripts/scriptFilename`.

* **`bool u_isKeyPressed(int keyCode)`**: Return `true` if the keyboard key with code `keyCode` is being pressed, `false` otherwise. The key code must match the definition of the SFML `sf::Keyboard::Key` enumeration.

* **`void u_haltTime(double duration)`**: Pause the game timer for `duration` seconds.

* **`void u_clearWalls()`**: Remove all existing walls.

* **`float u_getPlayerAngle()`**: Return the current angle of the player, in radians.

* **`void u_setPlayerAngle(float angle)`**: Set the current angle of the player to `angle`, in radians.

* **`bool u_isMouseButtonPressed(int buttonCode)`**: Return `true` if the mouse button with code `buttonCode` is being pressed, `false` otherwise. The button code must match the definition of the SFML `sf::Mouse::Button` enumeration.

* **`bool u_isFastSpinning()`**: Return `true` if the camera is currently "fast spinning", `false` otherwise.

* **`void u_forceIncrement()`**: Immediately force a difficulty increment, regardless of the chosen automatic increment parameters.

* **`float u_getDifficultyMult()`**: Return the current difficulty multiplier.

* **`float u_getSpeedMultDM()`**: Return the current speed multiplier, adjusted for the chosen difficulty multiplier.

* **`float u_getDelayMultDM()`**: Return the current delay multiplier, adjusted for the chosen difficulty multiplier.

* **`void u_swapPlayer(bool playSound)`**: Force-swaps (180 degrees) the player when invoked. If `playSound` is `true`, the swap sound will be played.

* **`void u_kill()`**: *Add to the main timeline*: kill the player. **This function is deprecated and will be removed in a future version. Please use t_kill instead!**

* **`void u_eventKill()`**: *Add to the event timeline*: kill the player. **This function is deprecated and will be removed in a future version. Please use e_kill instead!**

* **`void u_playSound(string soundId)`**: Play the sound with id `soundId`. The id must be registered in `assets.json`, under `"soundBuffers"`. **This function is deprecated and will be removed in a future version. Please use a_playSound instead!**

* **`void u_playPackSound(string fileName)`**: Dives into the `Sounds` folder of the current level pack and plays the specified file `fileName`. **This function is deprecated and will be removed in a future version. Please use a_playPackSound instead!**


## Audio Functions (a_)

Below are the audio functions, which can be identified with the "a_" prefix. This library is capable of controlling everything audio with the game, including the level music and sounds. Set the music, play a specific sound, all of this is in the domain of this prefix.

* **`void a_setMusic(string musicId)`**: Stop the current music and play the music with id `musicId`. The id is defined in the music `.json` file, under `"id"`.

* **`void a_setMusicSegment(string musicId, int segment)`**: Stop the current music and play the music with id `musicId`, starting at segment `segment`. Segments are defined in the music `.json` file, under `"segments"`.

* **`void a_setMusicSeconds(string musicId, float time)`**: Stop the current music and play the music with id `musicId`, starting at time `time` (in seconds).

* **`void a_playSound(string soundId)`**: Play the sound with id `soundId`. The id must be registered in `assets.json`, under `"soundBuffers"`.

* **`void a_playPackSound(string fileName)`**: Dives into the `Sounds` folder of the current level pack and plays the specified file `fileName`.

* **`void a_syncMusicToDM(bool value)`**: This function, when called, overrides the user's preference of adjusting the music's pitch to the difficulty multiplier. Useful for levels that rely on the music to time events.

* **`void a_setMusicPitch(float pitch)`**: Manually adjusts the pitch of the music by multiplying it by `pitch`. The amount the pitch shifts may change on DM multiplication and user's preference of the music pitch. **Negative values will not work!**

* **`void a_overrideBeepSound(string fileName)`**: Dives into the `Sounds` folder of the current level pack and sets the specified file `fileName` to be the new beep sound. This only applies to the particular level where this function is called.

* **`void a_overrideIncrementSound(string fileName)`**: Dives into the `Sounds` folder of the current level pack and sets the specified file `fileName` to be the new increment sound. This only applies to the particular level where this function is called.

* **`void a_overrideSwapSound(string fileName)`**: Dives into the `Sounds` folder of the current level pack and sets the specified file `fileName` to be the new swap sound. This only applies to the particular level where this function is called.

* **`void a_overrideDeathSound(string fileName)`**: Dives into the `Sounds` folder of the current level pack and sets the specified file `fileName` to be the new death sound. This only applies to the particular level where this function is called.


## Main Timeline Functions (t_)

Below are the main timeline functions, which can be identified with the "t_" prefix. These are functions that have effects on the main timeline itself, but they mainly consist of waiting functions. Using these functions helps time out your patterns and space them in the first place.

* **`void t_eval(string code)`**: *Add to the main timeline*: evaluate the Lua code specified in `code`.

* **`void t_clear()`**: Clear the main timeline.

* **`void t_kill()`**: *Add to the main timeline*: kill the player.

* **`void t_wait(double duration)`**: *Add to the main timeline*: wait for `duration` frames (under the assumption of a 60 FPS frame rate).

* **`void t_waitS(double duration)`**: *Add to the main timeline*: wait for `duration` seconds.

* **`void t_waitUntilS(double duration)`**: *Add to the main timeline*: wait until the timer reaches `duration` seconds.


## Event Timeline Functions (e_)

Below are the event timeline functions, which can be identified with the "e_" prefix. These are functions that are similar to the Main Timeline functions, but they instead are for the event timeline as opposed to the main timeline. Use these functions to help set up basic events for the game, such as handling messages. Use ``e_eval`` to do more advanced events.

* **`void e_eval(string code)`**: *Add to the event timeline*: evaluate the Lua code specified in `code`. (This is the closest you'll get to 1.92 events)

* **`void e_kill()`**: *Add to the event timeline*: kill the player.

* **`void e_stopTime(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` frames (under the assumption of a 60 FPS frame rate).

* **`void e_stopTimeS(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` seconds.

* **`void e_wait(double duration)`**: *Add to the event timeline*: wait for `duration` frames (under the assumption of a 60 FPS frame rate).

* **`void e_waitS(double duration)`**: *Add to the event timeline*: wait for `duration` seconds.

* **`void e_waitUntilS(double duration)`**: *Add to the event timeline*: wait until the timer reaches `duration` seconds.

* **`void e_messageAdd(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level.

* **`void e_messageAddImportant(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will be printed during every run of the level.

* **`void e_messageAddImportantSilent(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during every run of the level, and will not produce any sound.

* **`void e_clearMessages()`**: Remove all previously scheduled messages.

* **`void e_eventStopTime(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` frames (under the assumption of a 60 FPS frame rate). **This function is deprecated and will be removed in a future version. Please use e_stopTime instead!**

* **`void e_eventStopTimeS(double duration)`**: *Add to the event timeline*: pause the game timer for `duration` seconds. **This function is deprecated and will be removed in a future version. Please use e_stopTimeS instead!**

* **`void e_eventWait(double duration)`**: *Add to the event timeline*: wait for `duration` frames (under the assumption of a 60 FPS frame rate). **This function is deprecated and will be removed in a future version. Please use e_wait instead!**

* **`void e_eventWaitS(double duration)`**: *Add to the event timeline*: wait for `duration` seconds. **This function is deprecated and will be removed in a future version. Please use e_waitS instead!**

* **`void e_eventWaitUntilS(double duration)`**: *Add to the event timeline*: wait until the timer reaches `duration` seconds. **This function is deprecated and will be removed in a future version. Please use e_waitUntilS instead!**


## Level Functions (l_)

Below are the level functions, which can be identified with the "l_" prefix. These are functions that have a role in altering the level mechanics themselves, including all level properties and attributes. These typically get called en masse in `onInit` to initialize properties.

* **`float l_getSpeedMult()`**: Gets the speed multiplier of the level. The speed multiplier is the current speed of the walls. Is incremented by ``SpeedInc`` every increment and caps at ``speedMax``.

* **`void l_setSpeedMult(float value)`**: Sets the speed multiplier of the level to `value`. Changes do not apply to all walls immediately, and changes apply as soon as the next wall is created.

* **`float l_getPlayerSpeedMult()`**: Gets the speed multiplier of the player.

* **`void l_setPlayerSpeedMult(float value)`**: Sets the speed multiplier of the player.

* **`float l_getSpeedInc()`**: Gets the speed increment of the level. This is applied every level increment to the speed multiplier. Increments are additive.

* **`void l_setSpeedInc(float value)`**: Sets the speed increment of the level to `value`.

* **`float l_getSpeedMax()`**: Gets the maximum speed of the level. This is the highest that speed can go; speed can not get any higher than this.

* **`void l_setSpeedMax(float value)`**: Sets the maximum speed of the level to `value`. Keep in mind that speed keeps going past the speed max, so setting a higher speed max may make the speed instantly increase to the max.

* **`float l_getRotationSpeed()`**: Gets the rotation speed of the level. Is incremented by ``RotationSpeedInc`` every increment and caps at ``RotationSpeedMax``.

* **`void l_setRotationSpeed(float value)`**: Sets the rotation speed of the level to `value`. Changes apply immediately.

* **`float l_getRotationSpeedInc()`**: Gets the rotation speed increment of the level. This is applied every level increment to the rotation speed. Increments are additive.

* **`void l_setRotationSpeedInc(float value)`**: Sets the rotation speed increment of the level to `value`. Is effective on the next level increment.

* **`float l_getRotationSpeedMax()`**: Gets the maximum rotation speed of the level. This is the highest that rotation speed can go; rotation speed can not get any higher than this.

* **`void l_setRotationSpeedMax(float value)`**: Sets the maximum rotation speed of the level to `value`. Keep in mind that rotation speed keeps going past the max, so setting a higher rotation speed max may make the rotation speed instantly increase to the max.

* **`float l_getDelayMult()`**: Gets the delay multiplier of the level. The delay multiplier is the multiplier used to assist in spacing patterns, especially in cases of higher / lower speeds.  Is incremented by ``DelayInc`` every increment and is clamped between ``DelayMin`` and ``DelayMax``

* **`void l_setDelayMult(float value)`**: Sets the delay multiplier of the level to `value`. Changes do not apply to patterns immediately, and changes apply as soon as the next pattern is spawned.

* **`float l_getDelayInc()`**: Gets the delay increment of the level. This is applied every level increment to the delay multiplier. Increments are additive.

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

* **`void l_setPulseSpeed(float value)`**: Sets the speed the pulse goes from ``PulseMin`` to ``PulseMax`` by `value`. Can also be used to help sync a level up with it's music.

* **`float l_getPulseSpeedR()`**: Gets the speed the pulse goes from ``PulseMax`` to ``PulseMin``.

* **`void l_setPulseSpeedR(float value)`**: Sets the speed the pulse goes from ``PulseMax`` to ``PulseMin`` by `value`. Can also be used to help sync a level up with it's music.

* **`float l_getPulseDelayMax()`**: Gets the delay the level has to wait before it begins another pulse cycle.

* **`void l_setPulseDelayMax(float value)`**: Sets the delay the level has to wait before it begins another pulse cycle with `value`.

* **`float l_getPulseInitialDelay()`**: Gets the initial delay the level has to wait before it begins the first pulse cycle.

* **`void l_setPulseInitialDelay(float value)`**: Sets the initial delay the level has to wait before it begins the first pulse cycle with `value`.

* **`float l_getSwapCooldownMult()`**: Gets the multiplier that controls the cooldown for the player's 180 degrees swap mechanic.

* **`void l_setSwapCooldownMult(float value)`**: Sets the multiplier that controls the cooldown for the player's 180 degrees swap mechanic to `value`.

* **`float l_getBeatPulseMax()`**: Gets the maximum beatpulse size of the polygon in a level. This is the highest value that the polygon will "pulse" in size. Useful for syncing the level to the music.

* **`void l_setBeatPulseMax(float value)`**: Sets the maximum beatpulse size of the polygon in a level to `value`. Not to be confused with using this property to resize the polygon, which you should be using ``RadiusMin``.

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

* **`float l_getWallSpawnDistance()`**: Gets the distance at which standard walls spawn.

* **`void l_setWallSpawnDistance(float value)`**: Sets how far away the walls can spawn from the center. Higher values make walls spawn farther away, and will increase the player's wait for incoming walls.

* **`bool l_get3dRequired()`**: Gets whether 3D must be enabled in order to have a valid score in this level. By default, this value is ``false``.

* **`void l_set3dRequired(bool value)`**: Sets whether 3D must be enabled to `value` to have a valid score. Only set this to ``true`` if your level relies on 3D effects to work as intended.

* **`float l_getCameraShake()`**: Gets the intensity of the camera shaking in a level.

* **`void l_setCameraShake(float value)`**: Sets the intensity of the camera shaking in a level to `value`. This remains permanent until you either set this to 0 or the player dies.

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

* **`bool l_getManualPulseControl()`**: Gets whether the pulse effect is being controlled manually via Lua or automatically by the C++ engine.

* **`void l_setManualPulseControl(bool value)`**: Sets whether the pulse effect is being controlled manually via Lua or automatically by the C++ engine to `value`.

* **`bool l_getManualBeatPulseControl()`**: Gets whether the beat pulse effect is being controlled manually via Lua or automatically by the C++ engine.

* **`void l_setManualBeatPulseControl(bool value)`**: Sets whether the beat  pulse effect is being controlled manually via Lua or automatically by the C++ engine to `value`.

* **`unsigned long long l_getCurrentIncrements()`**: Gets the current amount of times the level has incremented. Very useful for keeping track of levels.

* **`void l_setCurrentIncrements(unsigned long long value)`**: Sets the current amount of times the level has incremented to `value`. This function is utterly pointless to use unless you are tracking this variable.

* **`void l_enableRndSideChanges(bool enabled)`**: Toggles random side changes to `enabled`, (not) allowing sides to change between ``SidesMin`` and ``SidesMax`` inclusively every level increment.

* **`void l_overrideScore(string variable)`**: Overrides the default scoring method and determines score based off the value of `variable`. This allows for custom scoring in levels. *Avoid using strings, otherwise scores won't sort properly. NOTE: Your variable must be global for this to work.*

* **`void l_addTracked(string variable, string name)`**: Add the variable `variable` to the list of tracked variables, with name `name`. Tracked variables are displayed in game, below the game timer. *NOTE: Your variable must be global for this to work.*

* **`void l_clearTracked()`**: Clears all tracked variables.

* **`void l_setRotation(float angle)`**: Set the background camera rotation to `angle` degrees.

* **`float l_getRotation()`**: Return the background camera rotation, in degrees.

* **`double l_getLevelTime()`**: Get the current game timer value, in seconds.

* **`bool l_getOfficial()`**: Return `true` if "official mode" is enabled, `false` otherwise.

* **`void l_resetTime()`**: Resets the lever time to zero, also resets increment time and pause time.

* **`float l_getPulse()`**: Gets the current pulse value, which will vary between `l_getPulseMin()` and `l_getPulseMax()` unless manually overridden.

* **`void l_setPulse(float value)`**: Sets the current pulse value to `value`.

* **`float l_getPulseDirection()`**: Gets the current pulse direction value, which will either be `-1` or `1` unless manually overridden.

* **`void l_setPulseDirection(float value)`**: Sets the current pulse direction value to `value`. Valid choices are `-1` or `1`.

* **`float l_getPulseDelay()`**: Gets the current pulse delay value, which will vary between `0` and `l_getPulseDelayMax()` unless manually overridden.

* **`void l_setPulseDelay(float value)`**: Sets the current pulse delay value to `value`.

* **`float l_getBeatPulse()`**: Gets the current beat pulse value, which will vary between `0` and `l_getBeatPulseMax()` unless manually overridden.

* **`void l_setBeatPulse(float value)`**: Sets the current beat pulse value to `value`.

* **`float l_getBeatPulseDelay()`**: Gets the current beat pulse delay value, which will vary between `0` and `l_getBeatPulseDelayMax()` unless manually overridden.

* **`void l_setBeatPulseDelay(float value)`**: Sets the current beat pulse delay value to `value`.


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

* **`void s_setCapColorByIndex(int index)`**: Set the color of the center polygon to match the style color with index `index`.

* **`tuple<intintintint> s_getMainColor()`**: Return the current main color computed by the level style.

* **`tuple<intintintint> s_getPlayerColor()`**: Return the current player color computed by the level style.

* **`tuple<intintintint> s_getTextColor()`**: Return the current text color computed by the level style.

* **`tuple<intintintint> s_get3DOverrideColor()`**: Return the current 3D override color computed by the level style.

* **`tuple<intintintint> s_getCapColorResult()`**: Return the current cap color result color computed by the level style.

* **`tuple<intintintint> s_getColor(int index)`**: Return the current color with index `index` computed by the level style.


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

* **`int cw_createDeadly()`**: Create a new deadly custom wall and return a integer handle to it.

* **`int cw_createNoCollision()`**: Create a new custom wall without collision and return a integer handle to it.

* **`void cw_destroy(int cwHandle)`**: Destroy the custom wall represented by `cwHandle`.

* **`void cw_setVertexPos(int cwHandle, int vertexIndex, float x, float y)`**: Given the custom wall represented by `cwHandle`, set the position of its vertex with index `vertexIndex` to `{x, y}`.

* **`void cw_moveVertexPos(int cwHandle, int vertexIndex, float offsetX, float offsetY)`**: Given the custom wall represented by `cwHandle`, add `{offsetX, offsetY}` to the position of its vertex with index `vertexIndex`.

* **`void cw_moveVertexPos4Same(int cwHandle, float offsetX, float offsetY)`**: Given the custom wall represented by `cwHandle`, add `{offsetX, offsetY}` to the position of its vertex with indices `0`, `1`, `2`, and `3`.

* **`void cw_setVertexColor(int cwHandle, int vertexIndex, int r, int g, int b, int a)`**: Given the custom wall represented by `cwHandle`, set the color of its vertex with index `vertexIndex` to `{r, g, b, a}`.

* **`void cw_setVertexPos4(int cwHandle, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)`**: Given the custom wall represented by `cwHandle`, set the position of its vertex with index `0` to `{x0, y0}`, index `1` to `{x1, y1}`, index `2` to `{x2, y2}`, index `3` to `{x3, y3}`. More efficient than invoking `cw_setVertexPos` four times in a row.

* **`void cw_setVertexColor4(int cwHandle, int r0, int g0, int b0, int a0, int r1, int g1, int b1, int a1, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)`**: Given the custom wall represented by `cwHandle`, set the color of its vertex with index `0` to `{r0, g0, b0, a0}`, index `1` to `{r1, g1, b1, a1}`, index `2` to `{r2, g2, b2, a2}`, index `3` to `{r3, g3, b3, a3}`. More efficient than invoking `cw_setVertexColor` four times in a row.

* **`void cw_setVertexColor4Same(int cwHandle, int r, int g, int b, int a)`**: Given the custom wall represented by `cwHandle`, set the color of its vertices with indiced `0`, `1`, `2`, and `3' to `{r, g, b, a}`. More efficient than invoking `cw_setVertexColor` four times in a row.

* **`void cw_setCollision(int cwHandle, bool collision)`**: Given the custom wall represented by `cwHandle`, set the collision of the custom wall to `collision`. If false, the player cannot die from this wall and can move through the wall. By default, all custom walls can collide with the player.

* **`void cw_setDeadly(int cwHandle, bool deadly)`**: Given the custom wall represented by `cwHandle`, set wherever it instantly kills player on touch. This is highly recommended for custom walls that are either very small or very thin and should definitively kill the player.

* **`void cw_setKillingSide(int cwHandle, unsigned int side)`**: Given the custom wall represented by `cwHandle`, set which one of its sides should beyond any doubt cause the death of the player. Acceptable values are `0` to `3`. In a standard wall, side `0` is the side closer to the center. This parameter is useless if the custom wall is deadly.

* **`bool cw_getCollision(int cwHandle)`**: Given the custom wall represented by `cwHandle`, get whether it can collide with player or not.

* **`bool cw_getDeadly(int cwHandle)`**: Given the custom wall represented by `cwHandle`, get whether it instantly kills the player on touch or not.

* **`unsigned int cw_getKillingSide(int cwHandle)`**: Given the custom wall represented by `cwHandle`, get which one of its sides always causes the death of the player.

* **`tuple<floatfloat> cw_getVertexPos(int cwHandle, int vertexIndex)`**: Given the custom wall represented by `cwHandle`, return the position of its vertex with index `vertexIndex`.

* **`tuple<floatfloatfloatfloatfloatfloatfloatfloat> cw_getVertexPos4(int cwHandle)`**: Given the custom wall represented by `cwHandle`, return the position of its vertics with indices `0`, `1`, `2`, and `3`, as a tuple.

* **`void cw_clear()`**: Remove all existing custom walls.


## Miscellaneous Functions

Below are the miscellaneous functions, which can have a variable prefix or no prefix at all. These are other functions that are listed that cannot qualify for one of the above eight categories and achieve some other purpose, with some functions not meant to be used by pack developers at all.

* **`void steam_unlockAchievement(string achievementId)`**: Unlock the Steam achievement with id `achievementId`.

* **`void m_messageAdd(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level. **This function is deprecated and will be removed in a future version. Please use e_messageAdd instead!**

* **`void m_messageAddImportant(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will be printed during every run of the level. **This function is deprecated and will be removed in a future version. Please use e_messageAddImportant instead!**

* **`void m_messageAddImportantSilent(string message, double duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during every run of the level, and will not produce any sound. **This function is deprecated and will be removed in a future version. Please use e_messageAddImportantSilent instead!**

* **`void m_clearMessages()`**: Remove all previously scheduled messages. **This function is deprecated and will be removed in a future version. Please use e_clearMessages instead!**
