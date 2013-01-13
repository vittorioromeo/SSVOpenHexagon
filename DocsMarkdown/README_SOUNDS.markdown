# Sounds #

The sounds.json file is very simple.
Every member points to a different sound file.
Only .ogg vorbis is currently supported.

You can easily convert any music/sound file by
using the program "foobar2000", which can be
easily found on Google.

---

## Custom sounds ##

Custom sounds work differently from normal sounds.
They can only be in the `.ogg` extension, and must placed in `Packs/<pack name>/Sounds/`.

There is no need for a JSON file - to play your custom sounds via LUA scripts or events just call the following commands:

LUA: <br>
`playSound(<pack name>_<sound name without extension>)`

Event: <br>
`"play_sound" ("<pack name>_<sound name without extension>")`

Obviously, you have to replace the placeholders with the name of the pack folder and the filename of the sound (without the extension). Please remember names are case sensitive.

You can find a working example in the VeeEndurance pack.