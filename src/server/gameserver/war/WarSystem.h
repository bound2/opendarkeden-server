#ifndef __WAR_SYSTEM_H__
#define __WAR_SYSTEM_H__

#include <atomic>

#include "Exception.h"
#include "GCWarList.h"
#include "Mutex.h"
#include "Scheduler.h"
#include "Types.h"

class War;
class WarSchedule;
class Player;
class GamePlayer;
class PlayerCreature;
class WarScheduleInfo;

class ActiveWarInfo {
public:
    ActiveWarInfo(ZoneID_t zoneID, GuildID_t attackGuildID = 0) : ZoneID(zoneID), AttackGuildID(attackGuildID) {}

    bool operator==(const ActiveWarInfo& awi) const {
        return ZoneID == awi.ZoneID;
    }

    bool operator!=(const ActiveWarInfo& awi) const {
        return ZoneID != awi.ZoneID;
    }

public:
    ZoneID_t ZoneID;
    GuildID_t AttackGuildID;
};

class WarSystem : public Scheduler {
public:
    WarSystem();
    ~WarSystem();

    void init();
    void load();

    bool addWarDelayed(War* pWar);
    bool endWar(PlayerCreature* pPC, ZoneID_t castleZoneID);
    bool removeWar(ZoneID_t castleZoneID);
    bool removeRaceWar();

    bool makeGCWarList();
    bool makeGCWarList_LOCKED();
    void sendGCWarList(Player* pPlayer);
    bool addRaceWarScheduleInfo(WarScheduleInfo* pWSI);

    virtual Work* heartbeat();

    // public :
    //	void	lock() 	{ m_Mutex.lock(); }
    //	void	unlock() 	{ m_Mutex.unlock(); }

public:
    static WarID_t getWarIDSuccessor() {
        return s_WarIDSuccessor;
    }
    static void setWarIDSuccessor(WarID_t wid) {
        s_WarIDSuccessor = wid;
    }

protected:
    VSDateTime getWarEndTime(WarType_t warType) const;
    bool addQueuedWar();
    bool addWar(War* pWar);
    War* getActiveRaceWarAtSameThread() const;
    bool checkStartRaceWar();

public:
    bool hasCastleActiveWar(ZoneID_t zoneID) const;
    bool getAttackGuildID(ZoneID_t zoneID, GuildID_t& guildID) const;
    War* getActiveWar(ZoneID_t zoneID) const;
    WarSchedule* getActiveWarSchedule_LOCKED(ZoneID_t zoneID);
    WarSchedule* getActiveWarSchedule(ZoneID_t zoneID);
    bool isModifyCastleOwner(ZoneID_t castleZoneID, PlayerCreature* pPC);

    // Whether the war fought over the shrines of Adam's holy land -- the race
    // war -- lets pPC take a shrine set for its race. With no race war running
    // there is no war to take a shrine in, so the answer is no: unlike
    // isModifyCastleOwner, which a siege reaches only while that siege runs,
    // this is asked every time a blood bible is placed, so a missing war is an
    // answer rather than an error.
    bool mayModifyShrineOwner(PlayerCreature* pPC) const;

    // Whether a race war is running. The zone threads ask this on every path
    // the holy land touches, without a lock, so it is a hint published by the
    // main thread's heartbeat: the race war object itself lives in the
    // schedules and is read only under m_Mutex, which the heartbeat holds
    // when it ends the war and frees it.
    bool hasActiveRaceWar() const {
        return m_bHasRaceWar;
    }

    bool isWarActive() const {
        return !isEmpty();
    }
    bool isRaceWarToday() const {
        return m_bRaceWarToday;
    }
    DWORD getRaceWarTimeParam() const {
        return m_RaceWarTimeParam;
    }

public:
    void broadcastWarList(GamePlayer* pGamePlayer) const;
    bool startRaceWar();
    void prepareRaceWar();

    bool canApplyBloodBibleSign() const {
        return !m_b20Minutes;
    }
    bool isSkyBlack() const {
        return m_b5Minutes;
    }

private:
    static WarID_t s_WarIDSuccessor;

    mutable Mutex m_Mutex;
    mutable Mutex m_MutexWarQueue;
    mutable Mutex m_MutexActiveWars;
    mutable Mutex m_MutexWarList;

    GCWarList m_GCWarList;
    list<War*> m_WarQueue;

    // The race war's state as the zone threads see it. These are written under
    // m_Mutex, by the main thread's heartbeat and by a GM starting the race
    // war early, and read through the lock-free getters above, so each is an
    // atomic: a reader gets a value a writer stored, and nothing more -- two
    // of them read one after the other may straddle a heartbeat.
    std::atomic<bool> m_bHasRaceWar;
    std::atomic<bool> m_bRaceWarToday;
    std::atomic<DWORD> m_RaceWarTimeParam;

    list<ActiveWarInfo> m_ActiveWars;

    Schedule* m_pRaceWarSchedule;

    std::atomic<bool> m_b20Minutes;
    std::atomic<bool> m_b5Minutes;
};

#endif
