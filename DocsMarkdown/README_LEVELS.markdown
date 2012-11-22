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
Basic gameplay parameters. Should be easy to understand them just by looking at the name.

`"events":[]`</br>
Level event scripting. Refer to README_SCRIPTING.