//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyGuildMemberIntroHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGModifyGuildMemberIntro.h"

#ifdef __GAME_SERVER__
#include "GameContext.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "PlayerCreature.h"
#include "SystemAvailabilitiesManager.h"
#endif // __GAME_SERVER__

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGModifyGuildMemberIntroHandler::execute(CGModifyGuildMemberIntro* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    SYSTEM_ASSERT(SYSTEM_GUILD);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Assert(pGamePlayer != NULL);

    Creature* pCreature = pGamePlayer->getCreature();
    Assert(pCreature != NULL);

    PlayerCreature* pPlayerCreature = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPlayerCreature != NULL);

    // Get the guild.
    Guild* pGuild = de::gameContext().guilds().getGuild(pPlayerCreature->getGuildID());
    if (pGuild == NULL)
        return;

    // Get the guild member information.
    GuildMember* pGuildMember = pGuild->getMember(pPlayerCreature->getName());
    if (pGuildMember == NULL)
        return;

    pGuildMember->saveIntro(pPacket->getGuildMemberIntro());

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}
