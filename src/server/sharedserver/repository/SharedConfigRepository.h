#ifndef __SHARED_CONFIG_REPOSITORY_H__
#define __SHARED_CONFIG_REPOSITORY_H__

#include <string>
#include <vector>

// The sharedserver's boot-time catalogues, read once in SharedServer::init()
// on the main thread and never written: the game-server groups
// (GameServerGroupInfo) and servers (GameServerInfo) it relays guild
// packets to, the per-zone resurrection coordinates it answers with
// (ZoneInfo) and its message texts (SSStringPool). Every statement runs on
// getConnection("DARKEDEN").
//
// Fields are typed to the driver getter used (getInt → int, getString →
// std::string); the callers narrow to their own types. The two MAX probes
// answer false over an empty table (MySQL returns one NULL row); their
// callers size an array from the answer.
//
// Not enclosed (SQL on the same tables elsewhere in the tree):
//  - GameServerGroupInfo: the loginserver's GameServerGroupInfoManager.cpp
//    and the gameserver's MySQLGameInfoRepository.cpp.
//  - GameServerInfo: ServerCore's GameServerInfoManager.cpp (compiled into
//    all three binaries), the loginserver's UserInfoManager.cpp, and the
//    gameserver's GameServerInfoManager.cpp (in no CMakeLists; never
//    compiled).
//  - ZoneInfo: the gameserver's MySQLZoneInfoRepository.cpp and the
//    loginserver's ZoneInfoManager.cpp. The sharedserver reads only the
//    Slayer and Vampire resurrection columns; the Ousters columns exist
//    and are not read.
//  - SSStringPool: nothing else.

struct SharedGameServerGroupRow {
    int worldID;
    int groupID;
    std::string groupName;
};

// GameServerInfo, every world; the caller keeps the rows of its own.
struct SharedGameServerRow {
    int serverID;
    std::string nickname;
    std::string ip;
    int tcpPort;
    int udpPort;
    int groupID;
    int stat;
    int worldID;
};

struct SharedResurrectLocationRow {
    int zoneID;
    int slayerZoneID;
    int slayerX;
    int slayerY;
    int vampireZoneID;
    int vampireX;
    int vampireY;
};

struct SharedStringRow {
    int id;
    std::string text;
};

class SharedConfigRepository {
public:
    virtual ~SharedConfigRepository() {}

    // MAX(WorldID) over GameServerGroupInfo; false when the table is empty.
    virtual bool loadMaxGameServerGroupWorldID(int& maxWorldID) = 0;
    virtual std::vector<SharedGameServerGroupRow> loadGameServerGroups() = 0;

    // MAX(GroupID) of one world's GameServerInfo rows; false when the world
    // has none.
    virtual bool loadMaxGameServerGroupID(int worldID, int& maxGroupID) = 0;
    virtual std::vector<SharedGameServerRow> loadGameServers() = 0;

    virtual std::vector<SharedResurrectLocationRow> loadResurrectLocations() = 0;

    virtual std::vector<SharedStringRow> loadStrings() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSharedConfigRepository.cpp.
SharedConfigRepository& defaultSharedConfigRepository();

#endif
