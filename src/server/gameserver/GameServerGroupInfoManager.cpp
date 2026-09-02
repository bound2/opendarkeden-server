//----------------------------------------------------------------------
//
// Filename    : GameServerGroupInfoManager.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GameServerGroupInfoManager.h"

#include "repository/GameInfoRepository.h"


//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------
GameServerGroupInfoManager::GameServerGroupInfoManager()

{
    m_MaxWorldID = 0;
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
GameServerGroupInfoManager::~GameServerGroupInfoManager()

{
    clear();
}

//----------------------------------------------------------------------
// clear GameServerGroupInfos
//----------------------------------------------------------------------
void GameServerGroupInfoManager::clear()

{
    __BEGIN_TRY

    // hashmap 안의 각 pair 의 second, 즉 GameServerGroupInfo 객체만을 삭제하고
    // pair 자체는 그대로 둔다. (GameServerGroupInfo가 힙에 생성되어 있다는 것에
    // 유의하라. 즉 필살삭제를 해야 한다. 하긴, GSIM이 destruct 된다는 것은
    // 로그인 서버가 셧다운된다는 것을 의미하니깐.. - -; )
    for (int i = 1; i < m_MaxWorldID; i++) {
        for (HashMapGameServerGroupInfo::iterator itr = m_GameServerGroupInfos[i].begin();
             itr != m_GameServerGroupInfos[i].end(); itr++) {
            SAFE_DELETE(itr->second);
        }

        // 이제 해쉬맵안에 있는 모든 pair 들을 삭제한다.
        m_GameServerGroupInfos[i].clear();
    }

    SAFE_DELETE_ARRAY(m_GameServerGroupInfos);

    __END_CATCH
}


//----------------------------------------------------------------------
// initialize GSIM
//----------------------------------------------------------------------
void GameServerGroupInfoManager::init()

{
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
void GameServerGroupInfoManager::load()

{
    __BEGIN_TRY

    clear();

    int maxWorldID = 0;
    if (!defaultGameInfoRepository().loadMaxWorldID(maxWorldID)) {
        throw Error("GameServerGroupInfo TABLE does not exist!");
    }

    m_MaxWorldID = maxWorldID + 2;

    m_GameServerGroupInfos = new HashMapGameServerGroupInfo[m_MaxWorldID];

    // The rows query used to sit in a hand-written try that turned a
    // SQLQueryException into an Error and swallowed any other Throwable
    // with a cout. The seam now converts a SQL failure the way every
    // repository does (DBError.log + a thrown const char*, see DB.h's
    // END_DB), so only the Throwable arm is left to keep: a failure in
    // addGameServerGroupInfo is still printed and swallowed.
    try {
        vector<GameServerGroupRow> rows = defaultGameInfoRepository().loadGameServerGroups();

        for (size_t r = 0; r < rows.size(); r++) {
            GameServerGroupInfo* pGameServerGroupInfo = new GameServerGroupInfo();
            WorldID_t WorldID = rows[r].worldID;
            pGameServerGroupInfo->setWorldID(WorldID);
            pGameServerGroupInfo->setGroupID(rows[r].groupID);
            pGameServerGroupInfo->setGroupName(rows[r].groupName);
            pGameServerGroupInfo->setStat(rows[r].stat);
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

    int GroupID = pGameServerGroupInfo->getGroupID();
    HashMapGameServerGroupInfo::iterator itr = m_GameServerGroupInfos[WorldID].find(GroupID);

    if (itr != m_GameServerGroupInfos[WorldID].end())
        throw DuplicatedException("duplicated game-server nickname");

    cout << "addGameServerGroupInfo: " << (int)WorldID << ", " << GroupID << " : "
         << pGameServerGroupInfo->getGroupName().c_str() << endl;

    m_GameServerGroupInfos[WorldID][GroupID] = pGameServerGroupInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// delete info
//----------------------------------------------------------------------
void GameServerGroupInfoManager::deleteGameServerGroupInfo(const ServerGroupID_t GroupID, WorldID_t WorldID) {
    __BEGIN_TRY

    HashMapGameServerGroupInfo::iterator itr = m_GameServerGroupInfos[WorldID].find(GroupID);

    if (itr != m_GameServerGroupInfos[WorldID].end()) {
        // GameServerGroupInfo 를 삭제한다.
        delete itr->second;

        // pair를 삭제한다.
        m_GameServerGroupInfos[WorldID].erase(itr);

    } else {
        // 그런 게임서버인포 객체를 찾을 수 없을 때
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
        // 그런 게임서버인포 객체를 찾을 수 없었을 때
        throw NoSuchElementException();
    }

    GameServerGroupInfo* pGameServerGroupInfo = NULL;

    HashMapGameServerGroupInfo::const_iterator itr = m_GameServerGroupInfos[WorldID].find(GroupID);

    if (itr != m_GameServerGroupInfos[WorldID].end()) {
        pGameServerGroupInfo = itr->second;
    } else {
        // 그런 게임서버인포 객체를 찾을 수 없었을 때
        throw NoSuchElementException();
    }

    return pGameServerGroupInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// get debug string
//----------------------------------------------------------------------
string GameServerGroupInfoManager::toString() const

{
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
            // for_each()를 사용할 것
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

// global variable definition
GameServerGroupInfoManager* g_pGameServerGroupInfoManager = NULL;
