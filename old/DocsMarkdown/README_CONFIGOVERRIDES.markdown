# Config overrides #

*Config overrides are used to override certain
parameters of the config.json files.*

Only the parameters that you specify will be
overridden, the other will be left untouched.

The config.json file will always be left 
untouched.

They are particularly useful to create shortcuts
for your favorite parameters.


----------


You can load SSVOpenHexagon.exe adding config
names as parameters in batch files (or by
command line), like this:

`SSVOpenHexagon.exe windowed`

This line makes Open Hexagon start by using
the "windowed.json" config override.

Remember you can only use parameters if the
corresponding config ovveride exists.