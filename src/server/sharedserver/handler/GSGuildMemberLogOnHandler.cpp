//----------------------------------------------------------------------
//
// Filename    : GSGuildMemberLogOnHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSGuildMemberLogOn.h"

#ifdef __SHARED_SERVER__

#include "DB.h"
#include "GameServerManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Properties.h"
#include "SGGuildMemberLogOnOK.h"

#endif

//----------------------------------------------------------------------
//
// GSGuildMemberLogOnHandler::execute()
//
//----------------------------------------------------------------------
void GSGuildMemberLogOnHandler::execute(GSGuildMemberLogOn* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    // Get the guild.
    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    // try { Assert(pGuild != NULL); } catch (Throwable& ) { return; }
    if (pGuild == NULL)
        return;

    // Check whether it is a member of the guild.
    GuildMember* pGuildMember = pGuild->getMember(pPacket->getName());
    // try { Assert(pGuildMember != NULL); } catch (Throwable& ) { return; }
    if (pGuildMember == NULL)
        return;

    pGuildMember->setLogOn(pPacket->getLogOn());

    // Build the packet to send to the game server.
    SGGuildMemberLogOnOK sgGuildMemberLogOnOK;
    sgGuildMemberLogOnOK.setGuildID(pGuild->getID());
    sgGuildMemberLogOnOK.setName(pPacket->getName());
    sgGuildMemberLogOnOK.setLogOn(pPacket->getLogOn());
    sgGuildMemberLogOnOK.setServerID(pPacket->getServerID());


    // Send the packet to the game server.
    g_pGameServerManager->broadcast(&sgGuildMemberLogOnOK);

#endif

    __END_DEBUG_EX __END_CATCH
}
