* **`void u_log(string message)`**: Print out `message` to the console.
* **`void u_execScript(string scriptFilename)`**: Execute the script located at `<pack>/Scripts/scriptFilename`.
* **`void u_playSound(string soundId)`**: Play the sound with id `soundId`. The id must be registered in `assets.json`, under `"soundBuffers"`.
* **`void u_setMusic(string musicId)`**: Stop the current music and play the music with id `musicId`. The id is defined in the music `.json` file, under `"id"`.
* **`void u_setMusicSegment(string musicId, int segment)`**: Stop the current music and play the music with id `musicId`, starting at segment `segment`. Segments are defined in the music `.json` file, under `"segments"`.
* **`void u_setMusicSeconds(string musicId, float time)`**: Stop the current music and play the music with id `musicId`, starting at time `time` (in seconds).
* **`bool u_isKeyPressed(int keyCode)`**: Return `true` if the keyboard key with code `keyCode` is being pressed, `false` otherwise. The key code must match the definition of the SFML `sf::Keyboard::Key` enumeration.
* **`void u_haltTime(float duration)`**: Pause the game timer for `duration` seconds.
* **`void u_timelineWait(float duration)`**: *Add to the main timeline*: wait for `duration` seconds.
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
* **`void m_messageAdd(string message, float duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level.
* **`void m_messageAddImportant(string message, float duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will be printed during every run of the level.
* **`void m_messageAddImportantSilent(string message, float duration)`**: *Add to the event timeline*: print a message with text `message` for `duration` seconds. The message will only be printed during the first run of the level, and will not produce any sound.
* **`void m_clearMessages()`**: Remove all previously scheduled messages.
* **`void t_wait(float duration)`**: *Add to the main timeline*: wait for `duration` sixths of a second.
* **`void t_waitS(float duration)`**: *Add to the main timeline*: wait for `duration` seconds.
* **`void t_waitUntilS(float duration)`**: *Add to the main timeline*: wait until the timer reaches `duration` seconds.
* **`void e_eventStopTime(float duration)`**: *Add to the event timeline*: pause the game timer for `duration` sixths of a second.
* **`void e_eventStopTimeS(float duration)`**: *Add to the event timeline*: pause the game timer for `duration` seconds.
* **`void e_eventWait(float duration)`**: *Add to the event timeline*: wait for `duration` sixths of a second.
* **`void e_eventWaitS(float duration)`**: *Add to the event timeline*: wait for `duration` seconds.
* **`void e_eventWaitUntilS(float duration)`**: *Add to the event timeline*: wait until the timer reaches `duration` seconds.
* **`float l_getSpeedMult()`**: Return the `SpeedMult` field of the level status.
* **`void l_setSpeedMult(float value)`**: Set the `SpeedMult` field of the level status to `value`
* **`float l_getSpeedInc()`**: Return the `SpeedInc` field of the level status.
* **`void l_setSpeedInc(float value)`**: Set the `SpeedInc` field of the level status to `value`
* **`float l_getRotationSpeed()`**: Return the `RotationSpeed` field of the level status.
* **`void l_setRotationSpeed(float value)`**: Set the `RotationSpeed` field of the level status to `value`
* **`float l_getRotationSpeedInc()`**: Return the `RotationSpeedInc` field of the level status.
* **`void l_setRotationSpeedInc(float value)`**: Set the `RotationSpeedInc` field of the level status to `value`
* **`float l_getRotationSpeedMax()`**: Return the `RotationSpeedMax` field of the level status.
* **`void l_setRotationSpeedMax(float value)`**: Set the `RotationSpeedMax` field of the level status to `value`
* **`float l_getDelayMult()`**: Return the `DelayMult` field of the level status.
* **`void l_setDelayMult(float value)`**: Set the `DelayMult` field of the level status to `value`
* **`float l_getDelayInc()`**: Return the `DelayInc` field of the level status.
* **`void l_setDelayInc(float value)`**: Set the `DelayInc` field of the level status to `value`
* **`float l_getFastSpin()`**: Return the `FastSpin` field of the level status.
* **`void l_setFastSpin(float value)`**: Set the `FastSpin` field of the level status to `value`
* **`float l_getIncTime()`**: Return the `IncTime` field of the level status.
* **`void l_setIncTime(float value)`**: Set the `IncTime` field of the level status to `value`
* **`float l_getPulseMin()`**: Return the `PulseMin` field of the level status.
* **`void l_setPulseMin(float value)`**: Set the `PulseMin` field of the level status to `value`
* **`float l_getPulseMax()`**: Return the `PulseMax` field of the level status.
* **`void l_setPulseMax(float value)`**: Set the `PulseMax` field of the level status to `value`
* **`float l_getPulseSpeed()`**: Return the `PulseSpeed` field of the level status.
* **`void l_setPulseSpeed(float value)`**: Set the `PulseSpeed` field of the level status to `value`
* **`float l_getPulseSpeedR()`**: Return the `PulseSpeedR` field of the level status.
* **`void l_setPulseSpeedR(float value)`**: Set the `PulseSpeedR` field of the level status to `value`
* **`float l_getPulseDelayMax()`**: Return the `PulseDelayMax` field of the level status.
* **`void l_setPulseDelayMax(float value)`**: Set the `PulseDelayMax` field of the level status to `value`
* **`float l_getPulseDelayHalfMax()`**: Return the `PulseDelayHalfMax` field of the level status.
* **`void l_setPulseDelayHalfMax(float value)`**: Set the `PulseDelayHalfMax` field of the level status to `value`
* **`float l_getBeatPulseMax()`**: Return the `BeatPulseMax` field of the level status.
* **`void l_setBeatPulseMax(float value)`**: Set the `BeatPulseMax` field of the level status to `value`
* **`float l_getBeatPulseDelayMax()`**: Return the `BeatPulseDelayMax` field of the level status.
* **`void l_setBeatPulseDelayMax(float value)`**: Set the `BeatPulseDelayMax` field of the level status to `value`
* **`float l_getRadiusMin()`**: Return the `RadiusMin` field of the level status.
* **`void l_setRadiusMin(float value)`**: Set the `RadiusMin` field of the level status to `value`
* **`float l_getWallSkewLeft()`**: Return the `WallSkewLeft` field of the level status.
* **`void l_setWallSkewLeft(float value)`**: Set the `WallSkewLeft` field of the level status to `value`
* **`float l_getWallSkewRight()`**: Return the `WallSkewRight` field of the level status.
* **`void l_setWallSkewRight(float value)`**: Set the `WallSkewRight` field of the level status to `value`
* **`float l_getWallAngleLeft()`**: Return the `WallAngleLeft` field of the level status.
* **`void l_setWallAngleLeft(float value)`**: Set the `WallAngleLeft` field of the level status to `value`
* **`float l_getWallAngleRight()`**: Return the `WallAngleRight` field of the level status.
* **`void l_setWallAngleRight(float value)`**: Set the `WallAngleRight` field of the level status to `value`
* **`float l_get3dEffectMultiplier()`**: Return the `3dEffectMultiplier` field of the level status.
* **`void l_set3dEffectMultiplier(float value)`**: Set the `3dEffectMultiplier` field of the level status to `value`
* **`int l_getCameraShake()`**: Return the `CameraShake` field of the level status.
* **`void l_setCameraShake(int value)`**: Set the `CameraShake` field of the level status to `value`
* **`bool l_getSwapEnabled()`**: Return the `SwapEnabled` field of the level status.
* **`void l_setSwapEnabled(bool value)`**: Set the `SwapEnabled` field of the level status to `value`
* **`bool l_getTutorialMode()`**: Return the `TutorialMode` field of the level status.
* **`void l_setTutorialMode(bool value)`**: Set the `TutorialMode` field of the level status to `value`
* **`bool l_getIncEnabled()`**: Return the `IncEnabled` field of the level status.
* **`void l_setIncEnabled(bool value)`**: Set the `IncEnabled` field of the level status to `value`
* **`bool l_getDarkenUnevenBackgroundChunk()`**: Return the `DarkenUnevenBackgroundChunk` field of the level status.
* **`void l_setDarkenUnevenBackgroundChunk(bool value)`**: Set the `DarkenUnevenBackgroundChunk` field of the level status to `value`
* **`void l_enableRndSideChanges(bool enabled)`**: Set random side changes to `enabled`.
* **`void l_darkenUnevenBackgroundChunk(bool enabled)`**: If `enabled` is true, one of the background's chunks will be darkened in case there is an uneven number of sides.
* **`void l_addTracked(string variable, string name)`**: Add the variable `variable` to the list of tracked variables, with name `name`. Tracked variables are displayed in game, below the game timer.
* **`void l_setRotation(float angle)`**: Set the background camera rotation to `angle` radians.
* **`float l_getRotation()`**: Return the background camera rotation, in radians.
* **`float l_getLevelTime()`**: Get the current game timer value, in seconds.
* **`bool l_getOfficial()`**: Return `true` if "official mode" is enabled, `false` otherwise.
* **`float s_getHueMin()`**: Return the `HueMin` field of the style data.
* **`void s_setHueMin(float value)`**: Set the `HueMin` field of the style data to `value`
* **`float s_getHueMax()`**: Return the `HueMax` field of the style data.
* **`void s_setHueMax(float value)`**: Set the `HueMax` field of the style data to `value`
* **`float s_getHueInc()`**: Return the `HueInc` field of the style data.
* **`void s_setHueInc(float value)`**: Set the `HueInc` field of the style data to `value`
* **`float s_getHueIncrement()`**: Return the `HueIncrement` field of the style data.
* **`void s_setHueIncrement(float value)`**: Set the `HueIncrement` field of the style data to `value`
* **`float s_getPulseMin()`**: Return the `PulseMin` field of the style data.
* **`void s_setPulseMin(float value)`**: Set the `PulseMin` field of the style data to `value`
* **`float s_getPulseMax()`**: Return the `PulseMax` field of the style data.
* **`void s_setPulseMax(float value)`**: Set the `PulseMax` field of the style data to `value`
* **`float s_getPulseInc()`**: Return the `PulseInc` field of the style data.
* **`void s_setPulseInc(float value)`**: Set the `PulseInc` field of the style data to `value`
* **`float s_getPulseIncrement()`**: Return the `PulseIncrement` field of the style data.
* **`void s_setPulseIncrement(float value)`**: Set the `PulseIncrement` field of the style data to `value`
* **`bool s_getHuePingPong()`**: Return the `HuePingPong` field of the style data.
* **`void s_setHuePingPong(bool value)`**: Set the `HuePingPong` field of the style data to `value`
* **`float s_getMaxSwapTime()`**: Return the `MaxSwapTime` field of the style data.
* **`void s_setMaxSwapTime(float value)`**: Set the `MaxSwapTime` field of the style data to `value`
* **`float s_get3dDepth()`**: Return the `3dDepth` field of the style data.
* **`void s_set3dDepth(float value)`**: Set the `3dDepth` field of the style data to `value`
* **`float s_get3dSkew()`**: Return the `3dSkew` field of the style data.
* **`void s_set3dSkew(float value)`**: Set the `3dSkew` field of the style data to `value`
* **`float s_get3dSpacing()`**: Return the `3dSpacing` field of the style data.
* **`void s_set3dSpacing(float value)`**: Set the `3dSpacing` field of the style data to `value`
* **`float s_get3dDarkenMult()`**: Return the `3dDarkenMult` field of the style data.
* **`void s_set3dDarkenMult(float value)`**: Set the `3dDarkenMult` field of the style data to `value`
* **`float s_get3dAlphaMult()`**: Return the `3dAlphaMult` field of the style data.
* **`void s_set3dAlphaMult(float value)`**: Set the `3dAlphaMult` field of the style data to `value`
* **`float s_get3dAlphaFalloff()`**: Return the `3dAlphaFalloff` field of the style data.
* **`void s_set3dAlphaFalloff(float value)`**: Set the `3dAlphaFalloff` field of the style data to `value`
* **`float s_get3dPulseMax()`**: Return the `3dPulseMax` field of the style data.
* **`void s_set3dPulseMax(float value)`**: Set the `3dPulseMax` field of the style data to `value`
* **`float s_get3dPulseMin()`**: Return the `3dPulseMin` field of the style data.
* **`void s_set3dPulseMin(float value)`**: Set the `3dPulseMin` field of the style data to `value`
* **`float s_get3dPulseSpeed()`**: Return the `3dPulseSpeed` field of the style data.
* **`void s_set3dPulseSpeed(float value)`**: Set the `3dPulseSpeed` field of the style data to `value`
* **`float s_get3dPerspectiveMult()`**: Return the `3dPerspectiveMult` field of the style data.
* **`void s_set3dPerspectiveMult(float value)`**: Set the `3dPerspectiveMult` field of the style data to `value`
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
