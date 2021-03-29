## Major Changes

- Changed ticks per second from 120TPS to 240TPS, improving the game's precision during collision detection and movement

- Level packs can now depend on other packs in order to promote code reuse and avoid repetition

- Change Lua runtime to LuaJIT, having 20x better performance on average
    - Caveat: supports features only up to Lua 5.1

- Added in-game debugging and experimentation tools in *Debug Mode*:
    - (F1): toggle Lua console, allowing to run code on demand and track variables in real-time
    - (F2): pause and un-pause the game -- Lua console is usable when the game is paused



## New Levels

- Added a new work-in-progress official level pack, *Orthoplex*, containing:
    - *Arcadia*: an unusual level with wavy patterns made out of custom walls
    - *Bipolarity*: fast-paced barrages and forced 180° player swaps



## Level Improvements

- Perfectly synchronized pulse and beatpulse with the music for all built-in levels
    - Exceptions: *Golden Ratio* and *Acceleradiant*, as they will be reworked in the future

- Moved *Baby Steps* to a new pack, *Tutorial*, making its role more clear to new players

- Added a graphical transition effect to the player's triangle when focusing/unfocusing



## Minor Improvements

- Added a few milliseconds of ignoring input after death to avoid accidental restarts

- Added new *Show Level Info* option that displays information about the current level in the bottom-left corner of the screen, useful for streaming/recording

- Added options to hide the timer and status text, useful for cinematic recordings

- Added **-printLuaDocs** command line argument to print out Lua documentation



## Bug Fixes

- Fixed crash when attempting to change keyboard bindings

- Fixed broken antialiasing menu setting, now supports up to 16x

- Fixed **t_eval** crashing the game on Lua error



## New Lua Functions

(Please refer to the [Lua reference on the official wiki](https://github.com/SuperV1234/SSVOpenHexagon/wiki/Lua-Reference) for a more detailed explanation.)

- **u_execDependencyScript**: to load a script from one of the pack's dependencies
    - As opposed to **u_execScript**, which loads a script from the current pack

- **l_[set/get]ManualPulseControl**: when set to **true** allows Lua scripts to manually control pulse values
    - **l_[set/get]Pulse**
    - **l_[set/get]PulseDirection**
    - **l_[set/get]PulseDelay**

- **l_[set/get]ManualBeatPulseControl**: when set to **true** allows Lua scripts to manually control beatpulse values
    - **l_[set/get]BeatPulse**
    - **l_[set/get]BeatPulseDelay**

- **l_[get/set]pulseInitialDelay**: control initial delay before any pulse effect starts, useful to sync up a level with the song's beat or to wait until the music reaches a certain point

- Custom wall optimized functions
    - **cw_createDeadly**: more optimized way of creating a deadly custom wall and getting back an handle
    - **cw_createNoCollision**: more optimized way of creating a no-collision custom wall and getting back an handle
    - **cw_moveVertexPos**: more optimized way of moving a custom wall's vertex by a specified offset
    - **cw_moveVertexPos4Same**: very optimized way of moving all vertices of a custom wall with the same offset
    - **cw_[set/get]VertexPos4**: more optimized way of controlling all vertex positions of a custom wall
    - **cw_[set/get]VertexColor4**: more optimized way of controlling all vertex colors of a custom wall
    - **cw_setVertexColor4Same**: very optimized way of setting all vertex colors of a custom wall to the same color

- **u_inMenu()**: returns **true** if a Lua script is being executed from the menu screen, **false** otherwise

- **s_getMainColor**, **s_getPlayerColor**, **s_getTextColor**, **s_get3DOverrideColor**, **s_getCapColorResult**, **s_getColor**: return a RGBA tuple of the current style's corresponding color



## Lua Function Cleanup

- Removed unused **pulseDelayHalf** and related Lua functions



## Miscellaneous

- Added a **pulseHelper.html** JavaScript tool in the GitHub repository to help level makers calculate pulse values that are in sync with the music's BPM
    - Beatpulse delay should be set to 3000/BPM (or one of its multiples)
    - Pulse values can be calculated through the tool -- the displayed equation should be as close to 3000/BPM (or one of its multiples) as possible



## Community Spotlight

- Check out the *Open Hexagon - Custom Level Aesthetics* video, showcasing levels from two excellent packs
    - *Polygonal Cubics*, by Babadrake
    - *The Travel Pack*, by Syyrion

- https://www.youtube.com/watch?v=h4Jfj3lzWD4

















