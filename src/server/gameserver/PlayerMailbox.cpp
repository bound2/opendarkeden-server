//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerMailbox.cpp
//////////////////////////////////////////////////////////////////////////////

#include "PlayerMailbox.h"

#include <atomic>
#include <ctime>
#include <exception>
#include <utility>

#include "Creature.h"
#include "Exception.h"
#include "GamePlayer.h"
#include "PCFinder.h"
#include "PlayerCreature.h"
#include "Utility.h"
#include "Zone.h"
#include "ZoneGroup.h"

namespace de {
namespace {

// A batch this deep in one pass means the owner stopped draining for a
// while (a player parked in the main thread's manager collecting guild
// events, say) or a producer is looping; either is worth a log line since
// the box itself is unbounded.
constexpr std::size_t kDepthWarning = 100;

// Caller holds the PCFinder lock. Null when there is no logged-in PC of
// that name, or when its Player is not attached yet -- Creature::getPlayer()
// asserts (throws) on a null player, and LGKickCharacter's author noted
// that case does occur, so it is treated as "not here".
GamePlayer* findLoggedInPlayer(const std::string& name) {
    Creature* pCreature = g_pPCFinder->getCreature_LOCKED(name);
    if (pCreature == nullptr || !pCreature->isPC())
        return nullptr;
    try {
        return dynamic_cast<GamePlayer*>(pCreature->getPlayer());
    } catch (Throwable&) {
        return nullptr;
    }
}

void logFailure(GamePlayer& player, const char* what) {
    try {
        throw;
    } catch (Throwable& t) {
        filelog("errorLog.txt", "player mailbox %s failed for %s: %s", what, player.getID().c_str(),
                t.toString().c_str());
    } catch (std::exception& e) {
        filelog("errorLog.txt", "player mailbox %s failed for %s: %s", what, player.getID().c_str(), e.what());
    } catch (...) {
        filelog("errorLog.txt", "player mailbox %s failed for %s: unknown exception", what, player.getID().c_str());
    }
}

std::size_t noteDepth(GamePlayer& player, std::size_t ran) {
    if (ran > kDepthWarning)
        filelog("errorLog.txt", "player mailbox for %s drained %u commands in one pass", player.getID().c_str(),
                (unsigned)ran);
    return ran;
}

// Commands are waiting that this owner may not run. Normal for a moment
// (a zone change), a problem when it lasts; logged when the backlog is deep
// or, rate-limited across all players, when it is merely present, so a
// player whose box never drains shows up before logout.
void noteStuck(GamePlayer& player, const char* why) {
    static std::atomic<std::time_t> lastLog{0};
    const std::size_t waiting = player.mailbox().size();
    if (waiting == 0)
        return;
    const std::time_t now = std::time(nullptr);
    if (waiting < kDepthWarning && now - lastLog.load(std::memory_order_relaxed) < 10)
        return;
    lastLog.store(now, std::memory_order_relaxed);
    filelog("errorLog.txt", "player mailbox for %s holds %u commands its owner cannot run (%s)", player.getID().c_str(),
            (unsigned)waiting, why);
}

template <typename Take> std::size_t runPending(GamePlayer& player, PlayerCreature& pc, Take&& take) {
    return noteDepth(player,
                     player.mailbox().drainIf(
                         std::forward<Take>(take), [&](PostedPlayerCommand& posted) { posted.command(pc, player); },
                         [&] { logFailure(player, "command"); }));
}

} // namespace

bool postToPlayer(const std::string& name, PlayerCommand command, GoneCommand ifGone, Scope scope) {
    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    GamePlayer* pGamePlayer = findLoggedInPlayer(name);
    if (pGamePlayer == nullptr)
        return false;
    // The GamePlayer outlives this section: its destructor removes the
    // creature from the PCFinder -- under this same lock -- before anything
    // else is torn down, so a player found here is not being destroyed
    // until we release the lock, and its abandon runs after our post.
    pGamePlayer->mailbox().post(PostedPlayerCommand{std::move(command), std::move(ifGone), scope});
    return true;

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
}

std::size_t drainPlayerMailbox(GamePlayer& player, const ZonePlayerManager& owner) {
    if (player.mailbox().empty())
        return 0;
    PlayerCreature* pc = dynamic_cast<PlayerCreature*>(player.getCreature());
    if (pc == nullptr)
        return 0; // nothing to run against right now; the commands wait
    Zone* pZone = pc->getZone();
    if (pZone != nullptr && pZone->getZoneGroup() != nullptr && pZone->getZoneGroup()->getZonePlayerManager() == &owner)
        return runPending(player, *pc, [](const PostedPlayerCommand&) { return true; });

    // Listed here, but the creature is in another group's zone (the
    // mismatch the manager logs as ZPMCheck, when it checks). Zone-scoped
    // commands are not ours to run; player-scoped ones are, since this
    // manager is the player's owner whatever its creature points at.
    noteStuck(player, "creature is in another group's zone");
    return runPending(player, *pc, [](const PostedPlayerCommand& posted) { return posted.scope == Scope::Player; });
}

std::size_t drainPlayerMailboxOnMainThread(GamePlayer& player) {
    if (player.mailbox().empty())
        return 0;
    PlayerCreature* pc = dynamic_cast<PlayerCreature*>(player.getCreature());
    if (pc == nullptr)
        return 0;
    std::size_t ran =
        runPending(player, *pc, [](const PostedPlayerCommand& posted) { return posted.scope == Scope::Player; });
    if (!player.mailbox().empty())
        noteStuck(player, "owned by the main thread");
    return ran;
}

std::size_t abandonPlayerMailbox(GamePlayer& player) {
    if (player.mailbox().empty())
        return 0;
    return noteDepth(player, player.mailbox().drain(
                                 [](PostedPlayerCommand& posted) {
                                     if (posted.ifGone)
                                         posted.ifGone();
                                 },
                                 [&] { logFailure(player, "ifGone handler"); }));
}

} // namespace de
