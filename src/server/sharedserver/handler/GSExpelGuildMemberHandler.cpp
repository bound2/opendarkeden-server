//----------------------------------------------------------------------
//
// Filename    : GSExpelGuildMemberHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSExpelGuildMember.h"

#ifdef __SHARED_SERVER__

#include "Guild.h"
#include "GuildDecision.h"
#include "GuildManager.h"
#include "GuildStepRunner.h"
#include "Properties.h"

#endif

//----------------------------------------------------------------------
//
// GSExpelGuildMemberHandler::execute()
//
//----------------------------------------------------------------------
void GSExpelGuildMemberHandler::execute(GSExpelGuildMember* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());

    ExpelGuildMemberRequest request;
    request.guildID = pPacket->getGuildID();
    request.name = pPacket->getName();
    request.sender = pPacket->getSender();
    request.guildExists = (pGuild != NULL);

    if (pGuild != NULL) {
        request.guildID = pGuild->getID();
        request.memberExists = (pGuild->getMember(pPacket->getName()) != NULL);
        request.guildRace = pGuild->getRace();
    }

    Outcome<SharedGuildEvents, SharedGuildRejection> outcome = decideExpelGuildMember(request);
    if (outcome.isRejected())
        return;

    runGuildSteps(outcome.events().steps, pGuild, "Expel");

    if (!outcome.events().checkBreakup)
        return;

    // Losing the member may have taken the guild below the count it needs.
    GuildBreakupRequest breakup;
    breakup.guildID = pPacket->getGuildID();
    breakup.guildRace = pGuild->getRace();
    breakup.guildState = pGuild->getState();
    breakup.activeMemberCount = pGuild->getActiveMemberCount();
    breakup.minMemberCount = MIN_GUILDMEMBER_COUNT;
    breakup.cause = pPacket->getName();
    breakup.oustersNoGuildID = kNoGuildIDOusters;
    breakup.notifyMembers = false;

    HashMapGuildMember& Members = pGuild->getMembers();
    for (HashMapGuildMemberItor itr = Members.begin(); itr != Members.end(); itr++)
        breakup.roster.push_back(SharedGuildRosterEntry(itr->second->getName(), itr->second->getRank()));

    Outcome<SharedGuildEvents, SharedGuildRejection> breakupOutcome = decideGuildBreakup(breakup);
    if (breakupOutcome.isRejected())
        return;

    runGuildSteps(breakupOutcome.events().steps, pGuild, "Expel");

#endif

    __END_DEBUG_EX __END_CATCH
}
