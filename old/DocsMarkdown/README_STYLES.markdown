# Styles #

Style json files in Open Hexagon contain all the
information to draw the background of any level.

It's easier to copy-paste an existing style file
and edit it.

Styles must have an unique id (that will be used
in the level json files).

`"hue_min"` and `"hue_max"` represent the range of
colors that will be used. </br>
`"hue_ping_pong"` will make the colors loop if
set to false, otherwise they will go back and
forth.</br>
`"hue_increment"` is the speed of color changes.</br>


----------

	
Colors can either be:</br>
	static: they don't change using hue</br>
	dynamic: they change with hue
	
If the colors are dynamic you can set:</br>
	`"dynamic_darkness"`: darkness of the color</br>
	`"dynamic_offset"`: subtract static color from
					  the main color

The `"?_static"` members contain a color in the
[R, G, B, ALPHA] format that will be used if
the color is static or `"dynamic_offset"` is true.


----------

## 3D effects ##

`"3D_depth"` (default: 15)</br>
Number of 3D layers

`"3D_skew"` (default: 0.18)</br>
3D perspective angle

`"3D_spacing"` (default: 1.0)</br>
Spacing between 3D layers

`"3D_darken_multiplier"` (default: 1.5)</br>
How much the color gets darkened layer after layer

`"3D_alpha_multiplier"` (default: 0.5)</br>
How much the color gets more transparent layer after layer

`"3D_alpha_falloff"` (default: 3.0)</br>
How much every layer is more transparent than the previous layer

`"3D_override_color"` (default: main color)</br>
Color of the 3D effect

`"3D_pulse_max"` (default: 3.2)</br>
Max 3D pulse (alters perspective)

`"3D_pulse_min"` (default: 0.0)</br>
Min 3D pulse (alters perspective)

`"3D_pulse_speed"` (default: 0.01)</br>
3D pulse speed (alters perspective)
