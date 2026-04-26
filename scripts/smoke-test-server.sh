#!/bin/bash
# Smoke test for the real SSVOpenHexagon server binary.
#
# What it does:
#   1. Verifies the server and `OHServerControl` binaries are built.
#   2. Launches the server in the background (running out of `_RELEASE/`
#      so it finds its config and assets).
#   3. Waits for the TCP listener to bind on the configured port.
#   4. Opens a raw TCP connection to prove the listener accepts (and
#      closes it — the server's selector should see the client show up
#      and then go away without freezing).
#   5. Sends a "verbose true" control message via `OHServerControl` and
#      greps the server log to confirm the server actually processed it.
#   6. Sends SIGINT for a graceful shutdown and waits for the process
#      to exit; fails if it doesn't exit within the grace period.
#
# Usage:  scripts/smoke-test-server.sh
# Exits 0 on success, non-zero with a clear message on any failure.
# No arguments; the ports are taken from `_RELEASE/config.json`
# (defaults: 50505 TCP, 50506 UDP control).

set -u

# -----------------------------------------------------------------------------
# Locate repo, binaries, and release directory relative to this script.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$REPO_ROOT/buildlx"
RELEASE_DIR="$REPO_ROOT/_RELEASE"

SERVER_BIN="$BUILD_DIR/SSVOpenHexagon"
CONTROL_BIN="$BUILD_DIR/OHServerControl"
SMOKE_CLIENT_BIN="$BUILD_DIR/OHSmokeClient"

SERVER_TCP_PORT=50505
SERVER_UDP_CONTROL_PORT=50506

SERVER_LOG="$(mktemp -t ohw-smoke-server.XXXXXX.log)"

# The server binary chdirs to its own `argv[0].parent_path()` on startup
# (see `Core/main.cpp` near line 629), so we have to invoke it via a
# symlink that lives inside `_RELEASE/` — that way `chdir` lands in the
# release directory where `config.json` and `Packs/` are.
SERVER_SYMLINK="$RELEASE_DIR/SSVOpenHexagon.smoke-test.$$"

# -----------------------------------------------------------------------------
# Colored logging helpers (fall back to plain text if stdout isn't a tty).
if [ -t 1 ]; then
    C_OK=$'\033[0;32m'; C_FAIL=$'\033[0;31m'; C_DIM=$'\033[0;90m'; C_RST=$'\033[0m'
else
    C_OK=''; C_FAIL=''; C_DIM=''; C_RST=''
fi

log()    { echo "${C_DIM}[smoke]${C_RST} $*"; }
ok()     { echo "${C_OK}[PASS]${C_RST} $*"; }
fail()   { echo "${C_FAIL}[FAIL]${C_RST} $*" >&2; }

# -----------------------------------------------------------------------------
# Global state + cleanup on any exit path.
SERVER_PID=""

cleanup() {
    local exit_code=$?
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        log "cleanup: stopping server pid $SERVER_PID"
        kill -INT "$SERVER_PID" 2>/dev/null || true
        # Wait up to 5s for graceful exit, then force.
        for _ in $(seq 1 50); do
            kill -0 "$SERVER_PID" 2>/dev/null || break
            sleep 0.1
        done
        if kill -0 "$SERVER_PID" 2>/dev/null; then
            log "cleanup: SIGINT didn't stop server, sending SIGKILL"
            kill -KILL "$SERVER_PID" 2>/dev/null || true
        fi
        wait "$SERVER_PID" 2>/dev/null || true
    fi
    [ -L "$SERVER_SYMLINK" ] && rm -f "$SERVER_SYMLINK"
    if [ $exit_code -ne 0 ] && [ -s "$SERVER_LOG" ]; then
        echo "----- server log tail (on failure) -----" >&2
        tail -n 40 "$SERVER_LOG" >&2
        echo "----- full log kept at: $SERVER_LOG -----" >&2
    else
        rm -f "$SERVER_LOG"
    fi
    exit $exit_code
}
trap cleanup EXIT INT TERM

# -----------------------------------------------------------------------------
# Step 0: pre-flight checks.
[ -x "$SERVER_BIN" ]       || { fail "server binary missing or not executable: $SERVER_BIN"; exit 2; }
[ -x "$CONTROL_BIN" ]      || { fail "control binary missing or not executable: $CONTROL_BIN"; exit 2; }
[ -x "$SMOKE_CLIENT_BIN" ] || { fail "smoke-client binary missing or not executable: $SMOKE_CLIENT_BIN"; exit 2; }
[ -d "$RELEASE_DIR" ]      || { fail "release directory missing: $RELEASE_DIR"; exit 2; }
[ -f "$RELEASE_DIR/config.json" ] || { fail "release config.json missing in $RELEASE_DIR"; exit 2; }

# Refuse to run if the port is already in use — otherwise the spawned
# server will fail to bind and the failure gets masked.
if ss -tln 2>/dev/null | grep -q ":$SERVER_TCP_PORT[[:space:]]"; then
    fail "TCP port $SERVER_TCP_PORT is already in use; stop the other server first"
    exit 2
fi

ok "pre-flight checks"

# -----------------------------------------------------------------------------
# Step 1: start the server.
ln -sf "$SERVER_BIN" "$SERVER_SYMLINK"
log "starting server via symlink: $SERVER_SYMLINK -server"
# `stdbuf -oL -eL` forces line-buffering on stdout/stderr — the server's
# C++ `std::cout` is fully-buffered when redirected to a file, and the
# periodic `runIteration_FlushLogs` only flushes once per second, which
# makes "did the server log X" checks racy for a script.
stdbuf -oL -eL "$SERVER_SYMLINK" -server > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!
log "server pid: $SERVER_PID"

# -----------------------------------------------------------------------------
# Step 2: wait for the TCP listener to bind. 15s budget covers
# cold-start on a dev machine; in practice the bind is sub-second.
log "waiting for :$SERVER_TCP_PORT to accept..."
listener_bound=0
for _ in $(seq 1 150); do
    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        fail "server exited during startup"
        exit 1
    fi
    if ss -tln 2>/dev/null | grep -q ":$SERVER_TCP_PORT[[:space:]]"; then
        listener_bound=1
        break
    fi
    sleep 0.1
done
[ $listener_bound -eq 1 ] || { fail "server did not bind :$SERVER_TCP_PORT within 15s"; exit 1; }
ok "listener bound on :$SERVER_TCP_PORT"

# -----------------------------------------------------------------------------
# Step 3a: bare TCP probe — connect and close immediately. Proves the
# accept path works and that an unceremonious disconnect doesn't freeze
# the event loop.
log "probing TCP accept path..."
if exec 3<>"/dev/tcp/127.0.0.1/$SERVER_TCP_PORT"; then
    exec 3<&- 3>&-
    ok "TCP connect + close succeeded"
else
    fail "could not open TCP connection to :$SERVER_TCP_PORT"
    exit 1
fi

# -----------------------------------------------------------------------------
# Step 3b: real protocol round-trip via `OHSmokeClient`. Sends a
# `CTSPPublicKey` and asserts the server replies with `STCPPublicKey`,
# proving the network path actually carries data both ways and that the
# server's packet decoder + processor work end-to-end.
log "running OHSmokeClient round-trip against :$SERVER_TCP_PORT..."
SMOKE_CLIENT_OUT="$(mktemp -t ohw-smoke-client.XXXXXX.log)"
if "$SMOKE_CLIENT_BIN" 127.0.0.1 "$SERVER_TCP_PORT" > "$SMOKE_CLIENT_OUT" 2>&1; then
    ok "smoke-client round-trip succeeded ($(grep -c '\[smoke-client\]' "$SMOKE_CLIENT_OUT") events logged)"
    rm -f "$SMOKE_CLIENT_OUT"
else
    smoke_rc=$?
    fail "smoke-client failed (rc=$smoke_rc)"
    echo "----- smoke-client output -----" >&2
    cat "$SMOKE_CLIENT_OUT" >&2
    rm -f "$SMOKE_CLIENT_OUT"
    exit 1
fi

# -----------------------------------------------------------------------------
# Step 4: exercise the UDP control channel. Sending "verbose true" must
# flip the server's `_verbose` flag and the server logs that it did.
log "sending control message 'verbose true' via OHServerControl..."
if ! "$CONTROL_BIN" "verbose true"; then
    fail "OHServerControl failed to send"
    exit 1
fi

# Poll for the server's acknowledgement in the log. With line-buffering
# (see `stdbuf` above) this should land within tens of milliseconds, but
# give it up to 5 s to absorb CI jitter.
verbose_seen=0
for _ in $(seq 1 50); do
    if grep -q "Enabled verbose mode" "$SERVER_LOG"; then
        verbose_seen=1
        break
    fi
    sleep 0.1
done
[ $verbose_seen -eq 1 ] || { fail "server did not log 'Enabled verbose mode' within 5s of control message"; exit 1; }
ok "server processed control message"

# -----------------------------------------------------------------------------
# Step 5: graceful shutdown on SIGINT.
log "sending SIGINT and waiting for graceful exit..."
kill -INT "$SERVER_PID"

shutdown_ok=0
for _ in $(seq 1 100); do  # up to 10 s
    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        shutdown_ok=1
        break
    fi
    sleep 0.1
done
[ $shutdown_ok -eq 1 ] || { fail "server did not exit within 10s of SIGINT"; exit 1; }

wait "$SERVER_PID" 2>/dev/null
server_rc=$?

# After SIGINT, main.cpp logs "Finished\n"; bash `wait` reflects the
# process exit code.
if grep -q "Finished" "$SERVER_LOG"; then
    ok "server shut down cleanly (rc=$server_rc, 'Finished' logged)"
else
    # Some shutdown paths exit before the final log line flushes; treat
    # a clean exit code (0, or 128+SIGINT=130) as acceptable.
    if [ "$server_rc" -eq 0 ] || [ "$server_rc" -eq 130 ]; then
        ok "server shut down cleanly (rc=$server_rc, no 'Finished' line but clean exit)"
    else
        fail "server exit was not clean (rc=$server_rc) and no 'Finished' line"
        exit 1
    fi
fi

# Let cleanup run with a clean slate.
SERVER_PID=""

echo
echo "${C_OK}Smoke test passed${C_RST}"
exit 0
