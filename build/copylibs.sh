#!/bin/bash

cp ./_deps/imgui-sfml-build/libImGui-SFML_d.dll ../_RELEASE/
cp ./_deps/imgui-sfml-build/libImGui-SFML.dll ../_RELEASE/

cp ./_deps/sfml-build/lib/libsfml-graphics-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-system-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-window-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-network-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-audio-d-2.dll ../_RELEASE/

cp ./_deps/sfml-build/lib/sfml-graphics-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/sfml-system-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/sfml-window-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/sfml-network-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/sfml-audio-2.dll ../_RELEASE/

cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll ../_RELEASE/
cp ./_deps/libsodium-cmake-build/libsodium.dll ../_RELEASE/
cp ./_deps/luajit-build/src/libluajit.dll ../_RELEASE/
cp ./_deps/zlib-build/libzlib1.dll ../_RELEASE/

cp ./_deps/imgui-sfml-build/libImGui-SFML_d.dll ./test/
cp ./_deps/imgui-sfml-build/libImGui-SFML.dll ./test/

cp ./_deps/sfml-build/lib/libsfml-graphics-d-2.dll ./test/
cp ./_deps/sfml-build/lib/libsfml-system-d-2.dll ./test/
cp ./_deps/sfml-build/lib/libsfml-window-d-2.dll ./test/
cp ./_deps/sfml-build/lib/libsfml-network-d-2.dll ./test/
cp ./_deps/sfml-build/lib/libsfml-audio-d-2.dll ./test/

cp ./_deps/sfml-build/lib/sfml-graphics-2.dll ./test/
cp ./_deps/sfml-build/lib/sfml-system-2.dll ./test/
cp ./_deps/sfml-build/lib/sfml-window-2.dll ./test/
cp ./_deps/sfml-build/lib/sfml-network-2.dll ./test/
cp ./_deps/sfml-build/lib/sfml-audio-2.dll ./test/

cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll ./test/
cp ./_deps/libsodium-cmake-build/libsodium.dll ./test/
cp ./_deps/luajit-build/src/libluajit.dll ./test/
cp ./_deps/zlib-build/libzlib1.dll ./test/

cp /c/msys64/mingw64/bin/libssp-0.dll ../_RELEASE/
cp /c/msys64/mingw64/bin/libssp-0.dll ./test/

cp /c/msys64/mingw64/bin/libstdc++-6.dll ../_RELEASE/
cp /c/msys64/mingw64/bin/libstdc++-6.dll ./test/

cp /c/msys64/mingw64/bin/libgcc_s_seh-1.dll ../_RELEASE/
cp /c/msys64/mingw64/bin/libgcc_s_seh-1.dll ./test/

cp /c/msys64/mingw64/bin/libwinpthread-1.dll ../_RELEASE/
cp /c/msys64/mingw64/bin/libwinpthread-1.dll ./test/

cp ../_RELEASE/discord_game_sdk.dll ./test
cp ../_RELEASE/steam_api64.dll ./test
cp ../_RELEASE/sdkencryptedappticket64.dll ./test
