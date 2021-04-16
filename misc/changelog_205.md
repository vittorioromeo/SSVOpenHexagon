### Major Changes

- Added online leaderboards to the game. See "Online Leaderboards" for more information.

- Added mouse support to the main menu and level selection screen. See "Menu Mouse Support" for more information.

- Game configuration does not depend on an external file anymore. See "Config Improvements" for more information.

- Greatly improved loading speed of assets (packs, levels, sounds, music, etc...) on game start-up.



### Online Leaderboards

- Open Hexagon now communicates with a server that can receive and validate replays in order to keep track of the best times.

- The top scores for each level are displayed in the level selection screen.

- In order to view and submit scores to the server, Open Hexagon players must now register with a unique name and password.

- Only one online account per Steam ID can be registered.

- Registration and login is performed directly from the new "Online" in-game menu.

- Scores will only be sent to the server when all of the following apply:

    - The player is registered and logged in.

    - Official mode is enabled.

    - The server supports the level you want to play. For now, only official levels are supported. Curated workshop packs will be chosen and added to the list of supported levels in the future.

    - The player stays connected to the server for the full duration of the gameplay session. Replays recorded offline or losing connection during a playthrough will invalidate the score submission.

    - The level files are untempered with. Modifying Lua or JSON files related to a level will invalidate the score submission.

- After hitting a wall on a ranked level (i.e. supported by the server), the generated replay will be encrypted and sent to the server. The server will then execute the replay internally, verifying its correctness. If everything goes well, the final score will be added to the official leaderboards.

- Scores retrieved from the server are cached for 6 seconds to avoid excessive load.

- Timestamps in the online leaderboard are displayed in the UTC timezone.

- Despite the presence of multiple security mechanisms to prevent cheating, any "unusual" scores will be manually checked and -- in the presence of cheating evidence -- the submitting Steam ID will be *permanently* banned from Open Hexagon online services. No exceptions. No ban removals.

    - Any security vulnerability or cheating avenue discovered should be privately reported to Vittorio Romeo via email (vittorio.romeo@outlook.com) or as a private message to "vee" on the official Discord server (http://discord.me/openhexagon). Please help keep the Open Hexagon community safe and cheater-free.



### Menu Mouse Support

- Menu items can now be selected and clicked with the mouse.

- If your mouse cursor is not visible, ensure that `"mouse_visible"` is set to `true` in your `config.json`.

- Double-clicking on a level in the level selection screen will start the game.

- Using the mouse wheel in the level selection screen will smoothly scroll the available levels and packs up and down.



### Config Improvements

- Removed the `default_config.json` file. Default configuration values are now hardcoded in the Open Hexagon executable.

- Starting Open Hexagon with missing configuration values in `config.json` will result in those being filled in from the hardcoded configuration values.



### Other Improvements

- The game window can now be seamlessly resized by dragging when in windowed mode.

- Level packs with missing dependencies will be displayed on the main menu screen.



### Bug Fixes

- Fixed an issue where custom walls would be spawned without collision.

- Fixed sounds not playing when many level packs are loaded due to an internal limitation of the sound player. There is now no explicit limit for loaded assets.

- Fixed an integer overflow issue related to 3D alpha and falloff values during 3D rendering. Some levels might look slightly different and the aforementioned values might have to be adjusted by pack developers to retain the original look.

- Fixed a minor graphical issue with the text "ROTATE TO START" appearing even if the corresponding option was disabled.



### Official Levels

- Tweaked pulse settings to ensure that the pulse effect matches the beat in "Aperoigon", "Flattering Shape", "Second Dimension", and "G-Force".

- Tweaked 3D effects in most official levels to reduce visual noise.

- Improved the tutorial ("Baby Steps") further by tweaking the text messages and objectives.



### Internal (C++)

- Added automated testing system that generates random replays and verifies their integrity.

- Added the new following JSON configuration options:

    - `"server_ip"` -- IPV4 address of the Open Hexagon server.

    - `"server_port"` -- port of the Open Hexagon server.

    - `"server_control_port"` -- control port of the Open Hexagon server. Used to send special control messages to the server for debugging.

- Major compilation time improvements:

    - Greatly refactored the game's codebase to avoid physical dependencies between components.

    - Added PCH support (pre-compiled headers) to the CMake build.

    - Added unity build support to the CMake build.

- `hg::HexagonDialogBox` now supports keyboard input (used for login and registration).

- Removed dependencies on `SSVU_ASSERT`, `ssvs::AssetManager`, and `ssvs::MusicPlayer`. Custom-made solutions have been written from scratch instead.

- Added dependencies on the following libraries (for the online leaderboard feature): `libsodium`, `sqlite3`, `sqlite_orm`, `boost::pfr`.

- Removed many obsolete or unused library functions, especially related to JSON.

- Improved performance of file reading and JSON parsing.

- Added an implementation of a thread pool, currently unused.

- Added new `OHServerControl` tool to communicate with the server directly.

