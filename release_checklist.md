# Release Checklist

## Server

1. `git push` from main development machine, check the branch

2. Start the `endeavouros64` VirtualBox virtual machine

3. Go into `SSVOpenHexagon` directory

4. Check the repository's branch on the virtual machine

5. Run `./vbox/build_and_upload_server.sh`

6. Copy libs with:

    ```bash
    for d in $(find _deps | grep "\.so"); do scp $d vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx; done
    ```

7. Verify that the server is working with `ssh vittorioromeo@139.162.199.162`, `sudo journalctl -u openhexagon-server -f`

## Linux Client

1. `git push` from main development machine, check the branch

2. Start the `ubuntu2204lts` VirtualBox virtual machine

3. Go into `SSVOpenHexagon` directory

4. Check the repository's branch on the virtual machine

5. `cd buildlx && ./make_release_client_vbox.sh && cd .. && ./prepare_release_linux.sh`

6. `cd _PREPARED_RELEASE_LINUX_TEST && ./run_ssvopenhexagon_linux.sh`

7. The Linux client build will automatically be copied to the main development machine's drive

8. Run `/c/OHWorkspace/steamworks/sdk/tools/SteamPipeGUI.exe` and upload to Steam

    - Use depot ID `1358092`, build path `C:\OHWorkspace\SSVOpenHexagon\_PREPARED_RELEASE_LINUX`
-
## Windows Client

1. Run `SSVOpenHexagon/buildrel/make_release_client_win10_msys.sh`

2. Run `SSVOpenHexagon/prepare_release.sh`

3. Try the game in `SSVOpenHexagon/_PREPARED_RELEASE_TEST`, make sure everything works

    - Verify basic gameplay

    - Verify server connection and login

    - Verify leaderboards

    - Verify replays

4. Run `/c/OHWorkspace/steamworks/sdk/tools/SteamPipeGUI.exe` and upload to Steam

    - Use depot ID `1358091`, build path `C:\OHWorkspace\SSVOpenHexagon\_PREPARED_RELEASE`

5. For non-betas, go to <https://partner.steamgames.com/apps/builds/1358090>, log in with build account (not personal one), and put the latest build live

## Lua Reference

1. Temporarily change `CMakeLists.txt` to enable Lua Reference code

2. Go to `SSVOpenHexagon/_RELEASE`

3. Run `./SSVOpenHexagon.exe -printLuaDocs > temp.md`

4. Open `temp.md` with an editor, find `## Utility Function` as the starting point and copy the Lua docs

5. Go to <https://github.com/SuperV1234/SSVOpenHexagon/wiki/Lua-Reference/_edit> and paste them there, after the `<!-- START GENERATED DOCS HERE -->` marker

## Patch Notes

1. Update `SSVOpenHexagon/art/eventcover.psd`

2. Go to <https://steamcommunity.com/games/1358090/partnerevents/create/>

3. Do the thing (formatting guidelines here: <https://steamcommunity.com/comment/Guide/formattinghelp>)

## Discord

1. Make post in `#announcements`

## Making a pack ranked

1. Get level validator strings, it's easy by adding a print in `LevelData::LevelData`.

2. Put level validator strings in `Config.cpp`.

3. TODO

## Other

- Ubuntu machine packages:

    - sudo apt-get install build-essential clang++-12 g++ g++-12 git libfreetype6-dev libgl1-mesa-dev libglew-dev libjpeg-dev libopenal-dev libpthread-stubs0-dev libsndfile1-dev libx11-dev libxrandr-dev lld ninja-build xorg-dev xserver-xorg-dev libudev-dev vim steam
