//////////////////////////////////////////////////////////////////////////////
// Filename    : FlagWarPlan.h
// Description : the decisions of a flag war's start and end, apart from the
//               zones. A flag war starts and ends on the main thread
//               (FlagManager's heartbeat), while the flags it drops, the
//               poles they are planted on and the players carrying them
//               belong to the zone groups' threads, so each change is posted
//               to its owner (war/WarZoneWork.h). These are the values those
//               posted commands carry and the order the end posts them in.
//////////////////////////////////////////////////////////////////////////////

#ifndef __FLAG_WAR_PLAN_H__
#define __FLAG_WAR_PLAN_H__

#include <map>
#include <mutex>
#include <utility>
#include <vector>

#include "Types.h"

namespace de::ctf {

// How many flags a war drops in one zone.
struct FlagDrop {
    ZoneID_t zoneID = 0;
    uint count = 0;
};

// The zones a war lets flags into, each with the number of flags dropped
// there: a player carrying a flag out of these zones drops it, and the
// war's status goes to the players in them.
inline std::map<ZoneID_t, uint> flagAllowMapOf(const std::vector<FlagDrop>& drops) {
    std::map<ZoneID_t, uint> allowed;
    for (const FlagDrop& drop : drops)
        allowed[drop.zoneID] = drop.count;
    return allowed;
}

// A field of flag poles, as FlagManager::addPoleField laid it out: poles two
// tiles apart from (left, top), width and height the field's extent in tiles.
struct PoleField {
    ZoneID_t zoneID = 0;
    ZoneCoord_t left = 0, top = 0, width = 0, height = 0;
};

// The tiles a sweep looks at for a flag planted on a pole: every second
// tile from the field's corner, as far as twice its extent. The sweep checks
// each tile against the zone and takes only a flag pole's flag, so the tiles
// past the field cost a look and nothing else.
inline std::vector<std::pair<ZoneCoord_t, ZoneCoord_t>> poleSweepTiles(const PoleField& field) {
    std::vector<std::pair<ZoneCoord_t, ZoneCoord_t>> tiles;
    for (uint x = field.left; x <= (uint)field.left + (uint)field.width * 2; x += 2)
        for (uint y = field.top; y <= (uint)field.top + (uint)field.height * 2; y += 2)
            tiles.emplace_back((ZoneCoord_t)x, (ZoneCoord_t)y);
    return tiles;
}

// The pole zones a sweep is posted to, each once, in the order the fields
// name them.
inline std::vector<ZoneID_t> poleZonesOf(const std::vector<PoleField>& fields) {
    std::vector<ZoneID_t> zones;
    for (const PoleField& field : fields) {
        bool bSeen = false;
        for (ZoneID_t zoneID : zones)
            bSeen = bSeen || zoneID == field.zoneID;
        if (!bSeen)
            zones.push_back(field.zoneID);
    }
    return zones;
}

// What a war's end posts, in order. Every flag's return comes before the
// pole sweep: a flag planted on a pole lies in the pole's zone, so its
// return and the sweep go to the same group, whose mailbox runs them in the
// order they were posted. The return takes the flag out through the
// corpse's position, which drops the winner's gem stone at a pole of the
// winning race; the sweep then clears the flags whose return missed, with
// no reward.
struct FlagWarEndStep {
    enum class Kind { ReturnFlag, SweepPoles };

    Kind kind = Kind::ReturnFlag;
    ItemID_t flagID = 0; // Kind::ReturnFlag
    ZoneID_t zoneID = 0; // Kind::SweepPoles

    bool operator==(const FlagWarEndStep& other) const {
        return kind == other.kind && flagID == other.flagID && zoneID == other.zoneID;
    }
};

inline std::vector<FlagWarEndStep> flagWarEndSteps(const std::vector<ItemID_t>& flagIDs,
                                                   const std::vector<PoleField>& fields) {
    std::vector<FlagWarEndStep> steps;
    for (ItemID_t flagID : flagIDs) {
        FlagWarEndStep step;
        step.kind = FlagWarEndStep::Kind::ReturnFlag;
        step.flagID = flagID;
        steps.push_back(step);
    }
    for (ZoneID_t zoneID : poleZonesOf(fields)) {
        FlagWarEndStep step;
        step.kind = FlagWarEndStep::Kind::SweepPoles;
        step.zoneID = zoneID;
        steps.push_back(step);
    }
    return steps;
}

// The ids of the flags a war dropped. The drops run on the zones' threads,
// each adding the flags it made; the end, on the main thread, takes the
// whole list. The posted drops hold the ledger by shared_ptr, so it outlives
// a drop that runs after the war let it go.
class FlagLedger {
public:
    void add(ItemID_t flagID) {
        std::lock_guard lock(m_Mutex);
        m_FlagIDs.push_back(flagID);
    }

    // Every id added so far, leaving the ledger empty.
    std::vector<ItemID_t> take() {
        std::lock_guard lock(m_Mutex);
        std::vector<ItemID_t> flagIDs;
        flagIDs.swap(m_FlagIDs);
        return flagIDs;
    }

private:
    std::mutex m_Mutex; // leaf: held only to add or take
    std::vector<ItemID_t> m_FlagIDs;
};

} // namespace de::ctf

#endif // __FLAG_WAR_PLAN_H__
