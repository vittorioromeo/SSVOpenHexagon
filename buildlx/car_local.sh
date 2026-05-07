#!/bin/bash
#
# Build the project, copy the binary into `_RELEASE/`, launch a local
# headless server in the background, then launch the client pointed at
# that server via the `localserver` ConfigOverride
# (`_RELEASE/ConfigOverrides/localserver.json`, which sets
# `server_ip = 127.0.0.1` + `server_port = 50505`).
#
# When the client exits -- normally or via Ctrl-C -- we tear the
# background server down. Any other interruption (script killed,
# terminal closed) also runs the cleanup via the EXIT/INT/TERM trap.

set -euo pipefail

# Build + stage the binary the same way `car.sh` does.
ninja
cp ./SSVOpenHexagon ../_RELEASE

# All subsequent commands run from `_RELEASE/` so the binary finds its
# `Packs/`, `ConfigOverrides/`, etc. relative to cwd.
cd ../_RELEASE

# Start the server. `-headless -server` skips window/audio init and
# enters the dedicated-server entrypoint.
./SSVOpenHexagon -headless -server &
SERVER_PID=$!

# Cleanup: stop the server on any exit path. `|| true` keeps the trap
# from failing the script if the server has already exited (e.g. it
# crashed before the client did, or Ctrl-C delivered SIGINT to both
# processes in the same group).
cleanup() {
    if kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "[car_local] stopping server (pid=$SERVER_PID)..."
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

# Brief pause so the server's TCP listener is bound before the client
# attempts to connect. 0.5s is plenty for a local server; tune higher
# if your machine is slow.
sleep 0.5

# Launch the client with the `localserver` override. Anything in
# `args` that matches a `ConfigOverrides/<name>.json` filename gets
# applied on top of `config.json` -- see `Config::loadConfig`.
./SSVOpenHexagon localserver
