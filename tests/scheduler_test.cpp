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
// registration decision, the castle owner decisions, the routing of a
// war's zone work and the flag war's plan at the bottom are headers of plain
// values, so they join them without pulling the scheduler, the zone or the
// guild table in; the captured packet a posted broadcast carries is a header
// over the kernel's packets and streams.

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "BloodBibleBonusInfo.h"
#include "GCBloodBibleStatus.h"
#include "GCHolyLandBonusInfo.h"
#include "GCMoveOK.h"
#include "GCNoticeEvent.h"
#include "GCRegenZoneStatus.h"
#include "GCSweeperBonusInfo.h"
#include "GCSystemMessage.h"
#include "SocketEncryptOutputStream.h"
#include "SweeperBonusInfo.h"
#include "VSDateTime.h"
#include "ctf/FlagWarPlan.h"
#include "war/CapturedPacket.h"
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

namespace {

// A scheduler that lets the test take a due schedule out the way
// WarScheduler::heartbeat does, without running it.
class PoppingScheduler : public Scheduler {
public:
    using Scheduler::popDueSchedule;
};

// A schedule whose run is its own: what a war schedule adds to the work.
class CountingSchedule : public Schedule {
public:
    CountingSchedule(Work* pWork, const VSDateTime& time, int& runs) : Schedule(pWork, time), m_Runs(runs) {}
    void run() override {
        m_Runs++;
        Schedule::run();
    }

private:
    int& m_Runs;
};

} // namespace

TEST(Scheduler, PopsADueScheduleWithoutRunningItAndLeavesOneNotDue) {
    PoppingScheduler scheduler;
    FlagWork* due = new FlagWork();
    scheduler.addSchedule(new Schedule(new FlagWork(), secondsFromNow(60 * 60)));
    scheduler.addSchedule(new Schedule(due, secondsFromNow(-60)));

    Schedule* popped = scheduler.popDueSchedule();
    ASSERT_NE(nullptr, popped);
    EXPECT_EQ(due, popped->getWork());
    EXPECT_FALSE(due->executed);
    EXPECT_EQ(1, scheduler.getSize());

    // The one left is not due, so nothing more comes out; the popped one
    // can go back and come out again.
    EXPECT_EQ(nullptr, scheduler.popDueSchedule());
    scheduler.addSchedule(popped);
    EXPECT_EQ(popped, scheduler.popDueSchedule());
    delete popped;
}

TEST(Scheduler, HeartbeatRunsTheSchedulesOwnRunOnceItIsDue) {
    Scheduler scheduler;
    int runs = 0;
    FlagWork* work = new FlagWork();
    Schedule* schedule = new CountingSchedule(work, secondsFromNow(60 * 60), runs);
    scheduler.addSchedule(schedule);

    EXPECT_FALSE(schedule->isDue());
    EXPECT_EQ(nullptr, scheduler.heartbeat());
    EXPECT_EQ(0, runs);

    schedule->setScheduledTime(secondsFromNow(-1));
    EXPECT_TRUE(schedule->isDue());
    Work* popped = scheduler.heartbeat();
    EXPECT_EQ(work, popped);
    EXPECT_EQ(1, runs);
    EXPECT_TRUE(work->executed);
    delete popped;
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
using de::war::relicMayLieIn;
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

// The storages a relic may lie in are exactly those a return reaches, so a
// relic whose row names one is always held by a zone or a player.
TEST(RelicStorage, ARelicLiesOnlyWhereAReturnReachesIt) {
    for (int storage : {(int)STORAGE_ZONE, (int)STORAGE_CORPSE, (int)STORAGE_INVENTORY, (int)STORAGE_EXTRASLOT}) {
        EXPECT_TRUE(relicMayLieIn(storage)) << storage;
        EXPECT_NE(Nowhere, itemHolderOf(storage, 1201, "1201")) << storage;
    }
}

// Every other storage has a gateway that refuses a relic, or no writer.
TEST(RelicStorage, NoGatewayLetsARelicIntoAnyOtherStorage) {
    for (int storage : {(int)STORAGE_GEAR, (int)STORAGE_BELT, (int)STORAGE_MOTORCYCLE, (int)STORAGE_STORE,
                        (int)STORAGE_BOX, (int)STORAGE_STASH, (int)STORAGE_GARBAGE, (int)STORAGE_TIMEOVER,
                        (int)STORAGE_GOODSINVENTORY, (int)STORAGE_PET_STASH, (int)STORAGE_EXCHANGE}) {
        EXPECT_FALSE(relicMayLieIn(storage)) << storage;
        EXPECT_EQ(Nowhere, itemHolderOf(storage, 1201, "Bearer")) << storage;
    }
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

// A broadcast to a list of zones (postBroadcast) is routed the same way: the
// group holding all of a holy land's zones gets one command that sends to
// each of them, and a notice to three zones in three groups makes three.
TEST(ZoneBroadcast, AGroupHoldingEveryZoneGetsOneCommandForThemAll) {
    auto batches = zonesByOwner<int>({1201, 1202, 1203, 1204}, [](ZoneID_t) { return 7; });

    ASSERT_EQ(1u, batches.size());
    EXPECT_EQ(7, batches[0].first);
    EXPECT_EQ((std::vector<ZoneID_t>{1201, 1202, 1203, 1204}), batches[0].second);
}

TEST(ZoneBroadcast, EachGroupHoldingAZoneGetsItsOwnCommand) {
    auto groupOf = [](ZoneID_t zoneID) -> int { return zoneID == 61 ? 1 : zoneID == 64 ? 2 : 3; };

    auto batches = zonesByOwner<int>({61, 64, 1007}, groupOf);

    ASSERT_EQ(3u, batches.size());
    EXPECT_EQ((std::vector<ZoneID_t>{61}), batches[0].second);
    EXPECT_EQ((std::vector<ZoneID_t>{64}), batches[1].second);
    EXPECT_EQ((std::vector<ZoneID_t>{1007}), batches[2].second);
}

//////////////////////////////////////////////////////////////////////////
// The packet a posted broadcast carries (war/CapturedPacket.h): the body
// written once on the calling thread, framed again by each player's stream.
//////////////////////////////////////////////////////////////////////////

using de::war::CapturedPacket;

// The bytes a player's stream holds after sending packet: the id, the size,
// that stream's sequence byte and the body, as SocketOutputStream::writePacket
// puts them there for a session with the given encrypt code.
std::vector<unsigned char> sent(const Packet& packet, uchar code, int times = 1) {
    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(code);
    for (int i = 0; i < times; i++)
        oStream.writePacket(&packet);
    const char* pBuffer = oStream.getBuffer();
    return std::vector<unsigned char>(pBuffer, pBuffer + oStream.length());
}

// Every encrypt code a session may use, the unencrypted one included.
void expectSentAlike(const Packet& original, const CapturedPacket& captured) {
    for (uchar code = 0; code <= 5; code++) {
        EXPECT_EQ(sent(original, code), sent(captured, code)) << "encrypt code " << (int)code;
        EXPECT_EQ(sent(original, code, 3), sent(captured, code, 3)) << "encrypt code " << (int)code;
    }
    EXPECT_EQ(original.getPacketID(), captured.getPacketID());
    EXPECT_EQ(original.getPacketSize(), captured.getPacketSize());
    EXPECT_EQ(original.getPacketName(), captured.getPacketName());
    EXPECT_EQ(original.toString(), captured.toString());
}

// The messages a war, a relic's return and a GM send to a castle, the holy
// land or a list of zones.
TEST(CapturedPacket, ASystemMessageIsSentAsTheOriginalWouldBe) {
    GCSystemMessage message;
    message.setMessage("The castle symbol returned to its guard shrine.");
    message.setColor(0x81A2B3C4);
    message.setType(SYSTEM_MESSAGE_HOLY_LAND);

    expectSentAlike(message, CapturedPacket(message));
}

TEST(CapturedPacket, ANoticeEventIsSentAsTheOriginalWouldBe) {
    GCNoticeEvent notice;
    notice.setCode(NOTICE_EVENT_PREMIUM_HALF_START);
    notice.setParameter(0x86A7B8C9);

    expectSentAlike(notice, CapturedPacket(notice));
}

TEST(CapturedPacket, TheRegenZoneAndBloodBibleStatusesAreSentAsTheOriginalsWouldBe) {
    GCRegenZoneStatus regenZones;
    for (uint i = 0; i < GCRegenZoneStatus::kZoneCount; i++)
        regenZones.setStatus(i, (BYTE)(i % 4));
    expectSentAlike(regenZones, CapturedPacket(regenZones));

    GCBloodBibleStatus bloodBible;
    bloodBible.setItemType(3);
    bloodBible.setZoneID(1201);
    bloodBible.setStorage(STORAGE_CORPSE);
    bloodBible.setRace(1);
    bloodBible.setShrineRace(1);
    bloodBible.setX(70);
    bloodBible.setY(81);
    expectSentAlike(bloodBible, CapturedPacket(bloodBible));
}

// These two own their entries and delete them with the packet, so a copy of
// the packet would free them twice; the capture holds bytes and outlives it.
TEST(CapturedPacket, ABonusListIsSentAsTheOriginalWouldBeAfterTheOriginalIsGone) {
    std::vector<unsigned char> expected;
    std::unique_ptr<CapturedPacket> pCaptured;
    {
        GCHolyLandBonusInfo bonuses;
        for (BYTE race = 0; race < 3; race++) {
            BloodBibleBonusInfo* pInfo = new BloodBibleBonusInfo();
            pInfo->setRace(race);
            bonuses.addBloodBibleBonusInfo(pInfo);
        }
        pCaptured = std::make_unique<CapturedPacket>(bonuses);
        expectSentAlike(bonuses, *pCaptured);
        expected = sent(bonuses, 3);
    }
    EXPECT_EQ(expected, sent(*pCaptured, 3));

    GCSweeperBonusInfo sweepers;
    for (BYTE race = 0; race < 3; race++) {
        SweeperBonusInfo* pInfo = new SweeperBonusInfo();
        pInfo->setRace(race);
        sweepers.addSweeperBonusInfo(pInfo);
    }
    expectSentAlike(sweepers, CapturedPacket(sweepers));
}

// A packet that encrypts its own fields writes them by the session's code, so
// no one capture fits every player: the capture refuses it.
TEST(CapturedPacket, APacketWrittenByTheSessionsCodeCannotBeCaptured) {
    GCMoveOK moveOK;
    moveOK.setXYDir(10, 20, 3);

    EXPECT_THROW(CapturedPacket{moveOK}, Throwable);

// A flag war's start and end (ctf/FlagWarPlan.h): the values the posted
// drops, returns and pole sweeps carry, and the order the end posts them in.
using de::ctf::FlagDrop;
using de::ctf::FlagLedger;
using de::ctf::FlagWarEndStep;
using de::ctf::PoleField;

PoleField poleField(ZoneID_t zoneID, ZoneCoord_t left, ZoneCoord_t top, ZoneCoord_t width, ZoneCoord_t height) {
    PoleField field;
    field.zoneID = zoneID;
    field.left = left;
    field.top = top;
    field.width = width;
    field.height = height;
    return field;
}

FlagWarEndStep returnFlag(ItemID_t flagID) {
    FlagWarEndStep step;
    step.kind = FlagWarEndStep::Kind::ReturnFlag;
    step.flagID = flagID;
    return step;
}

FlagWarEndStep sweepPoles(ZoneID_t zoneID) {
    FlagWarEndStep step;
    step.kind = FlagWarEndStep::Kind::SweepPoles;
    step.zoneID = zoneID;
    return step;
}

TEST(FlagWarPlan, AllowsFlagsInEveryZoneADropNames) {
    auto allowed = de::ctf::flagAllowMapOf({{21, 6}, {24, 7}, {1122, 20}});

    EXPECT_EQ((std::map<ZoneID_t, uint>{{21, 6}, {24, 7}, {1122, 20}}), allowed);
    EXPECT_TRUE(de::ctf::flagAllowMapOf({}).empty());
}

// A field of 2 by 1 poles laid out from (10, 20) is recorded as 4 by 2
// tiles; the sweep looks at every second tile as far as twice that.
TEST(FlagWarPlan, SweepsEverySecondTileOfAFieldAsFarAsTwiceItsExtent) {
    auto tiles = de::ctf::poleSweepTiles(poleField(61, 10, 20, 4, 2));

    ASSERT_EQ(15u, tiles.size());
    EXPECT_EQ(std::make_pair((ZoneCoord_t)10, (ZoneCoord_t)20), tiles.front());
    EXPECT_EQ(std::make_pair((ZoneCoord_t)10, (ZoneCoord_t)22), tiles[1]);
    EXPECT_EQ(std::make_pair((ZoneCoord_t)18, (ZoneCoord_t)24), tiles.back());
}

TEST(FlagWarPlan, SweepsEachPoleZoneOnceInTheOrderTheFieldsNameThem) {
    auto zones = de::ctf::poleZonesOf(
        {poleField(62, 0, 0, 2, 2), poleField(61, 0, 0, 2, 2), poleField(62, 8, 8, 2, 2), poleField(1122, 0, 0, 2, 2)});

    EXPECT_EQ((std::vector<ZoneID_t>{62, 61, 1122}), zones);
}

// A flag planted on a pole lies in the pole's zone, so its return and that
// zone's sweep go to one group's mailbox: the return, which pays the
// winner's gem stone, has to be there first.
TEST(FlagWarEnd, PostsEveryFlagsReturnBeforeAnyPoleSweep) {
    auto steps = de::ctf::flagWarEndSteps({501, 502, 503}, {poleField(61, 0, 0, 2, 2), poleField(62, 0, 0, 2, 2)});

    EXPECT_EQ((std::vector<FlagWarEndStep>{returnFlag(501), returnFlag(502), returnFlag(503), sweepPoles(61),
                                           sweepPoles(62)}),
              steps);
}

TEST(FlagWarEnd, AnEndWithNoFlagsStillSweepsThePoles) {
    EXPECT_EQ((std::vector<FlagWarEndStep>{sweepPoles(61)}), de::ctf::flagWarEndSteps({}, {poleField(61, 0, 0, 2, 2)}));
}

TEST(FlagLedger, TakeHandsOverEveryFlagAddedAndLeavesItEmpty) {
    FlagLedger ledger;
    ledger.add(501);
    ledger.add(502);

    EXPECT_EQ((std::vector<ItemID_t>{501, 502}), ledger.take());
    EXPECT_TRUE(ledger.take().empty());
}

} // namespace
