#!/usr/bin/env bash
# A DatabaseError is stopped where its failed statement has a meaning
# (docs/architecture/exceptions-and-outcomes.md). The sites a test drives own
# themselves (the login's kick wait, the flag war's start and finish, a
# player's own commands, the keep-alive). This audit holds, in the source,
# the catches no test drives without a loaded zone and a database, and the
# kick wait's wiring, which its test proves of the function it calls, not
# of the caller:
#
#   ZoneGroup::heartbeat skips the rest of a zone's tick;
#   ZoneGroup::processPlayers goes on past a failed sockets, commands or
#   players' heartbeat step;
#   GamePlayer::disconnect ends the local session past a failed save or
#   log-off;
#   CGLogoutHandler::execute takes the creature out of its zone past a save
#   that fails;
#   FlagWar::executeEnd chases the next flag past one it could not read (its
#   other catch, the round's clearing, is GameFlagStepFailure's too);
#   LoginPlayer::processCommand runs its kick wait through runLoginKickWait.
#
# A count proves a catch is present, not that it is placed right: the
# placement is the review's.
#
#   bash tests/tools/check_database_error_sites.sh [repo-root]
#
# Exit status: 0 when every site holds, 1 naming each that does not.

set -u
root=${1:-.}
status=0

# The lines matching pattern in one function body, comment lines left out:
# from its definition line to the first line that is a closing brace in
# column one.
count_in() {
	local file=$1 definition=$2 pattern=$3
	awk -v def="$definition" -v pat="$pattern" '
		index($0, def) == 1 { inside = 1 }
		inside && $0 !~ /^[ \t]*\/\// && index($0, pat) > 0 { n++ }
		inside && /^}/ { inside = 0 }
		END { print n + 0 }' "$file"
}

require() {
	local file=$1 definition=$2 pattern=$3 least=$4 what=$5
	local found
	if [ ! -f "$file" ]; then
		echo "ERROR: $file: no such file" >&2
		status=1
		return
	fi
	found=$(count_in "$file" "$definition" "$pattern")
	if [ "$found" -lt "$least" ]; then
		echo "ERROR: $file: $definition holds '$pattern' $found times, $least needed: $what" >&2
		status=1
	else
		echo "OK: $file: $definition holds '$pattern' $found times"
	fi
}

catches='catch (const DatabaseError&'

require "$root/src/server/gameserver/ZoneGroup.cpp" "void ZoneGroup::heartbeat()" "$catches" 1 \
	"a zone's failed step is skipped, not the end of the thread"
require "$root/src/server/gameserver/ZoneGroup.cpp" "void ZoneGroup::processPlayers()" "$catches" 3 \
	"the sockets, the commands and the players' heartbeat each go on"
require "$root/src/server/gameserver/GamePlayer.cpp" "void GamePlayer::disconnect(bool bDisconnected)" "$catches" 2 \
	"the save and the log-off each let the local session end"
require "$root/src/server/gameserver/handler/CGLogoutHandler.cpp" \
	"void CGLogoutHandler::execute(CGLogout* pPacket, Player* pPlayer)" "$catches" 1 \
	"the save lets the creature leave its zone"
require "$root/src/server/gameserver/ctf/FlagWar.cpp" "void FlagWar::executeEnd()" "$catches" 2 \
	"a flag's return and the round's clearing each let the end go on"
require "$root/src/server/loginserver/LoginPlayer.cpp" "void LoginPlayer::processCommand(bool Option)" \
	"de::runLoginKickWait(" 1 "the kick wait runs through the wrap LoginKickWait proves"
exit $status
