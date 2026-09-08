// The guild join, registration and confirmation decisions
// (src/server/gameserver/guild/GuildJoinDecision.cpp): every refusal with the
// input that triggers it, the precedence between them, the response code each
// refusal carries to each race - including the ones the server answers with
// silence - the quit-penalty date arithmetic at its boundary, and the
// repository calls the decisions make, in order. The repository is a fake, so
// no database is involved; the handlers themselves are not exercised here
// because they need a creature and a socket.

#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "FakeGuildRepository.h"
#include "GCNPCResponse.h"
#include "GuildJoinDecision.h"

namespace {

const int kPenaltyDays = 7;
const char* kPlayer = "Aria";
const char* kGuildName = "Nightwatch";

// GuildMember::GUILDMEMBER_RANK_LEAVE and GUILDMEMBER_RANK_DENY: quitting a
// guild carries the founding penalty, being expelled does not.
const int kRankLeave = 5;
const int kRankDeny = 4;

// The stamp GuildMember::leave writes for a local calendar day: tm_year and
// tm_mon as the C library keeps them.
std::string stampFor(int year, int monthZeroBased, int day) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%03d%02d%02d", year - 1900, monthZeroBased, day);
    return buffer;
}

// Local midnight of the day a stamp names, the instant the decision measures
// the penalty from.
time_t midnightOf(int year, int monthZeroBased, int day) {
    tm stamp = {};
    stamp.tm_year = year - 1900;
    stamp.tm_mon = monthZeroBased;
    stamp.tm_mday = day;
    return mktime(&stamp);
}

// A day well inside every timezone's range, and the first instant at which
// its penalty has run out.
const int kQuitYear = 2005;
const int kQuitMonth = 0; // January, zero based as stored
const int kQuitDay = 17;

std::string quitStamp() {
    return stampFor(kQuitYear, kQuitMonth, kQuitDay);
}

time_t penaltyEnd() {
    return midnightOf(kQuitYear, kQuitMonth, kQuitDay) + static_cast<time_t>(kPenaltyDays) * 24 * 3600;
}

// The response code a refusal carries to one race, or -1 where the server
// sends nothing at all.
int codeFor(const GuildJoinRejection& rejection, GuildJoinRace race) {
    uint16_t code = 0;
    if (!guildJoinResponseCode(rejection, race, code))
        return -1;
    return static_cast<int>(code);
}

GuildJoinRejection rejection(GuildJoinContext context, GuildJoinReason reason) {
    return GuildJoinRejection(context, reason);
}

// A Slayer asking to become a starting member of a loaded guild, comfortably
// over every threshold.
GuildJoinAttempt startingAttempt() {
    GuildJoinAttempt attempt;
    attempt.name = kPlayer;
    attempt.race = GUILD_JOIN_RACE_SLAYER;
    attempt.guildExists = true;
    attempt.rank = GUILD_JOIN_RANK_STARTING;
    attempt.stats.level = 50;
    attempt.stats.gold = 100000000;
    attempt.stats.fame = 500000;
    attempt.requirements.level = 30;
    attempt.requirements.gold = 10000000;
    attempt.requirements.fame = 100000;
    attempt.requirements.checkFame = true;
    attempt.waitMemberCount = 0;
    attempt.waitMemberLimit = 15;
    attempt.now = penaltyEnd();
    attempt.penaltyTermDays = kPenaltyDays;
    return attempt;
}

GuildJoinAttempt waitingAttempt() {
    GuildJoinAttempt attempt = startingAttempt();
    attempt.rank = GUILD_JOIN_RANK_WAITING;
    return attempt;
}

GuildRegistrationRequest registrationRequest() {
    GuildRegistrationRequest request;
    request.name = kPlayer;
    request.guildName = kGuildName;
    request.now = penaltyEnd();
    request.penaltyTermDays = kPenaltyDays;
    return request;
}

//////////////////////////////////////////////////////////////////////////////
// The quit penalty's date arithmetic.
//////////////////////////////////////////////////////////////////////////////

TEST(GuildQuitPenalty, RunsUntilTheWholeTermHasElapsed) {
    EXPECT_TRUE(guildQuitPenaltyPending(quitStamp(), penaltyEnd() - 1, kPenaltyDays));
    // difftime is compared with "<", so the instant the term is reached the
    // penalty is over.
    EXPECT_FALSE(guildQuitPenaltyPending(quitStamp(), penaltyEnd(), kPenaltyDays));
    EXPECT_FALSE(guildQuitPenaltyPending(quitStamp(), penaltyEnd() + 1, kPenaltyDays));
}

TEST(GuildQuitPenalty, MeasuresFromLocalMidnightOfTheStampedDay) {
    time_t midnight = midnightOf(kQuitYear, kQuitMonth, kQuitDay);
    EXPECT_TRUE(guildQuitPenaltyPending(quitStamp(), midnight, kPenaltyDays));
    EXPECT_TRUE(guildQuitPenaltyPending(quitStamp(), midnight + 23 * 3600, kPenaltyDays));
}

TEST(GuildQuitPenalty, ReadsTheStampBackIntoTheFieldsThatWroteIt) {
    // 17 January 2005 is "105" "00" "17": the year counts from 1900 and the
    // month is zero based on both sides, so it reads back as January again.
    EXPECT_EQ(std::string("1050017"), quitStamp());

    // "01" is therefore February, a month later, and its penalty is still
    // running at the instant January's has run out.
    std::string february = stampFor(kQuitYear, 1, kQuitDay);
    EXPECT_TRUE(guildQuitPenaltyPending(february, penaltyEnd(), kPenaltyDays));
}

TEST(GuildQuitPenalty, IgnoresAStampOfAnyOtherLength) {
    // A SQL NULL ExpireDate reads back as the empty string.
    EXPECT_FALSE(guildQuitPenaltyPending("", penaltyEnd() - 1, kPenaltyDays));
    EXPECT_FALSE(guildQuitPenaltyPending("105001", penaltyEnd() - 1, kPenaltyDays));
    EXPECT_FALSE(guildQuitPenaltyPending("10500170", penaltyEnd() - 1, kPenaltyDays));
}

TEST(GuildQuitPenalty, AZeroTermIsNoPenaltyAtAll) {
    EXPECT_FALSE(guildQuitPenaltyPending(quitStamp(), midnightOf(kQuitYear, kQuitMonth, kQuitDay), 0));
}

//////////////////////////////////////////////////////////////////////////////
// decideGuildJoinAttempt - CGTryJoinGuild.
//////////////////////////////////////////////////////////////////////////////

TEST(GuildJoinAttemptDecision, AMissingGuildIsRefusedBeforeAnythingIsRead) {
    FakeGuildRepository repository;
    GuildJoinAttempt attempt = startingAttempt();
    attempt.guildExists = false;

    auto decision = decideGuildJoinAttempt(repository, attempt);

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_GUILD_MISSING, decision.rejection().reason);
    EXPECT_TRUE(repository.calls.empty());
}

TEST(GuildJoinAttemptDecision, ACharacterWithNoMembershipRowMayJoin) {
    FakeGuildRepository repository;

    auto decision = decideGuildJoinAttempt(repository, startingAttempt());

    EXPECT_TRUE(decision.isOk());
    EXPECT_EQ(std::vector<std::string>{"loadMemberExpireDate(Aria)"}, repository.calls);
}

TEST(GuildJoinAttemptDecision, APendingQuitPenaltyIsRefused) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    GuildJoinAttempt attempt = startingAttempt();
    attempt.now = penaltyEnd() - 1;

    auto decision = decideGuildJoinAttempt(repository, attempt);

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_PENALTY_PENDING, decision.rejection().reason);
}

TEST(GuildJoinAttemptDecision, AnElapsedQuitPenaltyLetsTheJoinThrough) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    auto decision = decideGuildJoinAttempt(repository, startingAttempt());

    EXPECT_TRUE(decision.isOk());
}

TEST(GuildJoinAttemptDecision, ARowWithoutAStampIsALiveMembership) {
    FakeGuildRepository repository;
    // No ExpireDate: the character still belongs to guild 7.
    repository.addMember(kPlayer, 7, 0);

    auto decision = decideGuildJoinAttempt(repository, startingAttempt());

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_ALREADY_MEMBER, decision.rejection().reason);
}

TEST(GuildJoinAttemptDecision, ThresholdsAreCheckedLevelThenGoldThenFame) {
    FakeGuildRepository repository;

    GuildJoinAttempt attempt = startingAttempt();
    attempt.stats.level = attempt.requirements.level - 1;
    attempt.stats.gold = 0;
    attempt.stats.fame = 0;
    auto tooLow = decideGuildJoinAttempt(repository, attempt);
    ASSERT_TRUE(tooLow.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_LEVEL_TOO_LOW, tooLow.rejection().reason);

    attempt.stats.level = attempt.requirements.level;
    auto tooPoor = decideGuildJoinAttempt(repository, attempt);
    ASSERT_TRUE(tooPoor.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD, tooPoor.rejection().reason);

    attempt.stats.gold = attempt.requirements.gold;
    auto tooObscure = decideGuildJoinAttempt(repository, attempt);
    ASSERT_TRUE(tooObscure.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_NOT_ENOUGH_FAME, tooObscure.rejection().reason);

    attempt.stats.fame = attempt.requirements.fame;
    EXPECT_TRUE(decideGuildJoinAttempt(repository, attempt).isOk());
}

TEST(GuildJoinAttemptDecision, ARaceThatIsAskedNoFameIgnoresIt) {
    FakeGuildRepository repository;

    GuildJoinAttempt attempt = startingAttempt();
    attempt.race = GUILD_JOIN_RACE_VAMPIRE;
    attempt.requirements.checkFame = false;
    attempt.requirements.fame = 999999999;
    attempt.stats.fame = 0;

    EXPECT_TRUE(decideGuildJoinAttempt(repository, attempt).isOk());
}

TEST(GuildJoinAttemptDecision, AnOrdinaryApplicantNeedsRoomOnTheWaitingList) {
    FakeGuildRepository repository;

    GuildJoinAttempt attempt = waitingAttempt();
    attempt.waitMemberCount = attempt.waitMemberLimit - 1;
    EXPECT_TRUE(decideGuildJoinAttempt(repository, attempt).isOk());

    attempt.waitMemberCount = attempt.waitMemberLimit;
    auto full = decideGuildJoinAttempt(repository, attempt);
    ASSERT_TRUE(full.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_WAIT_LIST_FULL, full.rejection().reason);
}

TEST(GuildJoinAttemptDecision, AnOrdinaryApplicantIsHeldToNoThresholds) {
    FakeGuildRepository repository;

    GuildJoinAttempt attempt = waitingAttempt();
    attempt.stats.level = 0;
    attempt.stats.gold = 0;
    attempt.stats.fame = 0;

    EXPECT_TRUE(decideGuildJoinAttempt(repository, attempt).isOk());
}

TEST(GuildJoinAttemptDecision, AnyOtherRankOpensNoDialogueAndIsRefusedNothing) {
    FakeGuildRepository repository;

    GuildJoinAttempt attempt = startingAttempt();
    attempt.rank = GUILD_JOIN_RANK_OTHER;
    attempt.stats.level = 0;
    attempt.stats.gold = 0;
    attempt.stats.fame = 0;
    attempt.waitMemberCount = attempt.waitMemberLimit;

    EXPECT_TRUE(decideGuildJoinAttempt(repository, attempt).isOk());
    // The membership probe still happens, exactly once.
    EXPECT_EQ(std::vector<std::string>{"loadMemberExpireDate(Aria)"}, repository.calls);
}

TEST(GuildJoinAttemptDecision, ReadsTheMembershipRowExactlyOnce) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    GuildJoinAttempt attempt = startingAttempt();
    attempt.now = penaltyEnd() - 1;

    EXPECT_TRUE(decideGuildJoinAttempt(repository, attempt).isRejected());
    EXPECT_EQ(std::vector<std::string>{"loadMemberExpireDate(Aria)"}, repository.calls);
}

//////////////////////////////////////////////////////////////////////////////
// decideGuildRegistration - CGRegistGuild.
//////////////////////////////////////////////////////////////////////////////

TEST(GuildRegistrationDecision, AQuoteOrBackslashIsRefusedBeforeAnythingIsRead) {
    FakeGuildRepository repository;

    GuildRegistrationRequest request = registrationRequest();
    request.guildName = "Night'watch";
    auto quoted = decideGuildRegistration(repository, request);
    ASSERT_TRUE(quoted.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_NAME_INVALID, quoted.rejection().reason);

    request.guildName = "Night\\watch";
    auto escaped = decideGuildRegistration(repository, request);
    ASSERT_TRUE(escaped.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_NAME_INVALID, escaped.rejection().reason);

    EXPECT_TRUE(repository.calls.empty());
}

TEST(GuildRegistrationDecision, ANameAlreadyHeldIsRefusedBeforeTheMembershipProbe) {
    FakeGuildRepository repository;
    repository.addGuildName(kGuildName);

    auto decision = decideGuildRegistration(repository, registrationRequest());

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_NAME_IN_USE, decision.rejection().reason);
    EXPECT_EQ(std::vector<std::string>{"guildNameInUse(Nightwatch)"}, repository.calls);
}

TEST(GuildRegistrationDecision, ACharacterWithNoMembershipRowHasNothingToClear) {
    FakeGuildRepository repository;

    auto decision = decideGuildRegistration(repository, registrationRequest());

    ASSERT_TRUE(decision.isOk());
    EXPECT_FALSE(decision.events().clearStaleMemberRow);

    std::vector<std::string> expected;
    expected.push_back("guildNameInUse(Nightwatch)");
    expected.push_back("loadMemberRankExpireDate(Aria)");
    EXPECT_EQ(expected, repository.calls);
}

TEST(GuildRegistrationDecision, APendingPenaltyOnAVoluntaryQuitIsRefused) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    GuildRegistrationRequest request = registrationRequest();
    request.now = penaltyEnd() - 1;

    auto decision = decideGuildRegistration(repository, request);

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_PENALTY_PENDING, decision.rejection().reason);

    // The refusal comes before the DELETE, so the stale row survives.
    std::vector<std::string> expected;
    expected.push_back("guildNameInUse(Nightwatch)");
    expected.push_back("loadMemberRankExpireDate(Aria)");
    EXPECT_EQ(expected, repository.calls);
}

TEST(GuildRegistrationDecision, AnExpelledCharacterMayFoundAGuildAtOnce) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankDeny, quitStamp());

    GuildRegistrationRequest request = registrationRequest();
    request.now = penaltyEnd() - 1;

    auto decision = decideGuildRegistration(repository, request);

    ASSERT_TRUE(decision.isOk());
    EXPECT_TRUE(decision.events().clearStaleMemberRow);
}

TEST(GuildRegistrationDecision, AnElapsedPenaltyLeavesOnlyTheStaleRowToClear) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    auto decision = decideGuildRegistration(repository, registrationRequest());

    ASSERT_TRUE(decision.isOk());
    EXPECT_TRUE(decision.events().clearStaleMemberRow);
}

TEST(GuildRegistrationDecision, ARowWithoutAStampIsClearedRatherThanRefused) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, 0);

    auto decision = decideGuildRegistration(repository, registrationRequest());

    ASSERT_TRUE(decision.isOk());
    EXPECT_TRUE(decision.events().clearStaleMemberRow);
}

//////////////////////////////////////////////////////////////////////////////
// decideGuildJoinConfirm - CGJoinGuild.
//////////////////////////////////////////////////////////////////////////////

TEST(GuildJoinConfirmDecision, ACharacterWithNoMembershipRowMayGoThrough) {
    FakeGuildRepository repository;

    auto decision = decideGuildJoinConfirm(repository, kPlayer, penaltyEnd(), kPenaltyDays);

    EXPECT_TRUE(decision.isOk());
    EXPECT_EQ(std::vector<std::string>{"loadMemberGuildRankExpireDate(Aria)"}, repository.calls);
}

TEST(GuildJoinConfirmDecision, APendingQuitPenaltyIsRefusedSilently) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    auto decision = decideGuildJoinConfirm(repository, kPlayer, penaltyEnd() - 1, kPenaltyDays);

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_PENALTY_PENDING, decision.rejection().reason);
    EXPECT_EQ(-1, codeFor(decision.rejection(), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(-1, codeFor(decision.rejection(), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(-1, codeFor(decision.rejection(), GUILD_JOIN_RACE_OUSTERS));
}

TEST(GuildJoinConfirmDecision, ALiveMembershipIsRefusedSilently) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, 0);

    auto decision = decideGuildJoinConfirm(repository, kPlayer, penaltyEnd(), kPenaltyDays);

    ASSERT_TRUE(decision.isRejected());
    EXPECT_EQ(GUILD_JOIN_REJECT_ALREADY_MEMBER, decision.rejection().reason);
    EXPECT_EQ(-1, codeFor(decision.rejection(), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(-1, codeFor(decision.rejection(), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(-1, codeFor(decision.rejection(), GUILD_JOIN_RACE_OUSTERS));
}

TEST(GuildJoinConfirmDecision, AnElapsedQuitPenaltyLetsTheJoinThrough) {
    FakeGuildRepository repository;
    repository.addMember(kPlayer, 7, kRankLeave, quitStamp());

    EXPECT_TRUE(decideGuildJoinConfirm(repository, kPlayer, penaltyEnd(), kPenaltyDays).isOk());
}

TEST(GuildJoinConfirmDecision, EveryRefusalOfEveryReasonIsSilent) {
    const GuildJoinReason reasons[] = {
        GUILD_JOIN_REJECT_GUILD_MISSING, GUILD_JOIN_REJECT_PENALTY_PENDING, GUILD_JOIN_REJECT_ALREADY_MEMBER,
        GUILD_JOIN_REJECT_LEVEL_TOO_LOW, GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD, GUILD_JOIN_REJECT_NOT_ENOUGH_FAME,
        GUILD_JOIN_REJECT_NAME_INVALID,  GUILD_JOIN_REJECT_NAME_IN_USE,     GUILD_JOIN_REJECT_WAIT_LIST_FULL};

    for (GuildJoinReason reason : reasons) {
        EXPECT_EQ(-1, codeFor(rejection(GUILD_JOIN_CONTEXT_CONFIRM, reason), GUILD_JOIN_RACE_SLAYER));
        EXPECT_EQ(-1, codeFor(rejection(GUILD_JOIN_CONTEXT_CONFIRM, reason), GUILD_JOIN_RACE_VAMPIRE));
        EXPECT_EQ(-1, codeFor(rejection(GUILD_JOIN_CONTEXT_CONFIRM, reason), GUILD_JOIN_RACE_OUSTERS));
    }
}

//////////////////////////////////////////////////////////////////////////////
// decideGuildRequirements on its own.
//////////////////////////////////////////////////////////////////////////////

TEST(GuildRequirements, AreMetAtTheThresholdItself) {
    GuildJoinStats stats;
    stats.level = 30;
    stats.gold = 10000000;
    stats.fame = 100000;

    GuildJoinRequirements requirements;
    requirements.level = 30;
    requirements.gold = 10000000;
    requirements.fame = 100000;
    requirements.checkFame = true;

    EXPECT_TRUE(decideGuildRequirements(GUILD_JOIN_CONTEXT_REGIST, stats, requirements).isOk());
}

TEST(GuildRequirements, CarryTheContextTheyWereAskedIn) {
    GuildJoinStats stats;
    GuildJoinRequirements requirements;
    requirements.level = 1;

    auto starting = decideGuildRequirements(GUILD_JOIN_CONTEXT_STARTING, stats, requirements);
    ASSERT_TRUE(starting.isRejected());
    EXPECT_EQ(GUILD_JOIN_CONTEXT_STARTING, starting.rejection().context);

    auto regist = decideGuildRequirements(GUILD_JOIN_CONTEXT_REGIST, stats, requirements);
    ASSERT_TRUE(regist.isRejected());
    EXPECT_EQ(GUILD_JOIN_CONTEXT_REGIST, regist.rejection().context);
}

//////////////////////////////////////////////////////////////////////////////
// The refusal-to-response-code mapping, race by race.
//////////////////////////////////////////////////////////////////////////////

TEST(GuildJoinResponseCode, TheStartingDialogueAnswersEachRaceInItsOwnWords) {
    EXPECT_EQ(
        NPC_RESPONSE_TEAM_STARTING_FAIL_QUIT_TIMEOUT,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_PENALTY_PENDING), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(
        NPC_RESPONSE_CLAN_STARTING_FAIL_QUIT_TIMEOUT,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_PENALTY_PENDING), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(
        NPC_RESPONSE_GUILD_STARTING_FAIL_QUIT_TIMEOUT,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_PENALTY_PENDING), GUILD_JOIN_RACE_OUSTERS));

    EXPECT_EQ(
        NPC_RESPONSE_TEAM_STARTING_FAIL_ALREADY_JOIN,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_ALREADY_MEMBER), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(
        NPC_RESPONSE_CLAN_STARTING_FAIL_ALREADY_JOIN,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_ALREADY_MEMBER), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(
        NPC_RESPONSE_GUILD_STARTING_FAIL_ALREADY_JOIN,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_ALREADY_MEMBER), GUILD_JOIN_RACE_OUSTERS));

    EXPECT_EQ(NPC_RESPONSE_TEAM_STARTING_FAIL_LEVEL,
              codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_LEVEL_TOO_LOW), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(
        NPC_RESPONSE_CLAN_STARTING_FAIL_LEVEL,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_LEVEL_TOO_LOW), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(
        NPC_RESPONSE_GUILD_STARTING_FAIL_LEVEL,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_LEVEL_TOO_LOW), GUILD_JOIN_RACE_OUSTERS));

    EXPECT_EQ(
        NPC_RESPONSE_TEAM_STARTING_FAIL_MONEY,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(
        NPC_RESPONSE_CLAN_STARTING_FAIL_MONEY,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(
        NPC_RESPONSE_GUILD_STARTING_FAIL_MONEY,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD), GUILD_JOIN_RACE_OUSTERS));

    // Only a Slayer is asked for fame today, so only the TEAM_ code can
    // reach a client; the other two are here because the mapping is total.
    EXPECT_EQ(
        NPC_RESPONSE_TEAM_STARTING_FAIL_FAME,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_NOT_ENOUGH_FAME), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(
        NPC_RESPONSE_CLAN_STARTING_FAIL_FAME,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_NOT_ENOUGH_FAME), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(
        NPC_RESPONSE_GUILD_STARTING_FAIL_FAME,
        codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_NOT_ENOUGH_FAME), GUILD_JOIN_RACE_OUSTERS));
}

TEST(GuildJoinResponseCode, TheStartingDialogueJustClosesForAMissingGuildOrAFullList) {
    const GuildJoinRace races[] = {GUILD_JOIN_RACE_SLAYER, GUILD_JOIN_RACE_VAMPIRE, GUILD_JOIN_RACE_OUSTERS};

    for (GuildJoinRace race : races) {
        EXPECT_EQ(NPC_RESPONSE_QUIT_DIALOGUE,
                  codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_GUILD_MISSING), race));
        EXPECT_EQ(NPC_RESPONSE_QUIT_DIALOGUE,
                  codeFor(rejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_WAIT_LIST_FULL), race));
    }
}

TEST(GuildJoinResponseCode, TheRegistrationDialogueNamesTheNameAndTheDenial) {
    EXPECT_EQ(NPC_RESPONSE_TEAM_REGIST_FAIL_NAME,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_INVALID), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(NPC_RESPONSE_CLAN_REGIST_FAIL_NAME,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_INVALID), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(NPC_RESPONSE_GUILD_REGIST_FAIL_NAME,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_INVALID), GUILD_JOIN_RACE_OUSTERS));

    // A name that is merely taken gets the same answer as one that could
    // never be stored.
    EXPECT_EQ(NPC_RESPONSE_TEAM_REGIST_FAIL_NAME,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_IN_USE), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(NPC_RESPONSE_CLAN_REGIST_FAIL_NAME,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_IN_USE), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(NPC_RESPONSE_GUILD_REGIST_FAIL_NAME,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_IN_USE), GUILD_JOIN_RACE_OUSTERS));

    EXPECT_EQ(NPC_RESPONSE_TEAM_REGIST_FAIL_DENY,
              codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_PENALTY_PENDING), GUILD_JOIN_RACE_SLAYER));
    EXPECT_EQ(
        NPC_RESPONSE_CLAN_REGIST_FAIL_DENY,
        codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_PENALTY_PENDING), GUILD_JOIN_RACE_VAMPIRE));
    EXPECT_EQ(
        NPC_RESPONSE_GUILD_REGIST_FAIL_DENY,
        codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_PENALTY_PENDING), GUILD_JOIN_RACE_OUSTERS));
}

TEST(GuildJoinResponseCode, TheRegistrationDialogueSaysNothingAboutMissedThresholds) {
    const GuildJoinRace races[] = {GUILD_JOIN_RACE_SLAYER, GUILD_JOIN_RACE_VAMPIRE, GUILD_JOIN_RACE_OUSTERS};

    for (GuildJoinRace race : races) {
        EXPECT_EQ(-1, codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_LEVEL_TOO_LOW), race));
        EXPECT_EQ(-1, codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD), race));
        EXPECT_EQ(-1, codeFor(rejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NOT_ENOUGH_FAME), race));
    }
}

} // namespace
