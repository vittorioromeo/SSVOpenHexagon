#!/bin/bash

function copyTo
{
    cp ./_deps/imgui-sfml-build/libImGui-SFML.dll $1 &

    cp ./_deps/sfml-build/lib/sfml-graphics-3.dll $1 &
    cp ./_deps/sfml-build/lib/sfml-system-3.dll $1 &
    cp ./_deps/sfml-build/lib/sfml-window-3.dll $1 &
    cp ./_deps/sfml-build/lib/sfml-network-3.dll $1 &
    cp ./_deps/sfml-build/lib/sfml-audio-3.dll $1 &

    cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll $1 &
    cp ./_deps/libsodium-cmake-build/libsodium.dll $1 &
    cp ./_deps/luajit-build/src/libluajit.dll $1 &
    cp ./_deps/zlib-build/libzlib1.dll $1 &

    cp /c/msys64/mingw64/bin/libssp-0.dll $1 &
    cp /c/msys64/mingw64/bin/libstdc++-6.dll $1 &
    cp /c/msys64/mingw64/bin/libgcc_s_seh-1.dll $1 &
    cp /c/msys64/mingw64/bin/libwinpthread-1.dll $1 &
}

copyTo "../_RELEASE"
copyTo "./test"

cp ../_RELEASE/discord_game_sdk.dll ./test &
cp ../_RELEASE/steam_api64.dll ./test &
cp ../_RELEASE/sdkencryptedappticket64.dll ./test &

wait
