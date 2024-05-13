#!/bin/bash

function copyTo
{
    cp ./_deps/zlib-build/libzlib.dll $1 &
    cp /c/msys64/mingw64/bin/libstdc++-6.dll $1 &
    cp /c/msys64/mingw64/bin/libgcc_s_seh-1.dll $1 &
    cp /c/msys64/mingw64/bin/libwinpthread-1.dll $1 &
    cp /c/msys64/mingw64/bin/libopenal-1.dll $1 &
}

copyTo "../_RELEASE"
copyTo "./test"

cp ../_RELEASE/discord_game_sdk.dll ./test &
cp ../_RELEASE/steam_api64.dll ./test &
cp ../_RELEASE/sdkencryptedappticket64.dll ./test &

wait
