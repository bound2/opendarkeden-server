#!/usr/bin/env bash
# The gameserver's port check handler records a name only through
# de::recordPortCheck (PortCheck.h), which refuses a name character creation
# would refuse, or one no statement may carry, before the repository is
# asked (docs/architecture/repository-seams.md). The handler has no test of
# its own: it reaches the process-wide MySQL repository through
# defaultSessionRepository(), which nothing replaces, so this audit holds
# the wiring in the source.
#
#   bash tests/tools/check_port_check_wiring.sh [repo-root]
#
# Exit status: 0 when the handler records through recordPortCheck alone, 1
# naming the file otherwise.

set -u
root=${1:-.}
file="$root/src/server/gameserver/handler/CGPortCheckHandler.cpp"

if [ ! -f "$file" ]; then
	echo "ERROR: $file: no such file" >&2
	exit 1
fi

# The statements, with the comment lines left out.
code=$(grep -v -E '^[[:space:]]*//' "$file")

if ! grep -q 'de::recordPortCheck(defaultSessionRepository()' <<<"$code"; then
	echo "ERROR: $file: no call of de::recordPortCheck(defaultSessionRepository(), ...)" >&2
	exit 1
fi
if grep -q 'recordUserIP' <<<"$code"; then
	echo "ERROR: $file: records a name past the port check's name rule (recordUserIP)" >&2
	exit 1
fi

echo "OK: $file records a port check through de::recordPortCheck"
