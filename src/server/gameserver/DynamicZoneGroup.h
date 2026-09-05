/////////////////////////////////////////////////////////////////////////////
// DynamicZoneGroup.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_ZONE_GROUP_H__
#define __DYNAMIC_ZONE_GROUP_H__

#include <mutex>

#include <unordered_map>

#include "Types.h"

// forward declaration
class DynamicZone;

///////////////////////////////////////////////////////////
// class DynamicZoneGroup
///////////////////////////////////////////////////////////
class DynamicZoneGroup {
public:
    typedef unordered_map<ZoneID_t, DynamicZone*> HashMapDynamicZone;
    typedef HashMapDynamicZone::iterator HashMapDynamicZoneItor;
    typedef HashMapDynamicZone::const_iterator HashMapDynamicZoneConstItor;

public:
    DynamicZoneGroup();
    ~DynamicZoneGroup();

public:
    void clear();

public:
    int getDynamicZoneType() const {
        return m_DynamicZoneType;
    }
    void setDynamicZoneType(int dynamicZoneType) {
        m_DynamicZoneType = dynamicZoneType;
    }

    ZoneID_t getTemplateZoneID() const {
        return m_TemplateZoneID;
    }
    void setTemplateZoneID(ZoneID_t templateZoneID) {
        m_TemplateZoneID = templateZoneID;
    }

    // Both are called from whichever zone thread the requesting player is
    // on -- two players on two groups can ask at the same moment -- and
    // getAvailableDynamicZone() may create an instance (Zone load, a
    // ZoneInfo publish, an insert into the template group's zone map).
    // m_Mutex serialises them; the maps the creation publishes are
    // copy-on-write for their readers (Snapshot.h), so holding it while
    // the zone loads blocks only the next player wanting this type.
    bool canEnter();
    DynamicZone* getAvailableDynamicZone();

protected:
    void addDynamicZone(DynamicZone* pDynamicZone);
    uint getSize() {
        return m_DynamicZones.size();
    }

private:
    int m_DynamicZoneType;
    ZoneID_t m_TemplateZoneID;         // 틀이 되는 존의 ID
    HashMapDynamicZone m_DynamicZones; // guarded by m_Mutex after init
    uint m_MaxSize;
    mutable std::mutex m_Mutex;
};

#endif
