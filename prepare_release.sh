#!/bin/bash

rm -Rf ./_PREPARED_RELEASE
rm -Rf ./_PREPARED_RELEASE_TEST
mkdir -p ./_PREPARED_RELEASE

cp -r ./_RELEASE/Assets ./_PREPARED_RELEASE
cp -r ./_RELEASE/ConfigOverrides ./_PREPARED_RELEASE

mkdir -p ./_PREPARED_RELEASE/Packs
cp -r ./_RELEASE/Packs/base ./_PREPARED_RELEASE/Packs
cp -r ./_RELEASE/Packs/tutorial ./_PREPARED_RELEASE/Packs
cp -r ./_RELEASE/Packs/cube ./_PREPARED_RELEASE/Packs
cp -r ./_RELEASE/Packs/hypercube ./_PREPARED_RELEASE/Packs
cp -r ./_RELEASE/Packs/orthoplex ./_PREPARED_RELEASE/Packs

mkdir -p ./_PREPARED_RELEASE/Profiles

cp ./_RELEASE/SSVOpenHexagon.exe ./_PREPARED_RELEASE
cp ./_RELEASE/SSVOpenHexagon-Console.exe ./_PREPARED_RELEASE

cp ./_RELEASE/libzlib1.dll ./_PREPARED_RELEASE
cp ./_RELEASE/openal32.dll ./_PREPARED_RELEASE
cp ./_RELEASE/sfml-audio-3.dll ./_PREPARED_RELEASE
cp ./_RELEASE/sfml-graphics-3.dll ./_PREPARED_RELEASE
cp ./_RELEASE/sfml-network-3.dll ./_PREPARED_RELEASE
cp ./_RELEASE/sfml-system-3.dll ./_PREPARED_RELEASE
cp ./_RELEASE/sfml-window-3.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libImGui-SFML.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libluajit.dll ./_PREPARED_RELEASE
cp ./_RELEASE/steam_api64.dll ./_PREPARED_RELEASE
cp ./_RELEASE/discord_game_sdk.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libsodium.dll ./_PREPARED_RELEASE
cp ./_RELEASE/sdkencryptedappticket64.dll ./_PREPARED_RELEASE

cp ./_RELEASE/libssp-0.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libstdc++-6.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libgcc_s_seh-1.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libwinpthread-1.dll ./_PREPARED_RELEASE
cp ./_RELEASE/libopenal-1.dll ./_PREPARED_RELEASE

cp ./_RELEASE/steam_appid.txt ./_PREPARED_RELEASE
cp ./_RELEASE/windowed.bat ./_PREPARED_RELEASE
cp ./_RELEASE/windowed_no3D.bat ./_PREPARED_RELEASE
cp ./_RELEASE/fullscreen.bat ./_PREPARED_RELEASE
cp ./_RELEASE/fullscreen_no3D.bat ./_PREPARED_RELEASE
cp ./_RELEASE/highfps.bat ./_PREPARED_RELEASE
cp ./_RELEASE/noaudio.bat ./_PREPARED_RELEASE

cp ./_RELEASE/OHWorkshopUploader.exe ./_PREPARED_RELEASE

cd ./_PREPARED_RELEASE
upx -9 ./*.dll
upx -9 ./*.exe
cd ..

cp -r ./_PREPARED_RELEASE ./_PREPARED_RELEASE_TEST
