//----------------------------------------------------------------------
//
// Filename    : GameServerGroupInfoManager.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GameServerGroupInfoManager.h"

#include "DatabaseError.h"
#include "repository/SharedConfigRepository.h"

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------
GameServerGroupInfoManager::GameServerGroupInfoManager() {}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
GameServerGroupInfoManager::~GameServerGroupInfoManager() {
    // Delete only the second of each pair in the hash map, i.e. the
    // GameServerGroupInfo objects, and leave the pairs themselves. (Note that
    // they live on the heap, so they must be deleted explicitly. GSIM being
    // destructed means the login server is shutting down anyway.)
    for (int i = 1; i < m_MaxWorldID; i++) {
        for (HashMapGameServerGroupInfo::iterator itr = m_GameServerGroupInfos[i].begin();
             itr != m_GameServerGroupInfos[i].end(); itr++) {
            delete itr->second;
            itr->second = NULL;
        }

        // Now erase every pair in the hash map.
        m_GameServerGroupInfos[i].clear();
    }

    SAFE_DELETE_ARRAY(m_GameServerGroupInfos);
}


//----------------------------------------------------------------------
// initialize GSIM
//----------------------------------------------------------------------
void GameServerGroupInfoManager::init() {
    __BEGIN_TRY

    // just load data from GameServerGroupInfo table
    load();

    // just print to cout
    cout << toString() << endl;

    __END_CATCH
}

//----------------------------------------------------------------------
// load data from database
//----------------------------------------------------------------------
void GameServerGroupInfoManager::load() {
    __BEGIN_TRY

    SharedConfigRepository& repo = defaultSharedConfigRepository();

    // The table is sized from the largest WorldID; an empty table is a
    // startup error.
    int maxWorldID = 0;
    if (!repo.loadMaxGameServerGroupWorldID(maxWorldID)) {
        throw Error("GameServerGroupInfo TABLE does not exist!");
    }

    m_MaxWorldID = maxWorldID + 2;

    m_GameServerGroupInfos = new HashMapGameServerGroupInfo[m_MaxWorldID];

    vector<SharedGameServerGroupRow> rows;

    try {
        rows = repo.loadGameServerGroups();
    } catch (const DatabaseError& error) {
        // A SQL failure arrives as END_DB's DatabaseError carrying the line
        // it wrote to DBError.log; rethrown as the Error the startup path
        // expects, with that line in it.
        throw Error("GameServerGroupInfoManager::load : " + error.message());
    }

    try {
        for (size_t i = 0; i < rows.size(); i++) {
            GameServerGroupInfo* pGameServerGroupInfo = new GameServerGroupInfo();
            WorldID_t WorldID = rows[i].worldID;
            pGameServerGroupInfo->setWorldID(WorldID);
            pGameServerGroupInfo->setGroupID(rows[i].groupID);
            pGameServerGroupInfo->setGroupName(rows[i].groupName);
            addGameServerGroupInfo(pGameServerGroupInfo, WorldID);
        }
    } catch (Throwable& t) {
        cout << t.toString() << endl;
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// add info
//----------------------------------------------------------------------
void GameServerGroupInfoManager::addGameServerGroupInfo(GameServerGroupInfo* pGameServerGroupInfo, WorldID_t WorldID) {
    __BEGIN_TRY

    HashMapGameServerGroupInfo::iterator itr = m_GameServerGroupInfos[WorldID].find(pGameServerGroupInfo->getGroupID());

    if (itr != m_GameServerGroupInfos[WorldID].end())
        throw DuplicatedException("duplicated game-server nickname");

    m_GameServerGroupInfos[WorldID][pGameServerGroupInfo->getGroupID()] = pGameServerGroupInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// delete info
//----------------------------------------------------------------------
void GameServerGroupInfoManager::deleteGameServerGroupInfo(const ServerGroupID_t GroupID, WorldID_t WorldID) {
    __BEGIN_TRY

    HashMapGameServerGroupInfo::iterator itr = m_GameServerGroupInfos[WorldID].find(GroupID);

    if (itr != m_GameServerGroupInfos[WorldID].end()) {
        // Delete the GameServerGroupInfo.
        delete itr->second;

        // Erase the pair.
        m_GameServerGroupInfos[WorldID].erase(itr);

    } else {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// get GameServerGroupinfo by ServerGroupID
//----------------------------------------------------------------------
GameServerGroupInfo* GameServerGroupInfoManager::getGameServerGroupInfo(const ServerGroupID_t GroupID,
                                                                        WorldID_t WorldID) const {
    __BEGIN_TRY

    if (WorldID >= m_MaxWorldID) {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    GameServerGroupInfo* pGameServerGroupInfo = NULL;

    HashMapGameServerGroupInfo::const_iterator itr = m_GameServerGroupInfos[WorldID].find(GroupID);

    if (itr != m_GameServerGroupInfos[WorldID].end()) {
        pGameServerGroupInfo = itr->second;
    } else {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    return pGameServerGroupInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// get debug string
//----------------------------------------------------------------------
string GameServerGroupInfoManager::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GameServerGroupInfoManager(\n";

    for (int i = 1; i < m_MaxWorldID; i++) {
        if (m_GameServerGroupInfos[i].empty()) {
            msg << "EMPTY";

        } else {
            //--------------------------------------------------
            // *OPTIMIZATION*
            //
            // Could use for_each()
            //--------------------------------------------------
            for (HashMapGameServerGroupInfo::const_iterator itr = m_GameServerGroupInfos[i].begin();
                 itr != m_GameServerGroupInfos[i].end(); itr++)
                msg << itr->second->toString() << '\n';
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
