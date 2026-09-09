//////////////////////////////////////////////////////////////////////////////
// Filename    : TradePrepareDecision.cpp
// Description : the trade prepare protocol's rules.
//////////////////////////////////////////////////////////////////////////////

#include "TradePrepareDecision.h"

#include "CGTradePrepare.h"
#include "GCTradeError.h"
#include "GCTradePrepare.h"

namespace {

typedef Outcome<TradePrepareEvents, TradePrepareRejection> Result;

// A GCTradeError to the sender, echoing the object id its request named.
TradePrepareRejection tradeError(TradePrepareReason reason, BYTE code, const TradePrepareRequest& request,
                                 TradeCancel cancel) {
    return TradePrepareRejection(reason, true, code, TradePeer::Sender, request.targetObjectID, cancel);
}

// A refusal that sends nothing: the caller raises ProtocolException.
TradePrepareRejection protocolError(TradePrepareReason reason, const TradePrepareRequest& request) {
    return TradePrepareRejection(reason, false, 0, TradePeer::Sender, request.targetObjectID, TradeCancel::None);
}

// A GCTradePrepare the sender's own object id identifies, which is how the
// receiver learns who is asking.
TradePrepareEvents tellReceiver(const TradePrepareRequest& request, BYTE code, TradeCancel cancel) {
    TradePrepareEvents events;
    events.sendPrepare = true;
    events.sendTo = TradePeer::Receiver;
    events.sendObjectID = request.senderObjectID;
    events.sendCode = code;
    events.cancel = cancel;
    return events;
}

// The trade the sender asks for: refused while the sender is trading, and
// answered with BUSY while the receiver is.
Result decideRequest(const TradePrepareRequest& request, bool senderTrading, bool receiverTrading) {
    if (senderTrading)
        return Result::Rejected(tradeError(TradePrepareReason::AlreadyTrading, GC_TRADE_ERROR_CODE_ALREADY_TRADING,
                                           request, TradeCancel::Sender));

    if (receiverTrading) {
        // The one refusal that goes out as a GCTradePrepare, carrying the
        // object id the sender asked about rather than its own.
        return Result::Rejected(TradePrepareRejection(TradePrepareReason::TargetBusy, false, GC_TRADE_PREPARE_CODE_BUSY,
                                                      TradePeer::Sender, request.targetObjectID, TradeCancel::None));
    }

    TradePrepareEvents events = tellReceiver(request, GC_TRADE_PREPARE_CODE_REQUEST, TradeCancel::None);
    events.initTrade = true;
    return Result::Ok(events);
}

// An answer to a trade that has to be open: the receiver is told, and the
// trade is closed unless the answer keeps it going.
Result decideAnswer(const TradePrepareRequest& request, TradePrepareTopology& topology, BYTE code, TradeCancel cancel) {
    if (!topology.isTrading())
        return Result::Rejected(
            tradeError(TradePrepareReason::NotTrading, GC_TRADE_ERROR_CODE_NOT_TRADING, request, TradeCancel::None));

    return Result::Ok(tellReceiver(request, code, cancel));
}

} // namespace

Outcome<TradePrepareEvents, TradePrepareRejection> decideTradePrepare(const TradePrepareRequest& request,
                                                                      TradePrepareTopology& topology) {
    // A refusal here also drops whatever trade the sender was in, so a
    // client that keeps asking cannot leave its items staked.
    if (!request.targetExists)
        return Result::Rejected(tradeError(TradePrepareReason::TargetMissing, GC_TRADE_ERROR_CODE_TARGET_NOT_EXIST,
                                           request, TradeCancel::Sender));

    // Trading with oneself would move items between two copies of the same
    // character, so the connection is dropped instead.
    if (request.targetIsSelf)
        return Result::Rejected(protocolError(TradePrepareReason::SameCreature, request));

    if (!request.targetIsSameRacePC)
        return Result::Rejected(
            tradeError(TradePrepareReason::RaceDiffer, GC_TRADE_ERROR_CODE_RACE_DIFFER, request, TradeCancel::Sender));

    if (!request.bothInSafeZone)
        return Result::Rejected(
            tradeError(TradePrepareReason::NotSafe, GC_TRADE_ERROR_CODE_NOT_SAFE, request, TradeCancel::Sender));

    if (request.senderMounted || request.receiverMounted)
        return Result::Rejected(
            tradeError(TradePrepareReason::Motorcycle, GC_TRADE_ERROR_CODE_MOTORCYCLE, request, TradeCancel::Sender));

    // Read for every code, used by the request.
    const bool senderTrading = topology.senderHasTradeInfo();
    const bool receiverTrading = topology.receiverHasTradeInfo();

    switch (request.code) {
    case CG_TRADE_PREPARE_CODE_REQUEST:
        return decideRequest(request, senderTrading, receiverTrading);

    case CG_TRADE_PREPARE_CODE_CANCEL:
        return decideAnswer(request, topology, GC_TRADE_PREPARE_CODE_CANCEL, TradeCancel::Pair);

    // The trade stays open: this is the answer that starts it.
    case CG_TRADE_PREPARE_CODE_ACCEPT:
        return decideAnswer(request, topology, GC_TRADE_PREPARE_CODE_ACCEPT, TradeCancel::None);

    case CG_TRADE_PREPARE_CODE_REJECT:
        return decideAnswer(request, topology, GC_TRADE_PREPARE_CODE_REJECT, TradeCancel::Pair);

    case CG_TRADE_PREPARE_CODE_BUSY:
        return decideAnswer(request, topology, GC_TRADE_PREPARE_CODE_BUSY, TradeCancel::Pair);

    default:
        return Result::Rejected(protocolError(TradePrepareReason::UnknownCode, request));
    }
}
