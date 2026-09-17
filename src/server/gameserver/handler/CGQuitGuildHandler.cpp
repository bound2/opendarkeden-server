//////////////////////////////////////////////////////////////////////////////
// Filename    : CGQuitGuildHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGQuitGuild.h"

#ifdef __GAME_SERVER__
#include "GSQuitGuild.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "SharedServerManager.h"
#include "SystemAvailabilitiesManager.h"
#endif // __GAME_SERVER__

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGQuitGuildHandler::execute(CGQuitGuild* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    SYSTEM_ASSERT(SYSTEM_GUILD);

    PlayerCreature* pPlayerCreature = dynamic_cast<PlayerCreature*>(pPlayer);
    Assert(pPlayerCreature != NULL);

    // Get the guild the player belongs to.
    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    try {
        Assert(pGuild != NULL);
    } catch (Throwable& t) {
        return;
    }

    // Check that the player is a member of the guild.
    GuildMember* pGuildMember = pGuild->getMember(pPlayerCreature->getName());
    if (pGuildMember == NULL)
        return;

    GSQuitGuild gsQuitGuild;
    gsQuitGuild.setGuildID(pGuild->getID());
    gsQuitGuild.setName(pPlayerCreature->getName());

    g_pSharedServerManager->sendPacket(&gsQuitGuild);

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}
