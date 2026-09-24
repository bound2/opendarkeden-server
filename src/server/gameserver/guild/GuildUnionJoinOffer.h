//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionJoinOffer.h
// Description : whether a guild may offer to join the union another guild
//               leads, and when a union has nobody left to exist for, kept
//               apart from the union manager so the rules can be exercised
//               with neither the union tables nor a database.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_UNION_JOIN_OFFER_H__
#define __GUILD_UNION_JOIN_OFFER_H__

// What the union manager has read, when a guild (the applicant) offers to
// join the union a target guild leads, about the two guilds and that union.
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
    // one row per guild, so this includes an ESCAPE row.
    bool applicantHasOffer = false;
    // The applicant was forced out of a union within the penalty period.
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
    ALREADY_OFFER_SOMETHING,
    YOU_HAVE_PENALTY,
    NOT_ENOUGH_SLOT
};

// The first refusal that applies, or which way the offer is recorded. Every
// refusal is decided before a union is opened, so a refused offer leaves no
// union behind. A target that leads no union has no members yet, so the slot
// check refuses it only when the limit leaves no room at all.
UnionJoinOfferVerdict decideUnionJoinOffer(const UnionJoinOfferFacts& facts);

// A union exists for its member guilds and for the guilds asking to join it.
// One with no member row and no pending join offer is abandoned: the master
// guild alone is not a union, and nothing would ever fill it.
bool unionIsAbandoned(int memberRows, int pendingJoinOffers);

#endif
