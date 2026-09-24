// The readiness set the connection managers multiplex their
// descriptor-indexed player tables with (src/server/DescriptorPollSet.h). The
// managers watch one descriptor per table slot and ask, once a tick, which of
// them are ready, so the set has to answer for every descriptor the table can
// hold -- including the ones an fd_set could never address.

#include <poll.h>
#include <unistd.h>

#include <vector>

#include <gtest/gtest.h>
#include <sys/resource.h>
#include <sys/socket.h>

#include "DescriptorPollSet.h"

namespace {

const short kRead = de::DescriptorPollSet::kRead;
const short kWrite = de::DescriptorPollSet::kWrite;
const short kUrgent = de::DescriptorPollSet::kUrgent;
const short kAll = kRead | kWrite | kUrgent;

// Plays the kernel's part: fills the entries a manager would hand poll() with
// the given result, so the mapping from a result to what a manager asks can be
// checked without a socket.
void answer(de::DescriptorPollSet& set, int fd, short revents) {
    for (size_t i = 0; i < set.entries().size(); i++) {
        if (set.entries()[i].fd == fd)
            set.entries()[i].revents = revents;
    }
}

// A whole tick with a made-up answer for one descriptor.
void tickWith(de::DescriptorPollSet& set, int fd, short revents) {
    set.fill();
    answer(set, fd, revents);
    set.collect();
}

// A pair of connected sockets, closed with the fixture.
class SocketPair {
public:
    SocketPair() {
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_Fds) != 0)
            m_Fds[0] = m_Fds[1] = -1;
    }

    ~SocketPair() {
        close(0);
        close(1);
    }

    bool valid() const {
        return m_Fds[0] >= 0 && m_Fds[1] >= 0;
    }

    int operator[](int end) const {
        return m_Fds[end];
    }

    void close(int end) {
        if (m_Fds[end] >= 0) {
            ::close(m_Fds[end]);
            m_Fds[end] = -1;
        }
    }

    // Moves one end to the given descriptor, so a test can watch a descriptor
    // number an fd_set has no bit for. Returns false when the process is not
    // allowed that many open files.
    bool moveTo(int end, int fd) {
        rlimit limit;
        if (getrlimit(RLIMIT_NOFILE, &limit) == 0 && limit.rlim_cur <= static_cast<rlim_t>(fd)) {
            limit.rlim_cur = limit.rlim_max;
            setrlimit(RLIMIT_NOFILE, &limit);
        }

        if (::dup2(m_Fds[end], fd) < 0)
            return false;

        ::close(m_Fds[end]);
        m_Fds[end] = fd;
        return true;
    }

private:
    int m_Fds[2];
};

} // namespace

TEST(DescriptorPollSet, OnlyTheWatchedDescriptorsAreAskedAbout) {
    de::DescriptorPollSet set(100);
    set.watch(4, kRead);
    set.watch(37, kRead | kWrite);

    set.fill();

    ASSERT_EQ(2u, set.entries().size());
    EXPECT_EQ(4, set.entries()[0].fd);
    EXPECT_EQ(kRead, set.entries()[0].events);
    EXPECT_EQ(0, set.entries()[0].revents);
    EXPECT_EQ(37, set.entries()[1].fd);
    EXPECT_EQ(static_cast<short>(kRead | kWrite), set.entries()[1].events);
}

TEST(DescriptorPollSet, AnEmptySetAsksAboutNothingAndReportsNothing) {
    de::DescriptorPollSet set(100);

    EXPECT_EQ(0, set.pollOnce(0));
    EXPECT_TRUE(set.entries().empty());

    for (int fd = 0; fd < 100; fd++) {
        EXPECT_FALSE(set.isReadable(fd));
        EXPECT_FALSE(set.isWritable(fd));
        EXPECT_FALSE(set.isUrgent(fd));
    }
}

TEST(DescriptorPollSet, ReadableWritableAndUrgentComeFromTheirOwnResults) {
    de::DescriptorPollSet set(100);
    set.watch(9, kAll);

    tickWith(set, 9, POLLIN);
    EXPECT_TRUE(set.isReadable(9));
    EXPECT_FALSE(set.isWritable(9));
    EXPECT_FALSE(set.isUrgent(9));

    tickWith(set, 9, POLLOUT);
    EXPECT_FALSE(set.isReadable(9));
    EXPECT_TRUE(set.isWritable(9));
    EXPECT_FALSE(set.isUrgent(9));

    tickWith(set, 9, POLLPRI);
    EXPECT_FALSE(set.isReadable(9));
    EXPECT_FALSE(set.isWritable(9));
    EXPECT_TRUE(set.isUrgent(9));
}

// A hangup, a pending socket error and a descriptor that is not open make the
// manager's own read and write run and fail, which is what takes the
// connection down. None of them is out-of-band data.
TEST(DescriptorPollSet, AFailureIsReadyInEveryDirectionButIsNotUrgent) {
    de::DescriptorPollSet set(100);
    set.watch(9, kAll);

    const short failures[] = {POLLHUP, POLLERR, POLLNVAL};

    for (size_t i = 0; i < sizeof(failures) / sizeof(failures[0]); i++) {
        tickWith(set, 9, failures[i]);
        EXPECT_TRUE(set.isReadable(9));
        EXPECT_TRUE(set.isWritable(9));
        EXPECT_FALSE(set.isUrgent(9));
    }
}

// A manager that never asks to write to its listening socket is never told it
// can, whatever the kernel reports about it.
TEST(DescriptorPollSet, ReadinessIsReportedOnlyInTheDirectionsAskedFor) {
    de::DescriptorPollSet set(100);
    set.watch(3, kRead);

    tickWith(set, 3, POLLIN | POLLOUT | POLLPRI | POLLHUP);

    EXPECT_TRUE(set.isReadable(3));
    EXPECT_FALSE(set.isWritable(3));
    EXPECT_FALSE(set.isUrgent(3));
}

TEST(DescriptorPollSet, TheAnswersOfOneTickDoNotSurviveIntoTheNext) {
    de::DescriptorPollSet set(100);
    set.watch(9, kAll);

    tickWith(set, 9, POLLIN);
    ASSERT_TRUE(set.isReadable(9));

    set.fill();
    EXPECT_FALSE(set.isReadable(9));

    set.collect();
    EXPECT_FALSE(set.isReadable(9));
}

// The defect the managers' own delete paths rely on: a connection dropped part
// way through a tick must not be processed by the steps that follow.
TEST(DescriptorPollSet, DroppingADescriptorDropsWhatWasReportedForIt) {
    de::DescriptorPollSet set(100);
    set.watch(9, kAll);

    tickWith(set, 9, POLLIN | POLLOUT);
    ASSERT_TRUE(set.isReadable(9));

    set.unwatch(9);

    EXPECT_FALSE(set.watched(9));
    EXPECT_FALSE(set.isReadable(9));
    EXPECT_FALSE(set.isWritable(9));
}

// The wait runs without the lock that guards membership, so the descriptor may
// belong to another connection by the time the answer arrives.
TEST(DescriptorPollSet, AnAnswerAboutAReplacedConnectionIsDiscarded) {
    de::DescriptorPollSet set(100);
    set.watch(9, kAll);

    set.fill();
    answer(set, 9, POLLIN);

    set.unwatch(9);
    set.watch(9, kAll);

    set.collect();

    EXPECT_TRUE(set.watched(9));
    EXPECT_FALSE(set.isReadable(9));
}

TEST(DescriptorPollSet, AnAnswerAboutARemovedConnectionIsDiscarded) {
    de::DescriptorPollSet set(100);
    set.watch(9, kAll);

    set.fill();
    answer(set, 9, POLLIN);
    set.unwatch(9);
    set.collect();

    EXPECT_FALSE(set.isReadable(9));
}

// The bound that made this set necessary: the player table holds 2000
// descriptors and an fd_set holds FD_SETSIZE, so the descriptors in between
// had no bit to set. The set has a slot for each of them.
TEST(DescriptorPollSet, ADescriptorPastAnFdSetIsWatchedLikeAnyOther) {
    de::DescriptorPollSet set(2000);

    ASSERT_GT(set.tableSize(), FD_SETSIZE);

    set.watch(FD_SETSIZE, kAll);
    set.watch(1999, kAll);

    EXPECT_TRUE(set.watched(FD_SETSIZE));
    EXPECT_TRUE(set.watched(1999));

    tickWith(set, 1999, POLLIN);
    EXPECT_TRUE(set.isReadable(1999));
    EXPECT_FALSE(set.isReadable(FD_SETSIZE));
}

TEST(DescriptorPollSet, ADescriptorTheTableCannotHoldIsNotWatched) {
    de::DescriptorPollSet set(100);

    EXPECT_TRUE(set.holds(99));
    EXPECT_FALSE(set.holds(100));
    EXPECT_FALSE(set.holds(-1));

    set.watch(100, kAll);
    set.watch(-1, kAll);

    EXPECT_FALSE(set.watched(100));
    EXPECT_FALSE(set.watched(-1));

    set.fill();
    EXPECT_TRUE(set.entries().empty());
}

TEST(DescriptorPollSet, ATableWithNoSlotsHoldsNothing) {
    de::DescriptorPollSet set(0);

    EXPECT_EQ(0, set.tableSize());
    EXPECT_FALSE(set.holds(0));
    EXPECT_EQ(0, set.pollOnce(0));
}

// One real round against a connected pair: the answers come from the kernel
// rather than from the test.
TEST(DescriptorPollSet, ARealRoundReportsWhatTheSocketsAreDoing) {
    SocketPair sockets;
    ASSERT_TRUE(sockets.valid());

    de::DescriptorPollSet set(2000);
    ASSERT_TRUE(set.holds(sockets[0]));
    ASSERT_TRUE(set.holds(sockets[1]));

    set.watch(sockets[0], kAll);
    set.watch(sockets[1], kAll);

    // Nothing has been sent, so both ends take output and neither has input.
    EXPECT_GT(set.pollOnce(0), 0);
    EXPECT_TRUE(set.isWritable(sockets[0]));
    EXPECT_TRUE(set.isWritable(sockets[1]));
    EXPECT_FALSE(set.isReadable(sockets[0]));
    EXPECT_FALSE(set.isReadable(sockets[1]));

    ASSERT_EQ(1, ::write(sockets[0], "x", 1));

    EXPECT_GT(set.pollOnce(50), 0);
    EXPECT_TRUE(set.isReadable(sockets[1]));
    EXPECT_FALSE(set.isReadable(sockets[0]));

    // A peer that goes away leaves its end readable, which is how the managers
    // find out: their own read runs and fails.
    const int remaining = sockets[1];
    sockets.close(0);

    EXPECT_GT(set.pollOnce(50), 0);
    EXPECT_TRUE(set.isReadable(remaining));
}

// The same round on a descriptor an fd_set has no bit for. Skipped where the
// process may not open that many files.
TEST(DescriptorPollSet, ARealRoundWorksOnADescriptorPastAnFdSet) {
    SocketPair sockets;
    ASSERT_TRUE(sockets.valid());

    const int high = FD_SETSIZE + 476;
    if (!sockets.moveTo(1, high))
        GTEST_SKIP() << "the process may not open descriptor " << high;

    de::DescriptorPollSet set(2000);
    ASSERT_TRUE(set.holds(high));

    set.watch(sockets[0], kAll);
    set.watch(high, kAll);

    ASSERT_EQ(1, ::write(sockets[0], "x", 1));

    EXPECT_GT(set.pollOnce(50), 0);
    EXPECT_TRUE(set.isReadable(high));
}
