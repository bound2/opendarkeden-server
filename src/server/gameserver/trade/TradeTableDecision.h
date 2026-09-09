//////////////////////////////////////////////////////////////////////////////
// Filename    : TradeTableDecision.h
// Description : the rules of the three requests that change what lies on an
//               open trade table - an item put down, an item taken back, gold
//               staked or reclaimed - kept apart from the handlers so they can
//               be exercised with neither a creature nor an inventory.
//               Everything here needs a TradeTableTopology and plain values
//               only.
//////////////////////////////////////////////////////////////////////////////

#ifndef __TRADE_TABLE_DECISION_H__
#define __TRADE_TABLE_DECISION_H__

#include <vector>

#include "Outcome.h"
#include "Types.h"

// TradePeer: who a packet goes to. The whole trade protocol shares it.
#include "TradePrepareDecision.h"

//////////////////////////////////////////////////////////////////////////////
// Vocabulary
//////////////////////////////////////////////////////////////////////////////

// The two states a trade record is in: TRADE_TRADING while the table is still
// being laid, TRADE_FINISH once its owner has pressed OK.
enum class TradeStatus { Trading, Finished };

// One thing the caller performs, in the order the events list them.
enum class TradeTableAction {
    // A GCTradeVerify carrying code, to sendTo.
    SendVerify,
    // A GCTradeAddItem describing the item the request named, to sendTo,
    // carrying objectID as its target.
    SendAddItem,
    // A GCTradeRemoveItem naming that item, to sendTo, carrying objectID as
    // its target.
    SendRemoveItem,
    // A GCTradeMoney carrying code, amount and objectID, to sendTo.
    SendMoney,
    // Put the item on the sender's side of the table.
    StakeItem,
    // Take it off again.
    UnstakeItem,
    // Push the sender's next allowed OK out.
    DelayOK,
    // The sender's purse becomes amount.
    SetSenderGold,
    // The gold on the sender's side of the table becomes amount.
    SetStakedGold,
    // Both records go back to TRADE_TRADING, so a table that changed under an
    // OK has to be accepted again.
    ResumeTrading
};

struct TradeTableStep {
    TradeTableStep(TradeTableAction stepAction, TradePeer recipient, BYTE stepCode, ObjectID_t stepObjectID,
                   Gold_t stepAmount)
        : action(stepAction), sendTo(recipient), code(stepCode), objectID(stepObjectID), amount(stepAmount) {}

    TradeTableAction action;
    // Meaningful for the Send* actions.
    TradePeer sendTo;
    // GC_TRADE_VERIFY_CODE_* or GC_TRADE_MONEY_*, matching the action.
    BYTE code;
    // The object id the packet carries.
    ObjectID_t objectID;
    // SendMoney's amount, or the new value SetSenderGold / SetStakedGold
    // writes.
    Gold_t amount;
};

typedef std::vector<TradeTableStep> TradeTableEvents;

// Why a request was refused.
enum class TradeTableReason {
    // GCTradeError(GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST). The named creature
    // is not in the sender's zone.
    TargetMissing,
    // GCTradeError(GC_TRADE_ERROR_CODE_RACE_DIFFER). The target is a monster
    // or an NPC, or a player character of another race.
    RaceDiffer,
    // GCTradeError(GC_TRADE_ERROR_CODE_NOT_SAFE). Trading is only allowed
    // while both stand in a safe zone.
    NotSafe,
    // GCTradeError(GC_TRADE_ERROR_CODE_MOTORCYCLE). One of the two is
    // mounted: a slayer on a motorcycle, or an ousters with a summoned sylph.
    Motorcycle,
    // GCTradeError(GC_TRADE_ERROR_CODE_NOT_TRADING). No trade is open between
    // these two.
    NotTrading,
    // GCTradeError(GC_TRADE_ERROR_CODE_ADD_ITEM). The item is not in the
    // sender's inventory, may not be traded, or is already for sale.
    ItemNotAddable,
    // GCTradeVerify(GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL). An event gift box
    // the receiver may not be given. The trade survives.
    GiftBoxRefused,
    // GCTradeError(GC_TRADE_ERROR_CODE_REMOVE_ITEM). The item is not in the
    // sender's inventory.
    ItemNotRemovable,
    // GCTradeError(GC_TRADE_ERROR_CODE_INCREASE_MONEY). The sender's purse
    // does not hold the amount it offers to stake.
    NotEnoughGold,
    // GCTradeError(GC_TRADE_ERROR_CODE_DECREASE_MONEY). The sender's side of
    // the table does not hold the amount it asks back.
    NotEnoughStakedGold
};

// A refusal. Every one of them answers the sender.
struct TradeTableRejection {
    TradeTableRejection(TradeTableReason rejectReason, bool sendError, BYTE rejectCode, TradePeer recipient,
                        ObjectID_t objectID, bool dropTrade)
        : reason(rejectReason), isTradeError(sendError), code(rejectCode), sendTo(recipient), sendObjectID(objectID),
          cancelSenderTrade(dropTrade) {}

    TradeTableReason reason;
    // True for a GCTradeError, false for a GCTradeVerify.
    bool isTradeError;
    // GC_TRADE_ERROR_CODE_* or GC_TRADE_VERIFY_CODE_*, matching isTradeError.
    BYTE code;
    TradePeer sendTo;
    // The request's own target object id. A GCTradeVerify carries none.
    ObjectID_t sendObjectID;
    // cancelTrade(sender), performed before the packet goes out.
    bool cancelSenderTrade;
};

//////////////////////////////////////////////////////////////////////////////
// The trade records
//////////////////////////////////////////////////////////////////////////////

// One item lying on the sender's side of the table, as the rules read it.
struct TradeStakedItem {
    TradeStakedItem(bool giftBox, ItemType_t type) : isEventGiftBox(giftBox), itemType(type) {}

    bool isEventGiftBox;
    ItemType_t itemType;
};

// The reads that go to the zone's TradeManager and its two trade records, so
// the order they are made in - and the branches that never make them - are
// part of the contract. The gold of a record is read once even where the old
// handler read it twice.
class TradeTableTopology {
public:
    virtual ~TradeTableTopology() {}

    // Is a trade open between these two? (TradeManager::isTrading.)
    virtual bool isTrading() = 0;
    // The sender's own record: has it pressed OK?
    virtual TradeStatus senderStatus() = 0;
    // What already lies on the sender's side of the table.
    virtual std::vector<TradeStakedItem> senderStakedItems() = 0;
    // The gold on the sender's side.
    virtual Gold_t senderStakedGold() = 0;
    // The gold on the receiver's side.
    virtual Gold_t receiverStakedGold() = 0;
};

//////////////////////////////////////////////////////////////////////////////
// The requests
//////////////////////////////////////////////////////////////////////////////

// The pair a trade-table request is about, as the gate reads them. These are
// plain reads off a creature and its zone tile, with no lock and no side
// effect, unlike the topology's queries.
struct TradeTableGate {
    // The packet's field, echoed back on every refusal.
    ObjectID_t targetObjectID = 0;
    // Is a creature of that object id in the sender's zone?
    bool targetExists = false;
    // Is it a player character of the sender's race?
    bool targetIsSameRacePC = false;
    // Are both of them standing in a safe zone?
    bool bothInSafeZone = false;
    // Mounted: a slayer riding a motorcycle, or an ousters with a summoned
    // sylph. Only a pair of the same race can be mounted, and a vampire never
    // is.
    bool senderMounted = false;
    bool receiverMounted = false;
};

// A CGTradeAddItem and the item it names.
struct TradeAddItemRequest {
    ObjectID_t senderObjectID = 0;
    ObjectID_t targetObjectID = 0;

    // Is an item of that object id in the sender's inventory?
    bool itemFound = false;
    // May it be traded at all? (ItemUtil's canTrade: relics, couple rings,
    // quest, flag, time-limited, sweeper and several event classes may not.)
    bool itemTradeable = false;
    // The sender's store already offers it for sale.
    bool itemInStore = false;

    bool itemIsEventGiftBox = false;
    ItemType_t itemType = 0;

    // The four below are read only for a green gift box (an event gift box of
    // type 0), the one item whose receiver is inspected.
    //
    // The receiver has been given a green gift box before. Only the slayer
    // path consults this; the vampire and ousters paths never look at the
    // flag and pass false.
    bool receiverReceivedGreenGiftBox = false;
    // A red gift box lies in the receiver's inventory.
    bool receiverHasRedGiftBox = false;
    // The receiver's extra inventory slot holds something.
    bool receiverExtraSlotItemPresent = false;
    // And that something is a red gift box.
    bool receiverExtraSlotIsRedGiftBox = false;
};

// A CGTradeRemoveItem and the item it names.
struct TradeRemoveItemRequest {
    ObjectID_t senderObjectID = 0;
    ObjectID_t targetObjectID = 0;

    // Is an item of that object id in the sender's inventory?
    bool itemFound = false;
    // The slayer path pushes the sender's next allowed OK out; the vampire
    // and ousters paths leave it alone.
    bool delayOK = false;
};

// A CGTradeMoney, the amount it names and the two purses.
struct TradeMoneyRequest {
    ObjectID_t senderObjectID = 0;
    ObjectID_t targetObjectID = 0;

    // One of CG_TRADE_MONEY_*. Any other code does nothing at all.
    BYTE code = 0;
    Gold_t amount = 0;

    // The two purses, as the creatures hold them.
    Gold_t senderGold = 0;
    Gold_t receiverGold = 0;
};

//////////////////////////////////////////////////////////////////////////////
// The decisions
//////////////////////////////////////////////////////////////////////////////

// The gate all three requests pass, in this order: the target has to be in
// the zone, be a player character of the sender's race, stand with the sender
// in a safe zone, and neither may be mounted; then a trade has to be open
// between the two. Every refusal drops whatever trade the sender was in, so a
// client that keeps asking cannot leave its items staked.
[[nodiscard]] Outcome<void, TradeTableRejection> decideTradeTableGate(const TradeTableGate& gate,
                                                                      TradeTableTopology& topology);

// What does this CGTradeAddItem do?
//
// An item the sender does not hold, may not trade, or already offers for sale
// is refused and the trade dropped. An event gift box is then weighed against
// what the receiver holds and what already lies on the table: a red one may
// never be given, a green one only to a receiver with no red one anywhere,
// and only one of the boxes of type 2 to 5 may lie on a side at a time. Those
// refusals answer a verify packet and leave the trade standing. Otherwise the
// item is staked, the receiver is told, and both records go back to
// TRADE_TRADING - preceded by a verify packet when the sender had already
// pressed OK.
[[nodiscard]] Outcome<TradeTableEvents, TradeTableRejection> decideTradeAddItem(const TradeAddItemRequest& request,
                                                                                TradeTableTopology& topology);

// What does this CGTradeRemoveItem do?
//
// An item the sender does not hold is refused and the trade dropped. The rest
// mirrors the add: the item comes off the table, the receiver is told, and
// both records go back to TRADE_TRADING.
[[nodiscard]] Outcome<TradeTableEvents, TradeTableRejection>
decideTradeRemoveItem(const TradeRemoveItemRequest& request, TradeTableTopology& topology);

// What does this CGTradeMoney do?
//
// A stake moves gold from the sender's purse onto the table, and is refused
// when the purse is short; reclaiming moves it back, and is refused when the
// table is short. Either way the amount is trimmed so the side that receives
// it does not pass MAX_MONEY, and both peers are told what actually moved.
[[nodiscard]] Outcome<TradeTableEvents, TradeTableRejection> decideTradeMoney(const TradeMoneyRequest& request,
                                                                              TradeTableTopology& topology);

#endif // __TRADE_TABLE_DECISION_H__
