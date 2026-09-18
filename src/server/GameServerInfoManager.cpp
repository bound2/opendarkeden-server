//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerInfoManager.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GameServerInfoManager.h"

#include "repository/ServerInfoRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class GameServerInfoManager member methods
//////////////////////////////////////////////////////////////////////////////

GameServerInfoManager::GameServerInfoManager() {
    m_MaxWorldID = 0;
    m_MaxServerGroupID = 0;
}


GameServerInfoManager::~GameServerInfoManager() {
    clear();
}


void GameServerInfoManager::clear() {
    // Delete only the second of each pair in the hashmap, that is the GameServerInfo
    // object, and leave the pair itself. (note that GameServerInfo is created on
    // the heap, so it has to be deleted explicitly. then again, GSIM being destructed
    // means the login server is shutting down..)
    for (int j = 1; j < m_MaxWorldID; j++) {
        for (int i = 0; i < m_MaxServerGroupID; i++) {
            HashMapGameServerInfo::iterator itr = m_pGameServerInfos[j][i].begin();
            for (; itr != m_pGameServerInfos[j][i].end(); itr++) {
                SAFE_DELETE(itr->second);
            }

            // Now delete every pair in the hash map.
            m_pGameServerInfos[j][i].clear();
        }
    }

    if (m_pGameServerInfos != NULL) {
        for (int i = 1; i < m_MaxWorldID; i++)
            SAFE_DELETE_ARRAY(m_pGameServerInfos[i]);
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


    ServerInfoRepository& repo = defaultServerInfoRepository();

    // The tables are sized from the largest GroupID and WorldID; an
    // empty table is a startup error.
    int maxGroupID = 0;
    if (!repo.loadMaxServerGroupID(maxGroupID)) {
        cerr << "GameServerInfo TABLE does not exist!" << endl;
        throw Error("GameServerInfo TABLE does not exist!");
    }

    m_MaxServerGroupID = maxGroupID + 1;

    int maxWorldID = 0;
    if (!repo.loadMaxWorldID(maxWorldID)) {
        cerr << "GameServerInfo TABLE does not exist!" << endl;
        throw Error("GameServerInfo TABLE does not exist!");
    }

    m_MaxWorldID = maxWorldID + 2;

    m_pGameServerInfos = new HashMapGameServerInfo*[m_MaxWorldID];

    for (int i = 1; i < m_MaxWorldID; i++)
        m_pGameServerInfos[i] = new HashMapGameServerInfo[m_MaxServerGroupID];

    cout << "MAX SERVER GROUP = " << m_MaxServerGroupID << endl;

    vector<ServerInfoRow> servers = repo.loadServers();

    for (size_t i = 0; i < servers.size(); i++) {
        const ServerInfoRow& row = servers[i];
        GameServerInfo* pGameServerInfo = new GameServerInfo();

        pGameServerInfo->setServerID(row.serverID);
        pGameServerInfo->setNickname(row.nickname);
        pGameServerInfo->setIP(row.ip);
        pGameServerInfo->setTCPPort(row.tcpPort);
        pGameServerInfo->setUDPPort(row.udpPort);

        WorldID_t WorldID = row.worldID;
        pGameServerInfo->setWorldID(WorldID);

        ServerGroupID_t ServerGroupID = row.groupID;
        pGameServerInfo->setGroupID(ServerGroupID);

        pGameServerInfo->setServerStat((ServerStatus)row.stat);
        addGameServerInfo(pGameServerInfo, ServerGroupID, WorldID);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // The non-PK servers, from the player (login) database.
    ///////////////////////////////////////////////////////////////////////////////
    vector<ServerInfoNonPKRow> nonPK = repo.loadNonPKServers();

    for (size_t i = 0; i < nonPK.size(); i++) {
        WorldID_t worldID = nonPK[i].worldID;
        ServerGroupID_t serverGroupID = nonPK[i].serverGroupID;

        GameServerInfo* pGameServerInfo = getGameServerInfo(1, serverGroupID, worldID);

        pGameServerInfo->setNonPKServer();

        cout << "WorldID:" << (int)worldID << " ServerGroupID:" << (int)serverGroupID << " NonPK set" << endl;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Which server's war results this server's castles follow, from the
    // player (login) database.
    ///////////////////////////////////////////////////////////////////////////////
    vector<ServerInfoCastleStatRow> castleStats = repo.loadCastleStats();

    for (size_t i = 0; i < castleStats.size(); i++) {
        WorldID_t worldID = castleStats[i].worldID;
        ServerGroupID_t serverGroupID = castleStats[i].serverGroupID;
        ServerGroupID_t followServerID = castleStats[i].followServerID;

        GameServerInfo* pGameServerInfo = getGameServerInfo(1, serverGroupID, worldID);

        pGameServerInfo->setCastleFollowingServerID(followServerID);

        cout << "WorldID:" << (int)worldID << " ServerGroupID:" << (int)serverGroupID << " follows"
             << (int)followServerID << endl;
    }

    __END_CATCH
}

void GameServerInfoManager::addGameServerInfo(GameServerInfo* pGameServerInfo, const ServerGroupID_t ServerGroupID,
                                              WorldID_t WorldID) {
    __BEGIN_TRY

    if (ServerGroupID >= m_MaxServerGroupID) {
        throw DuplicatedException("ServerGroupID over Bounce");
    }

    if (WorldID >= m_MaxWorldID) {
        throw DuplicatedException("WorldID over Bounce");
    }

    HashMapGameServerInfo::iterator itr =
        m_pGameServerInfos[WorldID][ServerGroupID].find(pGameServerInfo->getServerID());

    if (itr != m_pGameServerInfos[WorldID][ServerGroupID].end()) {
        throw DuplicatedException("duplicated game-server ServerID");
    }

    m_pGameServerInfos[WorldID][ServerGroupID][pGameServerInfo->getServerID()] = pGameServerInfo;

    __END_CATCH
}

void GameServerInfoManager::deleteGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID,
                                                 WorldID_t WorldID) {
    __BEGIN_TRY

    if (ServerGroupID >= m_MaxServerGroupID) {
        throw DuplicatedException("ServerGroupID over Bounce");
    }
    if (WorldID >= m_MaxWorldID) {
        throw DuplicatedException("WorldID over Bounce");
    }
    HashMapGameServerInfo::iterator itr = m_pGameServerInfos[WorldID][ServerGroupID].find(ServerID);

    if (itr != m_pGameServerInfos[WorldID][ServerGroupID].end()) {
        // Delete the GameServerInfo.
        delete itr->second;

        // Delete the pair.
        m_pGameServerInfos[WorldID][ServerGroupID].erase(itr);
    } else {
        // When no such game server info object can be found
        throw NoSuchElementException();
    }

    __END_CATCH
}


GameServerInfo* GameServerInfoManager::getGameServerInfo(const ServerID_t ServerID, const ServerGroupID_t ServerGroupID,
                                                         WorldID_t WorldID) const {
    __BEGIN_TRY

    GameServerInfo* pGameServerInfo = NULL;

    if (WorldID >= m_MaxWorldID || ServerGroupID >= m_MaxServerGroupID) {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    HashMapGameServerInfo::const_iterator itr = m_pGameServerInfos[WorldID][ServerGroupID].find(ServerID);

    if (itr != m_pGameServerInfos[WorldID][ServerGroupID].end()) {
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

    for (int j = 1; j < m_MaxWorldID; j++) {
        for (int i = 0; i < m_MaxServerGroupID; i++) {
            if (m_pGameServerInfos[j][i].empty()) {
                msg << "EMPTY";
            } else {
                HashMapGameServerInfo::const_iterator itr = m_pGameServerInfos[j][i].begin();
                for (; itr != m_pGameServerInfos[j][i].end(); itr++) {
                    msg << itr->second->toString() << '\n';
                }
            }

            msg << ")";
        }
    }

    return msg.toString();

    __END_CATCH
}

// global variable definition
GameServerInfoManager* g_pGameServerInfoManager = NULL;
