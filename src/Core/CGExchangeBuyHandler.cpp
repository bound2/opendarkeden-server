//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuyHandler.cpp
// Written By  : Exchange System
// Description : Handler for CGExchangeBuy
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"
#include "GCExchangeBuy.h"

#ifdef __GAME_SERVER__
#include <stdlib.h>

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
    pair<bool, string> result = ExchangeService::buyListing(pPC, pPacket->getListingID(), pPacket->getIdempotencyKey());

    // Send response
    GCExchangeBuy gcPacket;
    gcPacket.setSuccess(result.first);
    gcPacket.setMessage(result.second);

    if (result.first) {
        // On success the service returns the new order id as a decimal string.
        // The client parses m_OrderID out of this reply, so it has to be carried
        // across; on failure the string is an error message and the id stays 0.
        gcPacket.setOrderID((int64_t)strtoll(result.second.c_str(), NULL, 10));
    }

    pPlayer->sendPacket(&gcPacket);

#endif // __GAME_SERVER__

    __END_CATCH
}
