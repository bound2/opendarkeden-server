// The union offer rules (src/server/gameserver/guild/GuildUnionJoinOffer.cpp):
// whether a guild may offer to join the union another guild leads, who may
// answer a pending offer, which offer rows a purge takes away, and when a
// union has nobody left to exist for. The union manager applies the answer
// under its own lock and only opens a union for an offer the rule lets
// through, so every refusal here is one that leaves no union behind. The
// manager itself is not linked: it needs the union tables.

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "GuildUnionJoinOffer.h"

namespace {

// An offer nothing refuses, to a guild that already leads a union with room.
UnionJoinOfferFacts admissible() {
    UnionJoinOfferFacts facts;
    facts.targetStanding = UnionJoinOfferFacts::TARGET_LEADS_UNION;
    facts.unionMemberCount = 3;
    facts.unionMemberLimit = 5;
    return facts;
}

TEST(GuildUnionJoinOfferTest, AnOfferToAUnionWithRoomIsRecorded) {
    EXPECT_EQ(UnionJoinOfferVerdict::RECORD_OFFER, decideUnionJoinOffer(admissible()));
}

// The target guild leads no union yet: the offer opens one, and only once
// nothing refuses it.
TEST(GuildUnionJoinOfferTest, AnOfferToAGuildInNoUnionOpensOne) {
    UnionJoinOfferFacts facts = admissible();
    facts.targetStanding = UnionJoinOfferFacts::TARGET_IN_NO_UNION;
    facts.unionMemberCount = 0;

    EXPECT_EQ(UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD, decideUnionJoinOffer(facts));
}

// Each refusal on its own, against a target that leads no union: the old
// code opened the union before the last three and left it empty.
TEST(GuildUnionJoinOfferTest, EveryRefusalIsDecidedBeforeAUnionIsOpened) {
    UnionJoinOfferFacts base = admissible();
    base.targetStanding = UnionJoinOfferFacts::TARGET_IN_NO_UNION;
    base.unionMemberCount = 0;

    UnionJoinOfferFacts facts = base;
    facts.applicantInUnion = true;
    EXPECT_EQ(UnionJoinOfferVerdict::ALREADY_IN_UNION, decideUnionJoinOffer(facts));

    facts = base;
    facts.tooManyMembers = true;
    EXPECT_EQ(UnionJoinOfferVerdict::TOO_MANY_MEMBER, decideUnionJoinOffer(facts));

    facts = base;
    facts.applicantHasOffer = true;
    EXPECT_EQ(UnionJoinOfferVerdict::ALREADY_OFFER_SOMETHING, decideUnionJoinOffer(facts));

    facts = base;
    facts.applicantHasPenalty = true;
    EXPECT_EQ(UnionJoinOfferVerdict::YOU_HAVE_PENALTY, decideUnionJoinOffer(facts));

    facts = base;
    facts.unionMemberLimit = 0;
    EXPECT_EQ(UnionJoinOfferVerdict::NOT_ENOUGH_SLOT, decideUnionJoinOffer(facts))
        << "a limit that leaves no room refuses before a union is opened";
}

// A guild that is only a member of a union cannot be offered to: its union
// is someone else's.
TEST(GuildUnionJoinOfferTest, TheTargetMustLeadItsUnion) {
    UnionJoinOfferFacts facts = admissible();
    facts.targetStanding = UnionJoinOfferFacts::TARGET_IS_MEMBER;

    EXPECT_EQ(UnionJoinOfferVerdict::TARGET_IS_NOT_MASTER, decideUnionJoinOffer(facts));
}

TEST(GuildUnionJoinOfferTest, AFullUnionTakesNoOffer) {
    UnionJoinOfferFacts facts = admissible();
    facts.unionMemberCount = 5;

    EXPECT_EQ(UnionJoinOfferVerdict::NOT_ENOUGH_SLOT, decideUnionJoinOffer(facts));

    facts.unionMemberCount = 4;
    EXPECT_EQ(UnionJoinOfferVerdict::RECORD_OFFER, decideUnionJoinOffer(facts));
}

// The refusals keep the order the client has always been answered in: with
// several reasons at once, the first of them is the one it is told.
TEST(GuildUnionJoinOfferTest, TheFirstRefusalInOrderIsTheAnswer) {
    UnionJoinOfferFacts facts = admissible();
    facts.applicantInUnion = true;
    facts.tooManyMembers = true;
    facts.targetStanding = UnionJoinOfferFacts::TARGET_IS_MEMBER;
    facts.applicantHasOffer = true;
    facts.applicantHasPenalty = true;
    facts.unionMemberCount = 5;
    EXPECT_EQ(UnionJoinOfferVerdict::ALREADY_IN_UNION, decideUnionJoinOffer(facts));

    facts.applicantInUnion = false;
    EXPECT_EQ(UnionJoinOfferVerdict::TOO_MANY_MEMBER, decideUnionJoinOffer(facts));

    facts.tooManyMembers = false;
    EXPECT_EQ(UnionJoinOfferVerdict::TARGET_IS_NOT_MASTER, decideUnionJoinOffer(facts));

    facts.targetStanding = UnionJoinOfferFacts::TARGET_LEADS_UNION;
    EXPECT_EQ(UnionJoinOfferVerdict::YOU_HAVE_PENALTY, decideUnionJoinOffer(facts));

    facts.applicantHasPenalty = false;
    EXPECT_EQ(UnionJoinOfferVerdict::ALREADY_OFFER_SOMETHING, decideUnionJoinOffer(facts));

    facts.applicantHasOffer = false;
    EXPECT_EQ(UnionJoinOfferVerdict::NOT_ENOUGH_SLOT, decideUnionJoinOffer(facts));
}

// The ESCAPE row that carries the penalty is an offer row too, since the
// table keeps one row per guild: a penalised guild has an offer on record
// and is told about the penalty, not about the offer.
TEST(GuildUnionJoinOfferTest, APenalisedGuildHearsOfItsPenalty) {
    UnionJoinOfferFacts facts = admissible();
    facts.applicantHasOffer = true;
    facts.applicantHasPenalty = true;

    EXPECT_EQ(UnionJoinOfferVerdict::YOU_HAVE_PENALTY, decideUnionJoinOffer(facts));

    // Once the penalty has run its ten days the purge takes the row, and the
    // guild may offer again.
    facts.applicantHasOffer = false;
    facts.applicantHasPenalty = false;
    EXPECT_EQ(UnionJoinOfferVerdict::RECORD_OFFER, decideUnionJoinOffer(facts));
}

// Every row the table holds lives the same ten days.
TEST(GuildUnionJoinOfferTest, AnOfferLivesTenDays) {
    EXPECT_EQ(10, kUnionOfferLifetimeDays);
}

//////////////////////////////////////////////////////////////////////////////
// Answering a pending offer
//////////////////////////////////////////////////////////////////////////////

TEST(GuildUnionJoinOfferTest, AMasterAnswersAnOfferToItsOwnUnion) {
    EXPECT_EQ(UnionOfferAnswerVerdict::ANSWER, decideUnionOfferAnswer(true, 42, 42));
}

// Any union master used to accept or clear the offer of whatever guild the
// packet named; an offer to another union now stays for that union's master.
TEST(GuildUnionJoinOfferTest, AnOfferToAnotherUnionIsNotYours) {
    EXPECT_EQ(UnionOfferAnswerVerdict::NOT_YOUR_UNION, decideUnionOfferAnswer(true, 42, 7));
}

// No offer, an answered one or an expired one: there is nothing to act on,
// whichever union the answer comes from.
TEST(GuildUnionJoinOfferTest, NoOfferIsNothingToAnswer) {
    EXPECT_EQ(UnionOfferAnswerVerdict::NO_OFFER, decideUnionOfferAnswer(false, 0, 7));
    EXPECT_EQ(UnionOfferAnswerVerdict::NO_OFFER, decideUnionOfferAnswer(false, 7, 7))
        << "the union id is not read when no offer was found";
}

//////////////////////////////////////////////////////////////////////////////
// Purging the offer rows
//////////////////////////////////////////////////////////////////////////////

UnionOfferState offer(unsigned unionID, int type, unsigned owner, bool expired) {
    UnionOfferState state;
    state.unionID = unionID;
    state.offerType = type;
    state.ownerGuildID = owner;
    state.expired = expired;
    return state;
}

typedef std::vector<std::size_t> Rows;
typedef std::vector<unsigned> Unions;

TEST(GuildUnionJoinOfferTest, NothingLiveIsPurged) {
    const std::vector<UnionOfferState> offers{offer(1, UNION_OFFER_JOIN, 100, false),
                                              offer(1, UNION_OFFER_QUIT, 101, false),
                                              offer(2, UNION_OFFER_ESCAPE, 102, false)};

    const UnionOfferPurge purge = decideUnionOfferPurge(offers, Unions{1, 2});

    EXPECT_TRUE(purge.expired.empty());
    EXPECT_TRUE(purge.orphaned.empty());
    EXPECT_TRUE(purge.unionsToCheck.empty());
}

// An expired row goes whatever its type. Only an expired JOIN can have been
// what kept a union alive, so only its union is checked, and once.
TEST(GuildUnionJoinOfferTest, EveryExpiredRowGoesAndAnExpiredJoinChecksItsUnion) {
    const std::vector<UnionOfferState> offers{
        offer(1, UNION_OFFER_JOIN, 100, true),  offer(1, UNION_OFFER_JOIN, 101, true),
        offer(2, UNION_OFFER_QUIT, 102, true),  offer(3, UNION_OFFER_ESCAPE, 103, true),
        offer(4, UNION_OFFER_JOIN, 104, false),
    };

    const UnionOfferPurge purge = decideUnionOfferPurge(offers, Unions{1, 2, 3, 4});

    EXPECT_EQ((Rows{0, 1, 2, 3}), purge.expired);
    EXPECT_TRUE(purge.orphaned.empty());
    EXPECT_EQ(Unions{1}, purge.unionsToCheck);
}

// A live JOIN or QUIT naming a union that has gone can be neither accepted
// nor denied, so it goes too; an ESCAPE row is the guild's penalty and stays
// its time whatever became of the union it left.
TEST(GuildUnionJoinOfferTest, ALiveOfferToAUnionThatHasGoneIsDropped) {
    const std::vector<UnionOfferState> offers{
        offer(28, UNION_OFFER_JOIN, 4950, false), offer(28, UNION_OFFER_QUIT, 4951, false),
        offer(28, UNION_OFFER_ESCAPE, 4952, false), offer(30, UNION_OFFER_JOIN, 4953, false)};

    const UnionOfferPurge purge = decideUnionOfferPurge(offers, Unions{30});

    EXPECT_TRUE(purge.expired.empty());
    EXPECT_EQ((Rows{0, 1}), purge.orphaned);
    EXPECT_TRUE(purge.unionsToCheck.empty());
}

// An expired JOIN to a union that has gone is simply expired: there is no
// union left to check.
TEST(GuildUnionJoinOfferTest, AnExpiredOfferToAUnionThatHasGoneChecksNothing) {
    const std::vector<UnionOfferState> offers{offer(28, UNION_OFFER_JOIN, 4950, true)};

    const UnionOfferPurge purge = decideUnionOfferPurge(offers, Unions{30});

    EXPECT_EQ(Rows{0}, purge.expired);
    EXPECT_TRUE(purge.orphaned.empty());
    EXPECT_TRUE(purge.unionsToCheck.empty());
}

// A union lives while it has a member guild or a guild asking to join it.
TEST(GuildUnionJoinOfferTest, AUnionWithNobodyLeftIsAbandoned) {
    EXPECT_TRUE(unionIsAbandoned(0, 0));
    EXPECT_FALSE(unionIsAbandoned(1, 0)) << "a member keeps it";
    EXPECT_FALSE(unionIsAbandoned(0, 1)) << "a pending join offer keeps it";
    EXPECT_FALSE(unionIsAbandoned(2, 3));
}

} // namespace
