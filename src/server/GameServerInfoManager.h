//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerInfoManager.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_SERVER_INFO_MANAGER_H__
#define __GAME_SERVER_INFO_MANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "GameServerInfo.h"
#include "Types.h"

typedef unordered_map<ServerID_t, GameServerInfo*> HashMapGameServerInfo;

//////////////////////////////////////////////////////////////////////////////
// class GameServerInfoManager;
// It holds an unordered_map of GameServerInfo keyed by the game server's ID
// internally.
//////////////////////////////////////////////////////////////////////////////

class GameServerInfoManager {
public:
    GameServerInfoManager();
    ~GameServerInfoManager();

public:
    void init();
    void load();

    // clear GameServerInfo objects
    void clear();

    void addGameServerInfo(GameServerInfo* pGameServerInfo, const ServerGroupID_t ServerGroupID, WorldID_t WorldID);
    void deleteGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID, WorldID_t WorldID);
    // GameServerInfo * getGameServerInfo ( const string & name ) const ;
    GameServerInfo* getGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID,
                                      WorldID_t WorldID) const;
    uint getSize(WorldID_t WorldID, const ServerGroupID_t ServerGroupID) const {
        return m_pGameServerInfos[WorldID][ServerGroupID].size();
    }
    string toString() const;

    // by sigi. 2002.5.30
    int getMaxWorldID() const {
        return m_MaxWorldID;
    }
    int getMaxServerGroupID() const {
        return m_MaxServerGroupID;
    }
    HashMapGameServerInfo** getGameServerInfos() const {
        return m_pGameServerInfos;
    }

private:
    // hash map of GameServerInfo
    // key   : GameServerID_t
    // value : GameServerInfo *
    HashMapGameServerInfo** m_pGameServerInfos;
    int m_MaxServerGroupID;
    int m_MaxWorldID;
};

#endif
