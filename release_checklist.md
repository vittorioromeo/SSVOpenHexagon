# Release Checklist

## Server

1. Start the `endeavouros64` VirtualBox virtual machine

2. Run `SSVOpenHexagon/vbox/build_and_upload_server.sh`

3. Verify that the server is working with `ssh vittorioromeo@139.162.199.162`, `sudo journalctl -u openhexagon-server -f`

## Client

1. Run `SSVOpenHexagon/buildrel/make_release_client_win10_msys.sh`

2. Run `SSVOpenHexagon/prepare_release.sh`

3. Try the game in `SSVOpenHexagon/_PREPARED_RELEASE_TEST`, make sure everything works

    - Verify basic gameplay

    - Verify server connection and login

    - Verify leaderboards

    - Verify replays

4. Run `/c/OHWorkspace/steamworks/sdk/tools/SteamPipeGUI.exe` and upload to Steam

5. For non-betas, go to <https://partner.steamgames.com/apps/builds/1358090>, log in with build account (not personal one), and put the latest build live

## Lua Reference

1. Go to `SSVOpenHexagon/_RELEASE`

2. Run `./SSVOpenHexagon.exe -printLuaDocs > temp.md`

3. Open `temp.md` with an editor, find `## Utility Function` as the starting point and copy the Lua docs

4. Go to <https://github.com/SuperV1234/SSVOpenHexagon/wiki/Lua-Reference/_edit> and paste them there, after the `<!-- START GENERATED DOCS HERE -->` marker

## Patch Notes

1. Update `SSVOpenHexagon/art/eventcover.psd`

2. Go to <https://steamcommunity.com/games/1358090/partnerevents/create/>

3. Do the thing (formatting guidelines here: <https://steamcommunity.com/comment/Guide/formattinghelp>)

## Discord

1. Make post in `#announcements`
