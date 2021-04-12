#!/bin/bash

cp ./_deps/imgui-sfml-build/libImGui-SFML_d.dll ../_RELEASE/

cp ./_deps/sfml-build/lib/libsfml-graphics-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-system-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-window-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-network-d-2.dll ../_RELEASE/
cp ./_deps/sfml-build/lib/libsfml-audio-d-2.dll ../_RELEASE/

cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll ../_RELEASE/

cp ./_deps/libsodium-cmake-build/libsodium.dll ../_RELEASE/

cp ./_deps/luajit-build/src/libluajit.dll ../_RELEASE/

cp ./_deps/zlib-build/libzlib1.dll ../_RELEASE/
