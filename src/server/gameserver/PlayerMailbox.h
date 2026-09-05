//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerMailbox.h
// Description : Run a command against a logged-in player on the thread
//               that owns the player, under that owner's lock.
//
//               This is how the cross-thread packet handlers (SG/LG/GG, on
//               the SharedServerManager / LoginServerManager threads) reach
//               a PlayerCreature without racing its owner. Before, they
//               mutated gold, guild id and kick flags holding only the
//               PCFinder lock, which serialises lookup and removal but not
//               the zone tick -- the "SG/LG/GG handlers" entry in CLAUDE.md's
//               known violations. Sending a packet to the player's socket is
//               still fine from any thread and does not need this.
//
//               The box belongs to the GamePlayer, not to a zone group, and
//               that is the point: a player's owner changes over its life
//               (the main thread's IncomingPlayerManager during login and
//               zone transfer, a ZonePlayerManager on a zone thread while
//               in a zone group) and the creature's own zone pointer does
//               not say which -- it is set as soon as the character loads,
//               before any zone thread owns it, and stays on the old zone
//               during a transfer. So the box moves with the player, keeps
//               its commands in posting order across group changes, and is
//               drained only from ZonePlayerManager::processCommands(), the
//               one place a zone thread processes a player it owns, under
//               the group mutex. The main thread never drains it: while it
//               owns the player nothing zone-related is safe to touch, so
//               the commands simply wait until the player is back in a
//               group. A player that logs out with commands pending runs
//               their ifGone handlers instead (GamePlayer::disconnect, right
//               after the PCFinder removal that makes it "gone").
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_PLAYER_MAILBOX_H
#define DARKEDEN_PLAYER_MAILBOX_H

#include <cstddef>
#include <functional>
#include <string>

#include "Mailbox.h"

class GamePlayer;
class Player;
class PlayerCreature;

namespace de {

using PlayerCommand = std::function<void(PlayerCreature& pc, Player& player)>;
using GoneCommand = std::function<void()>;

struct PostedPlayerCommand {
    PlayerCommand command;
    GoneCommand ifGone; // may be empty
};

using PlayerMailbox = Mailbox<PostedPlayerCommand>;

// Posts `command` to the player named `name` and returns true. Returns
// false, running nothing, when no logged-in PC of that name exists at the
// time of the call -- the caller's "offline" branch belongs there. Two
// things the caller must accept:
//
//  * The command runs later, on the owning zone thread's next pass over
//    the player, and only if the player is still logged in then. A player
//    that logged out in between is skipped, exactly as the old code
//    skipped a player it could not find -- unless `ifGone` is given, which
//    then runs in its place (on the thread that logs the player out), for
//    the handlers whose offline branch does something material, like
//    charging a fee in the database instead of in memory. Exactly one of
//    the two ever runs.
//  * Everything the command needs must be captured by value. A Guild* or
//    GuildMember* captured by pointer may be deleted before the command
//    runs (SGDeleteGuildOK deletes both); capture ids and names and look
//    the objects up again inside.
//
// The command runs without the PCFinder lock (its owner's lock is what
// protects it), so it may call postToPlayer itself, e.g. for another
// player. A command that throws -- any type -- is logged and dropped; the
// rest of the batch still runs.
bool postToPlayer(const std::string& name, PlayerCommand command, GoneCommand ifGone = nullptr);

// Owner side. drainPlayerMailbox runs what is pending for `player`; the
// caller is the thread that owns the player, holding the owner's lock
// (ZonePlayerManager::processCommands on the zone thread). Returns the
// number of commands taken. abandonPlayerMailbox runs the ifGone handlers
// of everything pending and drops the commands; called at logout.
std::size_t drainPlayerMailbox(GamePlayer& player);
std::size_t abandonPlayerMailbox(GamePlayer& player);

} // namespace de

#endif // DARKEDEN_PLAYER_MAILBOX_H
