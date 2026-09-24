//////////////////////////////////////////////////////////////////////////////
// Filename    : HolyLandManager.h
// Written By  : Bezz
// Description : Holds the Holy Land ( Zone * ).
//////////////////////////////////////////////////////////////////////////////

#ifndef __HOLY_LAND_MANAGER_H__
#define __HOLY_LAND_MANAGER_H__

#include <vector>

#include <unordered_map>

#include "Exception.h"
#include "Mutex.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class HolyLandManager;
//////////////////////////////////////////////////////////////////////////////

class Zone;
class Packet;

typedef unordered_map<ZoneID_t, Zone*> HashMapZone;
typedef unordered_map<ZoneID_t, Zone*>::iterator HashMapZoneItor;
typedef unordered_map<ZoneID_t, Zone*>::const_iterator HashMapZoneConstItor;

class HolyLandManager {
public:
    HolyLandManager();
    ~HolyLandManager();

    void clear() {
        lock();
        m_HolyLands.clear();
        unlock();
    }

public:
    void addHolyLand(Zone* pZone);

    const HashMapZone& getHolyLands() const {
        return m_HolyLands;
    }

    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

    void broadcast(Packet* pPacket) const;

    //	void sendBloodBibleStatus() const ;

    // The ids of the holy land's zones, as they stand when asked.
    vector<ZoneID_t> getHolyLandZoneIDs() const;

    // What the race war does across the holy land when it starts and ends:
    // stop or let run its time, kill its monsters, send away the players the
    // race war is not open to. Callable from any thread; each posts one
    // command to every zone group holding a holy-land zone, which does the
    // work on its own zones at the top of its next tick.
    void fixTimeband(uint timeband);
    void resumeTimeband();

    void killAllMonsters();

    void remainRaceWarPlayers();

    void refreshHolyLandPlayers();

private:
    HashMapZone m_HolyLands;

    mutable Mutex m_Mutex;
};

#endif
