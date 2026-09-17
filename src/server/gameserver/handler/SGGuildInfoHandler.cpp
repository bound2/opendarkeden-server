//----------------------------------------------------------------------
//
// Filename    : SGGuildInfoHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Properties.h"
#include "SGGuildInfo.h"

#ifdef __GAME_SERVER__

#include "Guild.h"
#include "GuildInfo2.h"
#include "GuildManager.h"
#include "GuildMemberInfo2.h"
#include "PCFinder.h"

#endif

//----------------------------------------------------------------------
//
// SGGuildInfoHandler::execute()
//
//----------------------------------------------------------------------
void SGGuildInfoHandler::execute(SGGuildInfo* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);

    g_pGuildManager->lock();

    // Delete every guild.
    g_pGuildManager->clear_NOBLOCKED();

    GuildInfo2* pGuildInfo = NULL;

    while ((pGuildInfo = pPacket->popFrontGuildInfoList()) != NULL) {
        Guild* pGuild = new Guild();
        pGuild->setID(pGuildInfo->getID());
        pGuild->setName(pGuildInfo->getName());
        pGuild->setType(pGuildInfo->getType());
        pGuild->setRace(pGuildInfo->getRace());
        pGuild->setState(pGuildInfo->getState());
        pGuild->setServerGroupID(pGuildInfo->getServerGroupID());
        pGuild->setZoneID(pGuildInfo->getZoneID());
        pGuild->setMaster(pGuildInfo->getMaster());
        pGuild->setDate(pGuildInfo->getDate());
        pGuild->setIntro(pGuildInfo->getIntro());

        GuildMemberInfo2* pGuildMemberInfo = NULL;
        while ((pGuildMemberInfo = pGuildInfo->popFrontGuildMemberInfoList()) != NULL) {
            GuildMember* pGuildMember = new GuildMember();
            pGuildMember->setGuildID(pGuildMemberInfo->getGuildID());
            pGuildMember->setName(pGuildMemberInfo->getName());
            pGuildMember->setRank(pGuildMemberInfo->getRank());
            pGuildMember->setLogOn(pGuildMemberInfo->getLogOn());


            if (pGuildMember->getLogOn() && g_pPCFinder->getCreature(pGuildMember->getName()) != NULL) {
                pGuild->addCurrentMember(pGuildMember->getName());
            }

            pGuild->addMember(pGuildMember);
        }

        g_pGuildManager->addGuild_NOBLOCKED(pGuild);
    }

    g_pGuildManager->unlock();

#endif

    __END_DEBUG_EX __END_CATCH
}
