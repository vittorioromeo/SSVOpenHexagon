# Scripting #

*Patterns and levels must be scripted using LUA. LUA is a versatile scripting language that allows the user to create any kind of pattern.*

[http://www.lua.org/manual/5.1/manual.html](http://www.lua.org/manual/5.1/manual.html "LUA manual")

There are many LUA editors that can check your syntax and make you avoid typos: I use [http://sourceforge.net/projects/luaedit/](http://sourceforge.net/projects/luaedit/)

----------

## Hardcoded LUA commands ##

*These commands can be called like any other function. They are hardcoded because they directly communicate with the C++ engine.*

`log(mLog)` </br>
Sends a log message to the console. Very useful for debugging!

`getSides()` </br>
Returns the current number of sides.

`getSpeedMult()` </br>
Returns the current speed multiplier.

`getDelayMult()` </br>
Returns the current delay multiplier.

`execScript(mFilename)` </br>
Executes another LUA file. Can both be used to include useful functions, and to execute a series of commands.
Example: `execFile("test.lua")`

`wait(mDuration)` </br>
Makes the pattern timeline wait `mDuration` frames. This will halt pattern spawning. If the timeline has no more commands, the `"lua_file"` of the level is executed again from the beginning.

`wall(mSide, mThickness)` </br>
Creates a wall with default speed, at side `mSide`, with thickness `mThickness`. Useful wall creation functions are commonly defined in `common.lua` and `commonpatterns.lua`. 

`wallAdj(mSide, mThickness, mSpeedAdj)` </br>
Creates a wall with default speed multiplied by `mSpeedAdj`, at side `mSide`, with thickness `mThickness`.

`execEvent(mId)` </br>
Executes an event file with id `mId` immediately. Similar to event command `"event_exec"`.

`enqueueEvent(mId)` </br>
Enqueues an event file with id `mId`. Similar to event command `"event_enqueue"`.

`getLevelValueInt(mValueName)` </br>
`getLevelValueFloat(mValueName)` </br> 
`getLevelValueString(mValueName)` </br> 
`getLevelValueBool(mValueName)` </br> 
`setLevelValueInt(mValueName, mValue)` </br>
`setLevelValueFloat(mValueName, mValue)`</br> 
`setLevelValueString(mValueName, mValue)` </br> 
`setLevelValueBool(mValueName, mValue)` </br> 
Manipulate the current level values. This allows you to change any value that is present in the level .json file. You can also create new values and use them as 'global variables' for your LUA scripts. Nothing will be saved to the actual .json file, it only works in memory.

`getStyleValueInt(mValueName)` </br>
`getStyleValueFloat(mValueName)` </br> 
`getStyleValueString(mValueName)` </br> 
`getStyleValueBool(mValueName)` </br> 
`setStyleValueInt(mValueName, mValue)` </br>
`setStyleValueFloat(mValueName, mValue)`</br> 
`setStyleValueString(mValueName, mValue)` </br> 
`setStyleValueBool(mValueName, mValue)` </br> 
Manipulate the current style values. This allows you to change any value that is present in the style .json file. You can also create new values and use them as 'global variables' for your LUA scripts. Nothing will be saved to the actual .json file, it only works in memory.

`playSound(mId)` </br>
Plays a sound with id mId.

`forceIncrement()` </br>
Increments the level difficulty.

`messageAdd(mMessage, mDuration)` </br>
Adds a message to the message queue.

`messageImportantAdd(mMessage, mDuration)` </br>
Adds an important message to the message queue.

`getDifficultyMult()` </br>
Gets the automatic difficulty variant multiplier.

`isKeyPressed(mKey)` </br>
Returns true if mKey is pressed. 0=A, 1=B, 3=C... (refer to [http://www.sfml-dev.org/documentation/2.0/classsf_1_1Keyboard.php](http://www.sfml-dev.org/documentation/2.0/classsf_1_1Keyboard.php))

`isFastSpinning()` </br>
Returns true if the game is currently doing a "fast spin".

----------

## Hardcoded level LUA functions ##

These functions are **REQUIRED** in level script files.

`function onLoad() ... end` </br>
This functions runs when the level starts or is restarted.

`function onStep() ... end` </br>
This function runs when the level timeline is empty. You should place your pattern selection logic here.

`function onIncrement() ... end` </br>
This function runs when the difficulty is incremented.

`function onUnload() ... end` </br>
This function runs when the level is closed or restarted.

`function onUpdate(mFrameTime) ... end` </br>
This function runs every frame.