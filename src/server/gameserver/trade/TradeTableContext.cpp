//////////////////////////////////////////////////////////////////////////////
// Filename    : TradeTableContext.cpp
// Description : the zone side of the trade table decisions.
//////////////////////////////////////////////////////////////////////////////

#include "TradeTableContext.h"

#include "Creature.h"
#include "CreatureUtil.h"
#include "Effect.h"
#include "Item.h"
#include "Ousters.h"
#include "Slayer.h"
#include "TradeManager.h"
#include "ZoneUtil.h"

namespace {

// A slayer on a motorcycle and an ousters with a summoned sylph both refuse
// to trade; a vampire is never mounted.
bool isMounted(Creature* pCreature) {
    if (pCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
        return pSlayer->hasRideMotorcycle();
    }

    if (pCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
        return pOusters->isFlag(Effect::EFFECT_CLASS_SUMMON_SYLPH);
    }

    return false;
}

} // namespace

ZoneTradeTableTopology::ZoneTradeTableTopology(TradeManager* pTradeManager, Creature* pSender, Creature* pReceiver)
    : m_pTradeManager(pTradeManager), m_pSender(pSender), m_pReceiver(pReceiver) {}

bool ZoneTradeTableTopology::isTrading() {
    return m_pTradeManager->isTrading(m_pSender, m_pReceiver);
}

TradeStatus ZoneTradeTableTopology::senderStatus() {
    return senderInfo()->getStatus() == TRADE_FINISH ? TradeStatus::Finished : TradeStatus::Trading;
}

std::vector<TradeStakedItem> ZoneTradeTableTopology::senderStakedItems() {
    std::vector<TradeStakedItem> staked;

    list<Item*> itemList = senderInfo()->getItemList();
    for (list<Item*>::const_iterator itr = itemList.begin(); itr != itemList.end(); ++itr) {
        Item* pItem = (*itr);
        staked.push_back(
            TradeStakedItem(pItem->getItemClass() == Item::ITEM_CLASS_EVENT_GIFT_BOX, pItem->getItemType()));
    }

    return staked;
}

Gold_t ZoneTradeTableTopology::senderStakedGold() {
    return senderInfo()->getGold();
}

Gold_t ZoneTradeTableTopology::receiverStakedGold() {
    return receiverInfo()->getGold();
}

TradeInfo* ZoneTradeTableTopology::senderInfo() const {
    return m_pTradeManager->getTradeInfo(m_pSender->getName());
}

TradeInfo* ZoneTradeTableTopology::receiverInfo() const {
    return m_pTradeManager->getTradeInfo(m_pReceiver->getName());
}

TradeTableGate tradeTableGateOf(Creature* pSender, Creature* pReceiver, ObjectID_t targetObjectID) {
    TradeTableGate gate;
    gate.targetObjectID = targetObjectID;
    gate.targetExists = pReceiver != NULL;

    if (pReceiver != NULL) {
        const bool sameRace = isSameRace(pReceiver, pSender);

        gate.targetIsSameRacePC = pReceiver->isPC() && sameRace;
        gate.bothInSafeZone = isInSafeZone(pSender) && isInSafeZone(pReceiver);
        // Only a pair of the same race is ever asked about a mount.
        gate.senderMounted = sameRace && isMounted(pSender);
        gate.receiverMounted = sameRace && isMounted(pReceiver);
    }

    return gate;
}
