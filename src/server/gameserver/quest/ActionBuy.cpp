////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionBuy.cpp
// Written By  :
// Description :
// Action where the NPC buys items from the player. Like the Sell
// action, it does nothing special; it only sends the player the
// current shop version as a packet.
////////////////////////////////////////////////////////////////////////////////

#include "ActionBuy.h"

#include "Creature.h"
#include "GCNPCResponse.h"
#include "GCShopMarketCondition.h"
#include "GamePlayer.h"
#include "NPC.h"

////////////////////////////////////////////////////////////////////////////////
// The ActionBuy action only sends the ShopVersion to the client, so
// there are no parameters that need reading.
//
// If a shop that buys only certain kinds of item is wanted later,
// a property can be added and read here. The NPC part would need
// changing too.
////////////////////////////////////////////////////////////////////////////////
void ActionBuy::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY
    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionBuy::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    NPC* pNPC = dynamic_cast<NPC*>(pCreature1);
    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);

    // First tell the client that the answer was received.
    GCNPCResponse okpkt;
    pPlayer->sendPacket(&okpkt);

    // Report the current market conditions.
    GCShopMarketCondition pkt;
    pkt.setObjectID(pNPC->getObjectID());
    pkt.setMarketCondBuy(pNPC->getMarketCondBuy());
    pkt.setMarketCondSell(pNPC->getMarketCondSell());
    pPlayer->sendPacket(&pkt);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionBuy::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionBuy(" << ")";
    return msg.toString();

    __END_CATCH
}
