#!/bin/bash

OH_VBOX_ROOT="/home/vittorioromeo/OHWorkspace/SSVOpenHexagon"
OH_SERVER_ROOT="/home/vittorioromeo/OHWorkspace/SSVOpenHexagon"

(cd /home/vittorioromeo/OHWorkspace/SSVUtils; git pull)
(cd /home/vittorioromeo/OHWorkspace/SSVStart; git pull)
(cd /home/vittorioromeo/OHWorkspace/SSVMenuSystem; git pull)
(cd /home/vittorioromeo/OHWorkspace/vrm_cmake; git pull)
(cd /home/vittorioromeo/OHWorkspace/vrm_pp; git pull)

git pull

echo "Building server release..." && \
cd "$OH_VBOX_ROOT/buildlx" && \
./make_release_server_vbox.sh && \
echo "Done" && \
\
echo "Uploading build binary to server..." && \
scp "$OH_VBOX_ROOT/_RELEASE/SSVOpenHexagonVbox" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/_RELEASE/SSVOpenHexagonVbox" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/sfml-build/lib/libsfml-audio.so.3.0"  "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/sfml-build/lib/libsfml-audio.so.3.0" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/sfml-build/lib/libsfml-network.so.3.0" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/sfml-build/lib/libsfml-network.so.3.0" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/luajit-build/src/libluajit.so" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/luajit-build/src/libluajit.so" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/zlib-build/libz.so.1" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/zlib-build/libz.so.1"  && \
scp "$OH_VBOX_ROOT/buildlx/_deps/libsodium-cmake-build/libsodium.so" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/libsodium-cmake-build/libsodium.so" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/imgui-sfml-build/libImGui-SFML.so" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/imgui-sfml-build/libImGui-SFML.so"  && \
scp "$OH_VBOX_ROOT/buildlx/_deps/sfml-build/lib/libsfml-graphics.so.3.0" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/sfml-build/lib/libsfml-graphics.so.3.0" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/sfml-build/lib/libsfml-window.so.3.0" "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/sfml-build/lib/libsfml-window.so.3.0" && \
scp "$OH_VBOX_ROOT/buildlx/_deps/sfml-build/lib/libsfml-system.so.3.0"  "vittorioromeo@139.162.199.162:/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/buildlx/_deps/sfml-build/lib/libsfml-system.so.3.0" && \
echo "Done" && \
\
echo "Running 'update_server.sh' on server..." && \
ssh vittorioromeo@139.162.199.162 -f "cd $OH_SERVER_ROOT/server && ./update_server.sh" && \
echo "Done"

