//////////////////////////////////////////////////////////////////////////////
// Filename    : login_kick_wait_test.cpp
// Description : The kick-wait step of LoginPlayer::processCommand
//               (runLoginKickWait, LoginKickRetry.h): the resend's and the
//               completion's failures are decided as the dispatch wrap
//               decides a handler's. A ProtocolException and a
//               ConnectException pass on as they are, for the manager to
//               retire the player; anything else, a DatabaseError from the
//               completion's account statements among them, is reported by
//               its kind and becomes a DisconnectException, so the manager's
//               loop goes on without the login instead of ending.
//////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "DatabaseError.h"
#include "Exception.h"
#include "LoginKickRetry.h"
#include "LoginPlayer.h"
#include "Socket.h"

namespace {

class WaitingPlayer : public LoginPlayer {
public:
    WaitingPlayer() : LoginPlayer(new Socket()) {
        setID("account");
        cacheLoginKickTarget({7, 9, 3, "Rowan"});
        setPlayerStatus(LPS_BEGIN_SESSION);
    }
    ~WaitingPlayer() override {
        setPlayerStatus(LPS_END_SESSION);
    }
};

struct Wait {
    Wait() {
        de::beginLoginKick(player, send);
    }

    bool tick() {
        return de::runLoginKickWait(player, player.getExpireTimeForKickCharacter(), send, complete, failed);
    }

    // Two resends counted; the third tick completes.
    void reachTheThirdTick() {
        ASSERT_TRUE(tick());
        ASSERT_TRUE(tick());
        ASSERT_EQ(completions, 0u);
    }

    WaitingPlayer player;
    Timeval now{100, 123456};
    unsigned sends = 0;
    unsigned completions = 0;
    std::vector<std::string> failures;
    std::function<void()> completionFailure;
    de::LoginKickRequest send = [&](LoginPlayer& current) {
        ++sends;
        current.setExpireTimeForKickCharacter(now);
        current.setPlayerStatus(LPS_WAITING_FOR_GL_KICK_VERIFY);
        return true;
    };
    std::function<void(LoginPlayer&)> complete = [&](LoginPlayer&) {
        ++completions;
        if (completionFailure)
            completionFailure();
    };
    de::LoginKickFailureReport failed = [&](const std::string& kind) { failures.push_back(kind); };
};

} // namespace

TEST(LoginKickWait, ACompletionThatReturnsIsNotReported) {
    Wait wait;
    wait.reachTheThirdTick();
    EXPECT_TRUE(wait.tick());
    EXPECT_EQ(wait.completions, 1u);
    EXPECT_TRUE(wait.failures.empty());
}

TEST(LoginKickWait, ADatabaseErrorFromTheCompletionRetiresTheLoginByItsKind) {
    Wait wait;
    wait.reachTheThirdTick();
    wait.completionFailure = [] { throw DatabaseError("UPDATE Player failed"); };
    EXPECT_THROW(wait.tick(), DisconnectException);
    EXPECT_EQ(wait.completions, 1u);
    ASSERT_EQ(wait.failures.size(), 1u);
    EXPECT_EQ(wait.failures[0], "DatabaseError");
    // The retry's own effects stand: the count saturated and the wait goes on
    // until the manager retires the player.
    EXPECT_EQ(wait.player.getKickCharacterCount(), 3);
    EXPECT_EQ(wait.player.getPlayerStatus(), LPS_WAITING_FOR_GL_KICK_VERIFY);
}

TEST(LoginKickWait, AProtocolFailurePassesOnUnreported) {
    Wait wait;
    wait.reachTheThirdTick();
    wait.completionFailure = [] { throw DisconnectException("logout"); };
    EXPECT_THROW(wait.tick(), DisconnectException);
    EXPECT_TRUE(wait.failures.empty());
}

TEST(LoginKickWait, AConnectFailurePassesOnUnreported) {
    Wait wait;
    wait.send = [](LoginPlayer&) -> bool { throw ConnectException("peer gone"); };
    EXPECT_THROW(wait.tick(), ConnectException);
    EXPECT_TRUE(wait.failures.empty());
}

TEST(LoginKickWait, EveryOtherFailureIsReportedByItsKindAndRetiresTheLogin) {
    struct Case {
        std::function<void()> raise;
        std::string kind;
    };
    const Case cases[] = {
        {[] { throw Error("completion failed"); }, "Error"},
        {[] { throw NoSuchElementException("no row"); }, "NoSuchElementException"},
        {[] { throw std::bad_alloc(); }, "std::exception"},
        {[] { throw 42; }, "unknown"},
    };
    for (const Case& failure : cases) {
        Wait wait;
        wait.reachTheThirdTick();
        wait.completionFailure = failure.raise;
        EXPECT_THROW(wait.tick(), DisconnectException) << failure.kind;
        ASSERT_EQ(wait.failures.size(), 1u) << failure.kind;
        EXPECT_EQ(wait.failures[0], failure.kind);
    }
}

TEST(LoginKickWait, AReportThatThrowsChangesNothing) {
    Wait wait;
    wait.reachTheThirdTick();
    wait.completionFailure = [] { throw DatabaseError("UPDATE Player failed"); };
    wait.failed = [](const std::string&) { throw std::bad_alloc(); };
    EXPECT_THROW(wait.tick(), DisconnectException);
    EXPECT_EQ(wait.player.getKickCharacterCount(), 3);
}
