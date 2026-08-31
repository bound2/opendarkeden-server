//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeListHandler.cpp
// Written By  : Exchange System
// Description : Handler for CGExchangeList
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeList.h"
#include "GCExchangeList.h"

#ifdef __GAME_SERVER__
#include <limits.h>

#include "../server/gameserver/exchange/ExchangeService.h"
#include "GamePlayer.h"
#include "PlayerCreature.h"
#endif

void CGExchangeListHandler::execute(CGExchangeList* pPacket, Player* pPlayer) {
    __BEGIN_TRY

#ifdef __GAME_SERVER__

    // Validate player
    if (pPlayer == NULL)
        return;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pPlayer);
    if (pPC == NULL)
        return;

    // Get server ID (TODO: implement getServerID in PlayerCreature)
    int16_t serverID = 1; // Default server ID

    // Clamp the client-supplied paging before anything uses it.
    //
    // The page size bounds two separate things: the SQL LIMIT in
    // ExchangeDB::getListings, and the size of the reply. GCExchangeList
    // declares its maximum packet size for kMaxListingsPerPage listings
    // (see GCExchangeListFactory::getPacketMaxSize), so a larger page would
    // build a reply the client discards as oversized.
    const int maxPageSize = (int)GCExchangeList::kMaxListingsPerPage;
    int pageSize = pPacket->getPageSize();
    if (pageSize < 1)
        pageSize = 1;
    else if (pageSize > maxPageSize)
        pageSize = maxPageSize;

    // Pages are 1-based. The upper bound keeps the OFFSET that
    // ExchangeDB::getListings computes as (page - 1) * pageSize from
    // overflowing int.
    int page = pPacket->getPage();
    if (page < 1)
        page = 1;
    else if (page > INT_MAX / pageSize)
        page = INT_MAX / pageSize;

    // Get listings from service
    vector<ExchangeListing> listings =
        ExchangeService::getListings(serverID, page, pageSize, pPacket->getItemClass(), pPacket->getItemType(),
                                     pPacket->getMinPrice(), pPacket->getMaxPrice());

    // Send response. The reply echoes the values actually used, not the ones
    // the client asked for.
    GCExchangeList gcPacket;
    gcPacket.setListings(listings);
    gcPacket.setPage(page);
    gcPacket.setPageSize(pageSize);
    gcPacket.setTotal(listings.size());

    pPlayer->sendPacket(&gcPacket);

#endif // __GAME_SERVER__

    __END_CATCH
}
