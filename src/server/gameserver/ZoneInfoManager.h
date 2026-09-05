//////////////////////////////////////////////////////////////////////////////
// Filename    : ZoneInfoManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __ZONE_INFO_MANAGER_H__
#define __ZONE_INFO_MANAGER_H__

#include <memory>

#include <unordered_map>

#include "Exception.h"
#include "Snapshot.h"
#include "Types.h"
#include "Zone.h"
#include "ZoneInfo.h"

//////////////////////////////////////////////////////////////////////////////
// class ZoneInfoManager;
// ZoneID 를 키값으로 해서 존 정보를 검색할 수 있는 기능을 제공한다.
//////////////////////////////////////////////////////////////////////////////

class ZoneInfoManager {
public:
    ZoneInfoManager();
    ~ZoneInfoManager();

public:
    void init();
    void load();

    void addZoneInfo(ZoneInfo* pZoneInfo);
    void deleteZoneInfo(ZoneID_t zoneID);
    ZoneInfo* getZoneInfo(ZoneID_t zoneID);
    ZoneInfo* getZoneInfoByName(const string& ZoneName);
    int size() const {
        return m_Tables.load()->byID.size();
    }

    vector<Zone*> getNormalFields() const;

    string toString() const;

private:
    // The three lookups, published copy-on-write (Snapshot.h): getZoneInfo()
    // runs on every thread for every transport, while addZoneInfo() runs at
    // load and, at run time, on whichever zone thread creates a dynamic
    // zone. A reader loads a snapshot without waiting on a writer; a writer replaces it.
    struct Tables {
        unordered_map<ZoneID_t, ZoneInfo*> byID; // zone info 의 해쉬맵
        unordered_map<string, ZoneInfo*> byFullName;
        unordered_map<string, ZoneInfo*> byShortName;
    };
    de::Snapshot<Tables> m_Tables;
};

extern ZoneInfoManager* g_pZoneInfoManager;

#endif
