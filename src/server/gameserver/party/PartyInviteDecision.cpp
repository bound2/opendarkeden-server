//////////////////////////////////////////////////////////////////////////////
// Filename    : PartyInviteDecision.cpp
// Description : the party invite protocol's rules.
//////////////////////////////////////////////////////////////////////////////

#include "PartyInviteDecision.h"

#include "CGPartyInvite.h"
#include "GCPartyError.h"
#include "GCPartyInvite.h"

namespace {

typedef Outcome<PartyInviteEvents, PartyInviteRejection> Result;

PartyInviteRejection partyError(PartyInviteReason reason, BYTE code, bool clearInvite) {
    return PartyInviteRejection(reason, true, code, clearInvite);
}

PartyInviteRejection inviteRefusal(PartyInviteReason reason, BYTE code) {
    return PartyInviteRejection(reason, false, code, false);
}

// A GCPartyInvite the requester's own object id identifies, which is how the
// target learns who is asking.
PartyInviteEvents tellTarget(const PartyInviteRequest& request, BYTE code) {
    PartyInviteEvents events;
    events.sendInvite = true;
    events.sendTo = PartyInvitePeer::Target;
    events.sendObjectID = request.requesterObjectID;
    events.sendCode = code;
    return events;
}

// The invite the requester asks for: refused while the target is busy with
// another invite, while both are in parties, and while either party is full.
Result decideRequest(const PartyInviteRequest& request, PartyInviteTopology& topology, bool targetBusy) {
    if (targetBusy)
        return Result::Rejected(inviteRefusal(PartyInviteReason::TargetBusy, GC_PARTY_INVITE_BUSY));

    if (request.requesterPartyID != 0 && request.targetPartyID != 0)
        return Result::Rejected(inviteRefusal(PartyInviteReason::BothInParties, GC_PARTY_INVITE_ANOTHER_PARTY));

    if (request.requesterPartyID != 0 && !topology.canAddMember(request.requesterPartyID))
        return Result::Rejected(inviteRefusal(PartyInviteReason::PartyFull, GC_PARTY_INVITE_MEMBER_FULL));

    if (request.targetPartyID != 0 && !topology.canAddMember(request.targetPartyID))
        return Result::Rejected(inviteRefusal(PartyInviteReason::PartyFull, GC_PARTY_INVITE_MEMBER_FULL));

    PartyInviteEvents events = tellTarget(request, GC_PARTY_INVITE_REQUEST);
    events.initInvite = true;
    return Result::Ok(events);
}

// The invite the target accepts. One of the two is in no party, so it joins
// the other's; when neither is, a party is created holding both.
PartyInviteEvents decideAccept(const PartyInviteRequest& request) {
    PartyInviteEvents events;
    events.cancelInvite = true;

    if (request.targetPartyID != 0 && request.requesterPartyID == 0) {
        events.join = PartyJoin::RequesterJoinsTargetParty;
        events.joinPartyID = request.targetPartyID;
        return events;
    }

    if (request.requesterPartyID != 0 && request.targetPartyID == 0) {
        events.join = PartyJoin::TargetJoinsRequesterParty;
        events.joinPartyID = request.requesterPartyID;
        return events;
    }

    // Neither is in a party the other can join. Two characters that are both
    // in parties reach this too: the request rules refuse that pairing, but
    // a party can be joined between the invite and the answer.
    events.join = PartyJoin::CreateParty;

    // Only a pairing that straddles the threshold raises the event, and only
    // for the character above it.
    if (request.requesterLevel >= kPartyQuestLevel && request.targetLevel < kPartyQuestLevel)
        events.questEvent = PartyQuestEvent::Requester;
    else if (request.targetLevel >= kPartyQuestLevel && request.requesterLevel < kPartyQuestLevel)
        events.questEvent = PartyQuestEvent::Target;

    return events;
}

// An answer to an invite that has to be open: the target is told, and the
// invite is closed.
Result decideAnswer(const PartyInviteRequest& request, PartyInviteTopology& topology, BYTE code) {
    if (!topology.isInviting())
        return Result::Rejected(partyError(PartyInviteReason::NotInviting, GC_PARTY_ERROR_NOT_INVITING, false));

    PartyInviteEvents events = tellTarget(request, code);
    events.cancelInvite = true;
    return Result::Ok(events);
}

} // namespace

Outcome<PartyInviteEvents, PartyInviteRejection> decidePartyInvite(const PartyInviteRequest& request,
                                                                   PartyInviteTopology& topology) {
    // A refusal here also drops whatever invite the requester was in.
    if (!request.targetExists)
        return Result::Rejected(partyError(PartyInviteReason::TargetMissing, GC_PARTY_ERROR_TARGET_NOT_EXIST, true));

    if (!request.targetIsSameRacePC)
        return Result::Rejected(partyError(PartyInviteReason::RaceDiffer, GC_PARTY_ERROR_RACE_DIFFER, true));

    // Read for every code, used by the request.
    const bool targetBusy = topology.targetHasInviteInfo();

    switch (request.code) {
    case CG_PARTY_INVITE_REQUEST:
        return decideRequest(request, topology, targetBusy);

    case CG_PARTY_INVITE_CANCEL:
        return decideAnswer(request, topology, GC_PARTY_INVITE_CANCEL);

    case CG_PARTY_INVITE_ACCEPT:
        if (!topology.isInviting())
            return Result::Rejected(partyError(PartyInviteReason::NotInviting, GC_PARTY_ERROR_NOT_INVITING, false));
        return Result::Ok(decideAccept(request));

    case CG_PARTY_INVITE_REJECT:
        return decideAnswer(request, topology, GC_PARTY_INVITE_REJECT);

    case CG_PARTY_INVITE_BUSY: {
        // The target says it cannot answer right now, and the requester is
        // told so with its own packet's target id.
        PartyInviteEvents events;
        events.sendInvite = true;
        events.sendTo = PartyInvitePeer::Requester;
        events.sendObjectID = request.targetObjectID;
        events.sendCode = GC_PARTY_INVITE_BUSY;
        return Result::Ok(events);
    }

    default:
        return Result::Rejected(inviteRefusal(PartyInviteReason::UnknownCode, 0));
    }
}
