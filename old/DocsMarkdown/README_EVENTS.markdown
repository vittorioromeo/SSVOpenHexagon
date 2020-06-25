# Events #

*Level event scripts can be used to control the game parameters and to do 
more cool stuff during gameplay. They make every level unique and more
interesting. Here's a list of commands:*

----------

`"event_time_stop" 	("duration")`</br>
Stops the game timer for "duration" frames.

`"timeline_wait" 	("duration")`</br>
Stops the pattern timeline for "duration" frames.

`"timeline_clear"`</br>
Clears the pattern timeline, stopping all the patterns.

`"message_add"		("duration", "message")`</br>
Queues a text message with the string "message" that lasts for "duration" frames.

`"message_clear"`</br>
Clears the screen from messages.

`"level_float_add"	("value_name", "value")`</br>
`"level_float_set"`</br>
`"level_float_subtract"`</br>
`"level_float_multiply"`</br>
`"level_float_divide"`</br>
`"level_int_add"`</br>
`"level_int_set"`</br>
`"level_int_subtract"`</br>
`"level_int_multiply"`</br>
`"level_int_divide"`</br>
Adds, sets, subtract, multiplies or divides a level JSON value by the name "value_name" by "value".

`"style_float_add"	("value_name", "value")`</br>
`"style_float_set"`</br>
`"style_float_subtract"`</br>
`"style_float_multiply"`</br>
`"style_float_divide"`</br>
`"style_int_add"`</br>
`"style_int_set"`</br>
`"style_int_subtract"`</br>
`"style_int_multiply"`</br>
`"style_int_divide"`</br>
Adds, sets, subtract, multiplies or divides a style JSON value by the name "value_name" by "value".

`"style_set" 		("id")`</br>
Changes the current style to "id".

`"music_set"			("id")`</br>
Changes the current music to "id", with a random segment.

`"music_set_segment" ("id", "segment_index")`</br>
Changes the current music to "id", at the segment number "segment_index".

`"music_set_seconds"	("id", "seconds")`</br>
Changes the current music to "id", starting at "seconds".

`"level_change"		("id")`</br>
Changes the level to "id", saving progress.

`"side_changing_stop"`</br>
Stops random side number changes on level up.

`"side_changing_start"`</br>
Resumes random side number changes on level up.

`"increment_stop"`</br>
Stops level up increments.

`"increment_start"`</br>
Resumes level up increments.

`"play_sound"	("id")`</br>
Plays sound with id "id".

`"menu"`</br>
Returns to menu, saving progress.

`"event_exec"	("id")`</br>
Executes event file with id "id" immediately.

`"event_enqueue"	("id")`</br>
Enqueues event file with id "id" in the event queue.

`"script_exec"	("value_name")`</br>
Runs LUA script with filename "value_name".</br> Example usage: `{ "type": "script_exec", "value_name": "test.lua" }`