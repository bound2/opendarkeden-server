//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuyHandler.cpp
// Written By  : Exchange System
// Description : Handler for CGExchangeBuy
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"
#include "GCExchangeBuy.h"

#ifdef __GAME_SERVER__
#include "PlayerCreature.h"
#include "GamePlayer.h"
#include "../server/gameserver/exchange/ExchangeService.h"
#endif

void CGExchangeBuyHandler::execute(CGExchangeBuy* pPacket, Player* pPlayer) {
    __BEGIN_TRY

#ifdef __GAME_SERVER__

    // Validate player
    if (pPlayer == NULL) return;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pPlayer);
    if (pPC == NULL) return;

    // Call service to buy
    pair<bool, string> result = ExchangeService::buyListing(
        pPC,
        pPacket->getListingID(),
        pPacket->getIdempotencyKey()
    );

    // Send response
    GCExchangeBuy gcPacket;
    gcPacket.setSuccess(result.first);
    gcPacket.setMessage(result.second);

    pPlayer->sendPacket(&gcPacket);

#endif // __GAME_SERVER__

    __END_CATCH
}
