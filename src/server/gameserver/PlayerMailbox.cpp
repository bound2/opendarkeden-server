//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerMailbox.cpp
//////////////////////////////////////////////////////////////////////////////

#include "PlayerMailbox.h"

#include <memory>
#include <utility>

#include "Creature.h"
#include "Exception.h"
#include "PCFinder.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "Zone.h"
#include "ZoneGroup.h"

namespace de {
namespace {

struct Lookup {
    PlayerCreature* pc = nullptr;
    Player* player = nullptr;
    ZoneGroup* group = nullptr; // null: logged in but not in any zone yet
};

// Caller holds the PCFinder lock.
Lookup findLoggedInPC(const std::string& name) {
    Lookup found;
    Creature* pCreature = g_pPCFinder->getCreature_LOCKED(name);
    if (pCreature == nullptr || !pCreature->isPC())
        return found;
    found.pc = dynamic_cast<PlayerCreature*>(pCreature);
    found.player = pCreature->getPlayer();
    if (found.pc == nullptr || found.player == nullptr)
        return Lookup{};
    Zone* pZone = pCreature->getZone();
    found.group = pZone != nullptr ? pZone->getZoneGroup() : nullptr;
    return found;
}

// What one postToPlayer() call carries. Shared, so a bounce to another
// group re-posts the same object instead of copying the closures.
struct Posted {
    std::string name;
    PlayerCommand command;
    GoneCommand ifGone;
};

void runOnGroup(ZoneGroup* pGroup, const std::shared_ptr<Posted>& posted);

void postToGroup(ZoneGroup* pGroup, const std::shared_ptr<Posted>& posted) {
    pGroup->post([pGroup, posted] { runOnGroup(pGroup, posted); });
}

// The posted command: re-find the player under the PCFinder lock, make sure
// this group still owns it, and only then run.
void runOnGroup(ZoneGroup* pGroup, const std::shared_ptr<Posted>& posted) {
    // Lock order group -> PCFinder is the one the CG handlers already use
    // (they run under the group mutex and take the PCFinder lock inside);
    // the reverse never happens because post() takes no group mutex.
    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    Lookup found = findLoggedInPC(posted->name);
    if (found.pc == nullptr) {
        // Logged out between post and drain.
        if (posted->ifGone)
            posted->ifGone();
        return;
    }
    if (found.group != pGroup) {
        // Moved to another group since the post: follow it. A player that
        // is logged in but momentarily in no zone keeps the command on
        // this group and is looked at again next tick.
        postToGroup(found.group != nullptr ? found.group : pGroup, posted);
        return;
    }
    posted->command(*found.pc, *found.player);

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
}

} // namespace

bool postToPlayer(const std::string& name, PlayerCommand command, GoneCommand ifGone) {
    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    Lookup found = findLoggedInPC(name);
    if (found.pc == nullptr)
        return false;
    if (found.group == nullptr) {
        // Pre-zone login phase: no zone thread owns this player yet. Run
        // now, under the PCFinder lock, as the handlers always did.
        command(*found.pc, *found.player);
        return true;
    }
    postToGroup(found.group, std::make_shared<Posted>(Posted{name, std::move(command), std::move(ifGone)}));
    return true;

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
}

} // namespace de
