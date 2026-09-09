//////////////////////////////////////////////////////////////////////////////
// Filename    : TradePrepareDecision.h
// Description : the trade prepare protocol's rules, kept apart from the
//               handler so they can be exercised with neither a creature nor
//               a zone. Everything here needs a TradePrepareTopology and
//               plain values only.
//////////////////////////////////////////////////////////////////////////////

#ifndef __TRADE_PREPARE_DECISION_H__
#define __TRADE_PREPARE_DECISION_H__

#include "Outcome.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Vocabulary
//////////////////////////////////////////////////////////////////////////////

// Who a packet goes to. The sender is the player whose CGTradePrepare this
// is; the receiver is the creature the packet names.
enum class TradePeer { Sender, Receiver };

// Which TradeManager::cancelTrade the caller performs, if any.
enum class TradeCancel {
    None,
    // cancelTrade(sender): drops whatever trade the sender is in, refunding
    // both sides' staked gold and telling the partner it finds.
    Sender,
    // cancelTrade(sender, receiver): drops the trade between these two, and
    // does nothing at all unless the pair really is trading.
    Pair
};

// Why the request was refused. Each carries one packet to the sender, except
// SameCreature and UnknownCode, which send nothing.
enum class TradePrepareReason {
    // GCTradeError(GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST). The named creature
    // is not in the sender's zone.
    TargetMissing,
    // Nothing is sent; the caller logs the name and throws
    // ProtocolException. A request naming the sender itself means one
    // character is logged in twice.
    SameCreature,
    // GCTradeError(GC_TRADE_ERROR_CODE_RACE_DIFFER). The target is a monster
    // or an NPC, or a player character of another race.
    RaceDiffer,
    // GCTradeError(GC_TRADE_ERROR_CODE_NOT_SAFE). Trading is only allowed
    // while both stand in a safe zone.
    NotSafe,
    // GCTradeError(GC_TRADE_ERROR_CODE_MOTORCYCLE). One of the two is
    // mounted: a slayer on a motorcycle, or an ousters with a summoned
    // sylph.
    Motorcycle,
    // GCTradeError(GC_TRADE_ERROR_CODE_ALREADY_TRADING). The sender is in a
    // trade already.
    AlreadyTrading,
    // GCTradePrepare(GC_TRADE_PREPARE_CODE_BUSY). The target is in a trade
    // already, so it cannot answer this one.
    TargetBusy,
    // GCTradeError(GC_TRADE_ERROR_CODE_NOT_TRADING). An answer to a trade
    // that is not open.
    NotTrading,
    // Nothing is sent; the caller throws ProtocolException.
    UnknownCode
};

// A refusal. Every one of them answers the sender, echoing the target object
// id the request carried.
struct TradePrepareRejection {
    TradePrepareRejection(TradePrepareReason rejectReason, bool sendError, BYTE rejectCode, TradePeer recipient,
                          ObjectID_t objectID, TradeCancel cancelTrade)
        : reason(rejectReason), isTradeError(sendError), code(rejectCode), sendTo(recipient), sendObjectID(objectID),
          cancel(cancelTrade) {}

    TradePrepareReason reason;
    // True for a GCTradeError, false for a GCTradePrepare. Meaningless for
    // SameCreature and UnknownCode, which send nothing.
    bool isTradeError;
    // GC_TRADE_ERROR_CODE_* or GC_TRADE_PREPARE_CODE_*, matching
    // isTradeError.
    BYTE code;
    // Every refusal the protocol has answers the sender.
    TradePeer sendTo;
    // The request's own target object id.
    ObjectID_t sendObjectID;
    // Performed before the packet goes out.
    TradeCancel cancel;
};

// What the caller performs, in this order: the packet, then the trade
// records.
struct TradePrepareEvents {
    // A GCTradePrepare to one of the two peers, carrying sendObjectID and
    // sendCode.
    bool sendPrepare = false;
    TradePeer sendTo = TradePeer::Receiver;
    ObjectID_t sendObjectID = 0;
    // One of GC_TRADE_PREPARE_CODE_*.
    BYTE sendCode = 0;

    // Open a trade between the two, so the receiver may answer it.
    bool initTrade = false;

    // Close it again.
    TradeCancel cancel = TradeCancel::None;
};

//////////////////////////////////////////////////////////////////////////////
// The request
//////////////////////////////////////////////////////////////////////////////

// A CGTradePrepare and the two creatures it is about.
//
// The gate flags are read up front although only some branches use them:
// they are plain reads off a creature and its zone tile, with no lock and no
// side effect, unlike the topology's queries.
struct TradePrepareRequest {
    // One of CG_TRADE_PREPARE_CODE_*.
    BYTE code = 0;
    ObjectID_t senderObjectID = 0;
    // The packet's field, echoed back on every refusal.
    ObjectID_t targetObjectID = 0;
    // Is a creature of that object id in the sender's zone?
    bool targetExists = false;
    // Does it carry the sender's own name?
    bool targetIsSelf = false;
    // Is it a player character of the sender's race?
    bool targetIsSameRacePC = false;
    // Are both of them standing in a safe zone?
    bool bothInSafeZone = false;
    // Mounted: a slayer riding a motorcycle, or an ousters with a summoned
    // sylph. Only a pair of the same race can be mounted, and a vampire
    // never is.
    bool senderMounted = false;
    bool receiverMounted = false;
};

// The trade records a decision reads. These are the reads that go to the
// zone's TradeManager, so the order they are made in is part of the
// contract.
class TradePrepareTopology {
public:
    virtual ~TradePrepareTopology() {}

    // Is the sender in a trade, with anybody?
    // (TradeManager::hasTradeInfo of the sender's name.)
    virtual bool senderHasTradeInfo() = 0;
    // The same for the receiver.
    virtual bool receiverHasTradeInfo() = 0;
    // Is a trade open between these two? (TradeManager::isTrading.)
    virtual bool isTrading() = 0;
};

//////////////////////////////////////////////////////////////////////////////
// The decision
//////////////////////////////////////////////////////////////////////////////

// What does this CGTradePrepare do?
//
// A request that names no creature, the sender itself, a creature that is
// not a player character of the same race, one of a pair that is not in a
// safe zone, or a mounted pair is refused before anything is asked of the
// topology. Every one of those refusals except the sender-itself one first
// drops whatever trade the sender was in. Past that:
//
//   REQUEST  refused when the sender is trading already, answered with BUSY
//            when the receiver is; otherwise the receiver is asked and the
//            trade is opened.
//   CANCEL   withdraws an open trade, telling the receiver, and closes it.
//   ACCEPT   tells the receiver the trade is on and leaves it open.
//   REJECT   turns an open trade down, telling the receiver, and closes it.
//   BUSY     tells the receiver it cannot be answered, and closes it.
//
// CANCEL, ACCEPT, REJECT and BUSY are all refused when no trade is open.
[[nodiscard]] Outcome<TradePrepareEvents, TradePrepareRejection> decideTradePrepare(const TradePrepareRequest& request,
                                                                                    TradePrepareTopology& topology);

#endif // __TRADE_PREPARE_DECISION_H__
