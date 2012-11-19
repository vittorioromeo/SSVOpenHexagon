Level scripts can be used to control the game parameters and to do 
more cool stuff during gameplay. They make every level unique and more
interesting. Here's a list of commands:

* means it has to be implemented

"event_time_stop" 		("duration")
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

"level_change"	("id")
Changes the level to "id", saving progress.

"menu"
Returns to menu, saving progress.


