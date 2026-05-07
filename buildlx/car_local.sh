#!/bin/bash
#
# Build the project, copy the binary into `_RELEASE/`, then launch a
# local headless server and a client pointed at it inside a tmux
# session split side-by-side, so both logs are visible at once. The
# client uses the `localserver` ConfigOverride
# (`_RELEASE/ConfigOverrides/localserver.json` -> `127.0.0.1:50505`).
#
# Tear-down: the client side runs `tmux kill-session` once it exits
# (normally or via Ctrl-C), which closes both panes -- so killing the
# client kills the server too.

set -euo pipefail

if ! command -v tmux >/dev/null 2>&1; then
    echo "[car_local] tmux not found in PATH; install it or use car.sh" >&2
    exit 1
fi

# Build + stage the binary the same way `car.sh` does.
ninja
cp ./SSVOpenHexagon ../_RELEASE

cd ../_RELEASE

SESSION="ohlocal"

# If a stale session from a previous run is still around, drop it so
# we start clean.
tmux kill-session -t "$SESSION" 2>/dev/null || true

# Server pane: dedicated headless server (`-headless -server`).
# Detached so we can split before attaching.
tmux new-session -d -s "$SESSION" -n play \
    "./SSVOpenHexagon -headless -server"

# Client pane (right of the server): tiny pause so the server's TCP
# listener is bound before the client tries to connect, then run the
# client with the `localserver` ConfigOverride. When the client exits
# -- any reason: clean exit, Ctrl-C, crash -- we kill the whole
# session, which also tears the server pane down.
tmux split-window -h -t "$SESSION:play" \
    "sleep 0.5; ./SSVOpenHexagon localserver; tmux kill-session -t '$SESSION'"

# Even split (50/50). `-h` above puts panes side-by-side; this just
# nudges the layout so neither pane is wider than the other.
tmux select-layout -t "$SESSION:play" even-horizontal

# Attach so the user sees both logs. When the session ends (because
# the client wrapper above ran `kill-session`), `attach` returns and
# we exit cleanly.
tmux attach-session -t "$SESSION"
