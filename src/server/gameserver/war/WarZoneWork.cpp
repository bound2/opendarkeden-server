//////////////////////////////////////////////////////////////////////////////
// Filename    : WarZoneWork.cpp
// Description : a war's zone work, posted to the threads that own it.
//////////////////////////////////////////////////////////////////////////////

#include "WarZoneWork.h"

#include <memory>
#include <string>
#include <utility>

#include "CapturedPacket.h"
#include "GameContext.h"
#include "GlobalItemPosition.h"
#include "GlobalItemPositionLoader.h"
#include "PlayerCreature.h"
#include "PlayerMailbox.h"
#include "Utility.h"
#include "WarZoneRouting.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneGroupManager.h"
#include "ZoneUtil.h"
#include "repository/ItemRepository.h"

namespace de::war {

namespace {

// The zone zoneID names, or NULL (logged) when this server has none.
Zone* findZone(ZoneID_t zoneID, const char* what) {
    try {
        return getZoneByZoneID(zoneID);
    } catch (Throwable& t) {
        filelog("WarError.log", "%s: no zone %u on this server: %s", what, (unsigned)zoneID, t.toString().c_str());
    }
    return NULL;
}

// Runs work on the zone zoneID, which the calling thread's group owns.
void runZoneWork(ZoneID_t zoneID, const ZoneWork& work) {
    Zone* pZone = getZoneByZoneID(zoneID);
    pZone->getZoneGroup()->assertOwned();

    work(*pZone);
}

// Where an item was found to be when the step taking it ran, for the logs.
std::string describe(const ItemPositionRow& row) {
    return "Storage=" + std::to_string(row.storage) + ",StorageID=" + std::to_string(row.storageID) + ",OwnerID='" +
           row.ownerID + "',X=" + std::to_string(row.x) + ",Y=" + std::to_string(row.y);
}

// A step found nothing where the row said: follow the row again, or stop.
// True when the next attempt's step was posted.
bool retryOrStop(Item::ItemClass itemClass, ItemID_t itemID, const ItemTaken& taken, int attemptsMade) {
    if (retryItemReturn(attemptsMade))
        return postItemReturn(itemClass, itemID, taken, attemptsMade);

    filelog("WarError.log", "item return: gave up on item %u of class %u after %d attempts", (unsigned)itemID,
            (unsigned)itemClass, attemptsMade);
    return false;
}

// The holder's step: take the item out of the place the row names, if it is
// still there, and hand it on. pc is the player the row names, for the
// positions a player holds, and NULL for a zone's.
void takeItem(const ItemPositionRow& row, Item::ItemClass itemClass, ItemID_t itemID, const ItemTaken& taken,
              int attemptsMade, PlayerCreature* pPC) {
    std::unique_ptr<GlobalItemPosition> pPosition(GlobalItemPositionLoader::getInstance()->makeGlobalItemPosition(row));
    if (pPosition == nullptr) {
        filelog("WarError.log", "item return: item %u of class %u lies where no position reaches (%s)",
                (unsigned)itemID, (unsigned)itemClass, describe(row).c_str());
        return;
    }

    pPosition->expectItem((int)itemClass, itemID);

    // A zone's positions take the zone's own mutex as well, as every pop from
    // another thread did, so the main thread's zone lockers stay excluded.
    Item* pItem = (pPC != NULL) ? pPosition->popItemFrom(*pPC) : pPosition->popItem(true);
    Zone* pFrom = (pItem != NULL) ? pPosition->getZone() : NULL;

    if (pItem == NULL || pFrom == NULL) {
        retryOrStop(itemClass, itemID, taken, attemptsMade);
        return;
    }

    taken(*pFrom, pItem);
}

} // namespace

bool postToZone(ZoneID_t zoneID, ZoneWork work) {
    Zone* pZone = findZone(zoneID, "postToZone");
    if (pZone == NULL)
        return false;

    pZone->getZoneGroup()->post([zoneID, work = std::move(work)] { runZoneWork(zoneID, work); });
    return true;
}

void postToZones(const std::vector<ZoneID_t>& zoneIDs, const ZoneWork& work) {
    std::vector<ZoneID_t> known;
    for (ZoneID_t zoneID : zoneIDs) {
        if (findZone(zoneID, "postToZones") != NULL)
            known.push_back(zoneID);
    }

    auto batches = zonesByOwner<ZoneGroupID_t>(
        known, [](ZoneID_t zoneID) { return getZoneByZoneID(zoneID)->getZoneGroup()->getZoneGroupID(); });

    for (auto& batch : batches) {
        ZoneGroup* pZoneGroup = de::gameContext().zoneGroups().getZoneGroup(batch.first);

        pZoneGroup->post([zones = std::move(batch.second), work] {
            for (ZoneID_t zoneID : zones) {
                try {
                    runZoneWork(zoneID, work);
                } catch (Throwable& t) {
                    filelog("WarError.log", "zone work on zone %u failed: %s", (unsigned)zoneID, t.toString().c_str());
                }
            }
        });
    }
}

void postToEveryZoneGroup(const std::function<void(ZoneGroup& zoneGroup)>& work) {
    for (const auto& entry : de::gameContext().zoneGroups().getZoneGroups()) {
        ZoneGroupID_t zoneGroupID = entry.first;

        entry.second->post([zoneGroupID, work] {
            ZoneGroup* pZoneGroup = de::gameContext().zoneGroups().getZoneGroup(zoneGroupID);
            pZoneGroup->assertOwned();

            work(*pZoneGroup);
        });
    }
}

void sendToEveryPlayer(Zone& zone, Packet& packet) {
    zone.broadcastPacket(&packet);
}

void postBroadcast(const std::vector<ZoneID_t>& zoneIDs, const Packet& packet, ZoneSend send) {
    std::shared_ptr<const CapturedPacket> pCaptured;
    try {
        pCaptured = std::make_shared<const CapturedPacket>(packet);
    } catch (InvalidProtocolException& e) {
        filelog("WarError.log", "broadcast of %s refused: %s", packet.getPacketName().c_str(), e.toString().c_str());
        return;
    }

    // Each zone sends its own copy: the senders take a mutable packet, and a
    // copy shares the captured body, which nothing changes.
    postToZones(zoneIDs, [pCaptured, send](Zone& zone) {
        CapturedPacket captured(*pCaptured);
        send(zone, captured);
    });
}

bool postItemReturn(Item::ItemClass itemClass, ItemID_t itemID, ItemTaken taken, int attemptsMade) {
    ItemPositionRow row;
    if (!defaultItemRepository().loadItemPosition(ItemObjectTableName[(int)itemClass], itemID, row)) {
        filelog("WarError.log", "item return: item %u of class %u has no row", (unsigned)itemID, (unsigned)itemClass);
        return false;
    }

    int attempt = attemptsMade + 1;
    ItemHolder holder = itemHolderOf(row.storage, (unsigned long)row.storageID, row.ownerID);

    switch (holder.kind) {
    case ItemHolder::Kind::Zone:
        return postToZone(holder.zoneID, [row, itemClass, itemID, taken, attempt](Zone&) {
            takeItem(row, itemClass, itemID, taken, attempt, NULL);
        });

    case ItemHolder::Kind::Player: {
        // The player's own box follows him between groups; a player who logs
        // out first drops the relics he carried and saves where they fell,
        // so the return reads the row again from there.
        bool bPosted = de::postToPlayer(
            holder.playerName,
            [row, itemClass, itemID, taken, attempt](PlayerCreature& pc, Player&) {
                takeItem(row, itemClass, itemID, taken, attempt, &pc);
            },
            [itemClass, itemID, taken, attempt] { retryOrStop(itemClass, itemID, taken, attempt); });

        if (bPosted)
            return true;

        // A holder who logs out drops his relics, saving where they fell,
        // before the player-creature finder forgets him; a row read before
        // the drop names him still, and read again it names the ground.
        filelog("WarError.log", "item return: item %u of class %u is held by %s, who is not logged in",
                (unsigned)itemID, (unsigned)itemClass, holder.playerName.c_str());
        return retryOrStop(itemClass, itemID, taken, attempt);
    }

    case ItemHolder::Kind::Nowhere:
        break;
    }

    filelog("WarError.log", "item return: item %u of class %u is held nowhere a position reaches (%s)",
            (unsigned)itemID, (unsigned)itemClass, describe(row).c_str());
    return false;
}

} // namespace de::war
