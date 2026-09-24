///////////////////////////////////////////////////////////////////
// WarScheduler.h
//
// It runs attached to a Zone.
///////////////////////////////////////////////////////////////////

#ifndef __WAR_SCHEDULER_H__
#define __WAR_SCHEDULER_H__

#include "Mutex.h"
#include "Scheduler.h"
#include "Types.h"
#include "War.h"
#include "Work.h"

const int MaxWarSchedule = 10;

class Zone;
class GCWarScheduleList;

class WarScheduler : public Scheduler {
public:
    WarScheduler(Zone* pZone);
    virtual ~WarScheduler();

public:
    void load();
    void tinysave(WarID_t warID, const string& query);

    bool addWar(War* pWar);
    bool canAddWar(WarType_t warType);

    Zone* getZone() const {
        return m_pZone;
    }

    // Every waiting and running guild war of this castle, cancelled in the
    // table and reloaded: what a change of the castle's owning race does.
    void cancelGuildSchedules();
    // The waiting guild wars of this castle that one guild takes part in,
    // cancelled in the table and reloaded, so a reload cannot bring back a
    // war a deleted guild is in. Runs on the owning zone group's thread and
    // takes this scheduler's mutex and no other.
    void cancelGuildSchedulesOf(GuildID_t gID);
    bool hasSchedule(GuildID_t gID);

    int getWarTypeCount(WarType_t warType);

    virtual Work* heartbeat();

protected:
    VSDateTime getLastWarDateTime(WarType_t warType) const;
    VSDateTime getNextWarDateTime(WarType_t warType) const;

public:
    static VSDateTime getNextWarDateTime(WarType_t warType, const VSDateTime& dt);
    bool makeGCWarScheduleList(GCWarScheduleList* pGCWarScheduleList) const;


public:
    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

private:
    Zone* m_pZone;

    mutable Mutex m_Mutex;
};

#endif // __WAR_SCHEDULER_H__
