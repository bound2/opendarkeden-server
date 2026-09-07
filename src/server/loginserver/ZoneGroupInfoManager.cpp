//----------------------------------------------------------------------
//
// Filename    : ZoneGroupInfoManager.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "ZoneGroupInfoManager.h"

#include "repository/LoginConfigRepository.h"

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------
ZoneGroupInfoManager::ZoneGroupInfoManager() noexcept {}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
ZoneGroupInfoManager::~ZoneGroupInfoManager() noexcept {
    // hashmap ���� �� pair �� second, �� ZoneGroupInfo ��ü���� �����ϰ�
    // pair ��ü�� �״�� �д�. (ZoneGroupInfo�� ���� �����Ǿ� �ִٴ� �Ϳ�
    // �����϶�. �� �ʻ������ �ؾ� �Ѵ�. �ϱ�, ZGIM�� destruct �ȴٴ� ����
    // �α��� ������ �˴ٿ�ȴٴ� ���� �ǹ��ϴϱ�.. - -; )
    for (HashMapZoneGroupInfo::iterator itr = m_ZoneGroupInfos.begin(); itr != m_ZoneGroupInfos.end(); itr++) {
        delete itr->second;
        itr->second = NULL;
    }

    // ���� �ؽ��ʾȿ� �ִ� ��� pair ���� �����Ѵ�.
    m_ZoneGroupInfos.clear();
}


//----------------------------------------------------------------------
// initialize GSIM
//----------------------------------------------------------------------
void ZoneGroupInfoManager::init() noexcept(false) {
    __BEGIN_TRY

    // just load data from ZoneGroupInfo table
    load();

    // just print to cout
    cout << toString() << endl;

    __END_CATCH
}

//----------------------------------------------------------------------
// load data from database
//----------------------------------------------------------------------
void ZoneGroupInfoManager::load() noexcept(false) {
    __BEGIN_TRY

    vector<LoginZoneGroupRow> rows;

    try {
        rows = defaultLoginConfigRepository().loadZoneGroups();
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); rethrown as the Error the
        // startup path expects.
        throw Error("ZoneGroupInfoManager::load : SQL error, see DBError.log");
    }

    for (size_t i = 0; i < rows.size(); i++) {
        ZoneGroupInfo* pZoneGroupInfo = new ZoneGroupInfo();
        pZoneGroupInfo->setZoneGroupID(rows[i].zoneGroupID);
        pZoneGroupInfo->setServerID(rows[i].serverID);
        addZoneGroupInfo(pZoneGroupInfo);
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// add info
//----------------------------------------------------------------------
void ZoneGroupInfoManager::addZoneGroupInfo(ZoneGroupInfo* pZoneGroupInfo) noexcept(false) {
    __BEGIN_TRY

    HashMapZoneGroupInfo::iterator itr = m_ZoneGroupInfos.find(pZoneGroupInfo->getZoneGroupID());

    if (itr != m_ZoneGroupInfos.end())
        throw DuplicatedException("duplicated zone id");

    m_ZoneGroupInfos[pZoneGroupInfo->getZoneGroupID()] = pZoneGroupInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// delete info
//----------------------------------------------------------------------
void ZoneGroupInfoManager::deleteZoneGroupInfo(ZoneGroupID_t zoneGroupID) noexcept(false) {
    __BEGIN_TRY

    HashMapZoneGroupInfo::iterator itr = m_ZoneGroupInfos.find(zoneGroupID);

    if (itr != m_ZoneGroupInfos.end()) {
        // ZoneGroupInfo �� �����Ѵ�.
        delete itr->second;

        // pair�� �����Ѵ�.
        m_ZoneGroupInfos.erase(itr);

    } else { // not found

        StringStream msg;
        msg << "ZoneGroupID : " << zoneGroupID;
        throw NoSuchElementException(msg.toString());
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// get info
//----------------------------------------------------------------------
ZoneGroupInfo* ZoneGroupInfoManager::getZoneGroupInfo(ZoneGroupID_t zoneGroupID) const {
    __BEGIN_TRY

    ZoneGroupInfo* pZoneGroupInfo = NULL;

    HashMapZoneGroupInfo::const_iterator itr = m_ZoneGroupInfos.find(zoneGroupID);

    if (itr != m_ZoneGroupInfos.end()) {
        pZoneGroupInfo = itr->second;

    } else { // not found

        StringStream msg;
        msg << "ZoneGroupID : " << zoneGroupID;
        throw NoSuchElementException(msg.toString());
    }

    return pZoneGroupInfo;

    __END_CATCH
}


//----------------------------------------------------------------------
// get debug string
//----------------------------------------------------------------------
string ZoneGroupInfoManager::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "ZoneGroupInfoManager(";

    if (m_ZoneGroupInfos.empty()) {
        msg << "EMPTY";

    } else {
        //--------------------------------------------------
        // *OPTIMIZATION*
        //
        // for_each()�� ����� ��
        //--------------------------------------------------
        for (HashMapZoneGroupInfo::const_iterator itr = m_ZoneGroupInfos.begin(); itr != m_ZoneGroupInfos.end(); itr++)
            msg << itr->second->toString();
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

// global variable definition
ZoneGroupInfoManager* g_pZoneGroupInfoManager = NULL;
