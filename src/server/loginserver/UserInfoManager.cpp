//----------------------------------------------------------------------
//
// Filename    : UserInfoManager.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "UserInfoManager.h"

#include "DatabaseError.h"
#include "repository/LoginConfigRepository.h"

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------
UserInfoManager::UserInfoManager() noexcept {}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
UserInfoManager::~UserInfoManager() noexcept {
    try {
        // hashmap ���� �� pair �� second, �� UserInfo ��ü���� �����ϰ�
        // pair ��ü�� �״�� �д�. (UserInfo�� ���� �����Ǿ� �ִٴ� �Ϳ�
        // �����϶�. �� �ʻ������ �ؾ� �Ѵ�. �ϱ�, ZGIM�� destruct �ȴٴ� ����
        // �α��� ������ �˴ٿ�ȴٴ� ���� �ǹ��ϴϱ�.. - -; )
        for (int i = 1; i < m_MaxWorldID; i++) {
            for (HashMapUserInfo::iterator itr = m_UserInfos[i].begin(); itr != m_UserInfos[i].end(); itr++) {
                delete itr->second;
                itr->second = NULL;
            }

            // ���� �ؽ��ʾȿ� �ִ� ��� pair ���� �����Ѵ�.
            m_UserInfos[i].clear();
        }

        delete[] m_UserInfos;
    } catch (...) {
        // destructor must not throw
    }
}


//----------------------------------------------------------------------
// initialize GSIM
//----------------------------------------------------------------------
void UserInfoManager::init() noexcept(false) {
    __BEGIN_TRY

    load();

    // just print to cout
    cout << toString() << endl;

    __END_CATCH
}

//----------------------------------------------------------------------
// load data from database
//----------------------------------------------------------------------
void UserInfoManager::load() noexcept(false) {
    __BEGIN_TRY

    LoginConfigRepository& repo = defaultLoginConfigRepository();

    // The table is sized from the largest WorldID; an empty table is a
    // startup error.
    int maxWorldID = 0;
    if (!repo.loadMaxGameServerGroupWorldID(maxWorldID)) {
        throw Error("GameServerGroupInfo TABLE does not exist!");
    }

    m_MaxWorldID = maxWorldID + 2;

    m_UserInfos = new HashMapUserInfo[m_MaxWorldID];

    vector<LoginGameServerGroupIDRow> rows;

    try {
        rows = repo.loadGameServerGroupIDs();
    } catch (const DatabaseError& error) {
        // A SQL failure arrives as END_DB's DatabaseError carrying the line
        // it wrote to DBError.log; rethrown as the Error the startup path
        // expects, with that line in it.
        throw Error("UserInfoManager::load : " + error.message());
    }

    for (size_t i = 0; i < rows.size(); i++) {
        UserInfo* pUserInfo = new UserInfo();
        WorldID_t WorldID = rows[i].worldID;
        pUserInfo->setWorldID(WorldID);
        pUserInfo->setServerGroupID(rows[i].groupID);
        pUserInfo->setUserNum(0);
        addUserInfo(pUserInfo);
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// add info
//----------------------------------------------------------------------
void UserInfoManager::addUserInfo(UserInfo* pUserInfo) noexcept(false) {
    __BEGIN_TRY

    HashMapUserInfo::iterator itr = m_UserInfos[pUserInfo->getWorldID()].find(pUserInfo->getServerGroupID());

    if (itr != m_UserInfos[pUserInfo->getWorldID()].end())
        throw DuplicatedException("duplicated zone id");

    m_UserInfos[pUserInfo->getWorldID()][pUserInfo->getServerGroupID()] = pUserInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// delete info
//----------------------------------------------------------------------
void UserInfoManager::deleteUserInfo(ZoneGroupID_t ServerGroupID, WorldID_t WorldID) noexcept(false) {
    __BEGIN_TRY

    HashMapUserInfo::iterator itr = m_UserInfos[WorldID].find(ServerGroupID);

    if (itr != m_UserInfos[WorldID].end()) {
        // UserInfo �� �����Ѵ�.
        delete itr->second;

        // pair�� �����Ѵ�.
        m_UserInfos[WorldID].erase(itr);

    } else { // not found

        StringStream msg;
        msg << "ServerGroupID: " << ServerGroupID;
        throw NoSuchElementException(msg.toString());
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// get info
//----------------------------------------------------------------------
UserInfo* UserInfoManager::getUserInfo(ZoneGroupID_t ServerGroupID, WorldID_t WorldID) const noexcept(false) {
    __BEGIN_TRY

    UserInfo* pUserInfo = NULL;

    HashMapUserInfo::const_iterator itr = m_UserInfos[WorldID].find(ServerGroupID);

    if (itr != m_UserInfos[WorldID].end()) {
        pUserInfo = itr->second;

    } else { // not found

        StringStream msg;
        msg << "ServerGroupID : " << ServerGroupID;
        throw NoSuchElementException(msg.toString());
    }

    return pUserInfo;

    __END_CATCH
}


//----------------------------------------------------------------------
// get debug string
//----------------------------------------------------------------------
string UserInfoManager::toString() const noexcept(false) {
    __BEGIN_TRY

    StringStream msg;

    msg << "UserInfoManager(";

    for (int i = 1; i < m_MaxWorldID; i++) {
        if (m_UserInfos[i].empty()) {
            msg << "EMPTY";

        } else {
            //--------------------------------------------------
            // *OPTIMIZATION*
            //
            // for_each()�� ����� ��
            //--------------------------------------------------
            for (HashMapUserInfo::const_iterator itr = m_UserInfos[i].begin(); itr != m_UserInfos[i].end(); itr++)
                msg << itr->second->toString();
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

// global variable definition
UserInfoManager* g_pUserInfoManager = NULL;
