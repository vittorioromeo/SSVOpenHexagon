# Online functionality #

*Since 1.8, Open Hexagon connects to an online server to check if new versions are available and to send high-scores.*

---

To enable or disable **online functionality**, press **F3** in the menu screen to open the options menu.

To be eligible for online scoring, you must enable **official mode**.</br>
**Official mode** locks some settings (rotation, pulse, player speed, etc...) in order to make the game fair for everyone competing for online scores.

To enable or disable **official mode**, press **F3** in the menu screen to open the options menu.

Both **online functionality** and **official mode** are enabled by default.

The high score server validates your score by receiving both the level JSON and LUA file. Editing the level files will still send your score, but it will appear only for you (and those who have the same modified files as you do).

**If you have performance issues (less than 20 FPS) and play in official mode, your score won't be sent.**

Connection to the server is encrypted thanks to a **secret key**. If you want to port Open Hexagon to another operating system, please contact me at **vittorio.romeo@outlook.com** to get the key.