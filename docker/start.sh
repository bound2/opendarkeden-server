#!/bin/bash
#
# Entrypoint for the containerized DarkEden server.
#
# Waits for the database to accept connections, then starts loginserver,
# sharedserver and gameserver in that order and keeps streaming their logs
# to stdout. The container exits as soon as one of the servers dies.
#
# Environment:
#   CONF_DIR         configuration directory   (default /home/darkeden/vs/conf)
#   DB_WAIT_RETRIES  database wait attempts, 2s apart          (default 150)

set -u

VS_HOME=/home/darkeden/vs
BIN_DIR="$VS_HOME/bin"
CONF_DIR="${CONF_DIR:-$VS_HOME/conf}"
DB_WAIT_RETRIES="${DB_WAIT_RETRIES:-150}"

cd "$BIN_DIR" || exit 1

log() { echo "[start.sh] $*"; }

# Read a "Key : Value" entry out of a server configuration file.
read_conf() {
    awk -F: -v key="$2" '
        $1 ~ "^[[:space:]]*"key"[[:space:]]*$" {
            gsub(/^[[:space:]]+|[[:space:]]+$/, "", $2)
            print $2
            exit
        }' "$1"
}

wait_for_db() {
    local host port i
    host=$(read_conf "$CONF_DIR/loginserver.conf" DB_HOST)
    port=$(read_conf "$CONF_DIR/loginserver.conf" DB_PORT)
    host="${host:-odk-mysql}"
    port="${port:-3306}"

    log "waiting for the database at $host:$port ..."
    for ((i = 1; i <= DB_WAIT_RETRIES; i++)); do
        if (exec 3<>"/dev/tcp/$host/$port") 2>/dev/null; then
            exec 3<&-
            log "database is up"
            return 0
        fi
        sleep 2
    done

    log "ERROR: database at $host:$port is not reachable"
    return 1
}

names=()
pids=()

start_server() {
    local name="$1"
    local conf="$CONF_DIR/$2"
    local logfile="$BIN_DIR/${name}_$(date '+%y%m%d_%H%M%S').out"

    if [ ! -f "$conf" ]; then
        log "ERROR: missing configuration file $conf"
        return 1
    fi

    log "starting $name (log: $logfile)"
    # Line-buffer the output, otherwise a crash loses whatever is still buffered.
    stdbuf -oL -eL ./"$name" -f "$conf" > "$logfile" 2>&1 &
    pids+=($!)
    names+=("$name")

    # Mirror the log file to the container output so `docker compose logs` works.
    tail -F -n +1 "$logfile" 2>/dev/null | sed -u "s/^/[$name] /" &
}

shutdown_all() {
    log "stopping servers ..."
    local pid
    for pid in "${pids[@]}"; do
        kill "$pid" 2>/dev/null
    done
    wait "${pids[@]}" 2>/dev/null
}

trap 'shutdown_all; exit 0' INT TERM

wait_for_db || exit 1

start_server loginserver loginserver.conf || exit 1
sleep 3
start_server sharedserver sharedserver.conf || { shutdown_all; exit 1; }
sleep 3
start_server gameserver gameserver.conf || { shutdown_all; exit 1; }

log "all servers started"

# A child that already exited stays around as a zombie until it is waited for,
# so `kill -0` is not enough - look at the process state instead.
is_alive() {
    local state
    state=$(sed -n 's/.*) \([A-Za-z]\).*/\1/p' "/proc/$1/stat" 2>/dev/null)
    [ -n "$state" ] && [ "$state" != "Z" ]
}

# Supervise: if any server exits, tear the rest down so docker notices.
while true; do
    for i in "${!pids[@]}"; do
        if ! is_alive "${pids[$i]}"; then
            wait "${pids[$i]}" 2>/dev/null
            log "${names[$i]} exited with status $? - shutting down"
            shutdown_all
            exit 1
        fi
    done
    sleep 5
done
