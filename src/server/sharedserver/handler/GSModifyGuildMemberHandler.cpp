//----------------------------------------------------------------------
//
// Filename    : GSModifyGuildMemberHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSModifyGuildMember.h"

#ifdef __SHARED_SERVER__

#include "Guild.h"
#include "GuildDecision.h"
#include "GuildManager.h"
#include "GuildStepRunner.h"
#include "Properties.h"

#endif

//----------------------------------------------------------------------
//
// GSModifyGuildMemberHandler::execute()
//
//----------------------------------------------------------------------
void GSModifyGuildMemberHandler::execute(GSModifyGuildMember* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    static_assert(kGuildMemberRankNormal == GuildMember::GUILDMEMBER_RANK_NORMAL);
    static_assert(kGuildMemberRankMaster == GuildMember::GUILDMEMBER_RANK_MASTER);
    static_assert(kGuildMemberRankSubmaster == GuildMember::GUILDMEMBER_RANK_SUBMASTER);
    static_assert(kGuildMemberRankWait == GuildMember::GUILDMEMBER_RANK_WAIT);

    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());

    ModifyGuildMemberRequest request;
    request.guildID = pPacket->getGuildID();
    request.name = pPacket->getName();
    request.sender = pPacket->getSender();
    request.requestedRank = pPacket->getGuildMemberRank();
    request.guildExists = (pGuild != NULL);

    if (pGuild != NULL) {
        request.guildID = pGuild->getID();
        request.guildMaster = pGuild->getMaster();
        request.guildRace = pGuild->getRace();

        GuildMember* pGuildMember = pGuild->getMember(pPacket->getName());
        request.memberExists = (pGuildMember != NULL);
        if (pGuildMember != NULL)
            request.memberRank = pGuildMember->getRank();
    }

    Outcome<SharedGuildEvents, SharedGuildRejection> outcome = decideModifyGuildMember(request);
    if (outcome.isRejected())
        return;

    runGuildSteps(outcome.events().steps, pGuild);

#endif

    __END_DEBUG_EX __END_CATCH
}
