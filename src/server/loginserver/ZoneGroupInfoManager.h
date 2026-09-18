//----------------------------------------------------------------------
//
// Filename    : ZoneGroupInfoManager.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __ZONE_GROUP_INFO_MANAGER_H__
#define __ZONE_GROUP_INFO_MANAGER_H__

// include files
#include <unordered_map>

#include "Exception.h"
#include "Types.h"
#include "ZoneGroupInfo.h"

typedef unordered_map<ZoneGroupID_t, ZoneGroupInfo*> HashMapZoneGroupInfo;

//----------------------------------------------------------------------
//
// class ZoneGroupInfoManager;
//
// Holds an unordered_map of that information keyed by the zone group id
// internally.
//
//----------------------------------------------------------------------

class ZoneGroupInfoManager {
public:
    // constructor
    ZoneGroupInfoManager() noexcept;

    // destructor
    ~ZoneGroupInfoManager() noexcept;

    // initialize manager
    void init() noexcept(false);

    // load from database
    void load() noexcept(false);

    // add info
    void addZoneGroupInfo(ZoneGroupInfo* pZoneGroupInfo) noexcept(false);

    // delete info
    void deleteZoneGroupInfo(ZoneGroupID_t zoneGroupID) noexcept(false);

    // get info
    ZoneGroupInfo* getZoneGroupInfo(ZoneGroupID_t zoneGroupID) const noexcept(false);

    // get count of info
    uint getSize() const noexcept {
        return m_ZoneGroupInfos.size();
    }

    // get debug string
    string toString() const;

private:
    // hash map of ZoneGroupInfo
    // key   : ZoneGroupID_t
    // value : ZoneGroupInfo *
    HashMapZoneGroupInfo m_ZoneGroupInfos;
};


#endif
