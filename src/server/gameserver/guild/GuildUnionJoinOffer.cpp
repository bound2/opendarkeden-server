//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionJoinOffer.cpp
// Description : decideUnionJoinOffer, unionIsAbandoned,
//               decideUnionOfferAnswer, decideUnionOfferPurge
//////////////////////////////////////////////////////////////////////////////

#include "GuildUnionJoinOffer.h"

#include <algorithm>

UnionJoinOfferVerdict decideUnionJoinOffer(const UnionJoinOfferFacts& facts) {
    if (facts.applicantInUnion)
        return UnionJoinOfferVerdict::ALREADY_IN_UNION;

    if (facts.tooManyMembers)
        return UnionJoinOfferVerdict::TOO_MANY_MEMBER;

    if (facts.targetStanding == UnionJoinOfferFacts::TARGET_IS_MEMBER)
        return UnionJoinOfferVerdict::TARGET_IS_NOT_MASTER;

    if (facts.applicantHasPenalty)
        return UnionJoinOfferVerdict::YOU_HAVE_PENALTY;

    if (facts.applicantHasOffer)
        return UnionJoinOfferVerdict::ALREADY_OFFER_SOMETHING;

    if (facts.unionMemberCount >= facts.unionMemberLimit)
        return UnionJoinOfferVerdict::NOT_ENOUGH_SLOT;

    if (facts.targetStanding == UnionJoinOfferFacts::TARGET_IN_NO_UNION)
        return UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD;

    return UnionJoinOfferVerdict::RECORD_OFFER;
}

bool unionIsAbandoned(int memberRows, int pendingJoinOffers) {
    return memberRows <= 0 && pendingJoinOffers <= 0;
}

UnionOfferAnswerVerdict decideUnionOfferAnswer(bool offerFound, unsigned offerUnionID, unsigned answeringUnionID) {
    if (!offerFound)
        return UnionOfferAnswerVerdict::NO_OFFER;

    if (offerUnionID != answeringUnionID)
        return UnionOfferAnswerVerdict::NOT_YOUR_UNION;

    return UnionOfferAnswerVerdict::ANSWER;
}

UnionOfferPurge decideUnionOfferPurge(const std::vector<UnionOfferState>& offers,
                                      const std::vector<unsigned>& unionIDs) {
    UnionOfferPurge purge;

    const auto contains = [](const std::vector<unsigned>& ids, unsigned id) {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    };

    for (std::size_t o = 0; o < offers.size(); o++) {
        const UnionOfferState& offer = offers[o];
        const bool namesAUnion = contains(unionIDs, offer.unionID);

        if (offer.expired) {
            purge.expired.push_back(o);

            if (offer.offerType == UNION_OFFER_JOIN && namesAUnion && !contains(purge.unionsToCheck, offer.unionID))
                purge.unionsToCheck.push_back(offer.unionID);
        } else if ((offer.offerType == UNION_OFFER_JOIN || offer.offerType == UNION_OFFER_QUIT) && !namesAUnion) {
            purge.orphaned.push_back(o);
        }
    }

    return purge;
}
