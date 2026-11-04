#!/usr/bin/env bash
# The zone-group tick stops a DatabaseError where its failed statement has a
# meaning (docs/architecture/exceptions-and-outcomes.md): ZoneGroup::heartbeat
# skips the rest of a zone's tick, ZoneGroup::processPlayers goes on past a
# failed sockets, commands or players' heartbeat step, and
# GamePlayer::disconnect ends the local session past a failed save or log-off.
# Those sites have no test of their own (a zone's heartbeat and a disconnect's
# statements need a loaded zone and a database), so this audit holds the
# catches in the source; a player's own commands and the flag war's steps are
# owned by game_server_runtime_tests, the login's kick wait by
# login_server_runtime_tests.
#
#   bash tests/tools/check_database_error_sites.sh [repo-root]
#
# Exit status: 0 when every site catches the error, 1 naming each that does
# not.

set -u
root=${1:-.}
status=0

# The catches of `const DatabaseError&` in one function body: from its
# definition line to the first line that is a closing brace in column one.
count_in() {
	local file=$1 definition=$2
	awk -v def="$definition" '
		index($0, def) == 1 { inside = 1 }
		inside && /catch \(const DatabaseError&/ { n++ }
		inside && /^}/ { inside = 0 }
		END { print n + 0 }' "$file"
}

require() {
	local file=$1 definition=$2 least=$3 what=$4
	local found
	if [ ! -f "$file" ]; then
		echo "ERROR: $file: no such file" >&2
		status=1
		return
	fi
	found=$(count_in "$file" "$definition")
	if [ "$found" -lt "$least" ]; then
		echo "ERROR: $file: $definition catches a DatabaseError $found times, $least needed: $what" >&2
		status=1
	else
		echo "OK: $file: $definition catches a DatabaseError $found times"
	fi
}

require "$root/src/server/gameserver/ZoneGroup.cpp" "void ZoneGroup::heartbeat()" 1 \
	"a zone's failed step is skipped, not the end of the thread"
require "$root/src/server/gameserver/ZoneGroup.cpp" "void ZoneGroup::processPlayers()" 3 \
	"the sockets, the commands and the players' heartbeat each go on"
require "$root/src/server/gameserver/GamePlayer.cpp" "void GamePlayer::disconnect(bool bDisconnected)" 2 \
	"the save and the log-off each let the local session end"
exit $status
