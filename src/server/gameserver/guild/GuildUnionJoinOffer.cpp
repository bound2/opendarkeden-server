//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionJoinOffer.cpp
// Description : decideUnionJoinOffer, unionIsAbandoned
//////////////////////////////////////////////////////////////////////////////

#include "GuildUnionJoinOffer.h"

UnionJoinOfferVerdict decideUnionJoinOffer(const UnionJoinOfferFacts& facts) {
    if (facts.applicantInUnion)
        return UnionJoinOfferVerdict::ALREADY_IN_UNION;

    if (facts.tooManyMembers)
        return UnionJoinOfferVerdict::TOO_MANY_MEMBER;

    if (facts.targetStanding == UnionJoinOfferFacts::TARGET_IS_MEMBER)
        return UnionJoinOfferVerdict::TARGET_IS_NOT_MASTER;

    if (facts.applicantHasOffer)
        return UnionJoinOfferVerdict::ALREADY_OFFER_SOMETHING;

    if (facts.applicantHasPenalty)
        return UnionJoinOfferVerdict::YOU_HAVE_PENALTY;

    if (facts.unionMemberCount >= facts.unionMemberLimit)
        return UnionJoinOfferVerdict::NOT_ENOUGH_SLOT;

    if (facts.targetStanding == UnionJoinOfferFacts::TARGET_IN_NO_UNION)
        return UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD;

    return UnionJoinOfferVerdict::RECORD_OFFER;
}

bool unionIsAbandoned(int memberRows, int pendingJoinOffers) {
    return memberRows <= 0 && pendingJoinOffers <= 0;
}
