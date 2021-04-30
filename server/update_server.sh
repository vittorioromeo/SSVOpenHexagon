#!/bin/bash

OH_ROOT="/home/vittorioromeo/OHWorkspace/SSVOpenHexagon"

echo "Reloading daemon..." && \
sudo systemctl daemon-reload && \
echo "Done" && \
\
echo "Stopping server..." && \
sudo systemctl stop openhexagon-server.service && \
echo "Server stopped" && \
\
echo "Copying '_RELEASE/SSVOpenHexagonVbox' to '_RELEASE/SSVOpenHexagonVbox_Current'" && \
cp "$OH_ROOT/_RELEASE/SSVOpenHexagonVbox" "$OH_ROOT/_RELEASE/SSVOpenHexagonVbox_Current" && \
echo "Copying done" && \
\
echo "Starting server again..." && \
sudo systemctl start openhexagon-server.service && \
echo "Server started" && \
\
echo "Printing status..." && \
sudo systemctl status openhexagon-server.service --no-pager && \
echo "Done" && \
\
sleep 3 &&
\
echo "Sending test control packet..." && \
"$OH_ROOT/_RELEASE/OHServerControl" "helloworld" && \
echo "Done"

