//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionJoinOffer.h
// Description : the rules of the guild union offers -- whether a guild may
//               offer to join the union another guild leads, who may answer
//               a pending offer, which offer rows a purge takes away, and
//               when a union has nobody left to exist for -- kept apart from
//               the union manager so the rules can be exercised with neither
//               the union tables nor a database.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_UNION_JOIN_OFFER_H__
#define __GUILD_UNION_JOIN_OFFER_H__

#include <cstddef>
#include <vector>

// How long a GuildUnionOffer row lives, in days: a JOIN or QUIT offer nobody
// answered, and the ESCAPE penalty. GuildUnion.h states the rule.
const int kUnionOfferLifetimeDays = 10;

// GuildUnionOffer.OfferType read as a number (OfferType+0): the ordinals of
// the column's enum of JOIN, QUIT and ESCAPE.
enum UnionOfferType { UNION_OFFER_JOIN = 1, UNION_OFFER_QUIT = 2, UNION_OFFER_ESCAPE = 3 };

// What the union manager has read, when a guild (the applicant) offers to
// join the union a target guild leads, about the two guilds and that union.
// The expired offer rows have been purged before these are read.
struct UnionJoinOfferFacts {
    enum TargetStanding {
        // The target guild is in no union: the offer opens one for it.
        TARGET_IN_NO_UNION,
        // The target guild is the master of a union.
        TARGET_LEADS_UNION,
        // The target guild belongs to a union as a member.
        TARGET_IS_MEMBER
    };

    // The applicant belongs to a union already, as master or member.
    bool applicantInUnion = false;
    // Either guild has more active members than a union guild may have.
    bool tooManyMembers = false;
    TargetStanding targetStanding = TARGET_IN_NO_UNION;
    // The applicant has a GuildUnionOffer row of any type. The table holds
    // one row per guild, so this includes an ESCAPE row, which the penalty
    // check answers first.
    bool applicantHasOffer = false;
    // The applicant was forced out of a union within the penalty period: it
    // has an ESCAPE row younger than kUnionOfferLifetimeDays.
    bool applicantHasPenalty = false;
    // The member rows of the target's union; 0 when it leads none.
    int unionMemberCount = 0;
    // How many member guilds a union may have (GUILD_UNION_MAX).
    int unionMemberLimit = 0;
};

enum class UnionJoinOfferVerdict {
    // The target leads a union: record the offer against it.
    RECORD_OFFER,
    // The target leads none: open a union for it, then record the offer.
    OPEN_UNION_AND_RECORD,
    // The refusals, in the order they are checked.
    ALREADY_IN_UNION,
    TOO_MANY_MEMBER,
    TARGET_IS_NOT_MASTER,
    YOU_HAVE_PENALTY,
    ALREADY_OFFER_SOMETHING,
    NOT_ENOUGH_SLOT
};

// The first refusal that applies, or which way the offer is recorded. Every
// refusal is decided before a union is opened, so a refused offer leaves no
// union behind. The penalty is answered before the pending-offer check: the
// ESCAPE row that carries it is an offer row too, and a penalised guild is
// told why it is refused. A target that leads no union has no members yet,
// so the slot check refuses it only when the limit leaves no room at all.
UnionJoinOfferVerdict decideUnionJoinOffer(const UnionJoinOfferFacts& facts);

// A union exists for its member guilds and for the guilds asking to join it.
// One with no member row and no pending join offer is abandoned: the master
// guild alone is not a union, and nothing would ever fill it.
bool unionIsAbandoned(int memberRows, int pendingJoinOffers);

// A union master answering a guild's pending offer (accepting or denying a
// JOIN or a QUIT).
enum class UnionOfferAnswerVerdict {
    // The offer names the answering master's union: act on it.
    ANSWER,
    // The guild has no pending offer of that kind: none was made, it was
    // answered already, or it expired.
    NO_OFFER,
    // The offer names another union; it stays for that union's master.
    NOT_YOUR_UNION
};

// `offerUnionID` is the union the guild's offer names, read only when
// `offerFound`; `answeringUnionID` is the union the answering master leads.
UnionOfferAnswerVerdict decideUnionOfferAnswer(bool offerFound, unsigned offerUnionID, unsigned answeringUnionID);

// One GuildUnionOffer row, as the purge reads it.
struct UnionOfferState {
    unsigned unionID = 0;
    // UnionOfferType.
    int offerType = 0;
    unsigned ownerGuildID = 0;
    // Older than kUnionOfferLifetimeDays.
    bool expired = false;
};

// What a purge of the offer rows takes away.
struct UnionOfferPurge {
    // Rows past their lifetime, of any type, as indexes into the rows read.
    std::vector<std::size_t> expired;
    // Live JOIN and QUIT rows naming a union that does not exist: an offer
    // to a union that is gone can be neither accepted nor denied. An ESCAPE
    // row is the guild's penalty, not the union's, and stays its time.
    std::vector<std::size_t> orphaned;
    // The existing unions an expired JOIN row named, each once: losing that
    // offer may leave one abandoned (unionIsAbandoned).
    std::vector<unsigned> unionsToCheck;
};

// `unionIDs` are the unions that exist, read after `offers`: an offer row is
// written only once its union's row is, so a row naming a union missing from
// the later read names one that has gone.
UnionOfferPurge decideUnionOfferPurge(const std::vector<UnionOfferState>& offers,
                                      const std::vector<unsigned>& unionIDs);

#endif
