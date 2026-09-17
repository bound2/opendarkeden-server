//////////////////////////////////////////////////////////////////////////////
// Filename    : CGQuitUnionAcceptHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGQuitUnionAccept.h"

#ifdef __GAME_SERVER__
#include "Assert.h"
#include "DB.h"
#include "Exception.h"
#include "GCGuildResponse.h"
#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCSystemMessage.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "GuildUnion.h"
#include "PCFinder.h"
#include "PacketUtil.h"
#include "PlayerCreature.h"
#include "StringPool.h"
#include "SystemAvailabilitiesManager.h"
#include "repository/GuildRepository.h"
#include "repository/MessageRepository.h"

#endif // __GAME_SERVER__

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGQuitUnionAcceptHandler::execute(CGQuitUnionAccept* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Assert(pGamePlayer != NULL);

    PlayerCreature* pPlayerCreature = dynamic_cast<PlayerCreature*>(pGamePlayer->getCreature());
    Assert(pPlayerCreature != NULL);

    SYSTEM_ASSERT(SYSTEM_GUILD);

    GCGuildResponse gcGuildResponse;

    GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(pPlayerCreature->getGuildID());
    if (pUnion == NULL) {
        gcGuildResponse.setCode(GuildUnionOfferManager::NOT_IN_UNION);
        pPlayer->sendPacket(&gcGuildResponse);

        return;
    }

    // Is the requester the master of its own guild, and is the union's master guild my guild?
    if (!g_pGuildManager->isGuildMaster(pPlayerCreature->getGuildID(), pPlayerCreature) ||
        pUnion->getMasterGuildID() != pPlayerCreature->getGuildID()) {
        // Send GC_GUILD_RESPONSE.
        // Content: not the guild master.

        gcGuildResponse.setCode(GuildUnionOfferManager::SOURCE_IS_NOT_MASTER);
        pPlayer->sendPacket(&gcGuildResponse);

        return;
    }

    uint result = GuildUnionOfferManager::Instance().acceptQuit(pPacket->getGuildID());

    gcGuildResponse.setCode(result);
    pPlayer->sendPacket(&gcGuildResponse);
    ////////////////////

    if (result == GuildUnionOfferManager::OK) {
        Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());

        if (pGuild == NULL) {
            return;
        }
        string TargetGuildMaster = pGuild->getMaster();


        GuildRepository& guilds = defaultGuildRepository();

        defaultMessageRepository().insertUnionNotice(UNION_NOTICE_PLAIN, TargetGuildMaster, g_pStringPool->c_str(375));

        // What if I am the only one left after accepting the withdrawal?
        if (guilds.countUnionMembersSpelled(UNION_SQL_PLAIN, pUnion->getUnionID()) == 0) {
            guilds.deleteUnionInfoOnly(UNION_SQL_PLAIN, pUnion->getUnionID());
            GuildUnionManager::Instance().reload();
        }

        // A union withdrawal can change the union information. Send the refreshed information again.
        Creature* pCreature = NULL;
        pCreature = pGamePlayer->getCreature();

        if (pCreature == NULL)
            return;

        GCModifyInformation gcModifyInformation;
        makeGCModifyInfoGuildUnion(&gcModifyInformation, pCreature);

        pPlayer->sendPacket(&gcModifyInformation);

        // Send the guild union information to the notified user again
        Creature* pTargetCreature = NULL;
        __ENTER_CRITICAL_SECTION((*g_pPCFinder))

        pTargetCreature = g_pPCFinder->getCreature_LOCKED(TargetGuildMaster);
        if (pTargetCreature == NULL) {
            return;
        }
        GCModifyInformation gcModifyInformation;
        makeGCModifyInfoGuildUnion(&gcModifyInformation, pTargetCreature);
        pTargetCreature->getPlayer()->sendPacket(&gcModifyInformation);

        __LEAVE_CRITICAL_SECTION((*g_pPCFinder))

        sendGCOtherModifyInfoGuildUnion(pTargetCreature);
        sendGCOtherModifyInfoGuildUnion(pCreature);

        // Tell the ones on other servers about the change.
        GuildUnionManager::Instance().sendModifyUnionInfo(dynamic_cast<PlayerCreature*>(pTargetCreature)->getGuildID());
        GuildUnionManager::Instance().sendModifyUnionInfo(dynamic_cast<PlayerCreature*>(pCreature)->getGuildID());
    }

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}
