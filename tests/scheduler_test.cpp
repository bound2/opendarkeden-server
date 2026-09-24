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
// Links only de-kernel plus the two war/ sources under test. The siege
// registration decision, the castle owner decisions and the routing of a
// war's zone work at the bottom are headers of plain values, so they join
// them without pulling the scheduler, the zone or the guild table in.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "VSDateTime.h"
#include "war/CastleOwnerDecision.h"
#include "war/Schedule.h"
#include "war/Scheduler.h"
#include "war/SiegeRegistrationDecision.h"
#include "war/WarZoneRouting.h"
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

//////////////////////////////////////////////////////////////////////////
// The siege registration decision
//////////////////////////////////////////////////////////////////////////

SiegeRegistrationState nothingScheduled() {
    return SiegeRegistrationState();
}

SiegeRegistrationState siegeWith(unsigned int challengers) {
    SiegeRegistrationState state;
    state.hasScheduledWar = true;
    state.isSiege = true;
    state.challengerCount = challengers;
    return state;
}

SiegeRegistrationState guildWarScheduled() {
    SiegeRegistrationState state;
    state.hasScheduledWar = true;
    state.isSiege = false;
    return state;
}

TEST(SiegeRegistration, OpensASiegeWhenTheCastleHasNoWarWaiting) {
    EXPECT_EQ(SIEGE_REGISTRATION_CREATE, decideSiegeRegistration(nothingScheduled()));
}

TEST(SiegeRegistration, JoinsASiegeWithARemainingSlot) {
    EXPECT_EQ(SIEGE_REGISTRATION_JOIN, decideSiegeRegistration(siegeWith(1)));
    EXPECT_EQ(SIEGE_REGISTRATION_JOIN, decideSiegeRegistration(siegeWith(MaxSiegeChallengerGuilds - 1)));
}

TEST(SiegeRegistration, RefusesASiegeWhoseSlotsAreTaken) {
    EXPECT_EQ(SIEGE_REGISTRATION_FULL, decideSiegeRegistration(siegeWith(MaxSiegeChallengerGuilds)));
    EXPECT_EQ(SIEGE_REGISTRATION_FULL, decideSiegeRegistration(siegeWith(MaxSiegeChallengerGuilds + 1)));
}

// A guild war is applied for by one guild and has no slot for another, so a
// castle it waits on takes no siege registration until it is over. Opening a
// siege beside it would put two wars on one castle.
TEST(SiegeRegistration, RefusesWhenAGuildWarAlreadyWaitsOnTheCastle) {
    EXPECT_EQ(SIEGE_REGISTRATION_FULL, decideSiegeRegistration(guildWarScheduled()));
}

//////////////////////////////////////////////////////////////////////////
// Castle owner changes (war/CastleOwnerDecision.h). A won castle war and a
// guild deletion each post their change to the castle zone's group, and the
// two may reach its mailbox in either order; each change is decided against
// the castle and the guild table as they stand when it runs there.
//////////////////////////////////////////////////////////////////////////

const GuildID_t Winner = 1001;
const GuildID_t Holder = 1002;

// The castle zone's thread running a posted war end: the winning guild is
// looked up in the guild table when the command runs.
void runWarEnd(CastleOwner& castle, Race_t winnerRace, GuildID_t winnerGuildID, bool winnerStillExists) {
    castle = castleWarWinnerOwner(winnerRace, winnerGuildID, winnerStillExists);
}

// The castle zone's thread running a posted guild deletion.
void runGuildDeletion(CastleOwner& castle, GuildID_t deletedGuildID) {
    std::optional<CastleOwner> owner = castleOwnerAfterGuildDeleted(castle.race, castle.guildID, deletedGuildID);
    if (owner)
        castle = *owner;
}

TEST(CastleOwner, EachRaceHasItsCommonGuild) {
    EXPECT_EQ(SlayerCommon, commonGuildIDOf(RACE_SLAYER));
    EXPECT_EQ(VampireCommon, commonGuildIDOf(RACE_VAMPIRE));
    EXPECT_EQ(OustersCommon, commonGuildIDOf(RACE_OUSTERS));
    EXPECT_TRUE(isCommonGuildID(SlayerCommon));
    EXPECT_TRUE(isCommonGuildID(VampireCommon));
    EXPECT_TRUE(isCommonGuildID(OustersCommon));
    EXPECT_FALSE(isCommonGuildID(Winner));
}

TEST(CastleOwner, AWonWarHandsTheCastleToTheWinningGuild) {
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, Winner}), castleWarWinnerOwner(RACE_VAMPIRE, Winner, true));
}

// A common guild is never deleted, so a war won by its race's players hands
// over the common castle whatever the guild table says.
TEST(CastleOwner, AWarWonForTheCommonGuildHandsOverTheCommonCastle) {
    EXPECT_EQ((CastleOwner{RACE_SLAYER, SlayerCommon}), castleWarWinnerOwner(RACE_SLAYER, SlayerCommon, false));
}

TEST(CastleOwner, AWarWonByAGuildDeletedSinceHandsOverTheWinnerRacesCommonCastle) {
    EXPECT_EQ((CastleOwner{RACE_OUSTERS, OustersCommon}), castleWarWinnerOwner(RACE_OUSTERS, Winner, false));
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, VampireCommon}), castleWarWinnerOwner(RACE_VAMPIRE, Winner, false));
}

TEST(CastleOwner, ADeletedGuildsCastleTurnsCommonForTheCastlesRace) {
    EXPECT_EQ((CastleOwner{RACE_SLAYER, SlayerCommon}), castleOwnerAfterGuildDeleted(RACE_SLAYER, Holder, Holder));
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, VampireCommon}), castleOwnerAfterGuildDeleted(RACE_VAMPIRE, Holder, Holder));
    EXPECT_EQ((CastleOwner{RACE_OUSTERS, OustersCommon}), castleOwnerAfterGuildDeleted(RACE_OUSTERS, Holder, Holder));
}

TEST(CastleOwner, ADeletionLeavesACastleAnotherGuildHolds) {
    EXPECT_FALSE(castleOwnerAfterGuildDeleted(RACE_SLAYER, Winner, Holder).has_value());
    EXPECT_FALSE(castleOwnerAfterGuildDeleted(RACE_SLAYER, SlayerCommon, SlayerCommon).has_value());
}

// The winning guild is deleted around the war's end. The war end reaches the
// castle first while the guild still exists, then the deletion turns the
// castle it just took common; or the deletion runs first, finds the castle
// still held by its old owner, and the war end, finding the winner gone,
// hands over the common castle itself. Either way the castle ends common.
TEST(CastleOwner, AWinnerDeletedAroundTheWarsEndLeavesTheCastleCommonInEitherOrder) {
    CastleOwner castle{RACE_SLAYER, Holder};
    runWarEnd(castle, RACE_VAMPIRE, Winner, true);
    runGuildDeletion(castle, Winner);
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, VampireCommon}), castle);

    castle = CastleOwner{RACE_SLAYER, Holder};
    runGuildDeletion(castle, Winner);
    runWarEnd(castle, RACE_VAMPIRE, Winner, false);
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, VampireCommon}), castle);
}

// The losing holder is deleted around the war's end: whichever change runs
// first, the winner holds the castle afterwards.
TEST(CastleOwner, AHolderDeletedAroundTheWarsEndLeavesTheCastleToTheWinnerInEitherOrder) {
    CastleOwner castle{RACE_SLAYER, Holder};
    runWarEnd(castle, RACE_VAMPIRE, Winner, true);
    runGuildDeletion(castle, Holder);
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, Winner}), castle);

    castle = CastleOwner{RACE_SLAYER, Holder};
    runGuildDeletion(castle, Holder);
    EXPECT_EQ((CastleOwner{RACE_SLAYER, SlayerCommon}), castle);
    runWarEnd(castle, RACE_VAMPIRE, Winner, true);
    EXPECT_EQ((CastleOwner{RACE_VAMPIRE, Winner}), castle);
}

//////////////////////////////////////////////////////////////////////////
// Routing a war's zone work (war/WarZoneRouting.h). A relic's return is
// posted first to whoever holds it, as its item-object row names the holder;
// the per-zone work of a war goes to the zones' groups, one command a group.
//////////////////////////////////////////////////////////////////////////

using de::war::corpseZoneIDOf;
using de::war::ItemHolder;
using de::war::itemHolderOf;
using de::war::kItemReturnAttempts;
using de::war::retryItemReturn;
using de::war::zonesByOwner;

ItemHolder zoneHolder(ZoneID_t zoneID) {
    ItemHolder holder;
    holder.kind = ItemHolder::Kind::Zone;
    holder.zoneID = zoneID;
    return holder;
}

ItemHolder playerHolder(const std::string& name) {
    ItemHolder holder;
    holder.kind = ItemHolder::Kind::Player;
    holder.playerName = name;
    return holder;
}

const ItemHolder Nowhere;

TEST(ItemHolder, AnItemOnTheGroundIsHeldByItsZone) {
    EXPECT_EQ(zoneHolder(1201), itemHolderOf(STORAGE_ZONE, 1201, ""));
}

// An item inside a corpse keeps the corpse's object id in StorageID and the
// corpse's zone, as text, in OwnerID: a castle symbol resting in its guard
// shrine is held by the shrine's zone.
TEST(ItemHolder, AnItemInACorpseIsHeldByTheCorpsesZone) {
    EXPECT_EQ(zoneHolder(1202), itemHolderOf(STORAGE_CORPSE, 7340, "1202"));
}

TEST(ItemHolder, AnItemCarriedInTheInventoryOrOnTheMouseIsHeldByThePlayer) {
    EXPECT_EQ(playerHolder("Bearer"), itemHolderOf(STORAGE_INVENTORY, 0, "Bearer"));
    EXPECT_EQ(playerHolder("Bearer"), itemHolderOf(STORAGE_EXTRASLOT, 0, "Bearer"));
}

// No position loader reaches these, so the war has nowhere to post the return.
TEST(ItemHolder, AnItemAnywhereElseIsHeldNowhereAReturnCanReach) {
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_GEAR, 0, "Bearer"));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_STASH, 0, "Bearer"));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_MOTORCYCLE, 0, "Bearer"));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_GARBAGE, 0, "Bearer"));
}

TEST(ItemHolder, ARowNamingNoZoneOrNoPlayerIsHeldNowhere) {
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_ZONE, 0, ""));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_ZONE, 70000, ""));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_CORPSE, 7340, ""));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_CORPSE, 7340, "12O2"));
    EXPECT_EQ(Nowhere, itemHolderOf(STORAGE_INVENTORY, 0, ""));
}

TEST(ItemHolder, ACorpsesZoneIsItsOwnerIdReadAsADecimalZoneId) {
    EXPECT_EQ(1201, corpseZoneIDOf("1201"));
    EXPECT_EQ(65535, corpseZoneIDOf("65535"));
    EXPECT_EQ(0, corpseZoneIDOf(""));
    EXPECT_EQ(0, corpseZoneIDOf("65536"));
    EXPECT_EQ(0, corpseZoneIDOf("1201 "));
    EXPECT_EQ(0, corpseZoneIDOf("-1"));
}

// A return's attempts, as postItemReturn makes them: each reads the row and
// posts a step to the holder it names, and a step that misses the item -- it
// moved after the row was read -- makes the next attempt. foundAt[i] says
// whether attempt i + 1 finds the item; the result is the attempt that took
// it, or 0 when the return gave up.
int runItemReturn(const std::vector<bool>& foundAt) {
    int attemptsMade = 0;
    do {
        ++attemptsMade;
        if (attemptsMade <= (int)foundAt.size() && foundAt[attemptsMade - 1])
            return attemptsMade;
    } while (retryItemReturn(attemptsMade));
    return 0;
}

TEST(ItemReturn, TakesTheItemWhereTheRowSaysItLies) {
    EXPECT_EQ(1, runItemReturn({true}));
}

// The player carrying a symbol is transported, or logs out, before the step
// posted to him runs: he drops it and saves where it fell, and the next read
// of the row sends the return to that zone.
TEST(ItemReturn, FollowsAnItemThatMovedBeforeItsHoldersStepRan) {
    EXPECT_EQ(2, runItemReturn({false, true}));
    EXPECT_EQ(kItemReturnAttempts, runItemReturn({false, false, true}));
}

TEST(ItemReturn, GivesUpOnARowThatNeverCatchesUp) {
    EXPECT_EQ(0, runItemReturn({false, false, false, true}));
    EXPECT_EQ(0, runItemReturn({}));
}

// The castles and holy lands of a war spread over several groups; each group
// gets one command holding its zones, in the order the war listed them.
TEST(ZonesByOwner, GroupsZonesByTheirOwnerInTheOrderFirstSeen) {
    auto groupOf = [](ZoneID_t zoneID) -> int { return zoneID / 100; };

    auto batches = zonesByOwner<int>({1201, 71, 1202, 72, 1301}, groupOf);

    ASSERT_EQ(3u, batches.size());
    EXPECT_EQ(12, batches[0].first);
    EXPECT_EQ((std::vector<ZoneID_t>{1201, 1202}), batches[0].second);
    EXPECT_EQ(0, batches[1].first);
    EXPECT_EQ((std::vector<ZoneID_t>{71, 72}), batches[1].second);
    EXPECT_EQ(13, batches[2].first);
    EXPECT_EQ((std::vector<ZoneID_t>{1301}), batches[2].second);
}

TEST(ZonesByOwner, NoZonesMakeNoCommands) {
    EXPECT_TRUE(zonesByOwner<int>({}, [](ZoneID_t) { return 0; }).empty());
}

} // namespace
