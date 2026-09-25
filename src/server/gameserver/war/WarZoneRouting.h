//////////////////////////////////////////////////////////////////////////////
// Filename    : WarZoneRouting.h
// Description : which thread a war's zone work belongs to. A war starts and
//               ends on whichever thread runs its schedule -- the main
//               thread for a castle war's end and for the race war, a castle
//               zone's thread for a castle war's start -- while the zones it
//               touches, and the relics lying in them or carried by their
//               players, belong to the zone groups' threads. So each piece of
//               zone work is posted to the owner (WarZoneWork.h), and these
//               are the decisions that route it, kept apart from the zones,
//               the players and the database so they can be exercised alone.
//////////////////////////////////////////////////////////////////////////////

#ifndef __WAR_ZONE_ROUTING_H__
#define __WAR_ZONE_ROUTING_H__

#include <string>
#include <utility>
#include <vector>

#include "Types.h"

namespace de::war {

// Who holds an item, as the position columns of its item-object row say.
// A zone holds what lies on its ground and what lies in a corpse on it; a
// player holds what is in the inventory or on the mouse. Anything else --
// gear, belt, stash, motorcycle, a zone id of 0, a row with no owner name --
// is a place no item position loader reaches, and the item is held nowhere
// the war can return it from.
struct ItemHolder {
    enum class Kind { Zone, Player, Nowhere };

    Kind kind = Kind::Nowhere;
    ZoneID_t zoneID = 0;    // Kind::Zone
    std::string playerName; // Kind::Player

    bool operator==(const ItemHolder& other) const {
        return kind == other.kind && zoneID == other.zoneID && playerName == other.playerName;
    }
};

// The zone id a corpse's row names in OwnerID: an item created inside a
// corpse records the corpse's zone there as decimal text (Item::create with
// STORAGE_CORPSE). Anything but a plain decimal zone id in range is 0.
inline ZoneID_t corpseZoneIDOf(const std::string& ownerID) {
    if (ownerID.empty() || ownerID.size() > 5)
        return 0;
    unsigned long zoneID = 0;
    for (char c : ownerID) {
        if (c < '0' || c > '9')
            return 0;
        zoneID = zoneID * 10 + (unsigned long)(c - '0');
    }
    if (zoneID > 0xffff)
        return 0;
    return (ZoneID_t)zoneID;
}

// The holder the row's Storage, StorageID and OwnerID columns name.
inline ItemHolder itemHolderOf(int storage, unsigned long storageID, const std::string& ownerID) {
    ItemHolder holder;

    switch (storage) {
    case STORAGE_ZONE:
        if (storageID != 0 && storageID <= 0xffff) {
            holder.kind = ItemHolder::Kind::Zone;
            holder.zoneID = (ZoneID_t)storageID;
        }
        break;
    case STORAGE_CORPSE:
        holder.zoneID = corpseZoneIDOf(ownerID);
        if (holder.zoneID != 0)
            holder.kind = ItemHolder::Kind::Zone;
        break;
    case STORAGE_INVENTORY:
    case STORAGE_EXTRASLOT:
        if (!ownerID.empty()) {
            holder.kind = ItemHolder::Kind::Player;
            holder.playerName = ownerID;
        }
        break;
    default:
        break;
    }

    return holder;
}

// The storages a relic may lie in: the ground, a corpse (a shrine or a
// relic table), a player's inventory and his mouse -- exactly the places
// itemHolderOf answers for. Every gateway into another storage refuses a
// relic: equipping takes only the classes each gear slot names, the belt
// and the Ousters armsband take potions, magazines and their like, the
// stash refuses through canPutInStash, trading and exchange listings
// through canTrade, selling and personal stores through canSell, the pet
// stash takes pets only, and nothing writes a motorcycle, store or box row.
// The garbage takes only what an item loader cannot place, and no loader
// places a relic on a player; the time-over storage takes only a
// time-limited item, which a war's relic never is; a mall delivery mints a
// new item rather than moving one. A relic's row written whole with another
// storage is logged (logRelicStorage, RelicUtil.h); the piecewise saves a
// move makes (tinysave) are the gateways' own, and those refuse the storage.
inline bool relicMayLieIn(int storage) {
    return storage == STORAGE_ZONE || storage == STORAGE_CORPSE || storage == STORAGE_INVENTORY ||
           storage == STORAGE_EXTRASLOT;
}

// How many times a return looks for an item. The row is read before the
// holder's step runs, and in between the item may move: a player carrying it
// is transported or logs out and drops it on the ground, saving the new
// position as it does. A holder step that finds nothing where the row said
// reads the row again and routes the return to the new holder, until this
// many attempts have been made; then it logs and stops, so an item whose row
// never catches up cannot bounce between threads for good.
constexpr int kItemReturnAttempts = 3;

inline bool retryItemReturn(int attemptsMade) {
    return attemptsMade < kItemReturnAttempts;
}

// Zone ids grouped by the zone group that owns each, the groups in the order
// their first zone appears and each group's zones in their own order, so one
// command per group does that group's zones in the order the war asked for
// them. groupOf(zoneID) names the owning group.
template <typename GroupID, typename GroupOf>
std::vector<std::pair<GroupID, std::vector<ZoneID_t>>> zonesByOwner(const std::vector<ZoneID_t>& zoneIDs,
                                                                    GroupOf groupOf) {
    std::vector<std::pair<GroupID, std::vector<ZoneID_t>>> batches;

    for (ZoneID_t zoneID : zoneIDs) {
        GroupID groupID = groupOf(zoneID);

        auto batch = batches.begin();
        while (batch != batches.end() && batch->first != groupID)
            ++batch;

        if (batch == batches.end())
            batches.emplace_back(groupID, std::vector<ZoneID_t>{zoneID});
        else
            batch->second.push_back(zoneID);
    }

    return batches;
}

} // namespace de::war

#endif // __WAR_ZONE_ROUTING_H__
