//----------------------------------------------------------------------
//
// Filename    : GameWorldInfoManager.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GameWorldInfoManager.h"

#include "DatabaseError.h"
#include "repository/ServerInfoRepository.h"

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------
GameWorldInfoManager::GameWorldInfoManager() {}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
GameWorldInfoManager::~GameWorldInfoManager() {
    // Delete only the second of each pair in the hashmap, that is the GameWorldInfo
    // object, and leave the pair itself. (note that GameWorldInfo is created on
    // the heap, so it has to be deleted explicitly. then again, GSIM being destructed
    // means the login server is shutting down.. )
    for (HashMapGameWorldInfo::iterator itr = m_GameWorldInfos.begin(); itr != m_GameWorldInfos.end(); itr++) {
        delete itr->second;
        itr->second = NULL;
    }

    // Now delete every pair in the hash map.
    m_GameWorldInfos.clear();
}


//----------------------------------------------------------------------
// initialize GSIM
//----------------------------------------------------------------------
void GameWorldInfoManager::init() {
    __BEGIN_TRY

    // just load data from GameWorldInfo table
    load();

    // just print to cout
    cout << toString() << endl;

    __END_CATCH
}

//----------------------------------------------------------------------
// load data from database
//----------------------------------------------------------------------
void GameWorldInfoManager::load() {
    __BEGIN_TRY

    // clear GameWorldInfos
    clear();

    vector<ServerInfoWorldRow> rows;

    try {
        rows = defaultServerInfoRepository().loadWorlds();
    } catch (const DatabaseError& error) {
        // A SQL failure arrives as END_DB's DatabaseError carrying the line
        // it wrote to DBError.log; rethrown as the Error the startup path
        // expects, with that line in it.
        throw Error("GameWorldInfoManager::load : " + error.message());
    }

    try {
        cout << "Loading GameWorldInfoManager...." << endl;

        for (size_t i = 0; i < rows.size(); i++) {
            GameWorldInfo* pGameWorldInfo = new GameWorldInfo();
            pGameWorldInfo->setID(rows[i].id);
            pGameWorldInfo->setName(rows[i].name);
            pGameWorldInfo->setStatus((WorldStatus)rows[i].stat);
            addGameWorldInfo(pGameWorldInfo);
        }

        cout << "End GameWorldInfoManager Load" << endl;
    } catch (Throwable& t) {
        cout << t.toString() << endl;
    }

    __END_CATCH
}
//----------------------------------------------------------------------
// clear info
//----------------------------------------------------------------------
void GameWorldInfoManager::clear() {
    __BEGIN_TRY

    HashMapGameWorldInfo::iterator itr = m_GameWorldInfos.begin();
    for (; itr != m_GameWorldInfos.end(); itr++) {
        GameWorldInfo* pGameWorldInfo = itr->second;
        SAFE_DELETE(pGameWorldInfo);
    }

    m_GameWorldInfos.clear();

    __END_CATCH
}

//----------------------------------------------------------------------
// add info
//----------------------------------------------------------------------
void GameWorldInfoManager::addGameWorldInfo(GameWorldInfo* pGameWorldInfo) {
    __BEGIN_TRY

    cout << pGameWorldInfo->toString() << endl;
    cout << "Size : " << m_GameWorldInfos.size() << endl;

    HashMapGameWorldInfo::iterator itr = m_GameWorldInfos.find(pGameWorldInfo->getID());

    if (itr != m_GameWorldInfos.end())
        throw DuplicatedException("duplicated game-server nickname");

    m_GameWorldInfos[pGameWorldInfo->getID()] = pGameWorldInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// delete info
//----------------------------------------------------------------------
void GameWorldInfoManager::deleteGameWorldInfo(const WorldID_t ID) {
    __BEGIN_TRY

    HashMapGameWorldInfo::iterator itr = m_GameWorldInfos.find(ID);

    if (itr != m_GameWorldInfos.end()) {
        // Delete the GameWorldInfo.
        delete itr->second;

        // Delete the pair.
        m_GameWorldInfos.erase(itr);

    } else {
        // When no such game server info object can be found
        throw NoSuchElementException();
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// get Worldinfo by WorldID
//----------------------------------------------------------------------
GameWorldInfo* GameWorldInfoManager::getGameWorldInfo(const WorldID_t ID) const {
    __BEGIN_TRY

    GameWorldInfo* pGameWorldInfo = NULL;

    HashMapGameWorldInfo::const_iterator itr = m_GameWorldInfos.find(ID);

    if (itr != m_GameWorldInfos.end()) {
        pGameWorldInfo = itr->second;
    } else {
        // When no such game server info object could be found
        throw NoSuchElementException();
    }

    return pGameWorldInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// get debug string
//----------------------------------------------------------------------
string GameWorldInfoManager::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GameWorldInfoManager(\n";

    if (m_GameWorldInfos.empty()) {
        msg << "EMPTY";

    } else {
        //--------------------------------------------------
        // *OPTIMIZATION*
        //
        // for_each() should be used
        //--------------------------------------------------
        for (HashMapGameWorldInfo::const_iterator itr = m_GameWorldInfos.begin(); itr != m_GameWorldInfos.end(); itr++)
            msg << itr->second->toString() << '\n';
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

// global variable definition
GameWorldInfoManager* g_pGameWorldInfoManager = NULL;
