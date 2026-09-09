// The trade prepare decision (src/server/gameserver/trade/TradePrepareDecision.cpp):
// every branch of every CGTradePrepare code, the precedence between the
// refusals that come before the packet code is even looked at, the packet,
// recipient and object id each answer carries, and which TradeManager
// mutation follows it. The topology is a recording fake, so the queries a
// branch makes - and the ones it does not make - are pinned too. The handler
// itself is not exercised here because it needs a creature and a socket.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGTradePrepare.h"
#include "GCTradeError.h"
#include "GCTradePrepare.h"
#include "TradePrepareDecision.h"

namespace {

typedef Outcome<TradePrepareEvents, TradePrepareRejection> TradeOutcome;
typedef std::vector<std::string> Calls;

const ObjectID_t kSenderOID = 4711;
const ObjectID_t kTargetOID = 815;

// An answer to a code the protocol does not define.
const BYTE kUnknownCode = CG_TRADE_PREPARE_CODE_MAX;

// Answers whatever a test seeds and appends every query to calls, in order.
class RecordingTopology : public TradePrepareTopology {
public:
    bool senderTrading = false;
    bool receiverTrading = false;
    bool trading = false;

    Calls calls;

    bool senderHasTradeInfo() override {
        calls.push_back("senderHasTradeInfo");
        return senderTrading;
    }

    bool receiverHasTradeInfo() override {
        calls.push_back("receiverHasTradeInfo");
        return receiverTrading;
    }

    bool isTrading() override {
        calls.push_back("isTrading");
        return trading;
    }
};

// A request naming another player character of the sender's race, both of
// them standing unmounted in a safe zone.
TradePrepareRequest requestOf(BYTE code) {
    TradePrepareRequest request;
    request.code = code;
    request.senderObjectID = kSenderOID;
    request.targetObjectID = kTargetOID;
    request.targetExists = true;
    request.targetIsSameRacePC = true;
    request.bothInSafeZone = true;
    return request;
}

Calls theTwoRecordReads() {
    Calls calls;
    calls.push_back("senderHasTradeInfo");
    calls.push_back("receiverHasTradeInfo");
    return calls;
}

Calls theTwoRecordReadsAndIsTrading() {
    Calls calls = theTwoRecordReads();
    calls.push_back("isTrading");
    return calls;
}

// Every code the protocol defines, plus one it does not.
const BYTE kAllCodes[] = {CG_TRADE_PREPARE_CODE_REQUEST, CG_TRADE_PREPARE_CODE_CANCEL, CG_TRADE_PREPARE_CODE_ACCEPT,
                          CG_TRADE_PREPARE_CODE_REJECT,  CG_TRADE_PREPARE_CODE_BUSY,   kUnknownCode};

//////////////////////////////////////////////////////////////////////////////
// The refusals that come before anything is asked of the topology.
//////////////////////////////////////////////////////////////////////////////

TEST(TradePrepareGate, ACreatureThatIsNotThereIsRefusedAndTheSenderSTradeIsDropped) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.targetExists = false;
    request.targetIsSameRacePC = false;
    request.bothInSafeZone = false;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::TargetMissing, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST, outcome.rejection().code);
    EXPECT_EQ(TradePeer::Sender, outcome.rejection().sendTo);
    EXPECT_EQ(kTargetOID, outcome.rejection().sendObjectID);
    EXPECT_EQ(TradeCancel::Sender, outcome.rejection().cancel);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(TradePrepareGate, ARequestNamingTheSenderItselfSendsNothingAndDropsNoTrade) {
    // One character logged in twice: the handler cuts the connection instead
    // of answering, and nothing is cancelled on the way out.
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.targetIsSelf = true;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::SameCreature, outcome.rejection().reason);
    EXPECT_EQ(TradeCancel::None, outcome.rejection().cancel);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(TradePrepareGate, AMissingCreatureIsAnsweredBeforeTheSenderItselfIsRecognised) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.targetExists = false;
    request.targetIsSelf = true;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::TargetMissing, outcome.rejection().reason);
}

TEST(TradePrepareGate, AMonsterOrAnotherRaceIsRefusedAndTheSenderSTradeIsDropped) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.targetIsSameRacePC = false;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::RaceDiffer, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(GC_TRADE_ERROR_CODE_RACE_DIFFER, outcome.rejection().code);
    EXPECT_EQ(kTargetOID, outcome.rejection().sendObjectID);
    EXPECT_EQ(TradeCancel::Sender, outcome.rejection().cancel);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(TradePrepareGate, TheSenderItselfIsRecognisedBeforeItsRace) {
    // The sender's own name always passes the race test, so the ordering
    // only shows when both flags are seeded against each other.
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.targetIsSelf = true;
    request.targetIsSameRacePC = false;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::SameCreature, outcome.rejection().reason);
}

TEST(TradePrepareGate, APairOutsideASafeZoneIsRefusedAndTheSenderSTradeIsDropped) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.bothInSafeZone = false;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::NotSafe, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(GC_TRADE_ERROR_CODE_NOT_SAFE, outcome.rejection().code);
    EXPECT_EQ(TradeCancel::Sender, outcome.rejection().cancel);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(TradePrepareGate, TheRaceIsAnsweredBeforeTheSafeZone) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.targetIsSameRacePC = false;
    request.bothInSafeZone = false;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::RaceDiffer, outcome.rejection().reason);
}

TEST(TradePrepareGate, AMountedSenderIsRefusedAndTheSenderSTradeIsDropped) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.senderMounted = true;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::Motorcycle, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(GC_TRADE_ERROR_CODE_MOTORCYCLE, outcome.rejection().code);
    EXPECT_EQ(TradeCancel::Sender, outcome.rejection().cancel);
    EXPECT_EQ(Calls(), topology.calls);
}

TEST(TradePrepareGate, AMountedReceiverIsRefusedTheSameWay) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.receiverMounted = true;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::Motorcycle, outcome.rejection().reason);
    EXPECT_EQ(TradeCancel::Sender, outcome.rejection().cancel);
}

TEST(TradePrepareGate, TheSafeZoneIsAnsweredBeforeTheMount) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);
    request.bothInSafeZone = false;
    request.senderMounted = true;

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::NotSafe, outcome.rejection().reason);
}

TEST(TradePrepareGate, EveryCodeIncludingAnUnknownOnePassesTheSameGates) {
    for (BYTE code : kAllCodes) {
        TradePrepareRequest request = requestOf(code);
        request.bothInSafeZone = false;

        RecordingTopology topology;
        TradeOutcome outcome = decideTradePrepare(request, topology);

        ASSERT_TRUE(outcome.isRejected()) << "code " << static_cast<int>(code);
        EXPECT_EQ(TradePrepareReason::NotSafe, outcome.rejection().reason) << "code " << static_cast<int>(code);
        EXPECT_EQ(Calls(), topology.calls) << "code " << static_cast<int>(code);
    }
}

//////////////////////////////////////////////////////////////////////////////
// REQUEST: one player asks another for a trade.
//////////////////////////////////////////////////////////////////////////////

TEST(TradePrepareRequest, TheReceiverIsAskedAndTheTradeIsOpened) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_TRUE(outcome.events().sendPrepare);
    EXPECT_EQ(TradePeer::Receiver, outcome.events().sendTo);
    // The receiver learns who is asking from the sender's object id.
    EXPECT_EQ(kSenderOID, outcome.events().sendObjectID);
    EXPECT_EQ(GC_TRADE_PREPARE_CODE_REQUEST, outcome.events().sendCode);
    EXPECT_TRUE(outcome.events().initTrade);
    EXPECT_EQ(TradeCancel::None, outcome.events().cancel);
    // The open trade is never queried: a request answers on the two records
    // alone.
    EXPECT_EQ(theTwoRecordReads(), topology.calls);
}

TEST(TradePrepareRequest, ASenderThatIsTradingAlreadyIsRefusedAndItsTradeIsDropped) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);

    RecordingTopology topology;
    topology.senderTrading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::AlreadyTrading, outcome.rejection().reason);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(GC_TRADE_ERROR_CODE_ALREADY_TRADING, outcome.rejection().code);
    EXPECT_EQ(TradePeer::Sender, outcome.rejection().sendTo);
    EXPECT_EQ(kTargetOID, outcome.rejection().sendObjectID);
    EXPECT_EQ(TradeCancel::Sender, outcome.rejection().cancel);
    // Both records are read before the code is looked at, so the receiver's
    // is read even though the answer does not depend on it.
    EXPECT_EQ(theTwoRecordReads(), topology.calls);
}

TEST(TradePrepareRequest, AReceiverThatIsTradingAlreadyIsBusyAndNothingIsCancelled) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);

    RecordingTopology topology;
    topology.receiverTrading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::TargetBusy, outcome.rejection().reason);
    // The only refusal that answers with a GCTradePrepare rather than a
    // GCTradeError, and it carries the object id the sender asked about.
    EXPECT_FALSE(outcome.rejection().isTradeError);
    EXPECT_EQ(GC_TRADE_PREPARE_CODE_BUSY, outcome.rejection().code);
    EXPECT_EQ(TradePeer::Sender, outcome.rejection().sendTo);
    EXPECT_EQ(kTargetOID, outcome.rejection().sendObjectID);
    EXPECT_EQ(TradeCancel::None, outcome.rejection().cancel);
    EXPECT_EQ(theTwoRecordReads(), topology.calls);
}

TEST(TradePrepareRequest, ASenderAlreadyTradingIsAnsweredBeforeABusyReceiver) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REQUEST);

    RecordingTopology topology;
    topology.senderTrading = true;
    topology.receiverTrading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::AlreadyTrading, outcome.rejection().reason);
}

//////////////////////////////////////////////////////////////////////////////
// The four answers to an open trade.
//////////////////////////////////////////////////////////////////////////////

TEST(TradePrepareAnswer, CancelTellsTheReceiverAndClosesTheTrade) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_CANCEL);

    RecordingTopology topology;
    topology.trading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_TRUE(outcome.events().sendPrepare);
    EXPECT_EQ(TradePeer::Receiver, outcome.events().sendTo);
    EXPECT_EQ(kSenderOID, outcome.events().sendObjectID);
    EXPECT_EQ(GC_TRADE_PREPARE_CODE_CANCEL, outcome.events().sendCode);
    EXPECT_FALSE(outcome.events().initTrade);
    EXPECT_EQ(TradeCancel::Pair, outcome.events().cancel);
    EXPECT_EQ(theTwoRecordReadsAndIsTrading(), topology.calls);
}

TEST(TradePrepareAnswer, AcceptTellsTheReceiverAndLeavesTheTradeOpen) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_ACCEPT);

    RecordingTopology topology;
    topology.trading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(TradePeer::Receiver, outcome.events().sendTo);
    EXPECT_EQ(kSenderOID, outcome.events().sendObjectID);
    EXPECT_EQ(GC_TRADE_PREPARE_CODE_ACCEPT, outcome.events().sendCode);
    // The one answer that keeps the trade: the items are moved next.
    EXPECT_EQ(TradeCancel::None, outcome.events().cancel);
    EXPECT_EQ(theTwoRecordReadsAndIsTrading(), topology.calls);
}

TEST(TradePrepareAnswer, RejectTellsTheReceiverAndClosesTheTrade) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_REJECT);

    RecordingTopology topology;
    topology.trading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(TradePeer::Receiver, outcome.events().sendTo);
    EXPECT_EQ(kSenderOID, outcome.events().sendObjectID);
    EXPECT_EQ(GC_TRADE_PREPARE_CODE_REJECT, outcome.events().sendCode);
    EXPECT_EQ(TradeCancel::Pair, outcome.events().cancel);
    EXPECT_EQ(theTwoRecordReadsAndIsTrading(), topology.calls);
}

TEST(TradePrepareAnswer, BusyTellsTheReceiverAndClosesTheTrade) {
    TradePrepareRequest request = requestOf(CG_TRADE_PREPARE_CODE_BUSY);

    RecordingTopology topology;
    topology.trading = true;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(TradePeer::Receiver, outcome.events().sendTo);
    EXPECT_EQ(kSenderOID, outcome.events().sendObjectID);
    EXPECT_EQ(GC_TRADE_PREPARE_CODE_BUSY, outcome.events().sendCode);
    EXPECT_EQ(TradeCancel::Pair, outcome.events().cancel);
    EXPECT_EQ(theTwoRecordReadsAndIsTrading(), topology.calls);
}

TEST(TradePrepareAnswer, EveryAnswerToATradeThatIsNotOpenIsRefusedAndCancelsNothing) {
    const BYTE answers[] = {CG_TRADE_PREPARE_CODE_CANCEL, CG_TRADE_PREPARE_CODE_ACCEPT, CG_TRADE_PREPARE_CODE_REJECT,
                            CG_TRADE_PREPARE_CODE_BUSY};

    for (BYTE code : answers) {
        TradePrepareRequest request = requestOf(code);

        RecordingTopology topology;
        TradeOutcome outcome = decideTradePrepare(request, topology);

        ASSERT_TRUE(outcome.isRejected()) << "code " << static_cast<int>(code);
        EXPECT_EQ(TradePrepareReason::NotTrading, outcome.rejection().reason) << "code " << static_cast<int>(code);
        EXPECT_TRUE(outcome.rejection().isTradeError);
        EXPECT_EQ(GC_TRADE_ERROR_CODE_NOT_TRADING, outcome.rejection().code);
        EXPECT_EQ(TradePeer::Sender, outcome.rejection().sendTo);
        EXPECT_EQ(kTargetOID, outcome.rejection().sendObjectID);
        EXPECT_EQ(TradeCancel::None, outcome.rejection().cancel);
        EXPECT_EQ(theTwoRecordReadsAndIsTrading(), topology.calls) << "code " << static_cast<int>(code);
    }
}

TEST(TradePrepareAnswer, AnOpenTradeIsTheOnlyThingAnAnswerAsksAbout) {
    // The two trade records are read for every code, but an answer decides
    // on the open trade alone: neither side counting as trading changes it.
    const BYTE answers[] = {CG_TRADE_PREPARE_CODE_CANCEL, CG_TRADE_PREPARE_CODE_ACCEPT, CG_TRADE_PREPARE_CODE_REJECT,
                            CG_TRADE_PREPARE_CODE_BUSY};

    for (BYTE code : answers) {
        TradePrepareRequest request = requestOf(code);

        RecordingTopology topology;
        topology.senderTrading = true;
        topology.receiverTrading = true;
        topology.trading = true;
        TradeOutcome outcome = decideTradePrepare(request, topology);

        ASSERT_TRUE(outcome.isOk()) << "code " << static_cast<int>(code);
        EXPECT_TRUE(outcome.events().sendPrepare);
        EXPECT_FALSE(outcome.events().initTrade);
    }
}

//////////////////////////////////////////////////////////////////////////////
// A code the protocol does not define.
//////////////////////////////////////////////////////////////////////////////

TEST(TradePrepareUnknownCode, SendsNothingCancelsNothingAndStillReadsBothRecords) {
    TradePrepareRequest request = requestOf(kUnknownCode);

    RecordingTopology topology;
    TradeOutcome outcome = decideTradePrepare(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(TradePrepareReason::UnknownCode, outcome.rejection().reason);
    EXPECT_EQ(TradeCancel::None, outcome.rejection().cancel);
    EXPECT_EQ(theTwoRecordReads(), topology.calls);
}

} // namespace
