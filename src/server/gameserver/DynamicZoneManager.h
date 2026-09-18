/////////////////////////////////////////////////////////////////////////////
// DynamicZoneManager.h
// Manages the zones that come and go on the server
/////////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_ZONE_MANAGER_H__
#define __DYNAMIC_ZONE_MANAGER_H__

#include <unordered_map>

#include "Mutex.h"
#include "Types.h"

class DynamicZoneGroup;

///////////////////////////////////////////////////////////
// class DynamicZoneManager
///////////////////////////////////////////////////////////
class DynamicZoneManager {
public:
    typedef unordered_map<int, DynamicZoneGroup*> HashMapDynamicZoneGroup;
    typedef HashMapDynamicZoneGroup::iterator HashMapDynamicZoneGroupItor;
    typedef HashMapDynamicZoneGroup::const_iterator HashMapDynamicZoneGroupConstItor;

public:
    DynamicZoneManager();
    ~DynamicZoneManager();

public:
    void init();
    void clear();

    // Add a new DynamicZoneGroup.
    void addDynamicZoneGroup(DynamicZoneGroup* pDynamicZoneGroup);

    // Find a DynamicZoneGroup and return it.
    DynamicZoneGroup* getDynamicZoneGroup(int dynamicZoneType);

    // get new dynamic zone id
    ZoneID_t getNewDynamicZoneID();

    static bool isDynamicZone(ZoneID_t zoneID);

private:
    Mutex m_Mutex;
    ZoneID_t m_DynamicZoneID;

    HashMapDynamicZoneGroup m_DynamicZoneGroups;
};

// global variable
extern DynamicZoneManager* g_pDynamicZoneManager;

#endif
