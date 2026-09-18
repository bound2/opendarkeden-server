//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExpelGuildHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGExpelGuild.h"

#ifdef __GAME_SERVER__
#include "Assert1.h"
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
void CGExpelGuildHandler::execute(CGExpelGuild* pPacket, Player* pPlayer)

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

    // What if I try to expel myself?
    if (pUnion->getMasterGuildID() == pPacket->getGuildID()) {
        gcGuildResponse.setCode(GuildUnionOfferManager::MASTER_CANNOT_QUIT);
        pPlayer->sendPacket(&gcGuildResponse);
        return;
    }

    if (GuildUnionManager::Instance().removeGuild(pUnion->getUnionID(), pPacket->getGuildID())) {
        gcGuildResponse.setCode(GuildUnionOfferManager::OK);
        pPlayer->sendPacket(&gcGuildResponse);

        ////////////////////

        Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());

        if (pGuild == NULL) {
            return;
        }
        string TargetGuildMaster = pGuild->getMaster();


        GuildRepository& guilds = defaultGuildRepository();

        defaultMessageRepository().insertUnionNotice(UNION_NOTICE_QUOTED_SPACED, TargetGuildMaster,
                                                     g_pStringPool->c_str(377));

        if (guilds.countUnionMembersSpelled(UNION_SQL_QUOTED, pUnion->getUnionID()) == 0) {
            guilds.deleteUnionInfoOnly(UNION_SQL_QUOTED, pUnion->getUnionID());
            GuildUnionManager::Instance().reload();
        }

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
        GCModifyInformation gcModifyInformation2;
        makeGCModifyInfoGuildUnion(&gcModifyInformation2, pTargetCreature);
        pTargetCreature->getPlayer()->sendPacket(&gcModifyInformation2);

        __LEAVE_CRITICAL_SECTION((*g_pPCFinder))

        sendGCOtherModifyInfoGuildUnion(pTargetCreature);
        sendGCOtherModifyInfoGuildUnion(pCreature);

        // Tell the ones on other servers about the change.
        GuildUnionManager::Instance().sendModifyUnionInfo(dynamic_cast<PlayerCreature*>(pTargetCreature)->getGuildID());
        GuildUnionManager::Instance().sendModifyUnionInfo(dynamic_cast<PlayerCreature*>(pCreature)->getGuildID());
    } else {
        gcGuildResponse.setCode(GuildUnionOfferManager::NOT_YOUR_UNION);
        pPlayer->sendPacket(&gcGuildResponse);
    }

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}
