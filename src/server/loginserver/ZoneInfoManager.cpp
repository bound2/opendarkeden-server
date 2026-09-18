//----------------------------------------------------------------------
//
// Filename    : ZoneInfoManager.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "ZoneInfoManager.h"

#include "DatabaseError.h"
#include "repository/LoginConfigRepository.h"

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------
ZoneInfoManager::ZoneInfoManager() {}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------
ZoneInfoManager::~ZoneInfoManager() {
    // Delete only the second of each pair in the hash map, i.e. the
    // ZoneInfo objects, and leave the pairs themselves. (Note that
    // they live on the heap, so they must be deleted explicitly. ZIM being
    // destructed means the login server is shutting down anyway.)
    for (HashMapZoneInfo::iterator itr = m_ZoneInfos.begin(); itr != m_ZoneInfos.end(); itr++) {
        delete itr->second;
        itr->second = NULL;
    }

    // Now erase every pair in the hash map.
    m_ZoneInfos.clear();
}


//----------------------------------------------------------------------
// initialize GSIM
//----------------------------------------------------------------------
void ZoneInfoManager::init() {
    __BEGIN_TRY

    // just load data from ZoneInfo table
    load();

    // just print to cout
    cout << toString() << endl;

    __END_CATCH
}

//----------------------------------------------------------------------
// load data from database
//----------------------------------------------------------------------
void ZoneInfoManager::load() {
    __BEGIN_TRY

    vector<LoginZoneRow> rows;

    try {
        rows = defaultLoginConfigRepository().loadZones();
    } catch (const DatabaseError& error) {
        // A SQL failure arrives as END_DB's DatabaseError carrying the line
        // it wrote to DBError.log; rethrown as the Error the startup path
        // expects, with that line in it.
        throw Error("ZoneInfoManager::load : " + error.message());
    }

    for (size_t i = 0; i < rows.size(); i++) {
        ZoneInfo* pZoneInfo = new ZoneInfo();
        pZoneInfo->setZoneID(rows[i].zoneID);
        pZoneInfo->setZoneGroupID(rows[i].zoneGroupID);
        addZoneInfo(pZoneInfo);
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// add info
//----------------------------------------------------------------------
void ZoneInfoManager::addZoneInfo(ZoneInfo* pZoneInfo) {
    __BEGIN_TRY

    HashMapZoneInfo::iterator itr = m_ZoneInfos.find(pZoneInfo->getZoneID());

    if (itr != m_ZoneInfos.end())
        throw DuplicatedException("duplicated zone id");

    m_ZoneInfos[pZoneInfo->getZoneID()] = pZoneInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// delete info
//----------------------------------------------------------------------
void ZoneInfoManager::deleteZoneInfo(ZoneID_t zoneID) {
    __BEGIN_TRY

    HashMapZoneInfo::iterator itr = m_ZoneInfos.find(zoneID);

    if (itr != m_ZoneInfos.end()) {
        // Delete the ZoneInfo.
        delete itr->second;

        // Erase the pair.
        m_ZoneInfos.erase(itr);

    } else { // not found

        StringStream msg;
        msg << "ZoneID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    __END_CATCH
}

//----------------------------------------------------------------------
// get info
//----------------------------------------------------------------------
ZoneInfo* ZoneInfoManager::getZoneInfo(ZoneID_t zoneID) {
    __BEGIN_TRY

    ZoneInfo* pZoneInfo = NULL;

    HashMapZoneInfo::const_iterator itr = m_ZoneInfos.find(zoneID);

    if (itr != m_ZoneInfos.end()) {
        pZoneInfo = itr->second;

    } else { // not found

        StringStream msg;
        msg << "ZoneID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    return pZoneInfo;

    __END_CATCH
}

//----------------------------------------------------------------------
// get debug string
//----------------------------------------------------------------------
string ZoneInfoManager::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "ZoneInfoManager(\n";

    if (m_ZoneInfos.empty()) {
        msg << "EMPTY";

    } else {
        //--------------------------------------------------
        // *OPTIMIZATION*
        //
        // Could use for_each()
        //--------------------------------------------------
        for (HashMapZoneInfo::const_iterator itr = m_ZoneInfos.begin(); itr != m_ZoneInfos.end(); itr++)
            msg << itr->second->toString() << '\n';
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
