Style json files in Open Hexagon contain all the
information to draw the background of any level.

It's easier to copy-paste an existing style file
and edit it.

Styles must have an unique id (that will be used
in the level json files).

"hue_min" and "hue_max" represent the range of
colors that will be used.
"hue_ping_pong" will make the colors loop if
set to false, otherwise they will go back and
forth.
"hue_increment" is the speed of color changes.
"hue_pulse" makes the background pulse using
the hue as a reference.

We have 3 colors:
	main: used by player, walls and text
	a: used by background
	b: used by background
	
These colors can either be:
	static: they don't change using hue
	dynamic: they change with hue
	
If the colors are dynamic you can set:
	"dynamic_darkness": darkness of the color
	"dynamic_offset": subtract static color from
					  the main color

The "?_static" members contain a color in the
[R, G, B, ALPHA] format that will be used if
the color is static or "dynamic_offset" is true.