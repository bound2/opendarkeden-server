#ifndef __SERVER_INFO_REPOSITORY_H__
#define __SERVER_INFO_REPOSITORY_H__

#include <string>
#include <vector>

// ServerCore's boot-time catalogues (compiled into all three binaries):
// the game servers (GameServerInfo) with the two flags layered on them
// (NonPKServerList, CastleStatInfo) and the worlds (WorldInfo), read once
// by GameServerInfoManager::load and GameWorldInfoManager::load.
//
// Connections: GameServerInfo and WorldInfo on the thread's DARKEDEN
// connection; NonPKServerList and CastleStatInfo on
// g_pDatabaseManager->getDistConnection("DARKEDEN"), the thread's dist
// connection (the name is ignored).
//
// Fields are typed to the driver getter used: getInt → int, getString →
// std::string; the callers narrow to WorldID_t, ServerGroupID_t and the
// status enums. The two MAX probes answer false over an empty table
// (MySQL returns one NULL row); the caller sizes its arrays from them.
//
// Not enclosed: GameServerInfo is also read by the sharedserver's
// SharedConfigRepository and by gameserver/GameServerInfoManager.cpp,
// WorldInfo by gameserver/GameWorldInfoManager.cpp — both stale copies
// in no CMakeLists, never compiled; NonPKServerList and CastleStatInfo:
// nothing else.

// GameServerInfo, every world, in the statement's column order.
struct ServerInfoRow {
    int serverID;
    std::string nickname;
    std::string ip;
    int tcpPort;
    int udpPort;
    int worldID;
    int groupID;
    int stat;
};

struct ServerInfoNonPKRow {
    int worldID;
    int serverGroupID;
};

struct ServerInfoCastleStatRow {
    int worldID;
    int serverGroupID;
    int followServerID;
};

struct ServerInfoWorldRow {
    int id;
    std::string name;
    int stat;
};

class ServerInfoRepository {
public:
    virtual ~ServerInfoRepository() {}

    // MAX(GroupID) / MAX(WorldID) over GameServerInfo; false when the
    // table is empty.
    virtual bool loadMaxServerGroupID(int& maxGroupID) = 0;
    virtual bool loadMaxWorldID(int& maxWorldID) = 0;
    virtual std::vector<ServerInfoRow> loadServers() = 0;
    virtual std::vector<ServerInfoNonPKRow> loadNonPKServers() = 0;
    virtual std::vector<ServerInfoCastleStatRow> loadCastleStats() = 0;
    virtual std::vector<ServerInfoWorldRow> loadWorlds() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLServerInfoRepository.cpp.
ServerInfoRepository& defaultServerInfoRepository();

#endif
