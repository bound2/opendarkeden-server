#ifndef __REGEN_ZONE_MANAGER_H__
#define __REGEN_ZONE_MANAGER_H__

#include <atomic>
#include <map>
#include <mutex>

#include "Exception.h"
#include "GCRegenZoneStatus.h"
#include "MonsterCorpse.h"
#include "Types.h"

class PlayerCreature;
class Zone;

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

    // Every zone thread asks who holds the tower; the tower's own thread
    // changes it, under RegenZoneManager's status mutex.
    RegenZoneIndex getOwner() const {
        return m_Owner.load();
    }
    void setOwner(RegenZoneIndex owner) {
        m_Owner.store(owner);
    }

    // Set by RegenZoneManager::load, before any zone thread runs, and never
    // changed afterwards.
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
    std::atomic<RegenZoneIndex> m_Owner;
    RegenZoneIndex m_OriginalOwner;
};

// The holy land's regen zone towers. The table of towers is filled by load()
// and never changes afterwards; each tower lies in a zone of its own, whose
// thread captures it (changeRegenZoneOwner) and sets it back when the race
// war ends (reloadOwner), so towers in different groups change at once.
//
// m_StatusMutex is a leaf over the status every player of the holy land is
// shown -- one GCRegenZoneStatus for all the towers -- and over the towers'
// owners: a tower's owner and its entry in the status change together under
// it, and a status is copied out under it to be sent. Nothing is taken while
// it is held. The owners are atomics besides, so canRegen and
// canTryRegenZone read them without it.
class RegenZoneManager {
    map<uint, RegenZoneInfo*> m_RegenZoneInfos;
    mutable std::mutex m_StatusMutex;
    GCRegenZoneStatus m_Status;
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

    // Put up and take down the marks the race war shows beside each tower,
    // from any thread: each is posted to the tower's zone thread.
    void putTryingPosition();
    void deleteTryingPosition();

    // On the tower's zone thread.
    void changeRegenZoneOwner(MonsterCorpse* pTower, Race_t race);

    bool canTryRegenZone(PlayerCreature* pPC, MonsterCorpse* pTower);
    bool canRegen(PlayerCreature* pPC, uint ID);

    void regeneratePC(PlayerCreature* pPC, uint ID);

    // Shows the towers' owners to every player of the holy land, from any
    // thread: posted to each holy land zone's group, where the command copies
    // the status as it stands when it runs, so the last status a group sends
    // carries every change made before it was posted, whichever tower's
    // thread posted first.
    void broadcastStatus() const;

    // A copy of the status, taken under m_StatusMutex.
    GCRegenZoneStatus getStatus() const;

    static RegenZoneManager* getInstance() {
        static RegenZoneManager theInstance;
        return &theInstance;
    }
};

#endif // __REGEN_ZONE_MANAGER_H__
