#ifndef __REGEN_ZONE_MANAGER_H__
#define __REGEN_ZONE_MANAGER_H__

#include <map>

#include "Exception.h"
#include "MonsterCorpse.h"
#include "Mutex.h"
#include "Types.h"

class PlayerCreature;
class Zone;
class GCRegenZoneStatus;

class RegenZoneInfo {
public:
    enum RegenZoneIndex { REGEN_ZONE_SLAYER, REGEN_ZONE_VAMPIRE, REGEN_ZONE_OUSTERS, REGEN_ZONE_DEFAULT };

    RegenZoneInfo(uint ID, MonsterCorpse* pTower, uint Owner) : m_ID(ID), m_pRegenZoneTower(pTower) {
        Assert(Owner < 4);
        m_Owner = (RegenZoneIndex)Owner;
    }
    uint getID() const {
        return m_ID;
    }
    MonsterCorpse* getTower() const {
        return m_pRegenZoneTower;
    }

    RegenZoneIndex getOwner() const {
        return m_Owner;
    }
    void setOwner(RegenZoneIndex owner) {
        m_Owner = owner;
    }

    RegenZoneIndex getOriginalOwner() const {
        return m_OriginalOwner;
    }
    void setOriginalOwner(RegenZoneIndex owner) {
        m_OriginalOwner = owner;
    }

    void putTryingPosition();
    void deleteTryingPosition();

private:
    uint m_ID;
    MonsterCorpse* m_pRegenZoneTower;
    RegenZoneIndex m_Owner;
    RegenZoneIndex m_OriginalOwner;
};

class RegenZoneManager {
    map<uint, RegenZoneInfo*> m_RegenZoneInfos;
    Mutex m_Mutex;
    GCRegenZoneStatus* m_pStatusPacket;
    RegenZoneManager();

public:
    ~RegenZoneManager();

    void load();
    // Called when the race war ends, from any thread: each tower's owner is
    // set on its zone's thread (reloadOwner), posted there.
    void reload();
    void reloadOwner(Zone& zone, uint ID, ZoneCoord_t ZoneX, ZoneCoord_t ZoneY, uint Owner);

    // The regen zone ID, or NULL. The table is filled by load() and never
    // changes afterwards.
    RegenZoneInfo* getRegenZoneInfo(uint ID) const;

    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

    // Put up and take down the marks the race war shows beside each tower,
    // from any thread: each is posted to the tower's zone thread.
    void putTryingPosition();
    void deleteTryingPosition();

    void changeRegenZoneOwner(MonsterCorpse* pTower, Race_t race);

    bool canTryRegenZone(PlayerCreature* pPC, MonsterCorpse* pTower);
    bool canRegen(PlayerCreature* pPC, uint ID);

    void regeneratePC(PlayerCreature* pPC, uint ID);

    void broadcastStatus();
    GCRegenZoneStatus* getStatusPacket() const {
        return m_pStatusPacket;
    }

    static RegenZoneManager* getInstance() {
        static RegenZoneManager theInstance;
        return &theInstance;
    }
};

#endif // __REGEN_ZONE_MANAGER_H__
