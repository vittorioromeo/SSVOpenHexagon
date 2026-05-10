#!/bin/bash

OH_VBOX_ROOT="/home/vittorioromeo/OHWorkspace/SSVOpenHexagon"
OH_SERVER_ROOT="/home/vittorioromeo/OHWorkspace/SSVOpenHexagon"

git pull

echo "Building server release..." && \
cd "$OH_VBOX_ROOT/buildlx" && \
./make_release_server_vbox.sh && \
echo "Done" && \
\
echo "Uploading build binary to server..." && \
scp "$OH_VBOX_ROOT/_RELEASE/SSVOpenHexagonVbox" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/_RELEASE/SSVOpenHexagonVbox" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/luajit-build/src/libluajit.so" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/luajit-build/src/libluajit.so" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/zlib-build/libz.so.1" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/zlib-build/libz.so.1"  && \
scp "$OH_VBOX_ROOT/buildlx/_deps/libsodium-cmake-build/libsodium.so" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/libsodium-cmake-build/libsodium.so" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/imgui-sfml-build/libImGui-SFML.so" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/imgui-sfml-build/libImGui-SFML.so"  && \
echo "Done" && \
\
echo "Running 'update_server.sh' on server..." && \
ssh vittorioromeo@139.162.199.162 -f "cd $OH_SERVER_ROOT/server && ./update_server.sh" && \
echo "Done"
