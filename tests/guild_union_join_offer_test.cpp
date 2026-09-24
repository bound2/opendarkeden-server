// Whether a guild may offer to join the union another guild leads, and when
// a union has nobody left to exist for
// (src/server/gameserver/guild/GuildUnionJoinOffer.cpp). The union manager
// applies the answer under its own lock and only opens a union for an offer
// the rule lets through, so every refusal here is one that leaves no union
// behind. The manager itself is not linked: it needs the union tables.

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
    EXPECT_EQ(UnionJoinOfferVerdict::ALREADY_OFFER_SOMETHING, decideUnionJoinOffer(facts))
        << "an ESCAPE row is an offer row too, so a penalised guild hears this";

    facts.applicantHasOffer = false;
    EXPECT_EQ(UnionJoinOfferVerdict::YOU_HAVE_PENALTY, decideUnionJoinOffer(facts));

    facts.applicantHasPenalty = false;
    EXPECT_EQ(UnionJoinOfferVerdict::NOT_ENOUGH_SLOT, decideUnionJoinOffer(facts));
}

// A union lives while it has a member guild or a guild asking to join it.
TEST(GuildUnionJoinOfferTest, AUnionWithNobodyLeftIsAbandoned) {
    EXPECT_TRUE(unionIsAbandoned(0, 0));
    EXPECT_FALSE(unionIsAbandoned(1, 0)) << "a member keeps it";
    EXPECT_FALSE(unionIsAbandoned(0, 1)) << "a pending join offer keeps it";
    EXPECT_FALSE(unionIsAbandoned(2, 3));
}

} // namespace
