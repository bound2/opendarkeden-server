// scheduler_test.cpp — the war-system Schedule/Scheduler pair and the
// VSDateTime clock they run on.
//
// Ported from the deleted cppunit suites src/server/gameserver/testAlone/
// (ScheduleTest, WarSystemTest) and src/server/gameserver/test/
// GameServerWarTest.cpp. Those tests drove a private VSDateTime fork with an
// injectable clock; the live Schedule::heartbeat() reads the wall clock
// directly, so these use scheduled times safely in the past or future
// relative to "now" instead of stepping a fake clock.
//
// Links only de-kernel plus the two war/ sources under test.

#include <gtest/gtest.h>

#include "VSDateTime.h"
#include "war/Schedule.h"
#include "war/Scheduler.h"
#include "war/Work.h"

namespace {

class FlagWork : public Work {
public:
    void execute() override {
        executed = true;
    }
    string toString() const override {
        return "FlagWork";
    }
    bool executed = false;
};

VSDateTime secondsFromNow(int secs) {
    return VSDateTime::currentDateTime().addSecs(secs);
}

//////////////////////////////////////////////////////////////////////////
// VSDateTime
//////////////////////////////////////////////////////////////////////////

TEST(VSDateTime, ConstructsFromDateAndTimeParts) {
    VSDateTime dt(VSDate(2003, 2, 2), VSTime(10, 33));
    EXPECT_EQ(2003, dt.date().year());
    EXPECT_EQ(2, dt.date().month());
    EXPECT_EQ(2, dt.date().day());
    EXPECT_EQ(10, dt.time().hour());
    EXPECT_EQ(33, dt.time().minute());
}

TEST(VSDateTime, AddSecsWithinTheSameDay) {
    VSDateTime dt(VSDate(2003, 2, 2), VSTime(10, 33));
    VSDateTime after = dt.addSecs(60 * 60);
    EXPECT_EQ(2003, after.date().year());
    EXPECT_EQ(2, after.date().month());
    EXPECT_EQ(2, after.date().day());
    EXPECT_EQ(11, after.time().hour());
    EXPECT_EQ(33, after.time().minute());
    EXPECT_EQ(3600, dt.secsTo(after));
}

TEST(VSDateTime, AddSecsRollsOverMidnight) {
    VSDateTime dt(VSDate(2003, 2, 2), VSTime(23, 30));
    VSDateTime after = dt.addSecs(60 * 60);
    EXPECT_EQ(3, after.date().day());
    EXPECT_EQ(0, after.time().hour());
    EXPECT_EQ(30, after.time().minute());
    EXPECT_TRUE(after > dt);
    EXPECT_TRUE(dt < after);
    EXPECT_TRUE(dt.addSecs(0) == dt);
}

//////////////////////////////////////////////////////////////////////////
// Schedule
//////////////////////////////////////////////////////////////////////////

TEST(Schedule, DoesNotFireBeforeItsTime) {
    FlagWork* work = new FlagWork();
    Schedule schedule(work, secondsFromNow(60 * 60)); // owns work
    EXPECT_FALSE(schedule.heartbeat());
    EXPECT_FALSE(work->executed);
}

TEST(Schedule, FiresOnceItsTimeHasPassed) {
    FlagWork* work = new FlagWork();
    Schedule schedule(work, secondsFromNow(-60 * 60));
    EXPECT_TRUE(schedule.heartbeat());
    EXPECT_TRUE(work->executed);
}

TEST(Schedule, PopWorkTransfersOwnership) {
    FlagWork* work = new FlagWork();
    Schedule schedule(work, secondsFromNow(60));
    EXPECT_EQ(work, schedule.popWork());
    EXPECT_EQ(nullptr, schedule.getWork());
    delete work; // the schedule no longer owns it
}

//////////////////////////////////////////////////////////////////////////
// Scheduler
//////////////////////////////////////////////////////////////////////////

TEST(Scheduler, HeartbeatOnEmptyReturnsNull) {
    Scheduler scheduler;
    EXPECT_TRUE(scheduler.isEmpty());
    EXPECT_EQ(nullptr, scheduler.heartbeat());
}

TEST(Scheduler, ReleasesOnlyDueWorkAndHandsItBack) {
    Scheduler scheduler;
    FlagWork* due = new FlagWork();
    FlagWork* later = new FlagWork();
    scheduler.addSchedule(new Schedule(later, secondsFromNow(60 * 60)));
    scheduler.addSchedule(new Schedule(due, secondsFromNow(-60)));
    EXPECT_EQ(2, scheduler.getSize());

    // The earliest schedule sits at the top regardless of insertion order.
    Work* popped = scheduler.heartbeat();
    EXPECT_EQ(due, popped);
    EXPECT_TRUE(due->executed);
    EXPECT_FALSE(later->executed);
    EXPECT_EQ(1, scheduler.getSize());
    delete popped; // popRecentWork() hands ownership to the caller

    // The remaining schedule is still in the future.
    EXPECT_EQ(nullptr, scheduler.heartbeat());
    EXPECT_EQ(1, scheduler.getSize());
    EXPECT_FALSE(later->executed);
}

TEST(Scheduler, ReleasesDueWorkEarliestFirst) {
    Scheduler scheduler;
    FlagWork* first = new FlagWork();
    FlagWork* second = new FlagWork();
    scheduler.addSchedule(new Schedule(second, secondsFromNow(-60)));
    scheduler.addSchedule(new Schedule(first, secondsFromNow(-120)));

    Work* a = scheduler.heartbeat();
    Work* b = scheduler.heartbeat();
    EXPECT_EQ(first, a);
    EXPECT_EQ(second, b);
    EXPECT_TRUE(scheduler.isEmpty());
    delete a;
    delete b;
}

TEST(Scheduler, ClearDeletesPendingSchedules) {
    Scheduler scheduler;
    scheduler.addSchedule(new Schedule(new FlagWork(), secondsFromNow(60)));
    scheduler.addSchedule(new Schedule(new FlagWork(), secondsFromNow(120)));
    scheduler.clear();
    EXPECT_TRUE(scheduler.isEmpty());
}

} // namespace
