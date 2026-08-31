//////////////////////////////////////////////////////////////////////
//
// Filename    : ZoneGroup.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZONE_GROUP_H__
#define __ZONE_GROUP_H__

// include files
#include <unordered_map>

#include "Exception.h"
#include "GMServerInfo.h"
#include "GameTime.h"
#include "Thread.h"
#include "Types.h"
#include "Zone.h"

// forward declaration
class ZoneThread;
class ZonePlayerManager;

// type redefinition
// typedef unordered_map<ZoneID_t,Zone*> ZONE_HASHMAP;

//////////////////////////////////////////////////////////////////////
//
// class ZoneGroup;
//
//////////////////////////////////////////////////////////////////////
class ZoneGroup {
    // friend declaration
    friend class ZoneThread;

public:
    // constructor
    ZoneGroup(ZoneGroupID_t zoneGroupID);

    // destructor
    ~ZoneGroup();


public:
    // initialize zone group
    void init();

    // load sub zones from database
    void load();

    // save sub zones to database
    void save();

    //
    void processPlayers();
    void heartbeat();

public:
    // add zone to zone group
    void addZone(Zone* pZone);

    // delete zone from zone group
    void deleteZone(ZoneID_t zoneID);
    Zone* removeZone(ZoneID_t zoneID);

    // get zone from zone group
    Zone* getZone(ZoneID_t zoneID) const;

    // #ifdef __NO_COMBAT__
    Zone* getCombatZone(ZoneID_t zoneID) const; // getZone과 같은 일을 수행하나 NULL을 리턴하는 것이 가능, 김경석
    // #endif

    //--------------------------------------------------
    // get/set methods
    //--------------------------------------------------
public:
    // get/set zone group id
    ZoneGroupID_t getZoneGroupID() const {
        return m_ZoneGroupID;
    }
    void setZoneGroupID(ZoneGroupID_t zoneGroupID) {
        m_ZoneGroupID = zoneGroupID;
    }

    // get zone player manager
    ZonePlayerManager* getZonePlayerManager() {
        return m_pZonePlayerManager;
    }
    void setZonePlayerManager(ZonePlayerManager* pZonePlayerManager) {
        m_pZonePlayerManager = pZonePlayerManager;
    }

    // get/set game time
    GameTime getGameTime() const {
        return m_GameTime;
    }
    void setGameTime(const GameTime& gameTime) {
        m_GameTime = gameTime;
    }

    void makeZoneUserInfo(GMServerInfo& gmServerInfo);

    const unordered_map<ZoneID_t, Zone*>& getZones() const {
        return m_Zones;
    }

    // get debug string
    string toString() const;

public:
    // Thread-ownership contract (see "Thread ownership" in CLAUDE.md):
    // zone-group state may only be touched while this group's mutex is
    // held — the group's ZoneGroupThread holds it for its whole tick, and
    // any other thread (e.g. GDRLairManager) must take it explicitly.
    // lock()/unlock() record the holding thread so assertOwned() can
    // verify the contract in debug builds. The check is armed by the
    // ZoneGroupThread when it starts; before that, single-threaded
    // startup/loading passes vacuously. The mutex is non-recursive, so
    // clearing the holder on unlock is sound.
    void lock() {
        m_Mutex.lock();
        m_LockHolder = Thread::self();
    }
    void unlock() {
        m_LockHolder = TID();
        m_Mutex.unlock();
    }

    void armOwnershipAssert() {
        m_OwnershipArmed = true;
    }
    void assertOwned() const;

    void initLoadValue();
    DWORD getLoadValue() const;


    //--------------------------------------------------
    // data members
    //--------------------------------------------------
private:
    // zone group id
    ZoneGroupID_t m_ZoneGroupID;

    // zone 의 해쉬맵
    unordered_map<ZoneID_t, Zone*> m_Zones;

    // zone player manager
    ZonePlayerManager* m_pZonePlayerManager;

    // game time
    GameTime m_GameTime;

    // Tick Time
    Timeval m_TickTime;

    DWORD m_LoadValue;

    mutable Mutex m_Mutex;

    // debug-only ownership tracking (see lock()/unlock()/assertOwned())
    volatile TID m_LockHolder;
    volatile bool m_OwnershipArmed;
};

#endif
