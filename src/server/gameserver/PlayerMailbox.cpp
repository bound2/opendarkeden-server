//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerMailbox.cpp
//////////////////////////////////////////////////////////////////////////////

#include "PlayerMailbox.h"

#include <exception>
#include <utility>

#include "Creature.h"
#include "Exception.h"
#include "GamePlayer.h"
#include "PCFinder.h"
#include "PlayerCreature.h"

namespace de {
namespace {

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

} // namespace

bool postToPlayer(const std::string& name, PlayerCommand command, GoneCommand ifGone) {
    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    GamePlayer* pGamePlayer = findLoggedInPlayer(name);
    if (pGamePlayer == nullptr)
        return false;
    // The GamePlayer outlives this section: disconnect() removes the
    // creature from the PCFinder -- under this same lock -- before the
    // player object is destroyed, so a player we found here is not being
    // torn down until we release the lock, and its abandon runs after.
    pGamePlayer->mailbox().post(PostedPlayerCommand{std::move(command), std::move(ifGone)});
    return true;

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
}

std::size_t drainPlayerMailbox(GamePlayer& player) {
    PlayerCreature* pc = dynamic_cast<PlayerCreature*>(player.getCreature());
    if (pc == nullptr)
        return abandonPlayerMailbox(player); // no creature to run against: treat as gone
    return player.mailbox().drain([&](PostedPlayerCommand& posted) { posted.command(*pc, player); },
                                  [&] { logFailure(player, "command"); });
}

std::size_t abandonPlayerMailbox(GamePlayer& player) {
    return player.mailbox().drain(
        [](PostedPlayerCommand& posted) {
            if (posted.ifGone)
                posted.ifGone();
        },
        [&] { logFailure(player, "ifGone handler"); });
}

} // namespace de
