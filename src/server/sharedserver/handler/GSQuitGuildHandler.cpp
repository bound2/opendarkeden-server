//----------------------------------------------------------------------
//
// Filename    : GSQuitGuildHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSQuitGuild.h"

#ifdef __SHARED_SERVER__

#include "Guild.h"
#include "GuildDecision.h"
#include "GuildManager.h"
#include "GuildStepRunner.h"
#include "Properties.h"
#include "StringPool.h"

#endif

//----------------------------------------------------------------------
//
// GSQuitGuildHandler::execute()
//
//----------------------------------------------------------------------
void GSQuitGuildHandler::execute(GSQuitGuild* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    static_assert(kGuildMemberRankNormal == GuildMember::GUILDMEMBER_RANK_NORMAL);
    static_assert(kGuildMemberRankMaster == GuildMember::GUILDMEMBER_RANK_MASTER);
    static_assert(kGuildMemberRankSubmaster == GuildMember::GUILDMEMBER_RANK_SUBMASTER);
    static_assert(kGuildStateCancel == Guild::GUILD_STATE_CANCEL);

    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());

    QuitGuildRequest request;
    request.guildID = pPacket->getGuildID();
    request.name = pPacket->getName();
    request.guildExists = (pGuild != NULL);

    if (pGuild != NULL) {
        request.guildID = pGuild->getID();
        request.guildRace = pGuild->getRace();
        request.guildState = pGuild->getState();

        GuildMember* pGuildMember = pGuild->getMember(pPacket->getName());
        request.memberExists = (pGuildMember != NULL);
        if (pGuildMember != NULL)
            request.memberRank = pGuildMember->getRank();

        // The guild master giving the registration up is the only branch that
        // reads the roster and the text it sends, so nothing else pays for
        // them - and a StringPool with no entry for the text reaches nothing
        // else either. Both refunds are the Slayer figures whatever the
        // guild's race, as they always have been.
        if (request.memberExists && request.guildState == Guild::GUILD_STATE_WAIT &&
            request.memberRank == GuildMember::GUILDMEMBER_RANK_MASTER) {
            request.masterRefund = RETURN_SLAYER_MASTER_GOLD;
            request.submasterRefund = RETURN_SLAYER_SUBMASTER_GOLD;

            if (isKnownGuildRace(request.guildRace))
                request.cancelMessageEmpty = g_pStringPool->getString(guildCancelMessageFor(request.guildRace)).empty();

            HashMapGuildMember& Members = pGuild->getMembers();
            for (HashMapGuildMemberItor itr = Members.begin(); itr != Members.end(); itr++)
                request.roster.push_back(SharedGuildRosterEntry(itr->second->getName(), itr->second->getRank()));
        }
    }

    Outcome<SharedGuildEvents, SharedGuildRejection> outcome = decideQuitGuild(request);
    if (outcome.isRejected()) {
        // A guild master refused an active guild's exit still leaves its quit
        // log behind.
        runGuildSteps(outcome.rejection().steps, pGuild, "Quit");
        return;
    }

    runGuildSteps(outcome.events().steps, pGuild, "Quit");

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
    // The Ousters character row is cleared to 0 here rather than to its
    // no-guild id.
    breakup.oustersNoGuildID = 0;
    breakup.notifyMembers = true;

    HashMapGuildMember& Members = pGuild->getMembers();
    for (HashMapGuildMemberItor itr = Members.begin(); itr != Members.end(); itr++)
        breakup.roster.push_back(SharedGuildRosterEntry(itr->second->getName(), itr->second->getRank()));

    Outcome<SharedGuildEvents, SharedGuildRejection> breakupOutcome = decideGuildBreakup(breakup);
    if (breakupOutcome.isRejected())
        return;

    runGuildSteps(breakupOutcome.events().steps, pGuild, "Quit");

#endif

    __END_DEBUG_EX __END_CATCH
}
