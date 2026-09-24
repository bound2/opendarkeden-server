// The deadline the workers holding a database connection run their
// keep-alive query by (src/server/KeepAlive.h). A deadline that did not
// advance from itself would land near the epoch, always in the past, and the
// query would run on every pass of the worker's loop.

#include <gtest/gtest.h>

#include "KeepAlive.h"

namespace {

timeval at(long seconds, long microseconds) {
    timeval t{};
    t.tv_sec = seconds;
    t.tv_usec = microseconds;
    return t;
}

} // namespace

TEST(KeepAlive, TheDelayIsAnHourAtLeastAndUnderAnHourAndAHalf) {
    EXPECT_EQ(60 * 60, de::keepAliveDelaySeconds(0));
    EXPECT_EQ(89 * 60, de::keepAliveDelaySeconds(29));
    EXPECT_EQ(60 * 60, de::keepAliveDelaySeconds(30));
    for (int draw = 0; draw < 1000; draw++) {
        EXPECT_GE(de::keepAliveDelaySeconds(draw), 60 * 60);
        EXPECT_LT(de::keepAliveDelaySeconds(draw), 90 * 60);
    }
}

TEST(KeepAlive, TheDelayIsWholeMinutes) {
    for (int draw = 0; draw < 1000; draw++)
        EXPECT_EQ(0, de::keepAliveDelaySeconds(draw) % 60);
}

TEST(KeepAlive, TheDeadlineAdvancesFromItselfNotFromTheEpoch) {
    const long now = 1'790'000'000;
    const timeval next = de::nextKeepAliveDeadline(at(now, 250'000), 7);
    EXPECT_EQ(now + 67 * 60, next.tv_sec);
    EXPECT_EQ(250'000, next.tv_usec);
}

TEST(KeepAlive, EachRunMovesTheDeadlineFurtherAhead) {
    const long now = 1'790'000'000;
    timeval deadline = at(now, 0);
    for (int draw = 0; draw < 5; draw++) {
        const timeval next = de::nextKeepAliveDeadline(deadline, draw);
        EXPECT_GT(next.tv_sec, deadline.tv_sec);
        deadline = next;
    }
    EXPECT_EQ(now + (60 + 61 + 62 + 63 + 64) * 60, deadline.tv_sec);
}

TEST(KeepAlive, TheDelayIsUsableWhileCompiling) {
    static_assert(de::keepAliveDelaySeconds(15) == 75 * 60);
    SUCCEED();
}
