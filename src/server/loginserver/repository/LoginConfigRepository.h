#ifndef __LOGIN_CONFIG_REPOSITORY_H__
#define __LOGIN_CONFIG_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The loginserver's boot-time catalogues, read once in LoginServer::init()
// on the main thread and never written: the game-server groups it lists
// to clients (GameServerGroupInfo, read twice — GameServerGroupInfoManager
// keeps the names and status, UserInfoManager the ids for its user
// counters), the zone-to-group and group-to-server maps a character
// selection routes through (ZoneInfo, ZoneGroupInfo) and the client
// version row (ClientVersion). Every statement runs on
// getConnection("DARKEDEN").
//
// Fields are typed to the driver getter used: getInt → int, getString →
// std::string, getWORD → WORD. The MAX probe answers false over an empty
// table (MySQL returns one NULL row); both its callers size an array from
// the answer.
//
// Not enclosed (SQL on the same tables elsewhere in the tree):
//  - GameServerGroupInfo: the sharedserver's SharedConfigRepository and
//    the gameserver's MySQLGameInfoRepository.cpp.
//  - ZoneInfo: the gameserver's MySQLZoneInfoRepository.cpp and the
//    sharedserver's SharedConfigRepository (the resurrection columns).
//  - ZoneGroupInfo: the gameserver's MySQLZoneInfoRepository.cpp; the
//    gameserver's CGSayHandler.cpp names it in a commented-out block.
//  - ClientVersion: nothing else.

struct LoginGameServerGroupRow {
    int worldID;
    int groupID;
    std::string groupName;
    int stat;
};

struct LoginGameServerGroupIDRow {
    int worldID;
    int groupID;
};

struct LoginZoneGroupRow {
    WORD zoneGroupID;
    WORD serverID;
};

struct LoginZoneRow {
    WORD zoneID;
    WORD zoneGroupID;
};

class LoginConfigRepository {
public:
    virtual ~LoginConfigRepository() {}

    // MAX(WorldID) over GameServerGroupInfo; false when the table is empty.
    virtual bool loadMaxGameServerGroupWorldID(int& maxWorldID) = 0;
    // WorldID, GroupID, GroupName, Stat.
    virtual std::vector<LoginGameServerGroupRow> loadGameServerGroups() = 0;
    // WorldID, GroupID only.
    virtual std::vector<LoginGameServerGroupIDRow> loadGameServerGroupIDs() = 0;

    virtual std::vector<LoginZoneGroupRow> loadZoneGroups() = 0;
    virtual std::vector<LoginZoneRow> loadZones() = 0;

    // The first ClientVersion row's Version; false when the table is empty.
    virtual bool loadClientVersion(int& version) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLLoginConfigRepository.cpp.
LoginConfigRepository& defaultLoginConfigRepository();

#endif
