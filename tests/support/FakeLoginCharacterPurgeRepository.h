#ifndef __FAKE_LOGIN_CHARACTER_PURGE_REPOSITORY_H__
#define __FAKE_LOGIN_CHARACTER_PURGE_REPOSITORY_H__

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "repository/LoginCharacterPurgeRepository.h"

// In-memory LoginCharacterPurgeRepository for the character-deletion test.
// The ownership read and the compare-and-set retirement carry behaviour;
// the purge writes record what they were asked to do.
//
// The character map starts empty, which is the real "no such character"
// case: loadActiveSlayerOwner leaves the caller's string untouched and
// returns false.
class FakeLoginCharacterPurgeRepository : public LoginCharacterPurgeRepository {
public:
    // --- what the reads see -----------------------------------------------
    struct SlayerRow {
        std::string playerID;
        Slot slot = SLOT1;
    };
    // name -> the ACTIVE Slayer row of that name.
    std::map<std::string, SlayerRow> activeSlayers;

    // Adds an ACTIVE Slayer row: the character exists, belongs to that
    // account and sits in that slot.
    void addActiveSlayer(const std::string& name, const std::string& playerID, Slot slot) {
        SlayerRow row;
        row.playerID = playerID;
        row.slot = slot;
        activeSlayers[name] = row;
    }

    // --- what was written --------------------------------------------------
    struct Retirement {
        WorldID_t worldID;
        std::string name;
        Slot slot;
    };
    std::vector<Retirement> retirements;

    struct Deletion {
        std::string playerID;
        WorldID_t worldID;
        std::string name;
    };
    std::vector<Deletion> recordedDeletions;
    std::vector<Retirement> purges;
    std::vector<std::string> destroyedItemOwners;

    // --- how often the reads were made, and against which world ------------
    int loadActiveSlayerOwnerCalls = 0;
    int retireSlayerCalls = 0;
    std::vector<WorldID_t> ownerLookupWorldIDs;

    bool loadActiveSlayerOwner(WorldID_t worldID, const std::string& name, std::string& playerID) {
        loadActiveSlayerOwnerCalls++;
        ownerLookupWorldIDs.push_back(worldID);
        std::map<std::string, SlayerRow>::const_iterator itr = activeSlayers.find(name);
        if (itr == activeSlayers.end())
            return false;
        playerID = itr->second.playerID;
        return true;
    }

    // The UPDATE names both the character and the slot, so a row in
    // another slot changes nothing and answers false.
    bool retireSlayer(WorldID_t worldID, const std::string& name, Slot slot) {
        retireSlayerCalls++;
        Retirement call;
        call.worldID = worldID;
        call.name = name;
        call.slot = slot;
        retirements.push_back(call);

        std::map<std::string, SlayerRow>::iterator itr = activeSlayers.find(name);
        if (itr == activeSlayers.end() || itr->second.slot != slot)
            return false;

        activeSlayers.erase(itr);
        return true;
    }

    void recordDeletion(const std::string& playerID, WorldID_t worldID, const std::string& name) {
        Deletion call;
        call.playerID = playerID;
        call.worldID = worldID;
        call.name = name;
        recordedDeletions.push_back(call);
    }

    void purgeCharacterRows(WorldID_t worldID, const std::string& name, Slot slot) {
        Retirement call;
        call.worldID = worldID;
        call.name = name;
        call.slot = slot;
        purges.push_back(call);
    }

    void destroyItems(const std::string& ownerID) {
        destroyedItemOwners.push_back(ownerID);
    }
};

#endif
