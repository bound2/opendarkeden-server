//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerInfoManager.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SHARED_SERVER_GAME_SERVER_INFO_MANAGER_H__
#define __SHARED_SERVER_GAME_SERVER_INFO_MANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "GameServerInfo.h"
#include "Types.h"

typedef unordered_map<ServerID_t, GameServerInfo*> HashMapGameServerInfo;
typedef HashMapGameServerInfo::iterator HashMapGameServerInfoItor;

//////////////////////////////////////////////////////////////////////////////
// class GameServerInfoManager;
// Holds an unordered_map of GameServerInfo keyed by the game server ID
// internally.
// Holds the GameServerInfo of a single World.
//////////////////////////////////////////////////////////////////////////////

class GameServerInfoManager {
public:
    GameServerInfoManager();
    ~GameServerInfoManager();

public:
    void init();
    void load();

    void addGameServerInfo(GameServerInfo* pGameServerInfo, const ServerGroupID_t ServerGroupID);
    void deleteGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID);
    GameServerInfo* getGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID) const;
    uint getSize(const ServerGroupID_t ServerGroupID) const {
        return m_pGameServerInfos[ServerGroupID].size();
    }
    string toString() const;

    // get MaxServerGroupID
    int getMaxServerGroupID() const {
        return m_MaxServerGroupID;
    }

private:
    // hash map of GameServerInfo
    // key   : GameServerID_t
    // value : GameServerInfo *
    HashMapGameServerInfo* m_pGameServerInfos;
    int m_MaxServerGroupID;
};

#endif
