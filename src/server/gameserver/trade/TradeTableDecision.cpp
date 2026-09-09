//////////////////////////////////////////////////////////////////////////////
// Filename    : TradeTableDecision.cpp
// Description : the rules of the three requests that change an open trade
//               table.
//////////////////////////////////////////////////////////////////////////////

#include "TradeTableDecision.h"

#include "CGTradeMoney.h"
#include "GCTradeError.h"
#include "GCTradeMoney.h"
#include "GCTradeVerify.h"

namespace {

typedef Outcome<TradeTableEvents, TradeTableRejection> Result;
typedef Outcome<void, TradeTableRejection> GateResult;

// A GCTradeError to the sender, echoing the object id the request named, that
// drops whatever trade the sender was in.
TradeTableRejection tradeError(TradeTableReason reason, BYTE code, ObjectID_t targetObjectID) {
    return TradeTableRejection(reason, true, code, TradePeer::Sender, targetObjectID, true);
}

// A GCTradeVerify to the sender. It carries no object id and the trade
// survives it.
TradeTableRejection verifyFailure(TradeTableReason reason, BYTE code) {
    return TradeTableRejection(reason, false, code, TradePeer::Sender, 0, false);
}

TradeTableStep sendVerify(BYTE code) {
    return TradeTableStep(TradeTableAction::SendVerify, TradePeer::Sender, code, 0, 0);
}

TradeTableStep sendMoney(TradePeer sendTo, BYTE code, ObjectID_t objectID, Gold_t amount) {
    return TradeTableStep(TradeTableAction::SendMoney, sendTo, code, objectID, amount);
}

TradeTableStep mutation(TradeTableAction action, Gold_t amount) {
    return TradeTableStep(action, TradePeer::Sender, 0, 0, amount);
}

// A gift box of type 2 to 5. Only one of them may lie on a side of the table.
bool isRationedGiftBox(bool isEventGiftBox, ItemType_t itemType) {
    return isEventGiftBox && itemType > 1 && itemType < 6;
}

// The sender's side of the table changes, so an OK already pressed on it no
// longer counts: the sender is told and both records go back to TRADE_TRADING.
void resumeTrading(TradeTableEvents& steps, TradeTableTopology& topology, BYTE verifyCode) {
    if (topology.senderStatus() == TradeStatus::Finished)
        steps.push_back(sendVerify(verifyCode));

    steps.push_back(mutation(TradeTableAction::ResumeTrading, 0));
}

// The amount that actually moves: whatever of it would carry the receiving
// side past MAX_MONEY is left where it is.
//
// The sum is computed in Gold_t, which is unsigned and 32 bits wide, exactly
// as the handlers did: an amount large enough to wrap it therefore trims to a
// larger amount rather than a smaller one.
Gold_t trimToMaxMoney(Gold_t receivingSide, Gold_t amount) {
    if (receivingSide + amount > MAX_MONEY)
        return amount - (receivingSide + amount - MAX_MONEY);

    return amount;
}

// Gold staked: it leaves the sender's purse for the sender's side of the
// table, trimmed so the receiver can still take it.
Result decideMoneyIncrease(const TradeMoneyRequest& request, TradeTableTopology& topology) {
    if (request.senderGold < request.amount)
        return Result::Rejected(
            tradeError(TradeTableReason::NotEnoughGold, GC_TRADE_ERROR_CODE_INCREASE_MONEY, request.targetObjectID));

    const Gold_t stakedGold = topology.senderStakedGold();
    const Gold_t finalAmount = trimToMaxMoney(request.receiverGold + stakedGold, request.amount);

    TradeTableEvents steps;
    steps.push_back(mutation(TradeTableAction::SetSenderGold, request.senderGold - finalAmount));
    steps.push_back(mutation(TradeTableAction::SetStakedGold, stakedGold + finalAmount));
    resumeTrading(steps, topology, GC_TRADE_VERIFY_CODE_MONEY_INCREASE);

    // The sender learns what actually left its purse, the receiver what
    // arrived on the table.
    steps.push_back(sendMoney(TradePeer::Sender, GC_TRADE_MONEY_INCREASE_RESULT, request.targetObjectID, finalAmount));
    steps.push_back(sendMoney(TradePeer::Receiver, GC_TRADE_MONEY_INCREASE, request.senderObjectID, finalAmount));
    return Result::Ok(steps);
}

// Gold reclaimed: it comes off the sender's side of the table back into the
// sender's purse. The trim weighs the purse against the gold on the
// receiver's side, not the sender's own stake.
Result decideMoneyDecrease(const TradeMoneyRequest& request, TradeTableTopology& topology) {
    const Gold_t stakedGold = topology.senderStakedGold();

    if (stakedGold < request.amount)
        return Result::Rejected(tradeError(TradeTableReason::NotEnoughStakedGold, GC_TRADE_ERROR_CODE_DECREASE_MONEY,
                                           request.targetObjectID));

    const Gold_t finalAmount = trimToMaxMoney(request.senderGold + topology.receiverStakedGold(), request.amount);

    TradeTableEvents steps;
    steps.push_back(mutation(TradeTableAction::SetSenderGold, request.senderGold + finalAmount));
    steps.push_back(mutation(TradeTableAction::SetStakedGold, stakedGold - finalAmount));
    resumeTrading(steps, topology, GC_TRADE_VERIFY_CODE_MONEY_DECREASE);

    steps.push_back(sendMoney(TradePeer::Sender, GC_TRADE_MONEY_DECREASE_RESULT, request.targetObjectID, finalAmount));
    steps.push_back(sendMoney(TradePeer::Receiver, GC_TRADE_MONEY_DECREASE, request.senderObjectID, finalAmount));
    return Result::Ok(steps);
}

// A green gift box may only go to a receiver who holds no red one, in the
// inventory or in the extra slot, and who has not been given a green one
// before. A receiver whose extra slot is empty is told so; one whose extra
// slot holds something else is told nothing and the box goes over anyway.
Outcome<TradeTableEvents, TradeTableRejection> decideGreenGiftBox(const TradeAddItemRequest& request) {
    TradeTableEvents steps;

    if (request.receiverReceivedGreenGiftBox)
        return Result::Rejected(verifyFailure(TradeTableReason::GiftBoxRefused, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL));

    if (request.receiverHasRedGiftBox)
        return Result::Rejected(verifyFailure(TradeTableReason::GiftBoxRefused, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL));

    if (request.receiverExtraSlotItemPresent) {
        if (request.receiverExtraSlotIsRedGiftBox)
            return Result::Rejected(
                verifyFailure(TradeTableReason::GiftBoxRefused, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL));
    } else {
        steps.push_back(sendVerify(GC_TRADE_VERIFY_CODE_ADD_ITEM_OK));
    }

    return Result::Ok(steps);
}

} // namespace

Outcome<void, TradeTableRejection> decideTradeTableGate(const TradeTableGate& gate, TradeTableTopology& topology) {
    if (!gate.targetExists)
        return GateResult::Rejected(
            tradeError(TradeTableReason::TargetMissing, GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST, gate.targetObjectID));

    if (!gate.targetIsSameRacePC)
        return GateResult::Rejected(
            tradeError(TradeTableReason::RaceDiffer, GC_TRADE_ERROR_CODE_RACE_DIFFER, gate.targetObjectID));

    if (!gate.bothInSafeZone)
        return GateResult::Rejected(
            tradeError(TradeTableReason::NotSafe, GC_TRADE_ERROR_CODE_NOT_SAFE, gate.targetObjectID));

    if (gate.senderMounted || gate.receiverMounted)
        return GateResult::Rejected(
            tradeError(TradeTableReason::Motorcycle, GC_TRADE_ERROR_CODE_MOTORCYCLE, gate.targetObjectID));

    if (!topology.isTrading())
        return GateResult::Rejected(
            tradeError(TradeTableReason::NotTrading, GC_TRADE_ERROR_CODE_NOT_TRADING, gate.targetObjectID));

    return GateResult::Ok();
}

Outcome<TradeTableEvents, TradeTableRejection> decideTradeAddItem(const TradeAddItemRequest& request,
                                                                  TradeTableTopology& topology) {
    if (!request.itemFound || !request.itemTradeable || request.itemInStore)
        return Result::Rejected(
            tradeError(TradeTableReason::ItemNotAddable, GC_TRADE_ERROR_CODE_ADD_ITEM, request.targetObjectID));

    TradeTableEvents steps;

    if (request.itemIsEventGiftBox && request.itemType == 0) {
        Result green = decideGreenGiftBox(request);
        if (green.isRejected())
            return green;

        steps = green.events();
    } else if (request.itemIsEventGiftBox && request.itemType == 1) {
        // A red gift box is never a trade item.
        return Result::Rejected(verifyFailure(TradeTableReason::GiftBoxRefused, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL));
    }

    // Read for every item, used by the rationed boxes.
    const std::vector<TradeStakedItem> staked = topology.senderStakedItems();

    if (isRationedGiftBox(request.itemIsEventGiftBox, request.itemType)) {
        for (std::vector<TradeStakedItem>::const_iterator itr = staked.begin(); itr != staked.end(); ++itr) {
            if (isRationedGiftBox(itr->isEventGiftBox, itr->itemType))
                return Result::Rejected(
                    verifyFailure(TradeTableReason::GiftBoxRefused, GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL));
        }

        steps.push_back(sendVerify(GC_TRADE_VERIFY_CODE_ADD_ITEM_OK));
    } else if (request.itemIsEventGiftBox && request.itemType >= 6) {
        steps.push_back(sendVerify(GC_TRADE_VERIFY_CODE_ADD_ITEM_OK));
    }

    steps.push_back(mutation(TradeTableAction::StakeItem, 0));
    resumeTrading(steps, topology, GC_TRADE_VERIFY_CODE_ADD_ITEM_WHEN_ACCEPT);
    steps.push_back(TradeTableStep(TradeTableAction::SendAddItem, TradePeer::Receiver, 0, request.senderObjectID, 0));
    return Result::Ok(steps);
}

Outcome<TradeTableEvents, TradeTableRejection> decideTradeRemoveItem(const TradeRemoveItemRequest& request,
                                                                     TradeTableTopology& topology) {
    if (!request.itemFound)
        return Result::Rejected(
            tradeError(TradeTableReason::ItemNotRemovable, GC_TRADE_ERROR_CODE_REMOVE_ITEM, request.targetObjectID));

    TradeTableEvents steps;
    steps.push_back(mutation(TradeTableAction::UnstakeItem, 0));

    if (request.delayOK)
        steps.push_back(mutation(TradeTableAction::DelayOK, 0));

    resumeTrading(steps, topology, GC_TRADE_VERIFY_CODE_REMOVE_ITEM);
    steps.push_back(
        TradeTableStep(TradeTableAction::SendRemoveItem, TradePeer::Receiver, 0, request.senderObjectID, 0));
    return Result::Ok(steps);
}

Outcome<TradeTableEvents, TradeTableRejection> decideTradeMoney(const TradeMoneyRequest& request,
                                                                TradeTableTopology& topology) {
    if (request.code == CG_TRADE_MONEY_INCREASE)
        return decideMoneyIncrease(request, topology);

    if (request.code == CG_TRADE_MONEY_DECREASE)
        return decideMoneyDecrease(request, topology);

    // A code the protocol does not define is ignored: nothing is sent and
    // nothing changes.
    return Result::Ok(TradeTableEvents());
}
