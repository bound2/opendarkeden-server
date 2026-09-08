#ifndef __FAKE_LOGIN_CHARACTER_REPOSITORY_H__
#define __FAKE_LOGIN_CHARACTER_REPOSITORY_H__

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "repository/LoginCharacterRepository.h"

// In-memory LoginCharacterRepository for the character-creation tests.
// Only the probes and inserts creation uses carry behaviour; the
// selection and character-list reads are present to satisfy the
// interface and answer "no row".
//
// The balance tables answer from maps that start empty, which is the
// real "no such row" case: the loaders leave the caller's value
// untouched and return false.
class FakeLoginCharacterRepository : public LoginCharacterRepository {
public:
    // --- what the probes see ---------------------------------------------
    std::set<std::string> existingNames;
    std::set<std::pair<std::string, std::string>> occupiedSlots; // (playerID, slot)

    // rankType -> RankEXPInfo.GoalExp at Level 1.
    std::map<int, int> rankGoalExp;
    // Level 1 goals of the two race exp tables. Absent means no row.
    std::map<int, int> vampireGoalExp;
    std::map<int, int> oustersGoalExp;
    // (attr, level) -> value.
    std::map<std::pair<int, int>, int> attrGoalExp;
    std::map<std::pair<int, int>, int> attrAccumExp;

    // --- what was written -------------------------------------------------
    std::vector<LoginNewSlayer> insertedSlayers;
    std::vector<LoginNewVampire> insertedVampires;
    std::vector<LoginNewOusters> insertedOusters;
    std::vector<std::pair<std::string, LoginFlagSetPreset>> insertedFlagSets;

    // --- how often the reads were made ------------------------------------
    int rankGoalExpCalls = 0;
    int vampireGoalExpCalls = 0;
    int oustersGoalExpCalls = 0;
    int attrGoalExpCalls = 0;
    int attrAccumExpCalls = 0;
    int slayerNameExistsCalls = 0;
    int slotOccupiedCalls = 0;

    bool slayerNameExists(WorldID_t, const std::string& name) {
        slayerNameExistsCalls++;
        return existingNames.count(name) != 0;
    }

    bool slotOccupied(WorldID_t, const std::string& playerID, const std::string& slot) {
        slotOccupiedCalls++;
        return occupiedSlots.count(std::make_pair(playerID, slot)) != 0;
    }

    bool loadSlayerNameInSlot(WorldID_t, const std::string&, int, std::string&) {
        return false;
    }

    bool loadRankGoalExp(WorldID_t, int rankType, int& goalExp) {
        rankGoalExpCalls++;
        std::map<int, int>::const_iterator itr = rankGoalExp.find(rankType);
        if (itr == rankGoalExp.end())
            return false;
        goalExp = itr->second;
        return true;
    }

    bool loadVampireGoalExp(WorldID_t, int& goalExp) {
        vampireGoalExpCalls++;
        std::map<int, int>::const_iterator itr = vampireGoalExp.find(1);
        if (itr == vampireGoalExp.end())
            return false;
        goalExp = itr->second;
        return true;
    }

    bool loadOustersGoalExp(WorldID_t, int& goalExp) {
        oustersGoalExpCalls++;
        std::map<int, int>::const_iterator itr = oustersGoalExp.find(1);
        if (itr == oustersGoalExp.end())
            return false;
        goalExp = itr->second;
        return true;
    }

    bool loadAttrGoalExp(WorldID_t, LoginAttrTable attr, int level, int& goalExp) {
        attrGoalExpCalls++;
        std::map<std::pair<int, int>, int>::const_iterator itr = attrGoalExp.find(std::make_pair((int)attr, level));
        if (itr == attrGoalExp.end())
            return false;
        goalExp = itr->second;
        return true;
    }

    bool loadAttrAccumExp(WorldID_t, LoginAttrTable attr, int level, int& accumExp) {
        attrAccumExpCalls++;
        std::map<std::pair<int, int>, int>::const_iterator itr = attrAccumExp.find(std::make_pair((int)attr, level));
        if (itr == attrAccumExp.end())
            return false;
        accumExp = itr->second;
        return true;
    }

    void insertSlayer(WorldID_t, const LoginNewSlayer& row) {
        insertedSlayers.push_back(row);
        existingNames.insert(row.name);
        occupiedSlots.insert(std::make_pair(row.playerID, row.slot));
    }

    void insertVampire(WorldID_t, const LoginNewVampire& row) {
        insertedVampires.push_back(row);
    }

    void insertOusters(WorldID_t, const LoginNewOusters& row) {
        insertedOusters.push_back(row);
    }

    void insertFlagSet(WorldID_t, const std::string& name, LoginFlagSetPreset preset) {
        insertedFlagSets.push_back(std::make_pair(name, preset));
    }

    bool loadCharacterForSelect(WorldID_t, LoginRaceTable, const std::string&, const std::string&, LoginSelectRow&) {
        return false;
    }

    void setCharacterServerGroup(WorldID_t, int, const std::string&) {}

    std::vector<LoginSlayerListRow> loadSlayerList(WorldID_t, const std::string&) {
        return std::vector<LoginSlayerListRow>();
    }

    bool loadVampireListRow(WorldID_t, const std::string&, const std::string&, LoginVampireListRow&) {
        return false;
    }

    bool loadOustersListRow(WorldID_t, const std::string&, const std::string&, LoginOustersListRow&) {
        return false;
    }
};

#endif
