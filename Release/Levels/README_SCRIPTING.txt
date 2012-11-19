Level scripts can be used to control the game parameters and to do 
more cool stuff during gameplay. They make every level unique and more
interesting. Here's a list of commands:

* means it has to be implemented

"event_time_stop" 	("duration")
Stops the game timer for "duration" frames.

"timeline_wait" 	("duration")
Stops the pattern timeline for "duration" frames.

"timeline_clear"
Clears the pattern timeline, stopping all the patterns.

"timeline_add"* 	("pattern")
Adds "pattern" to the end of the pattern timeline.

"message_add"		("duration", "message")
Queues a text message with the string "message" that lasts for "duration" frames.

"message_clear"
Clears the screen from messages.

"value_float_add"	("value_name", "value")
"value_float_set"
"value_float_subtract"
"value_float_multiply"
"value_float_divide"
"value_int_add"
"value_int_set"
"value_int_subtract"
"value_int_multiply"
"value_int_divide"
Adds, sets, subtract, multiplies or divides a JSON value by the name "value_name" by "value".

"style_set" 		("id")
Changes the current style to "id".

"music_set"			("id")
Changes the current music to "id", with a random segment.

"music_set_segment" ("id", "segment_index")
Changes the current music to "id", at the segment number "segment_index".

"music_set_seconds"	("id", "seconds")
Changes the current music to "id", starting at "seconds".

"level_change"		("id")
Changes the level to "id", saving progress.

"side_changing_stop"
Stops random side number changes on level up.

"side_changing_start"
Resumes random side number changes on level up.

"increment_stop"
Stops level up increments.

"increment_start"
Resumes level up increments.

"pivot_pulse_max_set"	("value")
Sets how far the center pulses. Default is 85;

"pivot_pulse_speed_set"	("value")
Sets how fast the center pulses. Default is 1.

"menu"
Returns to menu, saving progress.


