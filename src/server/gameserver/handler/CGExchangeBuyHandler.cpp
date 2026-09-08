//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuyHandler.cpp
// Written By  : Exchange System
// Description : Handler for CGExchangeBuy
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"
#include "GCExchangeBuy.h"

#ifdef __GAME_SERVER__
#include <string>

#include "../server/gameserver/exchange/ExchangeService.h"
#include "GamePlayer.h"
#include "PlayerCreature.h"
#endif

void CGExchangeBuyHandler::execute(CGExchangeBuy* pPacket, Player* pPlayer) {
    __BEGIN_TRY

#ifdef __GAME_SERVER__

    // Validate player
    if (pPlayer == NULL)
        return;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pPlayer);
    if (pPC == NULL)
        return;

    // Call service to buy
    Outcome<ExchangePurchase, ExchangeRejection> result =
        ExchangeService::buyListing(pPC, pPacket->getListingID(), pPacket->getIdempotencyKey());

    // Send response
    GCExchangeBuy gcPacket;

    if (result.isOk()) {
        const ExchangePurchase& purchase = result.events();
        gcPacket.setSuccess(true);
        // The reply carries the new order id twice: in m_OrderID, and as the
        // decimal string in m_Message, which is what the client reads.
        gcPacket.setMessage(std::to_string(purchase.orderID));
        gcPacket.setOrderID(purchase.orderID);
    } else {
        // A refusal puts the reason's text in m_Message and leaves the id 0.
        gcPacket.setSuccess(false);
        gcPacket.setMessage(result.rejection().message());
    }

    pPlayer->sendPacket(&gcPacket);

#endif // __GAME_SERVER__

    __END_CATCH
}
