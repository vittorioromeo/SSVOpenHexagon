# Levels #

*Level files contain all the information about a level and the event script.*

----------


`"id"` </br>An unique identifier that will be used from other files to refer to the level.

`"name"` </br>
Cosmetic identifier, shows up in the menu as the level name.

`"style_id"`, `"music_id"`</br>
Unique identifiers of the style and music files the level uses.

`"lua_file"`</br>
Lua file the level uses. It is executed every time the timeline is empty. Think about it as an infinite loop.

`"speed_multiplier"`</br>
`"speed_increment"`</br>
`"rotation_speed"`</br>
`"rotation_increment"`</br>
`"delay_multiplier"`</br>
`"delay_increment"`</br>
`"fast_spin"`</br>
`"sides"`</br>
`"sides_min"`</br>
`"sides_max"`</br>
`"increment_time"`</br>
`"rotation_speed_max"`</br>
Basic gameplay parameters. Should be easy to understand them just by looking at the name.

`"wall_skew_left"`</br>
`"wall_skew_right"`</br>
`"wall_angle_left"`</br>
`"wall_angle_right"`</br>
Graphical effect parameters.

`"pulse_min"`</br>
`"pulse_max"`</br>
Min/max values for the pulse zoom effect.

`"pulse_speed"`</br>
`"pulse_speed_r"`</br>
Speed and reverse speed of the pulse zoom effect.

`"pulse_delay_max"`</br>
`"pulse_delay_half_max"`</br>
Time between every pulse zoom. The second parameter is taken into account 2 times per zoom (one when the zoom is at the max value, one when the zoom is at the min value).


`"beatpulse_max"`</br>
`"beatpulse_delay_max"`</br>
These two parameters control the center polygon (the one the player spins around). The first one is the maximum added radius it can reach, the second one how much time passes from a radius expansion to another radius expansion.

`"events":[]`</br>
Level event scripting. Refer to README_EVENTS.