#!/usr/bin/env bash
# Cross-checks the server's wire-layout inventory against the client's.
#
# Both repos commit the same file shape (tests/wire-layout.txt:
# packet-id<TAB>name<TAB>max-body-size, one line per packet factory), each
# generated from its own packet classes. The client's are hand-maintained
# copies of the server's, so this diff is the ground truth for whether the
# two protocol copies still agree on ids and maximum body sizes. Field
# ORDER is not visible here; that is what the per-packet goldens are for.
#
# Usage, from the directory that contains both checkouts:
#     bash server/tests/tools/wire_inventory_diff.sh [client-dir]
# or from the server repo root:
#     bash tests/tools/wire_inventory_diff.sh ../client
#
# Exit status is 1 on any disagreement, so it can gate a CI job later.
# Packets the server legitimately does not share with the client (and vice
# versa) are listed with reasons in tests/wire-layout-exceptions.txt.
set -uo pipefail

here=$(cd "$(dirname "$0")" && pwd)
server_root=$(cd "$here/../.." && pwd)
client_root="${1:-$server_root/../client}"

server_file="$server_root/tests/wire-layout.txt"
client_file="$client_root/tests/wire-layout.txt"
exceptions="$server_root/tests/wire-layout-exceptions.txt"

for f in "$server_file" "$client_file"; do
    [ -f "$f" ] || { echo "missing $f" >&2; exit 2; }
done

# join(1) needs the key in LC_ALL=C lexical order, not numeric.
strip() { grep -vE '^\s*(#|$)' "$1" | tr -d '\r' | LC_ALL=C sort -t$'\t' -k1,1; }
excepted() { grep -vE '^\s*(#|$)' "$exceptions" 2>/dev/null | tr -d '\r' | awk '{print $1}' | sort -u; }

s=$(mktemp); c=$(mktemp); x=$(mktemp)
trap 'rm -f "$s" "$c" "$x"' EXIT
strip "$server_file" > "$s"
strip "$client_file" > "$c"
excepted > "$x"

fail=0
report() { # <label> <lines>
    local label="$1" lines="$2"
    if [ -n "$lines" ]; then
        echo "[FAIL] $label:"
        echo "$lines" | sed 's/^/         /'
        fail=1
    else
        echo "[OK]   $label: none"
    fi
}

# Same id + name, different max size — the real "layout drifted" signal.
size_mismatch=$(join -t$'\t' -j1 <(cut -f1,2 "$s" | paste -d$'\t' - <(cut -f3 "$s")) \
                              <(cut -f1,2 "$c" | paste -d$'\t' - <(cut -f3 "$c")) \
                | awk -F'\t' '$2 == $4 && $3 != $5 { printf "%s\t%s\tserver=%s\tclient=%s\n", $1, $2, $3, $5 }')
report "max-size mismatches (same id and name)" "$size_mismatch"

# Same id, different name — a renamed or renumbered packet.
name_mismatch=$(join -t$'\t' -j1 <(cut -f1,2 "$s") <(cut -f1,2 "$c") \
                | awk -F'\t' '$2 != $3 { printf "%s\tserver=%s\tclient=%s\n", $1, $2, $3 }')
report "name mismatches (same id)" "$name_mismatch"

# Present on one side only, minus the documented exceptions (by name).
server_only=$(join -t$'\t' -v1 -j1 <(cut -f1,2 "$s") <(cut -f1,2 "$c") | grep -vFwf "$x" || true)
client_only=$(join -t$'\t' -v2 -j1 <(cut -f1,2 "$s") <(cut -f1,2 "$c") | grep -vFwf "$x" || true)
report "server-only packets (not in exceptions)" "$server_only"
report "client-only packets (not in exceptions)" "$client_only"

echo "server: $(wc -l < "$s") packets, client: $(wc -l < "$c") packets, exceptions: $(wc -l < "$x")"
exit $fail
