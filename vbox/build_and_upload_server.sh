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
echo "Done" && \
\
echo "Running 'update_server.sh' on server..." && \
ssh vittorioromeo@139.162.199.162 -f "cd $OH_SERVER_ROOT/server && ./update_server.sh" && \
echo "Done"

