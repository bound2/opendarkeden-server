//////////////////////////////////////////////////////////////////////////////
// Filename    : game_flag_step_failure_test.cpp
// Description : A flag war step whose statement fails leaves one schedule,
//               not two, and the main thread's heartbeat goes on:
//               FlagManager::heartbeat stops the DatabaseError, logs it and
//               queues the step again a minute later, and the start and the
//               finish queue their next step only after the work that can
//               throw, so a failed step queues nothing. A start whose
//               clearing fails leaves the war off; a finish whose history
//               fails, as it does under the required sql_mode, ends the
//               war regardless.
//////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "DatabaseError.h"
#include "GameContext.h"
#include "Properties.h"
#include "VSDateTime.h"
#include "ZoneGroupManager.h"
#include "ctf/FlagManager.h"
#include "ctf/FlagWar.h"
#include "war/Schedule.h"
#include "war/Work.h"

namespace {

// A step whose statement fails every time it runs.
class FailingStatement : public Work {
public:
    void execute() override {
        ++runs;
        throw DatabaseError("INSERT INTO FlagWarHistory failed");
    }
    string toString() const override {
        return "FailingStatement";
    }
    unsigned runs = 0;
};

// The steps are protected: a war a test drives exposes them.
class DrivenFlagWar : public FlagWar {
public:
    using FlagWar::executeFinish;
    using FlagWar::executeStart;
    using FlagWar::FlagWar;
};

// The war's database work refused, so the steps' order can be seen without
// a database: the start and the end count their calls and throw.
class RefusingFlagManager : public FlagManager {
public:
    explicit RefusingFlagManager(de::GameContext& context) : FlagManager(context) {}
    bool startFlagWar(int) override {
        ++starts;
        throw DatabaseError("DELETE FROM FlagWarStat failed");
    }
    bool endFlagWar() override {
        ++ends;
        throw DatabaseError("SELECT FROM FlagWarStat failed");
    }
    unsigned starts = 0;
    unsigned ends = 0;
};

// The round's clearing refused, so the start cannot begin a round.
class RefusingClear : public FlagManager {
public:
    explicit RefusingClear(de::GameContext& context) : FlagManager(context) {}
    void deleteFlagWarStats() override {
        ++clears;
        throw DatabaseError("DELETE FROM FlagWarStat failed");
    }
    unsigned clears = 0;
};

// The round's history refused, as the required sql_mode refuses it, with
// nothing else of the round's in a database.
class RefusingHistory : public FlagManager {
public:
    explicit RefusingHistory(de::GameContext& context) : FlagManager(context) {}
    void deleteFlagWarStats() override {}
    void recordFlagWarHistory() override {
        ++records;
        throw DatabaseError("SELECT FROM FlagWarStat failed");
    }
    unsigned records = 0;
};

} // namespace

TEST(GameFlagStepFailure, AStepWhoseStatementFailsIsQueuedOnceAMinuteLater) {
    de::GameContext context;
    FlagManager manager(context);
    manager.clear();
    FailingStatement* step = new FailingStatement();
    manager.addSchedule(new Schedule(step, VSDateTime::currentDateTime()));

    const VSDateTime before = VSDateTime::currentDateTime();
    Work* ran = NULL;
    ASSERT_NO_THROW(ran = manager.heartbeat());
    EXPECT_EQ(ran, nullptr);
    EXPECT_EQ(step->runs, 1u);

    ASSERT_EQ(manager.getSize(), 1);
    Schedule* next = manager.getRecentSchedule();
    ASSERT_NE(next, nullptr);
    EXPECT_EQ(next->getWork(), step);
    EXPECT_FALSE(next->isDue());
    EXPECT_GE(before.secsTo(next->getScheduledTime()), 59);
    EXPECT_LE(before.secsTo(next->getScheduledTime()), 61);

    // Not due, so the next heartbeat leaves it where it is.
    EXPECT_EQ(manager.heartbeat(), nullptr);
    EXPECT_EQ(step->runs, 1u);
    EXPECT_EQ(manager.getSize(), 1);
}

TEST(GameFlagStepFailure, AStartOrAFinishWhoseStatementFailsQueuesNothing) {
    de::GameContext context;
    ZoneGroupManager zoneGroups;
    context.setZoneGroupManager(&zoneGroups);
    RefusingFlagManager manager(context);
    manager.clear();
    DrivenFlagWar war(manager, context);

    EXPECT_THROW(war.executeStart(), DatabaseError);
    EXPECT_EQ(manager.starts, 1u);
    EXPECT_EQ(manager.getSize(), 0);

    EXPECT_THROW(war.executeFinish(), DatabaseError);
    EXPECT_EQ(manager.ends, 1u);
    EXPECT_EQ(manager.getSize(), 0);
}

TEST(GameFlagStepFailure, AStartWhoseClearingFailsLeavesTheWarOff) {
    de::GameContext context;
    ZoneGroupManager zoneGroups;
    context.setZoneGroupManager(&zoneGroups);
    RefusingClear manager(context);
    manager.clear();

    EXPECT_THROW(manager.startFlagWar(60), DatabaseError);
    EXPECT_EQ(manager.clears, 1u);
    EXPECT_FALSE(manager.hasFlagWar());
}

TEST(GameFlagStepFailure, AFinishWhoseHistoryFailsEndsTheWarRegardless) {
    de::GameContext context;
    ZoneGroupManager zoneGroups;
    context.setZoneGroupManager(&zoneGroups);
    Properties config;
    config.setProperty("Dimension", "0");
    config.setProperty("WorldID", "0");
    config.setProperty("ServerID", "0");
    context.setConfig(&config);
    RefusingHistory manager(context);
    manager.clear();

    ASSERT_TRUE(manager.startFlagWar(60));
    ASSERT_TRUE(manager.hasFlagWar());

    // The end also runs the round's script, which this tree has not: the
    // shell's complaint on stderr is all that leaves.
    bool ended = false;
    EXPECT_NO_THROW(ended = manager.endFlagWar());
    EXPECT_TRUE(ended);
    EXPECT_EQ(manager.records, 1u);
    EXPECT_FALSE(manager.hasFlagWar());
}
