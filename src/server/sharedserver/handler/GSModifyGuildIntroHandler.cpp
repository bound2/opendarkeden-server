//----------------------------------------------------------------------
//
// Filename    : GSModifyGuildIntroHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSModifyGuildIntro.h"

#ifdef __SHARED_SERVER__

#include "GameServerManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "SGModifyGuildIntroOK.h"
#include "SharedContext.h"

#endif

//----------------------------------------------------------------------
//
// GSModifyGuildIntroHandler::execute()
//
//----------------------------------------------------------------------
void GSModifyGuildIntroHandler::execute(GSModifyGuildIntro* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    // Get the guild.
    Guild* pGuild = de::sharedContext().guilds().getGuild(pPacket->getGuildID());
    if (pGuild == NULL)
        return;

    pGuild->saveIntro(pPacket->getGuildIntro());

    // Build the packet to send to the game server.
    SGModifyGuildIntroOK sgModifyGuildIntroOK;
    sgModifyGuildIntroOK.setGuildID(pGuild->getID());
    sgModifyGuildIntroOK.setGuildIntro(pPacket->getGuildIntro());

    // Send the packet to the game server.
    de::sharedContext().gameServers().broadcast(&sgModifyGuildIntroOK);

#endif

    __END_DEBUG_EX __END_CATCH
}
