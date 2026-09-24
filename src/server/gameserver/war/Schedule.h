///////////////////////////////////////////////////////////////////
// Schedule class for scheduled work
///////////////////////////////////////////////////////////////////

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "Types.h"
#include "VSDateTime.h"

class Work;
class Scheduler;

class Schedule {
public:
    enum ScheduleType {
        SCHEDULE_TYPE_ONCE,     // 0
        SCHEDULE_TYPE_PERIODIC, // 1
    };

public:
    Schedule(Work* pWork, const VSDateTime& Time, ScheduleType type = SCHEDULE_TYPE_ONCE);
    virtual ~Schedule() noexcept;

public:
    // Runs the work once its time has come; true when it ran.
    bool heartbeat();

    // Whether the scheduled time has come, by the wall clock.
    bool isDue() const;
    // Executes the work, whatever the time. A subclass adds what running
    // means for it (a war schedule records the war's new state).
    virtual void run();

    const VSDateTime& getScheduledTime() const {
        return m_ScheduledTime;
    }
    ScheduleType getType() const {
        return m_ScheduleType;
    }

    Work* getWork() {
        return m_pWork;
    }
    const Work* getWork() const {
        return m_pWork;
    }
    Work* popWork();

    void setScheduledTime(const VSDateTime& dt) {
        m_ScheduledTime = dt;
    }

    friend class Scheduler;

public:
    virtual string toString() const;

protected:
    ScheduleType m_ScheduleType;
    Work* m_pWork;
    VSDateTime m_ScheduledTime;

    Scheduler* m_pScheduler;
};

#endif // __SCHEDULE_H__
