//////////////////////////////////////////////////////////////////////////////
// Filename    : TradeTableContext.h
// Description : binds the trade table decisions to a zone: the topology that
//               answers their queries out of the zone's TradeManager, and the
//               gate facts read off the two creatures. This is the half that
//               needs a creature; TradeTableDecision.h is the half that does
//               not.
//////////////////////////////////////////////////////////////////////////////

#ifndef __TRADE_TABLE_CONTEXT_H__
#define __TRADE_TABLE_CONTEXT_H__

#include "TradeTableDecision.h"

class Creature;
class TradeInfo;
class TradeManager;

// The zone's trade records. Every query but isTrading() is made after the
// gate has established that both records exist.
class ZoneTradeTableTopology : public TradeTableTopology {
public:
    ZoneTradeTableTopology(TradeManager* pTradeManager, Creature* pSender, Creature* pReceiver);

    bool isTrading() override;
    TradeStatus senderStatus() override;
    std::vector<TradeStakedItem> senderStakedItems() override;
    Gold_t senderStakedGold() override;
    Gold_t receiverStakedGold() override;

    // The two records, for the mutations the handlers perform.
    TradeInfo* senderInfo() const;
    TradeInfo* receiverInfo() const;

private:
    TradeManager* m_pTradeManager;
    Creature* m_pSender;
    Creature* m_pReceiver;
};

// The pair a trade-table request names. A receiver the zone does not hold
// leaves every flag false.
TradeTableGate tradeTableGateOf(Creature* pSender, Creature* pReceiver, ObjectID_t targetObjectID);

#endif // __TRADE_TABLE_CONTEXT_H__
