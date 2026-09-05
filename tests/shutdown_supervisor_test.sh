#!/usr/bin/env bash
set -euo pipefail
probe=$(realpath "$1")
supervisor=$(realpath "$2")
scratch=$(mktemp -d)
supervisor_pid=
db_pid=
cleanup() {
    if [ -n "$supervisor_pid" ]; then
        kill -TERM "$supervisor_pid" 2>/dev/null || true
        wait "$supervisor_pid" 2>/dev/null || true
    fi
    if [ -n "$db_pid" ]; then kill "$db_pid" 2>/dev/null || true; fi
    # Exact test-owned mktemp directory, never a checkout or cache volume.
    rm -rf -- "$scratch"
}
trap cleanup EXIT
mkdir "$scratch/bin" "$scratch/conf"
cd "$scratch/bin"
"$probe" db &
db_pid=$!
for ((i=0; i<100; i++)); do
    [ ! -f db.port ] || break
    sleep 0.01
done
test -f db.port
port=$(<db.port)
for name in loginserver sharedserver gameserver; do
    ln -s "$probe" "$name"
    printf 'DB_HOST : 127.0.0.1\nDB_PORT : %s\n' "$port" > "$scratch/conf/$name.conf"
done
VS_HOME="$scratch" bash "$supervisor" > supervisor.log 2>&1 &
supervisor_pid=$!
for ((i=0; i<200; i++)); do
    [ ! -f gameserver.ready ] || break
    sleep 0.05
done
test -f gameserver.ready
kill -TERM "$supervisor_pid"
wait "$supervisor_pid"
supervisor_pid=
for name in gameserver loginserver sharedserver; do
    test -f "$name.stopped"
done
echo 'SIGTERM drained gameserver before stopping login/shared services'
