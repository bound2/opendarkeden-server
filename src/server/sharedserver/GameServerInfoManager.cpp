//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerInfoManager.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GameServerInfoManager.h"

#include "Properties.h"
#include "repository/SharedConfigRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class GameServerInfoManager member methods
//////////////////////////////////////////////////////////////////////////////

GameServerInfoManager::GameServerInfoManager() {}


GameServerInfoManager::~GameServerInfoManager() {
    // Delete only the second of each pair in the hash map, i.e. the
    // GameServerInfo objects, and leave the pairs themselves. (Note that
    // they live on the heap, so they must be deleted explicitly. GSIM being
    // destructed means the login server is shutting down anyway.)
    for (int i = 0; i < m_MaxServerGroupID; i++) {
        HashMapGameServerInfoItor itr = m_pGameServerInfos[i].begin();
        for (; itr != m_pGameServerInfos[i].end(); itr++) {
            SAFE_DELETE(itr->second);
        }

        // Now erase every pair in the hash map.
        m_pGameServerInfos[i].clear();
    }

    if (m_pGameServerInfos != NULL) {
        SAFE_DELETE_ARRAY(m_pGameServerInfos);
    }
}


void GameServerInfoManager::init() {
    __BEGIN_TRY

    // just load data from GameServerInfo table
    load();

    // just print to cout
    cout << toString() << endl;

    __END_CATCH
}

void GameServerInfoManager::load() {
    __BEGIN_TRY

    SharedConfigRepository& repo = defaultSharedConfigRepository();

    WorldID_t WorldID = g_pConfig->getPropertyInt("WorldID");

    // The table is sized from this world's largest GroupID; a world with
    // no servers is a startup error.
    int maxGroupID = 0;
    if (!repo.loadMaxGameServerGroupID(WorldID, maxGroupID)) {
        throw Error("GameServerInfo TABLE does not exist!");
    }

    m_MaxServerGroupID = maxGroupID + 1;

    m_pGameServerInfos = new HashMapGameServerInfo[m_MaxServerGroupID];

    cout << "MAX SERVER GROUP = " << m_MaxServerGroupID << endl;

    // Every world's servers are read; only this world's are kept.
    vector<SharedGameServerRow> rows = repo.loadGameServers();

    for (size_t i = 0; i < rows.size(); i++) {
        const SharedGameServerRow& row = rows[i];

        if (row.worldID == WorldID) {
            GameServerInfo* pGameServerInfo = new GameServerInfo();

            pGameServerInfo->setServerID(row.serverID);
            pGameServerInfo->setNickname(row.nickname);
            pGameServerInfo->setIP(row.ip);
            pGameServerInfo->setTCPPort(row.tcpPort);
            pGameServerInfo->setUDPPort(row.udpPort);
            pGameServerInfo->setWorldID(WorldID);

            ServerGroupID_t ServerGroupID = row.groupID;
            pGameServerInfo->setGroupID(ServerGroupID);
            pGameServerInfo->setServerStat((ServerStatus)row.stat);

            addGameServerInfo(pGameServerInfo, ServerGroupID);
        }
    }

    __END_CATCH
}

void GameServerInfoManager::addGameServerInfo(GameServerInfo* pGameServerInfo, const ServerGroupID_t ServerGroupID) {
    __BEGIN_TRY

    if (ServerGroupID >= m_MaxServerGroupID) {
        throw DuplicatedException("ServerGroupID over Bounce");
    }

    HashMapGameServerInfoItor itr = m_pGameServerInfos[ServerGroupID].find(pGameServerInfo->getServerID());

    if (itr != m_pGameServerInfos[ServerGroupID].end()) {
        throw DuplicatedException("duplicated game-server ServerID");
    }

    m_pGameServerInfos[ServerGroupID][pGameServerInfo->getServerID()] = pGameServerInfo;

    __END_CATCH
}

void GameServerInfoManager::deleteGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID) {
    __BEGIN_TRY

    if (ServerGroupID >= m_MaxServerGroupID) {
        throw DuplicatedException("ServerGroupID over Bounce");
    }

    HashMapGameServerInfoItor itr = m_pGameServerInfos[ServerGroupID].find(ServerID);

    if (itr != m_pGameServerInfos[ServerGroupID].end()) {
        // Delete the GameServerInfo.
        delete itr->second;

        // Erase the pair.
        m_pGameServerInfos[ServerGroupID].erase(itr);
    } else {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    __END_CATCH
}


GameServerInfo* GameServerInfoManager::getGameServerInfo(const ServerID_t ServerID,
                                                         const ServerGroupID_t ServerGroupID) const {
    __BEGIN_TRY

    GameServerInfo* pGameServerInfo = NULL;

    if (ServerGroupID >= m_MaxServerGroupID) {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    HashMapGameServerInfoItor itr = m_pGameServerInfos[ServerGroupID].find(ServerID);

    if (itr != m_pGameServerInfos[ServerGroupID].end()) {
        pGameServerInfo = itr->second;
    } else {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    return pGameServerInfo;

    __END_CATCH
}

string GameServerInfoManager::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GameServerInfoManager(\n";

    for (int i = 0; i < m_MaxServerGroupID; i++) {
        if (m_pGameServerInfos[i].empty()) {
            msg << "EMPTY";
        } else {
            HashMapGameServerInfoItor itr = m_pGameServerInfos[i].begin();
            for (; itr != m_pGameServerInfos[i].end(); itr++) {
                msg << itr->second->toString() << '\n';
            }
        }

        msg << ")";
    }

    return msg.toString();

    __END_CATCH
}
