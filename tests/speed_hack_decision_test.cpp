// The gameserver's client-heartbeat plausibility rule
// (src/server/gameserver/SpeedHackDecision.cpp): when a heartbeat counts as
// early, how many early ones a session may carry before the rule refuses, and
// what an on-time heartbeat pays back. The clock is a plain number here, so
// neither a player, a socket nor a real time source is involved; the caller
// that turns a false into a disconnect is
// src/server/gameserver/handler/CGVerifyTimeHandler.cpp.

#include <gtest/gtest.h>

#include "SpeedHackDecision.h"

namespace {

// The values GamePlayer.cpp passes: a heartbeat every 60 seconds, 5 seconds of
// slack, and 3 early heartbeats carried before the next one is refused.
de::HeartbeatVerifyParams liveParams() {
    de::HeartbeatVerifyParams params;
    params.checkDelaySec = 60;
    params.maxTimeGapSec = 5;
    params.maxEarlyCount = 3;
    return params;
}

TEST(SpeedHackDecisionTest, FirstHeartbeatIsAlwaysAccepted) {
    de::HeartbeatVerifyState state;

    EXPECT_TRUE(de::verifyHeartbeat(liveParams(), 1000, state));

    // It only opens the window the next heartbeat is measured in.
    EXPECT_EQ(1060, state.nextVerifySec);
    EXPECT_EQ(0, state.earlyCount);
}

TEST(SpeedHackDecisionTest, HeartbeatOnTheIntervalIsAccepted) {
    de::HeartbeatVerifyState state;

    de::verifyHeartbeat(liveParams(), 1000, state);

    EXPECT_TRUE(de::verifyHeartbeat(liveParams(), 1060, state));
    EXPECT_EQ(1120, state.nextVerifySec);
    EXPECT_EQ(0, state.earlyCount);
}

TEST(SpeedHackDecisionTest, HeartbeatInsideTheGapIsAccepted) {
    de::HeartbeatVerifyState state;

    de::verifyHeartbeat(liveParams(), 1000, state);

    // 4 seconds early is inside the 5-second gap.
    EXPECT_TRUE(de::verifyHeartbeat(liveParams(), 1056, state));
    EXPECT_EQ(0, state.earlyCount);
}

TEST(SpeedHackDecisionTest, HeartbeatOnTheGapBoundaryCountsAsEarly) {
    de::HeartbeatVerifyState state;

    de::verifyHeartbeat(liveParams(), 1000, state);

    // The comparison is strict: arriving exactly a gap early is early.
    EXPECT_TRUE(de::verifyHeartbeat(liveParams(), 1055, state));
    EXPECT_EQ(1, state.earlyCount);
}

TEST(SpeedHackDecisionTest, AnEarlyHeartbeatIsStillAcceptedAndCounted) {
    de::HeartbeatVerifyState state;

    de::verifyHeartbeat(liveParams(), 1000, state);

    EXPECT_TRUE(de::verifyHeartbeat(liveParams(), 1001, state));
    EXPECT_EQ(1, state.earlyCount);
    // The next heartbeat is measured from where the client is, not from where
    // it should have been.
    EXPECT_EQ(1061, state.nextVerifySec);
}

TEST(SpeedHackDecisionTest, TheFifthConsecutiveEarlyHeartbeatIsRefused) {
    de::HeartbeatVerifyState state;
    const de::HeartbeatVerifyParams params = liveParams();

    de::verifyHeartbeat(params, 1000, state);

    // Four early heartbeats are carried; the fifth finds the count above the
    // maximum and is refused.
    EXPECT_TRUE(de::verifyHeartbeat(params, 1001, state));
    EXPECT_TRUE(de::verifyHeartbeat(params, 1002, state));
    EXPECT_TRUE(de::verifyHeartbeat(params, 1003, state));
    EXPECT_TRUE(de::verifyHeartbeat(params, 1004, state));
    EXPECT_EQ(4, state.earlyCount);

    EXPECT_FALSE(de::verifyHeartbeat(params, 1005, state));
}

TEST(SpeedHackDecisionTest, AnOnTimeHeartbeatPaysBackOneEarlyOne) {
    de::HeartbeatVerifyState state;
    const de::HeartbeatVerifyParams params = liveParams();

    de::verifyHeartbeat(params, 1000, state);

    EXPECT_TRUE(de::verifyHeartbeat(params, 1001, state));
    EXPECT_TRUE(de::verifyHeartbeat(params, 1002, state));
    EXPECT_EQ(2, state.earlyCount);

    EXPECT_TRUE(de::verifyHeartbeat(params, 1062, state));
    EXPECT_EQ(1, state.earlyCount);

    EXPECT_TRUE(de::verifyHeartbeat(params, 1122, state));
    EXPECT_EQ(0, state.earlyCount);
}

TEST(SpeedHackDecisionTest, TheCountNeverFallsBelowZero) {
    de::HeartbeatVerifyState state;
    const de::HeartbeatVerifyParams params = liveParams();

    long now = 1000;
    de::verifyHeartbeat(params, now, state);

    for (int i = 0; i < 5; i++) {
        now += 60;
        EXPECT_TRUE(de::verifyHeartbeat(params, now, state));
    }

    EXPECT_EQ(0, state.earlyCount);
}

// A session that alternates between early and on-time heartbeats never
// reaches the maximum: this is what the CGVerifyTime handler relies on to
// leave a player with a jittery connection alone.
TEST(SpeedHackDecisionTest, AlternatingEarlyAndOnTimeNeverRefuses) {
    de::HeartbeatVerifyState state;
    const de::HeartbeatVerifyParams params = liveParams();

    long now = 1000;
    de::verifyHeartbeat(params, now, state);

    for (int i = 0; i < 20; i++) {
        now += 1;
        EXPECT_TRUE(de::verifyHeartbeat(params, now, state));
        now += 60;
        EXPECT_TRUE(de::verifyHeartbeat(params, now, state));
    }
}

} // namespace
