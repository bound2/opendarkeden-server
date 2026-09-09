#ifndef __FAKE_WORLD_TOPOLOGY_H__
#define __FAKE_WORLD_TOPOLOGY_H__

// A WorldSelectionTopology over seeded tables that records every query in
// order, so a test can pin the lookups a decision makes - and the ones it
// does not.

#include <string>
#include <vector>

#include "WorldSelection.h"

class FakeWorldTopology : public WorldSelectionTopology {
public:
    // One configured group: what the group table says and how many accounts
    // it carries.
    struct Group {
        ServerGroupRow row;
        UserNum_t userNum = 0;
    };

    // How many worlds the world table holds.
    int worlds = 1;
    // The status of each world, indexed by world id. A world with no entry
    // here is open.
    std::vector<WorldStatus> worldStatuses;
    // The groups of the world under test, indexed by group id.
    std::vector<Group> groups;

    // Every query, in the order it was made.
    std::vector<std::string> calls;

    void addGroup(ServerGroupID_t groupID, const std::string& name, BYTE stat, UserNum_t userNum) {
        Group group;
        group.row.groupID = groupID;
        group.row.groupName = name;
        group.row.stat = stat;
        group.userNum = userNum;
        groups.push_back(group);
    }

    int worldCount() override {
        calls.push_back("worldCount");
        return worlds;
    }

    WorldStatus worldStatus(WorldID_t worldID) override {
        calls.push_back("worldStatus(" + std::to_string((int)worldID) + ")");

        if (worldID < worldStatuses.size())
            return worldStatuses[worldID];

        return WORLD_OPEN;
    }

    int serverGroupCount(WorldID_t worldID) override {
        calls.push_back("serverGroupCount(" + std::to_string((int)worldID) + ")");
        return static_cast<int>(groups.size());
    }

    ServerGroupRow serverGroup(ServerGroupID_t groupID, WorldID_t worldID) override {
        calls.push_back("serverGroup(" + std::to_string((int)groupID) + "," + std::to_string((int)worldID) + ")");
        return at(groupID).row;
    }

    UserNum_t serverGroupUserNum(ServerGroupID_t groupID, WorldID_t worldID) override {
        calls.push_back("serverGroupUserNum(" + std::to_string((int)groupID) + "," + std::to_string((int)worldID) +
                        ")");
        return at(groupID).userNum;
    }

private:
    // A group id the tables do not hold answers an empty free group rather
    // than throwing; the tests that care seed every id they ask for.
    Group at(ServerGroupID_t groupID) const {
        if (groupID < groups.size())
            return groups[groupID];

        return Group();
    }
};

#endif
