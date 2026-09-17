//----------------------------------------------------------------------
//
// Filename    : GSRequestGuildInfoHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSRequestGuildInfo.h"

#ifdef __SHARED_SERVER__

#include "GameServerManager.h"
#include "GameServerPlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Properties.h"
#include "SGGuildInfo.h"

#endif

//----------------------------------------------------------------------
//
// GSRequestGuildInfoHandler::execute()
//
//----------------------------------------------------------------------
void GSRequestGuildInfoHandler::execute(GSRequestGuildInfo* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    GameServerPlayer* pGSPlayer = dynamic_cast<GameServerPlayer*>(pPlayer);
    Assert(pGSPlayer != NULL);

    SGGuildInfo sgGuildInfo;

    g_pGuildManager->makeSGGuildInfo(sgGuildInfo);

    // Send the packet to the game server.
    pGSPlayer->sendPacket(&sgGuildInfo);

#endif

    __END_DEBUG_EX __END_CATCH
}
