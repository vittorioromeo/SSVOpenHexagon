# Music #

Music files have to be in the .ogg vorbis
format. I included the "vorbisgain.exe" program,
which helps removing distorsion caused by too
high volumes (it's a known SFML bug).

Simply place your music file in this folder,
then run vorbisgain.exe using the file as a 
parameter (either via command line or batch
file), like this:

`vorbisgain.exe drFinkelfracken.ogg`

After that, you're ready to make a .json file
for the music (it is required).

Copy paste an existing .json file, it's easier.

The "id" member requires you to invent an 
unique id for the music data. It doesn't have
to be the same as the music file. It mustn't
be the same as any other id.

The "file_name" member is the name of the 
music file with the .ogg extension.

Add your music information to make it show up
in the menu.

The "segments" array contains objects with
"time" members - every value is in seconds, and
represents the start of a segment in the song
which is suitable to gameplay.

If your song has a long intro, or different
parts that you want to hear in random sequences,
you can add the corresponding second as a new 
object in the "segments" array, like I've done
for the existing songs.

If you want the full songs to always play,
add a single object, with "time" equal to 0.

You can easily convert any music/sound file by
using the program "foobar2000", which can be
easily found on Google.