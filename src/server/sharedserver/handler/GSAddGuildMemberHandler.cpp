//----------------------------------------------------------------------
//
// Filename    : GSAddGuildMemberHandler.cpp
// Written By  :
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert.h"
#include "GSAddGuildMember.h"

#ifdef __SHARED_SERVER__

#include "GameServerManager.h"
#include "Guild.h"
#include "GuildDecision.h"
#include "GuildManager.h"
#include "GuildStepRunner.h"
#include "SGAddGuildMemberOK.h"

namespace {

// A guild waiting for its registration becomes active once it holds more than
// this many active members. MIN_GUILDMEMBER_COUNT, the count it has to keep
// afterwards, is a different number.
const int kGuildActivationThreshold = 4;

} // namespace

#endif

//----------------------------------------------------------------------
//
// GSAddGuildHandler::execute()
//
//----------------------------------------------------------------------
void GSAddGuildMemberHandler::execute(GSAddGuildMember* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    static_assert(kGuildMemberRankWait == GuildMember::GUILDMEMBER_RANK_WAIT);
    static_assert(kGuildStateWait == Guild::GUILD_STATE_WAIT);
    static_assert(kGuildStateActive == Guild::GUILD_STATE_ACTIVE);

    // The member is written and announced whatever the guild's own state is.
    GuildMember* pGuildMember = new GuildMember();
    pGuildMember->setGuildID(pPacket->getGuildID());
    pGuildMember->setName(pPacket->getName());
    pGuildMember->setRank(pPacket->getGuildMemberRank());

    if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_WAIT) {
        pGuildMember->setRequestDateTime(VSDateTime::currentDateTime());
    }

    pGuildMember->create();

    pGuildMember->saveIntro(pPacket->getGuildMemberIntro());

    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    pGuild->addMember(pGuildMember);

    SGAddGuildMemberOK sgAddGuildMemberOK;
    sgAddGuildMemberOK.setGuildID(pGuildMember->getGuildID());
    sgAddGuildMemberOK.setName(pGuildMember->getName());
    sgAddGuildMemberOK.setGuildMemberRank(pGuildMember->getRank());
    sgAddGuildMemberOK.setServerGroupID(pPacket->getServerGroupID());

    g_pGameServerManager->broadcast(&sgAddGuildMemberOK);

    // The new member may be the one that carries the guild's registration
    // through.
    GuildActivationRequest request;
    request.guildID = pGuild->getID();
    request.guildRace = pGuild->getRace();
    request.guildState = pGuild->getState();
    request.activeMemberCount = pGuild->getActiveMemberCount();
    request.activationThreshold = kGuildActivationThreshold;

    HashMapGuildMember& Members = pGuild->getMembers();
    for (HashMapGuildMemberItor itr = Members.begin(); itr != Members.end(); itr++)
        request.roster.push_back(SharedGuildRosterEntry(itr->second->getName(), itr->second->getRank()));

    Outcome<SharedGuildEvents, SharedGuildRejection> outcome = decideGuildActivation(request);
    if (outcome.isRejected())
        return;

    runGuildSteps(outcome.events().steps, pGuild);

#endif

    __END_DEBUG_EX __END_CATCH
}
