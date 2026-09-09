// The loginserver's world and server-group decisions
// (src/server/loginserver/WorldSelection.cpp): both refusals of a world
// selection, the clamping and the refusal of a group selection, every step of
// the population ladder at its own boundary, the absolute cap and the down
// flag that override it, and the server list a world answers with. The
// topology is a recording fake, so the lookups a decision makes - and the
// ones it does not - are pinned too. The handlers themselves are not
// exercised here because they need a socket and the login server's tables.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "FakeWorldTopology.h"
#include "WorldSelection.h"

namespace {

typedef std::vector<std::string> Calls;

// The default ladder: every step measured from 800, the cap at 1500.
const ServerLoadThresholds kDefault;

// The China build's ladder: very busy runs to 1000 over the offset and the
// cap sits at 1800.
ServerLoadThresholds chinaThresholds() {
    ServerLoadThresholds thresholds;
    thresholds.veryBusyBelow = 1000;
    thresholds.userMax = 1800;
    return thresholds;
}

BYTE statusOf(UserNum_t userNum) {
    return serverGroupStatusFor(userNum, SERVER_FREE, kDefault);
}

//////////////////////////////////////////////////////////////////////////////
// The population ladder, at every boundary it has.
//////////////////////////////////////////////////////////////////////////////

TEST(ServerGroupStatus, AnEmptyGroupIsFree) {
    EXPECT_EQ(SERVER_FREE, statusOf(0));
}

TEST(ServerGroupStatus, TheLastFreePopulationIsOneBelowTheFirstStep) {
    // 100 + 800.
    EXPECT_EQ(SERVER_FREE, statusOf(899));
    EXPECT_EQ(SERVER_NORMAL, statusOf(900));
}

TEST(ServerGroupStatus, TheLastNormalPopulationIsOneBelowTheSecondStep) {
    // 250 + 800.
    EXPECT_EQ(SERVER_NORMAL, statusOf(1049));
    EXPECT_EQ(SERVER_BUSY, statusOf(1050));
}

TEST(ServerGroupStatus, TheLastBusyPopulationIsOneBelowTheThirdStep) {
    // 400 + 800.
    EXPECT_EQ(SERVER_BUSY, statusOf(1199));
    EXPECT_EQ(SERVER_VERY_BUSY, statusOf(1200));
}

TEST(ServerGroupStatus, TheLastVeryBusyPopulationIsOneBelowTheFourthStep) {
    // 500 + 800.
    EXPECT_EQ(SERVER_VERY_BUSY, statusOf(1299));
    EXPECT_EQ(SERVER_FULL, statusOf(1300));
}

TEST(ServerGroupStatus, TheStepsAreMeasuredFromTheOffset) {
    // Raising the offset moves every step with it; the cap does not move.
    ServerLoadThresholds thresholds;
    thresholds.userModify = 0;

    EXPECT_EQ(SERVER_FREE, serverGroupStatusFor(99, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_NORMAL, serverGroupStatusFor(100, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_NORMAL, serverGroupStatusFor(249, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_BUSY, serverGroupStatusFor(250, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_BUSY, serverGroupStatusFor(399, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_VERY_BUSY, serverGroupStatusFor(400, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_VERY_BUSY, serverGroupStatusFor(499, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(500, SERVER_FREE, thresholds));
}

TEST(ServerGroupStatus, TheChinaLadderStretchesTheVeryBusyStep) {
    // 1000 + 800: a population the plain ladder calls full is still only
    // very busy here.
    EXPECT_EQ(SERVER_VERY_BUSY, serverGroupStatusFor(1300, SERVER_FREE, chinaThresholds()));
    EXPECT_EQ(SERVER_VERY_BUSY, serverGroupStatusFor(1799, SERVER_FREE, chinaThresholds()));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1800, SERVER_FREE, chinaThresholds()));
}

TEST(ServerGroupStatus, TheCapMakesAGroupFullBelowTheLastStep) {
    // A cap under the ladder's own full point: the ladder still says normal
    // at 1000, the cap says full.
    ServerLoadThresholds thresholds;
    thresholds.userMax = 1000;

    EXPECT_EQ(SERVER_NORMAL, serverGroupStatusFor(999, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1000, SERVER_FREE, thresholds));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1001, SERVER_FREE, thresholds));
}

TEST(ServerGroupStatus, TheCapIsReachedAtItsOwnValue) {
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1499, SERVER_FREE, kDefault));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1500, SERVER_FREE, kDefault));

    EXPECT_EQ(SERVER_VERY_BUSY, serverGroupStatusFor(1799, SERVER_FREE, chinaThresholds()));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1800, SERVER_FREE, chinaThresholds()));
}

TEST(ServerGroupStatus, ADownGroupReadsDownWhateverItIsCarrying) {
    EXPECT_EQ(SERVER_DOWN, serverGroupStatusFor(0, SERVER_DOWN, kDefault));
    EXPECT_EQ(SERVER_DOWN, serverGroupStatusFor(1200, SERVER_DOWN, kDefault));
    EXPECT_EQ(SERVER_DOWN, serverGroupStatusFor(5000, SERVER_DOWN, kDefault));
}

TEST(ServerGroupStatus, TheDownFlagWinsOverTheCap) {
    EXPECT_EQ(SERVER_DOWN, serverGroupStatusFor(1500, SERVER_DOWN, kDefault));
}

TEST(ServerGroupStatus, AnyOtherGroupFlagIsIgnored) {
    // Only SERVER_DOWN is read off the group table; the rest of the table's
    // status is recomputed from the population.
    EXPECT_EQ(SERVER_FREE, serverGroupStatusFor(0, SERVER_FULL, kDefault));
    EXPECT_EQ(SERVER_FULL, serverGroupStatusFor(1300, SERVER_FREE, kDefault));
}

//////////////////////////////////////////////////////////////////////////////
// The list a world answers with.
//////////////////////////////////////////////////////////////////////////////

TEST(ServerList, AWorldWithNoGroupsAnswersAnEmptyListAndAsksNothingElse) {
    FakeWorldTopology topology;

    const std::vector<ServerListEntry> entries = serverListFor(3, kDefault, topology);

    EXPECT_TRUE(entries.empty());

    Calls expected;
    expected.push_back("serverGroupCount(3)");
    EXPECT_EQ(expected, topology.calls);
}

TEST(ServerList, EveryGroupKeepsItsIdAndNameAndGetsTheStatusItsPopulationGives) {
    FakeWorldTopology topology;
    topology.addGroup(0, "Vampire", SERVER_FREE, 0);
    topology.addGroup(1, "Slayer", SERVER_FREE, 1050);
    topology.addGroup(2, "Ousters", SERVER_DOWN, 0);

    const std::vector<ServerListEntry> entries = serverListFor(1, kDefault, topology);

    ASSERT_EQ(3u, entries.size());

    EXPECT_EQ(0, entries[0].groupID);
    EXPECT_EQ("Vampire", entries[0].groupName);
    EXPECT_EQ(SERVER_FREE, entries[0].stat);

    EXPECT_EQ(1, entries[1].groupID);
    EXPECT_EQ("Slayer", entries[1].groupName);
    EXPECT_EQ(SERVER_BUSY, entries[1].stat);

    EXPECT_EQ(2, entries[2].groupID);
    EXPECT_EQ("Ousters", entries[2].groupName);
    EXPECT_EQ(SERVER_DOWN, entries[2].stat);
}

TEST(ServerList, TheGroupsAreReadByIndexAndTheirPopulationByTheIdTheTableGave) {
    // The group table is walked 0..count-1; the population is asked for
    // under the id the row itself carries.
    FakeWorldTopology topology;
    topology.addGroup(10, "one", SERVER_FREE, 0);
    topology.addGroup(11, "two", SERVER_FREE, 0);

    const std::vector<ServerListEntry> entries = serverListFor(7, kDefault, topology);
    EXPECT_EQ(2u, entries.size());

    Calls expected;
    expected.push_back("serverGroupCount(7)");
    expected.push_back("serverGroup(0,7)");
    expected.push_back("serverGroupUserNum(10,7)");
    expected.push_back("serverGroup(1,7)");
    expected.push_back("serverGroupUserNum(11,7)");
    EXPECT_EQ(expected, topology.calls);
}

TEST(ServerList, TheChinaLadderIsCarriedIntoTheList) {
    FakeWorldTopology topology;
    topology.addGroup(0, "one", SERVER_FREE, 1300);

    const std::vector<ServerListEntry> plain = serverListFor(0, kDefault, topology);
    ASSERT_EQ(1u, plain.size());
    EXPECT_EQ(SERVER_FULL, plain[0].stat);

    const std::vector<ServerListEntry> china = serverListFor(0, chinaThresholds(), topology);
    ASSERT_EQ(1u, china.size());
    EXPECT_EQ(SERVER_VERY_BUSY, china[0].stat);
}

//////////////////////////////////////////////////////////////////////////////
// Selecting a world.
//////////////////////////////////////////////////////////////////////////////

TEST(SelectWorld, AWorldPastTheConfiguredCountIsRefusedBeforeItsStatusIsRead) {
    FakeWorldTopology topology;
    topology.worlds = 2;

    Outcome<void, SelectWorldRejection> outcome = decideSelectWorld(3, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(SelectWorldReason::UnknownWorld, outcome.rejection().reason);
    // The refusal carries the count the error log names beside the id.
    EXPECT_EQ(2, outcome.rejection().worldCount);

    Calls expected;
    expected.push_back("worldCount");
    EXPECT_EQ(expected, topology.calls);
}

TEST(SelectWorld, AWorldIdEqualToTheCountIsStillAccepted) {
    // The count is used as the last id, not as one past it.
    FakeWorldTopology topology;
    topology.worlds = 2;

    Outcome<void, SelectWorldRejection> outcome = decideSelectWorld(2, topology);

    EXPECT_TRUE(outcome.isOk());

    Calls expected;
    expected.push_back("worldCount");
    expected.push_back("worldStatus(2)");
    EXPECT_EQ(expected, topology.calls);
}

TEST(SelectWorld, AClosedWorldIsRefused) {
    FakeWorldTopology topology;
    topology.worlds = 3;
    topology.worldStatuses.push_back(WORLD_OPEN);
    topology.worldStatuses.push_back(WORLD_CLOSE);

    Outcome<void, SelectWorldRejection> outcome = decideSelectWorld(1, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(SelectWorldReason::WorldClosed, outcome.rejection().reason);
    EXPECT_EQ(3, outcome.rejection().worldCount);
}

TEST(SelectWorld, AnOpenWorldIsAccepted) {
    FakeWorldTopology topology;
    topology.worlds = 3;
    topology.worldStatuses.push_back(WORLD_OPEN);
    topology.worldStatuses.push_back(WORLD_OPEN);

    EXPECT_TRUE(decideSelectWorld(1, topology).isOk());
}

TEST(SelectWorld, AnUnknownWorldIsAnsweredBeforeAClosedOne) {
    // A world past the count is never asked for its status, so the
    // out-of-range refusal is the one that goes out.
    FakeWorldTopology topology;
    topology.worlds = 1;
    topology.worldStatuses.push_back(WORLD_CLOSE);
    topology.worldStatuses.push_back(WORLD_CLOSE);

    Outcome<void, SelectWorldRejection> outcome = decideSelectWorld(9, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(SelectWorldReason::UnknownWorld, outcome.rejection().reason);
}

TEST(SelectWorld, TheDecisionReadsNoGroupTable) {
    // The list is built separately, so an accepted world touches neither the
    // group table nor the populations.
    FakeWorldTopology topology;
    topology.worlds = 1;
    topology.addGroup(0, "one", SERVER_FREE, 0);

    EXPECT_TRUE(decideSelectWorld(1, topology).isOk());

    Calls expected;
    expected.push_back("worldCount");
    expected.push_back("worldStatus(1)");
    EXPECT_EQ(expected, topology.calls);
}

//////////////////////////////////////////////////////////////////////////////
// Selecting a server group.
//////////////////////////////////////////////////////////////////////////////

SelectServerRequest requestOf(WorldID_t worldID, ServerGroupID_t serverGroupID) {
    SelectServerRequest request;
    request.worldID = worldID;
    request.serverGroupID = serverGroupID;
    return request;
}

TEST(SelectServer, AGroupThatIsUpIsAccepted) {
    FakeWorldTopology topology;
    topology.worlds = 2;
    topology.addGroup(0, "one", SERVER_FREE, 0);
    topology.addGroup(1, "two", SERVER_FREE, 0);

    Outcome<SelectedServer, SelectServerRejection> outcome = decideSelectServer(requestOf(1, 1), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(1, outcome.events().worldID);
    EXPECT_EQ(1, outcome.events().serverGroupID);

    Calls expected;
    expected.push_back("worldCount");
    expected.push_back("serverGroupCount(1)");
    expected.push_back("serverGroup(1,1)");
    EXPECT_EQ(expected, topology.calls);
}

TEST(SelectServer, ADownGroupIsRefusedAndTheRefusalNamesIt) {
    FakeWorldTopology topology;
    topology.worlds = 1;
    topology.addGroup(0, "one", SERVER_FREE, 0);
    topology.addGroup(1, "two", SERVER_DOWN, 0);

    Outcome<SelectedServer, SelectServerRejection> outcome = decideSelectServer(requestOf(1, 1), topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(SelectServerReason::ServerClosed, outcome.rejection().reason);
    EXPECT_EQ(1, outcome.rejection().serverGroupID);
}

TEST(SelectServer, AWorldPastTheCountIsServedTheLastWorldInsteadOfBeingRefused) {
    FakeWorldTopology topology;
    topology.worlds = 2;
    topology.addGroup(0, "one", SERVER_FREE, 0);

    Outcome<SelectedServer, SelectServerRejection> outcome = decideSelectServer(requestOf(9, 0), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(2, outcome.events().worldID);

    // The clamped world is the one the group table is read for.
    Calls expected;
    expected.push_back("worldCount");
    expected.push_back("serverGroupCount(2)");
    expected.push_back("serverGroup(0,2)");
    EXPECT_EQ(expected, topology.calls);
}

TEST(SelectServer, AGroupPastTheCountIsServedTheLastGroupInsteadOfBeingRefused) {
    FakeWorldTopology topology;
    topology.worlds = 1;
    topology.addGroup(0, "one", SERVER_FREE, 0);
    topology.addGroup(1, "two", SERVER_FREE, 0);

    Outcome<SelectedServer, SelectServerRejection> outcome = decideSelectServer(requestOf(1, 7), topology);

    ASSERT_TRUE(outcome.isOk());
    // The count itself, not one below it: the clamp keeps the legacy
    // off-by-one that lets a client reach one past the last row.
    EXPECT_EQ(2, outcome.events().serverGroupID);
    EXPECT_EQ("serverGroup(2,1)", topology.calls.back());
}

TEST(SelectServer, AGroupIdInsideTheCountIsNotClamped) {
    FakeWorldTopology topology;
    topology.worlds = 4;
    topology.addGroup(0, "one", SERVER_FREE, 0);
    topology.addGroup(1, "two", SERVER_FREE, 0);
    topology.addGroup(2, "three", SERVER_FREE, 0);

    Outcome<SelectedServer, SelectServerRejection> outcome = decideSelectServer(requestOf(2, 2), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(2, outcome.events().worldID);
    EXPECT_EQ(2, outcome.events().serverGroupID);
}

TEST(SelectServer, TheDecisionReadsNoPopulation) {
    FakeWorldTopology topology;
    topology.worlds = 1;
    topology.addGroup(0, "one", SERVER_FREE, 1300);

    EXPECT_TRUE(decideSelectServer(requestOf(1, 0), topology).isOk());

    Calls expected;
    expected.push_back("worldCount");
    expected.push_back("serverGroupCount(1)");
    expected.push_back("serverGroup(0,1)");
    EXPECT_EQ(expected, topology.calls);
}

} // namespace
