// The trade table decisions (src/server/gameserver/trade/TradeTableDecision.cpp):
// the gate all three requests pass and its precedence, every branch of the
// item, gift box and gold rules, the packet, recipient and object id each
// answer carries, the trade record mutations that go with it and the order
// they are performed in. The topology is a recording fake, so the queries a
// branch makes - and the ones it does not make - are pinned too. The handlers
// themselves are not exercised here because they need a creature, an
// inventory and a socket.

#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGTradeMoney.h"
#include "GCTradeError.h"
#include "GCTradeMoney.h"
#include "GCTradeVerify.h"
#include "TradeTableDecision.h"

namespace {

typedef Outcome<TradeTableEvents, TradeTableRejection> TradeOutcome;
typedef Outcome<void, TradeTableRejection> GateOutcome;
typedef std::vector<std::string> Lines;

const ObjectID_t kSenderOID = 4711;
const ObjectID_t kTargetOID = 815;

// An item class the gift box rules ignore.
const ItemType_t kOrdinaryType = 3;

// Answers whatever a test seeds and appends every query to calls, in order.
class RecordingTopology : public TradeTableTopology {
public:
    bool trading = true;
    TradeStatus status = TradeStatus::Trading;
    std::vector<TradeStakedItem> staked;
    Gold_t stakedGold = 0;
    Gold_t receiverStake = 0;

    Lines calls;

    bool isTrading() override {
        calls.push_back("isTrading");
        return trading;
    }

    TradeStatus senderStatus() override {
        calls.push_back("senderStatus");
        return status;
    }

    std::vector<TradeStakedItem> senderStakedItems() override {
        calls.push_back("senderStakedItems");
        return staked;
    }

    Gold_t senderStakedGold() override {
        calls.push_back("senderStakedGold");
        return stakedGold;
    }

    Gold_t receiverStakedGold() override {
        calls.push_back("receiverStakedGold");
        return receiverStake;
    }
};

const char* peerName(TradePeer peer) {
    return peer == TradePeer::Sender ? "sender" : "receiver";
}

// One step, rendered so a whole event list reads as text in a failure.
std::string describe(const TradeTableStep& step) {
    std::ostringstream out;

    switch (step.action) {
    case TradeTableAction::SendVerify:
        out << "SendVerify(" << peerName(step.sendTo) << ", code=" << static_cast<int>(step.code) << ")";
        break;
    case TradeTableAction::SendAddItem:
        out << "SendAddItem(" << peerName(step.sendTo) << ", oid=" << step.objectID << ")";
        break;
    case TradeTableAction::SendRemoveItem:
        out << "SendRemoveItem(" << peerName(step.sendTo) << ", oid=" << step.objectID << ")";
        break;
    case TradeTableAction::SendMoney:
        out << "SendMoney(" << peerName(step.sendTo) << ", code=" << static_cast<int>(step.code)
            << ", oid=" << step.objectID << ", amount=" << step.amount << ")";
        break;
    case TradeTableAction::StakeItem:
        out << "StakeItem";
        break;
    case TradeTableAction::UnstakeItem:
        out << "UnstakeItem";
        break;
    case TradeTableAction::DelayOK:
        out << "DelayOK";
        break;
    case TradeTableAction::SetSenderGold:
        out << "SetSenderGold(" << step.amount << ")";
        break;
    case TradeTableAction::SetStakedGold:
        out << "SetStakedGold(" << step.amount << ")";
        break;
    case TradeTableAction::ResumeTrading:
        out << "ResumeTrading";
        break;
    }

    return out.str();
}

Lines stepsOf(const TradeOutcome& outcome) {
    Lines lines;

    const TradeTableEvents& events = outcome.events();
    for (TradeTableEvents::const_iterator itr = events.begin(); itr != events.end(); ++itr)
        lines.push_back(describe(*itr));

    return lines;
}

// A pair that passes the gate: another player character of the sender's race,
// both standing unmounted in a safe zone.
TradeTableGate openGate() {
    TradeTableGate gate;
    gate.targetObjectID = kTargetOID;
    gate.targetExists = true;
    gate.targetIsSameRacePC = true;
    gate.bothInSafeZone = true;
    return gate;
}

// An ordinary item the sender holds and may trade.
TradeAddItemRequest addItemRequest() {
    TradeAddItemRequest request;
    request.senderObjectID = kSenderOID;
    request.targetObjectID = kTargetOID;
    request.itemFound = true;
    request.itemTradeable = true;
    request.itemType = kOrdinaryType;
    return request;
}

// A green gift box: an event gift box of type 0.
TradeAddItemRequest greenGiftBoxRequest() {
    TradeAddItemRequest request = addItemRequest();
    request.itemIsEventGiftBox = true;
    request.itemType = 0;
    return request;
}

TradeAddItemRequest giftBoxRequest(ItemType_t itemType) {
    TradeAddItemRequest request = addItemRequest();
    request.itemIsEventGiftBox = true;
    request.itemType = itemType;
    return request;
}

TradeRemoveItemRequest removeItemRequest() {
    TradeRemoveItemRequest request;
    request.senderObjectID = kSenderOID;
    request.targetObjectID = kTargetOID;
    request.itemFound = true;
    return request;
}

TradeMoneyRequest moneyRequest(BYTE code, Gold_t amount) {
    TradeMoneyRequest request;
    request.senderObjectID = kSenderOID;
    request.targetObjectID = kTargetOID;
    request.code = code;
    request.amount = amount;
    return request;
}

//////////////////////////////////////////////////////////////////////////////
// The gate
//////////////////////////////////////////////////////////////////////////////

TEST(TradeTableGateTest, LetsAnOpenTradeThrough) {
    RecordingTopology topology;

    GateOutcome outcome = decideTradeTableGate(openGate(), topology);

    EXPECT_TRUE(outcome.isOk());
    EXPECT_EQ(topology.calls, Lines({"isTrading"}));
}

TEST(TradeTableGateTest, RefusesACreatureTheZoneDoesNotHold) {
    RecordingTopology topology;
    TradeTableGate gate = openGate();
    gate.targetExists = false;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::TargetMissing);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST);
    EXPECT_EQ(outcome.rejection().sendTo, TradePeer::Sender);
    EXPECT_EQ(outcome.rejection().sendObjectID, kTargetOID);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    // The trade records are never asked about.
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeTableGateTest, RefusesAnythingButAPlayerCharacterOfTheSameRace) {
    RecordingTopology topology;
    TradeTableGate gate = openGate();
    gate.targetIsSameRacePC = false;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::RaceDiffer);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_RACE_DIFFER);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeTableGateTest, RefusesAPairOutsideASafeZone) {
    RecordingTopology topology;
    TradeTableGate gate = openGate();
    gate.bothInSafeZone = false;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::NotSafe);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_NOT_SAFE);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeTableGateTest, RefusesAMountedSender) {
    RecordingTopology topology;
    TradeTableGate gate = openGate();
    gate.senderMounted = true;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::Motorcycle);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_MOTORCYCLE);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeTableGateTest, RefusesAMountedReceiver) {
    RecordingTopology topology;
    TradeTableGate gate = openGate();
    gate.receiverMounted = true;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::Motorcycle);
}

TEST(TradeTableGateTest, RefusesAPairThatIsNotTrading) {
    RecordingTopology topology;
    topology.trading = false;

    GateOutcome outcome = decideTradeTableGate(openGate(), topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::NotTrading);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_NOT_TRADING);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    EXPECT_EQ(topology.calls, Lines({"isTrading"}));
}

TEST(TradeTableGateTest, MissingTargetOutranksEveryOtherRefusal) {
    RecordingTopology topology;
    topology.trading = false;

    TradeTableGate gate;
    gate.targetObjectID = kTargetOID;
    gate.senderMounted = true;
    gate.receiverMounted = true;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::TargetMissing);
}

TEST(TradeTableGateTest, RaceOutranksTheSafeZoneAndTheMounts) {
    RecordingTopology topology;
    topology.trading = false;

    TradeTableGate gate = openGate();
    gate.targetIsSameRacePC = false;
    gate.bothInSafeZone = false;
    gate.senderMounted = true;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::RaceDiffer);
}

TEST(TradeTableGateTest, TheSafeZoneOutranksTheMounts) {
    RecordingTopology topology;
    topology.trading = false;

    TradeTableGate gate = openGate();
    gate.bothInSafeZone = false;
    gate.receiverMounted = true;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::NotSafe);
}

TEST(TradeTableGateTest, TheMountsOutrankTheOpenTrade) {
    RecordingTopology topology;
    topology.trading = false;

    TradeTableGate gate = openGate();
    gate.senderMounted = true;

    GateOutcome outcome = decideTradeTableGate(gate, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::Motorcycle);
    EXPECT_TRUE(topology.calls.empty());
}

//////////////////////////////////////////////////////////////////////////////
// Adding an item
//////////////////////////////////////////////////////////////////////////////

TEST(TradeAddItemTest, StakesAnOrdinaryItemAndTellsTheReceiver) {
    RecordingTopology topology;

    TradeOutcome outcome = decideTradeAddItem(addItemRequest(), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"StakeItem", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
    EXPECT_EQ(topology.calls, Lines({"senderStakedItems", "senderStatus"}));
}

TEST(TradeAddItemTest, VerifiesWhenTheSenderHasAlreadyPressedOK) {
    RecordingTopology topology;
    topology.status = TradeStatus::Finished;

    TradeOutcome outcome = decideTradeAddItem(addItemRequest(), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome),
              Lines({"StakeItem", "SendVerify(sender, code=0)", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
    EXPECT_EQ(GC_TRADE_VERIFY_CODE_ADD_ITEM_WHEN_ACCEPT, 0);
}

TEST(TradeAddItemTest, RefusesAnItemTheSenderDoesNotHold) {
    RecordingTopology topology;
    TradeAddItemRequest request = addItemRequest();
    request.itemFound = false;
    request.itemTradeable = false;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::ItemNotAddable);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_ADD_ITEM);
    EXPECT_EQ(outcome.rejection().sendObjectID, kTargetOID);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeAddItemTest, RefusesAnItemThatMayNotBeTraded) {
    RecordingTopology topology;
    TradeAddItemRequest request = addItemRequest();
    request.itemTradeable = false;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::ItemNotAddable);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeAddItemTest, RefusesAnItemTheSenderOffersForSale) {
    RecordingTopology topology;
    TradeAddItemRequest request = addItemRequest();
    request.itemInStore = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::ItemNotAddable);
}

TEST(TradeAddItemTest, AnUntradeableGiftBoxIsRefusedAsAnItemNotAsAGiftBox) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.itemTradeable = false;
    request.receiverHasRedGiftBox = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::ItemNotAddable);
    EXPECT_TRUE(outcome.rejection().isTradeError);
}

TEST(TradeAddItemTest, ConfirmsAGreenGiftBoxToAReceiverWithAnEmptyExtraSlot) {
    RecordingTopology topology;

    TradeOutcome outcome = decideTradeAddItem(greenGiftBoxRequest(), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome),
              Lines({"SendVerify(sender, code=11)", "StakeItem", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
    EXPECT_EQ(GC_TRADE_VERIFY_CODE_ADD_ITEM_OK, 11);
    EXPECT_EQ(topology.calls, Lines({"senderStakedItems", "senderStatus"}));
}

TEST(TradeAddItemTest, PassesAGreenGiftBoxWithoutAWordWhenTheExtraSlotHoldsSomethingElse) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.receiverExtraSlotItemPresent = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"StakeItem", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
}

TEST(TradeAddItemTest, RefusesAGreenGiftBoxToAReceiverHoldingARedOne) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.receiverHasRedGiftBox = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
    EXPECT_FALSE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL);
    EXPECT_EQ(outcome.rejection().sendTo, TradePeer::Sender);
    // The trade survives a refused gift box.
    EXPECT_FALSE(outcome.rejection().cancelSenderTrade);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeAddItemTest, RefusesAGreenGiftBoxWhenTheExtraSlotHoldsARedOne) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.receiverExtraSlotItemPresent = true;
    request.receiverExtraSlotIsRedGiftBox = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
    EXPECT_FALSE(outcome.rejection().cancelSenderTrade);
}

TEST(TradeAddItemTest, RefusesASecondGreenGiftBoxToTheSameReceiver) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.receiverReceivedGreenGiftBox = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
}

TEST(TradeAddItemTest, TheGreenGiftBoxFlagOutranksTheRedBoxChecks) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.receiverReceivedGreenGiftBox = true;
    request.receiverHasRedGiftBox = true;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
}

// The vampire and ousters paths never read the flag and pass false, so the
// same receiver takes a second green box from them.
TEST(TradeAddItemTest, AReceiverWhoseFlagIsNotConsultedTakesTheGreenGiftBox) {
    RecordingTopology topology;
    TradeAddItemRequest request = greenGiftBoxRequest();
    request.receiverReceivedGreenGiftBox = false;

    TradeOutcome outcome = decideTradeAddItem(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome),
              Lines({"SendVerify(sender, code=11)", "StakeItem", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
}

TEST(TradeAddItemTest, ARedGiftBoxIsNeverATradeItem) {
    RecordingTopology topology;

    TradeOutcome outcome = decideTradeAddItem(giftBoxRequest(1), topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
    EXPECT_FALSE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL);
    EXPECT_FALSE(outcome.rejection().cancelSenderTrade);
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeAddItemTest, ConfirmsARationedGiftBoxWhenTheTableHoldsNone) {
    RecordingTopology topology;
    topology.staked.push_back(TradeStakedItem(false, 4));
    topology.staked.push_back(TradeStakedItem(true, 0));
    topology.staked.push_back(TradeStakedItem(true, 6));

    TradeOutcome outcome = decideTradeAddItem(giftBoxRequest(2), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome),
              Lines({"SendVerify(sender, code=11)", "StakeItem", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
    EXPECT_EQ(topology.calls, Lines({"senderStakedItems", "senderStatus"}));
}

TEST(TradeAddItemTest, RefusesASecondRationedGiftBoxOnTheSameTable) {
    RecordingTopology topology;
    topology.staked.push_back(TradeStakedItem(true, 5));

    TradeOutcome outcome = decideTradeAddItem(giftBoxRequest(3), topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL);
    EXPECT_FALSE(outcome.rejection().cancelSenderTrade);
    // The status is never read: the table alone decides.
    EXPECT_EQ(topology.calls, Lines({"senderStakedItems"}));
}

TEST(TradeAddItemTest, EveryRationedTypeIsWeighedAgainstEveryOther) {
    for (ItemType_t itemType = 2; itemType < 6; itemType++) {
        RecordingTopology topology;
        topology.staked.push_back(TradeStakedItem(true, 2));

        TradeOutcome outcome = decideTradeAddItem(giftBoxRequest(itemType), topology);

        ASSERT_TRUE(outcome.isRejected()) << "type " << itemType;
        EXPECT_EQ(outcome.rejection().reason, TradeTableReason::GiftBoxRefused);
    }
}

TEST(TradeAddItemTest, ConfirmsAGiftBoxOfTypeSixOrAboveWhateverLiesOnTheTable) {
    RecordingTopology topology;
    topology.staked.push_back(TradeStakedItem(true, 4));

    TradeOutcome outcome = decideTradeAddItem(giftBoxRequest(6), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome),
              Lines({"SendVerify(sender, code=11)", "StakeItem", "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
}

TEST(TradeAddItemTest, AGreenGiftBoxIsConfirmedOnceNotTwice) {
    RecordingTopology topology;
    topology.status = TradeStatus::Finished;

    TradeOutcome outcome = decideTradeAddItem(greenGiftBoxRequest(), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SendVerify(sender, code=11)", "StakeItem", "SendVerify(sender, code=0)",
                                       "ResumeTrading", "SendAddItem(receiver, oid=4711)"}));
}

//////////////////////////////////////////////////////////////////////////////
// Taking an item back
//////////////////////////////////////////////////////////////////////////////

TEST(TradeRemoveItemTest, TakesTheItemOffTheTableAndTellsTheReceiver) {
    RecordingTopology topology;

    TradeOutcome outcome = decideTradeRemoveItem(removeItemRequest(), topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"UnstakeItem", "ResumeTrading", "SendRemoveItem(receiver, oid=4711)"}));
    EXPECT_EQ(topology.calls, Lines({"senderStatus"}));
}

TEST(TradeRemoveItemTest, PushesTheNextOKOutForASlayer) {
    RecordingTopology topology;
    TradeRemoveItemRequest request = removeItemRequest();
    request.delayOK = true;

    TradeOutcome outcome = decideTradeRemoveItem(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome),
              Lines({"UnstakeItem", "DelayOK", "ResumeTrading", "SendRemoveItem(receiver, oid=4711)"}));
}

TEST(TradeRemoveItemTest, VerifiesWhenTheSenderHasAlreadyPressedOK) {
    RecordingTopology topology;
    topology.status = TradeStatus::Finished;
    TradeRemoveItemRequest request = removeItemRequest();
    request.delayOK = true;

    TradeOutcome outcome = decideTradeRemoveItem(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"UnstakeItem", "DelayOK", "SendVerify(sender, code=1)", "ResumeTrading",
                                       "SendRemoveItem(receiver, oid=4711)"}));
    EXPECT_EQ(GC_TRADE_VERIFY_CODE_REMOVE_ITEM, 1);
}

TEST(TradeRemoveItemTest, RefusesAnItemTheSenderDoesNotHold) {
    RecordingTopology topology;
    TradeRemoveItemRequest request = removeItemRequest();
    request.itemFound = false;

    TradeOutcome outcome = decideTradeRemoveItem(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::ItemNotRemovable);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_REMOVE_ITEM);
    EXPECT_EQ(outcome.rejection().sendObjectID, kTargetOID);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    EXPECT_TRUE(topology.calls.empty());
}

//////////////////////////////////////////////////////////////////////////////
// Staking and reclaiming gold
//////////////////////////////////////////////////////////////////////////////

TEST(TradeMoneyTest, IgnoresACodeTheProtocolDoesNotDefine) {
    RecordingTopology topology;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE + 1, 100);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_TRUE(outcome.events().empty());
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeMoneyTest, StakesGoldAndTellsBothPeersWhatMoved) {
    RecordingTopology topology;
    topology.stakedGold = 40;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 100);
    request.senderGold = 1000;
    request.receiverGold = 7;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(900)", "SetStakedGold(140)", "ResumeTrading",
                                       "SendMoney(sender, code=2, oid=815, amount=100)",
                                       "SendMoney(receiver, code=0, oid=4711, amount=100)"}));
    EXPECT_EQ(GC_TRADE_MONEY_INCREASE_RESULT, 2);
    EXPECT_EQ(GC_TRADE_MONEY_INCREASE, 0);
    EXPECT_EQ(topology.calls, Lines({"senderStakedGold", "senderStatus"}));
}

TEST(TradeMoneyTest, StakesNothingWithoutComplaint) {
    RecordingTopology topology;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 0);
    request.senderGold = 0;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(0)", "SetStakedGold(0)", "ResumeTrading",
                                       "SendMoney(sender, code=2, oid=815, amount=0)",
                                       "SendMoney(receiver, code=0, oid=4711, amount=0)"}));
}

TEST(TradeMoneyTest, StakesExactlyThePurse) {
    RecordingTopology topology;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 500);
    request.senderGold = 500;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(0)", "SetStakedGold(500)", "ResumeTrading",
                                       "SendMoney(sender, code=2, oid=815, amount=500)",
                                       "SendMoney(receiver, code=0, oid=4711, amount=500)"}));
}

TEST(TradeMoneyTest, RefusesOneCoinMoreThanThePurseHolds) {
    RecordingTopology topology;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 501);
    request.senderGold = 500;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::NotEnoughGold);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_INCREASE_MONEY);
    EXPECT_EQ(outcome.rejection().sendObjectID, kTargetOID);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    // The record is not asked about until the purse has answered.
    EXPECT_TRUE(topology.calls.empty());
}

TEST(TradeMoneyTest, TrimsAStakeThatWouldCarryTheReceiverPastMaxMoney) {
    RecordingTopology topology;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 100);
    request.senderGold = 1000;
    request.receiverGold = MAX_MONEY - 10;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(990)", "SetStakedGold(10)", "ResumeTrading",
                                       "SendMoney(sender, code=2, oid=815, amount=10)",
                                       "SendMoney(receiver, code=0, oid=4711, amount=10)"}));
}

TEST(TradeMoneyTest, TheGoldAlreadyStakedCountsTowardTheReceiverCap) {
    RecordingTopology topology;
    topology.stakedGold = 900;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 200);
    request.senderGold = 5000;
    request.receiverGold = MAX_MONEY - 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(4900)", "SetStakedGold(1000)", "ResumeTrading",
                                       "SendMoney(sender, code=2, oid=815, amount=100)",
                                       "SendMoney(receiver, code=0, oid=4711, amount=100)"}));
}

TEST(TradeMoneyTest, VerifiesAStakeWhenTheSenderHasAlreadyPressedOK) {
    RecordingTopology topology;
    topology.status = TradeStatus::Finished;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_INCREASE, 100);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(900)", "SetStakedGold(100)", "SendVerify(sender, code=2)",
                                       "ResumeTrading", "SendMoney(sender, code=2, oid=815, amount=100)",
                                       "SendMoney(receiver, code=0, oid=4711, amount=100)"}));
    EXPECT_EQ(GC_TRADE_VERIFY_CODE_MONEY_INCREASE, 2);
}

TEST(TradeMoneyTest, ReclaimsGoldAndTellsBothPeersWhatMoved) {
    RecordingTopology topology;
    topology.stakedGold = 500;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE, 200);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(1200)", "SetStakedGold(300)", "ResumeTrading",
                                       "SendMoney(sender, code=3, oid=815, amount=200)",
                                       "SendMoney(receiver, code=1, oid=4711, amount=200)"}));
    EXPECT_EQ(GC_TRADE_MONEY_DECREASE_RESULT, 3);
    EXPECT_EQ(GC_TRADE_MONEY_DECREASE, 1);
    EXPECT_EQ(topology.calls, Lines({"senderStakedGold", "receiverStakedGold", "senderStatus"}));
}

TEST(TradeMoneyTest, ReclaimsExactlyWhatLiesOnTheTable) {
    RecordingTopology topology;
    topology.stakedGold = 200;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE, 200);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(1200)", "SetStakedGold(0)", "ResumeTrading",
                                       "SendMoney(sender, code=3, oid=815, amount=200)",
                                       "SendMoney(receiver, code=1, oid=4711, amount=200)"}));
}

TEST(TradeMoneyTest, ReclaimsNothingWithoutComplaint) {
    RecordingTopology topology;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE, 0);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(1000)", "SetStakedGold(0)", "ResumeTrading",
                                       "SendMoney(sender, code=3, oid=815, amount=0)",
                                       "SendMoney(receiver, code=1, oid=4711, amount=0)"}));
}

TEST(TradeMoneyTest, RefusesOneCoinMoreThanTheTableHolds) {
    RecordingTopology topology;
    topology.stakedGold = 200;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE, 201);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(outcome.rejection().reason, TradeTableReason::NotEnoughStakedGold);
    EXPECT_TRUE(outcome.rejection().isTradeError);
    EXPECT_EQ(outcome.rejection().code, GC_TRADE_ERROR_CODE_DECREASE_MONEY);
    EXPECT_TRUE(outcome.rejection().cancelSenderTrade);
    // The stake is read to refuse; the receiver's side is not.
    EXPECT_EQ(topology.calls, Lines({"senderStakedGold"}));
}

// The cap on a reclaim weighs the sender's purse against the gold on the
// receiver's side of the table, not against the sender's own stake.
TEST(TradeMoneyTest, TrimsAReclaimAgainstTheGoldOnTheReceiverSide) {
    RecordingTopology topology;
    topology.stakedGold = 500;
    topology.receiverStake = 900;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE, 200);
    request.senderGold = MAX_MONEY - 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(1999999100)", "SetStakedGold(400)", "ResumeTrading",
                                       "SendMoney(sender, code=3, oid=815, amount=100)",
                                       "SendMoney(receiver, code=1, oid=4711, amount=100)"}));
}

TEST(TradeMoneyTest, VerifiesAReclaimWhenTheSenderHasAlreadyPressedOK) {
    RecordingTopology topology;
    topology.status = TradeStatus::Finished;
    topology.stakedGold = 500;
    TradeMoneyRequest request = moneyRequest(CG_TRADE_MONEY_DECREASE, 200);
    request.senderGold = 1000;

    TradeOutcome outcome = decideTradeMoney(request, topology);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(stepsOf(outcome), Lines({"SetSenderGold(1200)", "SetStakedGold(300)", "SendVerify(sender, code=3)",
                                       "ResumeTrading", "SendMoney(sender, code=3, oid=815, amount=200)",
                                       "SendMoney(receiver, code=1, oid=4711, amount=200)"}));
    EXPECT_EQ(GC_TRADE_VERIFY_CODE_MONEY_DECREASE, 3);
}

} // namespace
