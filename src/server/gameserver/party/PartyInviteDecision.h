//////////////////////////////////////////////////////////////////////////////
// Filename    : PartyInviteDecision.h
// Description : the party invite protocol's rules, kept apart from the
//               handler so they can be exercised with neither a creature nor
//               a zone. Everything here needs a PartyInviteTopology and plain
//               values only.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PARTY_INVITE_DECISION_H__
#define __PARTY_INVITE_DECISION_H__

#include "Outcome.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Vocabulary
//////////////////////////////////////////////////////////////////////////////

// Who a packet goes to. The requester is the player whose CGPartyInvite this
// is; the target is the creature the packet names.
enum class PartyInvitePeer { Requester, Target };

// Why the request was refused. Each carries one packet to the requester,
// except UnknownCode, which is a client that does not speak the protocol.
enum class PartyInviteReason {
    // GCPartyError(GC_PARTY_ERROR_TARGET_NOT_EXIST). The named creature is
    // not in the zone.
    TargetMissing,
    // GCPartyError(GC_PARTY_ERROR_RACE_DIFFER). The target is a monster or
    // an NPC, or a player character of another race.
    RaceDiffer,
    // GCPartyInvite(GC_PARTY_INVITE_BUSY). The target is already in an
    // invite with somebody.
    TargetBusy,
    // GCPartyInvite(GC_PARTY_INVITE_ANOTHER_PARTY). Two parties cannot be
    // merged by inviting across them.
    BothInParties,
    // GCPartyInvite(GC_PARTY_INVITE_MEMBER_FULL). One of the two parties is
    // at PARTY_MAX_SIZE.
    PartyFull,
    // GCPartyError(GC_PARTY_ERROR_NOT_INVITING). An answer to an invite
    // that is not open.
    NotInviting,
    // Nothing is sent; the caller throws ProtocolException.
    UnknownCode
};

// A refusal. Every one of them answers the requester, echoing the target
// object id the request carried.
struct PartyInviteRejection {
    PartyInviteRejection(PartyInviteReason rejectReason, bool sendError, BYTE rejectCode, bool clearInvite)
        : reason(rejectReason), isPartyError(sendError), code(rejectCode), cancelRequesterInvite(clearInvite) {}

    PartyInviteReason reason;
    // True for a GCPartyError, false for a GCPartyInvite. Meaningless for
    // UnknownCode, which sends nothing.
    bool isPartyError;
    // GC_PARTY_ERROR_* or GC_PARTY_INVITE_*, matching isPartyError.
    BYTE code;
    // Drop whatever invite the requester was in, before the packet goes out.
    bool cancelRequesterInvite;
};

// What an accepted invite does to the two parties.
enum class PartyJoin {
    None,
    // The requester joins the party the target is already in.
    RequesterJoinsTargetParty,
    // The target joins the party the requester is already in.
    TargetJoinsRequesterParty,
    // Neither is in a party the other can join, so a new one is created
    // with both in it.
    CreateParty
};

// Which side of a newly created party gets the quest event, if either does.
enum class PartyQuestEvent { None, Requester, Target };

// A character of this level or over triggers the quest event when it parties
// with one below it.
const int kPartyQuestLevel = 25;

// What the caller performs, in this order: the packet, then the party
// mutations, then the invite record.
struct PartyInviteEvents {
    // A GCPartyInvite to one of the two peers, carrying sendObjectID and
    // sendCode.
    bool sendInvite = false;
    PartyInvitePeer sendTo = PartyInvitePeer::Requester;
    ObjectID_t sendObjectID = 0;
    // One of GC_PARTY_INVITE_*.
    BYTE sendCode = 0;

    // Open an invite between the two, so the target may answer it.
    bool initInvite = false;

    // The party mutation. joinPartyID names the party the newcomer is added
    // to, and is meaningless for CreateParty: the caller registers a fresh
    // id for that.
    PartyJoin join = PartyJoin::None;
    int joinPartyID = 0;

    // Only a newly created party raises this.
    PartyQuestEvent questEvent = PartyQuestEvent::None;

    // Close the invite between the two.
    bool cancelInvite = false;
};

//////////////////////////////////////////////////////////////////////////////
// The request
//////////////////////////////////////////////////////////////////////////////

// A CGPartyInvite and the two creatures it is about.
//
// requesterPartyID, targetPartyID and the two levels are read up front
// although only some branches use them: they are plain reads off a creature
// with no lock and no side effect, unlike the topology's three queries.
struct PartyInviteRequest {
    // One of CG_PARTY_INVITE_*.
    BYTE code = 0;
    ObjectID_t requesterObjectID = 0;
    // The packet's field, echoed back on every refusal.
    ObjectID_t targetObjectID = 0;
    // Is a creature of that object id in the requester's zone?
    bool targetExists = false;
    // Is it a player character of the requester's race?
    bool targetIsSameRacePC = false;
    // 0 for a character in no party.
    int requesterPartyID = 0;
    int targetPartyID = 0;
    int requesterLevel = 0;
    int targetLevel = 0;
};

// The invite records and the party sizes a decision reads. These are the
// reads that take a lock, so the order they are made in is part of the
// contract.
class PartyInviteTopology {
public:
    virtual ~PartyInviteTopology() {}

    // Is the target already in an invite, with anybody?
    // (PartyInviteInfoManager::getInviteInfo of the target's name.)
    virtual bool targetHasInviteInfo() = 0;
    // Is an invite open from the requester to the target?
    // (PartyInviteInfoManager::isInviting.)
    virtual bool isInviting() = 0;
    // Is the party under PARTY_MAX_SIZE?
    // (GlobalPartyManager::canAddMember.)
    virtual bool canAddMember(int partyID) = 0;
};

//////////////////////////////////////////////////////////////////////////////
// The decision
//////////////////////////////////////////////////////////////////////////////

// What does this CGPartyInvite do?
//
// A request that names no creature, or one that is not a player character of
// the same race, is refused before anything is asked of the topology. Past
// that:
//
//   REQUEST  refused when the target is in another invite, when both are
//            already in parties, and when either party is full; otherwise
//            the target is asked and the invite is opened.
//   CANCEL   withdraws an open invite, telling the target.
//   ACCEPT   joins whichever of the two is in no party to the other's
//            party, or creates one holding both, then closes the invite.
//   REJECT   turns an open invite down, telling the target.
//   BUSY     echoes the refusal back to the requester and touches nothing.
//
// CANCEL, ACCEPT and REJECT are all refused when no invite is open.
[[nodiscard]] Outcome<PartyInviteEvents, PartyInviteRejection> decidePartyInvite(const PartyInviteRequest& request,
                                                                                 PartyInviteTopology& topology);

#endif // __PARTY_INVITE_DECISION_H__
