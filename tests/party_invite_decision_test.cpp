// The party invite decision (src/server/gameserver/party/PartyInviteDecision.cpp):
// every branch of every CGPartyInvite code, the precedence between the
// refusals, the packet and recipient each answer carries, the three ways an
// accepted invite changes the two parties, and the level threshold that
// decides which side gets the quest event. The topology is a recording fake,
// so the queries a branch makes - and the ones it does not make - are pinned
// too. The handler itself is not exercised here because it needs a creature
// and a socket.

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "CGPartyInvite.h"
#include "GCPartyError.h"
#include "GCPartyInvite.h"
#include "PartyInviteDecision.h"

namespace {

typedef std::vector<std::string> Calls;

const ObjectID_t kRequesterOID = 4711;
const ObjectID_t kTargetOID = 815;

const int kRequesterParty = 7;
const int kTargetParty = 9;

// Answers whatever a test seeds and appends every query to calls, in order.
// A party nobody seeded has room.
class RecordingTopology : public PartyInviteTopology {
public:
    bool targetBusy = false;
    bool inviting = false;
    int fullPartyID = 0;

    Calls calls;

    bool targetHasInviteInfo() override {
        calls.push_back("targetHasInviteInfo");
        return targetBusy;
    }

    bool isInviting() override {
        calls.push_back("isInviting");
        return inviting;
    }

    bool canAddMember(int partyID) override {
        calls.push_back("canAddMember(" + std::to_string(partyID) + ")");
        return partyID != fullPartyID;
    }
};

// A request naming a player character of the requester's race, with neither
// side in a party.
PartyInviteRequest requestOf(BYTE code) {
    PartyInviteRequest request;
    request.code = code;
    request.requesterObjectID = kRequesterOID;
    request.targetObjectID = kTargetOID;
    request.targetExists = true;
    request.targetIsSameRacePC = true;
    return request;
}

Calls callsOf(const char* first) {
    Calls calls;
    calls.push_back(first);
    return calls;
}

Calls callsOf(const char* first, const char* second) {
    Calls calls = callsOf(first);
    calls.push_back(second);
    return calls;
}

//////////////////////////////////////////////////////////////////////////////
// The two refusals that come before anything is asked of the topology.
//////////////////////////////////////////////////////////////////////////////

TEST(PartyInviteGate, ACreatureThatIsNotThereIsRefusedAndTheRequesterSInviteIsDropped) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.targetExists = false;
    request.targetIsSameRacePC = false;

    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::TargetMissing, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isPartyError);
    EXPECT_EQ(GC_PARTY_ERROR_TARGET_NOT_EXIST, outcome.rejection().code);
    EXPECT_TRUE(outcome.rejection().cancelRequesterInvite);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(PartyInviteGate, AMonsterOrAnotherRaceIsRefusedAndTheRequesterSInviteIsDropped) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.targetIsSameRacePC = false;

    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::RaceDiffer, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isPartyError);
    EXPECT_EQ(GC_PARTY_ERROR_RACE_DIFFER, outcome.rejection().code);
    EXPECT_TRUE(outcome.rejection().cancelRequesterInvite);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(PartyInviteGate, AMissingCreatureIsAnsweredBeforeItsRace) {
    // A creature that is not there is neither a player character nor of the
    // right race, and the missing-creature answer is the one that goes out.
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_ACCEPT);
    request.targetExists = false;
    request.targetIsSameRacePC = false;

    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::TargetMissing, outcome.rejection().reason);
}

//////////////////////////////////////////////////////////////////////////////
// CG_PARTY_INVITE_REQUEST
//////////////////////////////////////////////////////////////////////////////

TEST(PartyInviteRequestCode, AsksTheTargetAndOpensTheInvite) {
    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_REQUEST), topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    EXPECT_TRUE(events.sendInvite);
    EXPECT_EQ(PartyInvitePeer::Target, events.sendTo);
    // The target is told who is asking, not who was asked.
    EXPECT_EQ(kRequesterOID, events.sendObjectID);
    EXPECT_EQ(GC_PARTY_INVITE_REQUEST, events.sendCode);

    EXPECT_TRUE(events.initInvite);
    EXPECT_EQ(PartyJoin::None, events.join);
    EXPECT_EQ(PartyQuestEvent::None, events.questEvent);
    EXPECT_FALSE(events.cancelInvite);

    // Neither side is in a party, so no party is measured.
    EXPECT_EQ(callsOf("targetHasInviteInfo"), topology.calls);
}

TEST(PartyInviteRequestCode, ATargetAlreadyInAnInviteIsBusy) {
    RecordingTopology topology;
    topology.targetBusy = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_REQUEST), topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::TargetBusy, outcome.rejection().reason);
    EXPECT_FALSE(outcome.rejection().isPartyError);
    EXPECT_EQ(GC_PARTY_INVITE_BUSY, outcome.rejection().code);
    EXPECT_FALSE(outcome.rejection().cancelRequesterInvite);
    EXPECT_EQ(callsOf("targetHasInviteInfo"), topology.calls);
}

TEST(PartyInviteRequestCode, TwoPartiesCannotBeMerged) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.requesterPartyID = kRequesterParty;
    request.targetPartyID = kTargetParty;

    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::BothInParties, outcome.rejection().reason);
    EXPECT_FALSE(outcome.rejection().isPartyError);
    EXPECT_EQ(GC_PARTY_INVITE_ANOTHER_PARTY, outcome.rejection().code);
    // Neither party is measured once the pairing itself is refused.
    EXPECT_EQ(callsOf("targetHasInviteInfo"), topology.calls);
}

TEST(PartyInviteRequestCode, ABusyTargetIsAnsweredBeforeTheTwoParties) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.requesterPartyID = kRequesterParty;
    request.targetPartyID = kTargetParty;

    RecordingTopology topology;
    topology.targetBusy = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::TargetBusy, outcome.rejection().reason);
}

TEST(PartyInviteRequestCode, AFullRequesterPartyIsRefused) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.requesterPartyID = kRequesterParty;

    RecordingTopology topology;
    topology.fullPartyID = kRequesterParty;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::PartyFull, outcome.rejection().reason);
    EXPECT_FALSE(outcome.rejection().isPartyError);
    EXPECT_EQ(GC_PARTY_INVITE_MEMBER_FULL, outcome.rejection().code);
    EXPECT_EQ(callsOf("targetHasInviteInfo", "canAddMember(7)"), topology.calls);
}

TEST(PartyInviteRequestCode, AFullTargetPartyIsRefused) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.targetPartyID = kTargetParty;

    RecordingTopology topology;
    topology.fullPartyID = kTargetParty;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::PartyFull, outcome.rejection().reason);
    EXPECT_EQ(GC_PARTY_INVITE_MEMBER_FULL, outcome.rejection().code);
    EXPECT_EQ(callsOf("targetHasInviteInfo", "canAddMember(9)"), topology.calls);
}

TEST(PartyInviteRequestCode, TheRequesterSPartyIsMeasuredFirst) {
    // Only one side can be in a party at this point, so the two measurements
    // never both happen; the requester's is the one that is written first.
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_REQUEST);
    request.requesterPartyID = kRequesterParty;

    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_TRUE(outcome.events().initInvite);
    EXPECT_EQ(callsOf("targetHasInviteInfo", "canAddMember(7)"), topology.calls);
}

//////////////////////////////////////////////////////////////////////////////
// CG_PARTY_INVITE_CANCEL and CG_PARTY_INVITE_REJECT
//////////////////////////////////////////////////////////////////////////////

TEST(PartyInviteCancelCode, TellsTheTargetAndClosesTheInvite) {
    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_CANCEL), topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    EXPECT_TRUE(events.sendInvite);
    EXPECT_EQ(PartyInvitePeer::Target, events.sendTo);
    EXPECT_EQ(kRequesterOID, events.sendObjectID);
    EXPECT_EQ(GC_PARTY_INVITE_CANCEL, events.sendCode);

    EXPECT_FALSE(events.initInvite);
    EXPECT_EQ(PartyJoin::None, events.join);
    EXPECT_TRUE(events.cancelInvite);

    EXPECT_EQ(callsOf("targetHasInviteInfo", "isInviting"), topology.calls);
}

TEST(PartyInviteRejectCode, TellsTheTargetAndClosesTheInvite) {
    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_REJECT), topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    EXPECT_TRUE(events.sendInvite);
    EXPECT_EQ(PartyInvitePeer::Target, events.sendTo);
    EXPECT_EQ(kRequesterOID, events.sendObjectID);
    EXPECT_EQ(GC_PARTY_INVITE_REJECT, events.sendCode);
    EXPECT_EQ(PartyJoin::None, events.join);
    EXPECT_TRUE(events.cancelInvite);

    EXPECT_EQ(callsOf("targetHasInviteInfo", "isInviting"), topology.calls);
}

TEST(PartyInviteAnswers, EveryAnswerToAnInviteThatIsNotOpenIsRefused) {
    const BYTE codes[] = {CG_PARTY_INVITE_CANCEL, CG_PARTY_INVITE_ACCEPT, CG_PARTY_INVITE_REJECT};

    for (BYTE code : codes) {
        RecordingTopology topology;
        topology.inviting = false;

        Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(requestOf(code), topology);

        ASSERT_TRUE(outcome.isRejected());
        EXPECT_EQ(PartyInviteReason::NotInviting, outcome.rejection().reason);
        EXPECT_TRUE(outcome.rejection().isPartyError);
        EXPECT_EQ(GC_PARTY_ERROR_NOT_INVITING, outcome.rejection().code);
        // The refusal leaves the requester's own invite alone.
        EXPECT_FALSE(outcome.rejection().cancelRequesterInvite);
        EXPECT_EQ(callsOf("targetHasInviteInfo", "isInviting"), topology.calls);
    }
}

//////////////////////////////////////////////////////////////////////////////
// CG_PARTY_INVITE_ACCEPT
//////////////////////////////////////////////////////////////////////////////

TEST(PartyInviteAcceptCode, TheRequesterJoinsThePartyTheTargetIsIn) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_ACCEPT);
    request.targetPartyID = kTargetParty;

    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    // An accepted invite answers nobody with a packet of its own.
    EXPECT_FALSE(events.sendInvite);
    EXPECT_EQ(PartyJoin::RequesterJoinsTargetParty, events.join);
    EXPECT_EQ(kTargetParty, events.joinPartyID);
    EXPECT_EQ(PartyQuestEvent::None, events.questEvent);
    EXPECT_TRUE(events.cancelInvite);

    EXPECT_EQ(callsOf("targetHasInviteInfo", "isInviting"), topology.calls);
}

TEST(PartyInviteAcceptCode, TheTargetJoinsThePartyTheRequesterIsIn) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_ACCEPT);
    request.requesterPartyID = kRequesterParty;

    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    EXPECT_FALSE(events.sendInvite);
    EXPECT_EQ(PartyJoin::TargetJoinsRequesterParty, events.join);
    EXPECT_EQ(kRequesterParty, events.joinPartyID);
    EXPECT_TRUE(events.cancelInvite);
}

TEST(PartyInviteAcceptCode, TwoCharactersInNoPartyGetANewOne) {
    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_ACCEPT), topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    EXPECT_FALSE(events.sendInvite);
    EXPECT_EQ(PartyJoin::CreateParty, events.join);
    EXPECT_TRUE(events.cancelInvite);
}

TEST(PartyInviteAcceptCode, TwoCharactersAlreadyInPartiesGetANewOneToo) {
    // The request rules refuse this pairing, but a party can be joined
    // between the invite and the answer, and the answer creates a party
    // holding both rather than refusing.
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_ACCEPT);
    request.requesterPartyID = kRequesterParty;
    request.targetPartyID = kTargetParty;

    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(PartyJoin::CreateParty, outcome.events().join);
    // No party is measured on the way in, full or not.
    EXPECT_EQ(callsOf("targetHasInviteInfo", "isInviting"), topology.calls);
}

//////////////////////////////////////////////////////////////////////////////
// The quest event a newly created party raises.
//////////////////////////////////////////////////////////////////////////////

PartyQuestEvent questEventFor(int requesterLevel, int targetLevel) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_ACCEPT);
    request.requesterLevel = requesterLevel;
    request.targetLevel = targetLevel;

    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);
    return std::move(outcome).events().questEvent;
}

TEST(PartyInviteQuestEvent, TheCharacterAtTheThresholdGetsItWhenTheOtherIsBelow) {
    EXPECT_EQ(PartyQuestEvent::Requester, questEventFor(kPartyQuestLevel, kPartyQuestLevel - 1));
    EXPECT_EQ(PartyQuestEvent::Target, questEventFor(kPartyQuestLevel - 1, kPartyQuestLevel));
}

TEST(PartyInviteQuestEvent, NobodyGetsItWhenBothAreOnTheSameSideOfTheThreshold) {
    EXPECT_EQ(PartyQuestEvent::None, questEventFor(kPartyQuestLevel, kPartyQuestLevel));
    EXPECT_EQ(PartyQuestEvent::None, questEventFor(kPartyQuestLevel - 1, kPartyQuestLevel - 1));
    EXPECT_EQ(PartyQuestEvent::None, questEventFor(60, 40));
    EXPECT_EQ(PartyQuestEvent::None, questEventFor(1, 1));
}

TEST(PartyInviteQuestEvent, AJoinIntoAnExistingPartyRaisesNothing) {
    PartyInviteRequest request = requestOf(CG_PARTY_INVITE_ACCEPT);
    request.targetPartyID = kTargetParty;
    request.requesterLevel = kPartyQuestLevel - 1;
    request.targetLevel = kPartyQuestLevel;

    RecordingTopology topology;
    topology.inviting = true;

    Outcome<PartyInviteEvents, PartyInviteRejection> outcome = decidePartyInvite(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(PartyQuestEvent::None, outcome.events().questEvent);
}

//////////////////////////////////////////////////////////////////////////////
// CG_PARTY_INVITE_BUSY and everything else
//////////////////////////////////////////////////////////////////////////////

TEST(PartyInviteBusyCode, IsEchoedBackToTheRequesterAndChangesNothing) {
    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_BUSY), topology);

    ASSERT_TRUE(outcome.isOk());
    const PartyInviteEvents& events = outcome.events();

    EXPECT_TRUE(events.sendInvite);
    EXPECT_EQ(PartyInvitePeer::Requester, events.sendTo);
    // The requester is told which creature is busy.
    EXPECT_EQ(kTargetOID, events.sendObjectID);
    EXPECT_EQ(GC_PARTY_INVITE_BUSY, events.sendCode);

    EXPECT_FALSE(events.initInvite);
    EXPECT_EQ(PartyJoin::None, events.join);
    EXPECT_FALSE(events.cancelInvite);

    // Whether an invite is open is not asked.
    EXPECT_EQ(callsOf("targetHasInviteInfo"), topology.calls);
}

TEST(PartyInviteUnknownCode, IsReportedSoTheCallerCanRefuseTheClient) {
    RecordingTopology topology;
    Outcome<PartyInviteEvents, PartyInviteRejection> outcome =
        decidePartyInvite(requestOf(CG_PARTY_INVITE_MAX), topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(PartyInviteReason::UnknownCode, outcome.rejection().reason);
    EXPECT_FALSE(outcome.rejection().cancelRequesterInvite);
    EXPECT_EQ(callsOf("targetHasInviteInfo"), topology.calls);
}

} // namespace
