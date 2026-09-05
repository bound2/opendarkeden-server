//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerMailbox.h
// Description : Run a command against a logged-in player on the thread
//               that owns the player's zone group, under that group's mutex.
//
//               This is how the cross-thread packet handlers (SG/LG/GG, on
//               the SharedServerManager / LoginServerManager threads) reach
//               a PlayerCreature without racing the zone thread. Before, they
//               mutated gold, guild id and kick flags holding only the
//               PCFinder lock, which serialises lookup and removal but not
//               the zone tick -- the "SG/LG/GG handlers" entry in CLAUDE.md's
//               known violations. Sending a packet to the player's socket is
//               still fine from any thread and does not need this.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_PLAYER_MAILBOX_H
#define DARKEDEN_PLAYER_MAILBOX_H

#include <functional>
#include <string>

class Player;
class PlayerCreature;

namespace de {

using PlayerCommand = std::function<void(PlayerCreature& pc, Player& player)>;
using GoneCommand = std::function<void()>;

// Posts `command` to the zone group that owns the player named `name` and
// returns true. Returns false, running nothing, when no logged-in PC of
// that name exists at the time of the call -- the caller's "offline" branch
// belongs there. Two things the caller must accept:
//
//  * The command runs later, on the zone thread's next tick, and only if
//    the player is still logged in then; it is looked up again at that
//    point, and if it has meanwhile moved to another zone group the
//    command follows it. A player that logged out in between is skipped,
//    exactly as the old code skipped a player it could not find -- unless
//    `ifGone` is given, which then runs (on the zone thread) in its place,
//    for the handlers whose offline branch does something material, like
//    charging a fee in the database instead of in memory.
//  * Everything the command needs must be captured by value. A Guild* or
//    GuildMember* captured by pointer may be deleted before the command
//    runs (SGDeleteGuildOK deletes both); capture ids and names and look
//    the objects up again inside.
//
// A PC that is in the PCFinder but not yet in any zone (the pre-zone login
// phase) has no owning group; the command then runs immediately, under the
// PCFinder lock, which is what the old handlers did for every player.
bool postToPlayer(const std::string& name, PlayerCommand command, GoneCommand ifGone = nullptr);

} // namespace de

#endif // DARKEDEN_PLAYER_MAILBOX_H
