//////////////////////////////////////////////////////////////////////////////
// Filename    : CGQuitUnionHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGQuitUnion.h"

#ifdef __GAME_SERVER__
#include "Assert.h"
#include "DB.h"
#include "GCGuildResponse.h"
#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCSystemMessage.h"
#include "GameContext.h"
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
void CGQuitUnionHandler::execute(CGQuitUnion* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        //	//cout << "enter cgquitunionhandler::execute" << endl;
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
        //		//cout << " out 1 " << endl;
        return;
    }

    uint tempUnionID = pUnion->getUnionID();


    // Is the requester the master of the guild it belongs to?
    if (!de::gameContext().guilds().isGuildMaster(pPlayerCreature->getGuildID(), pPlayerCreature)
        //|| pUnion->getMasterGuildID() != pPlayerCreature->getGuildID()
    ) {
        // Send GC_GUILD_RESPONSE.
        // Content: not the guild master.

        gcGuildResponse.setCode(GuildUnionOfferManager::SOURCE_IS_NOT_MASTER);
        pPlayer->sendPacket(&gcGuildResponse);

        return;
    }


    // Apply normally
    if (pPacket->getQuitMethod() == CGQuitUnion::QUIT_NORMAL) {
        uint result = GuildUnionOfferManager::Instance().offerQuit(pPlayerCreature->getGuildID());

        gcGuildResponse.setCode(result);
        pPlayer->sendPacket(&gcGuildResponse);
    }
    // Withdraw by force
    else if (pPacket->getQuitMethod() == CGQuitUnion::QUIT_QUICK) {
        // The guild master's master id..
        string TargetGuildMaster = de::gameContext().guilds().getGuild(pUnion->getMasterGuildID())->getMaster();

        if (GuildUnionManager::Instance().removeGuild(pUnion->getUnionID(), pPlayerCreature->getGuildID())) {
            gcGuildResponse.setCode(GuildUnionOfferManager::OK);
            pPlayer->sendPacket(&gcGuildResponse);

            /* Apply the penalty that blocks joining another union for 10 days. TODO
             */
            MessageRepository& messages = defaultMessageRepository();
            GuildRepository& guildRows = defaultGuildRepository();

            string escapeGuildName = de::gameContext().guilds().getGuildName(pPlayerCreature->getGuildID());
            string escapeGuildNotice = "[" + escapeGuildName + "] " + de::gameContext().strings().c_str(378);

            messages.insertUnionNotice(UNION_NOTICE_PLAIN, TargetGuildMaster, escapeGuildNotice);
            guildRows.insertEscapeOffer(tempUnionID, pPacket->getGuildID());

            // See whether the union has members.. and if not?
            if (guildRows.countUnionMembersSpelled(UNION_SQL_PLAIN, tempUnionID) == 0) {
                guildRows.deleteUnionInfoOnly(UNION_SQL_PLAIN, tempUnionID);
                messages.insertUnionNotice(UNION_NOTICE_PLAIN, TargetGuildMaster,
                                           de::gameContext().strings().c_str(379));
                GuildUnionManager::Instance().reload();
            }

            Creature* pCreature = NULL;
            pCreature = pGamePlayer->getCreature();

            if (pCreature == NULL)
                return;

            GCModifyInformation gcModifyInformation;
            makeGCModifyInfoGuildUnion(&gcModifyInformation, pCreature);

            pPlayer->sendPacket(&gcModifyInformation);


            Creature* pTargetCreature = NULL;
            PCFinder& pcFinder = de::gameContext().playerCreatures();

            __ENTER_CRITICAL_SECTION(pcFinder)

            pTargetCreature = pcFinder.getCreature_LOCKED(TargetGuildMaster);
            if (pTargetCreature == NULL) {
                return;
            }
            GCModifyInformation gcModifyInformation2;
            makeGCModifyInfoGuildUnion(&gcModifyInformation2, pTargetCreature);
            pTargetCreature->getPlayer()->sendPacket(&gcModifyInformation2);

            __LEAVE_CRITICAL_SECTION(pcFinder)

            //////////////////////////////


            sendGCOtherModifyInfoGuildUnion(pTargetCreature);
            sendGCOtherModifyInfoGuildUnion(pCreature);

            // Tell the ones on other servers about the change.
            GuildUnionManager::Instance().sendModifyUnionInfo(
                dynamic_cast<PlayerCreature*>(pTargetCreature)->getGuildID());
            GuildUnionManager::Instance().sendModifyUnionInfo(dynamic_cast<PlayerCreature*>(pCreature)->getGuildID());
        } else {
            gcGuildResponse.setCode(GuildUnionOfferManager::NOT_YOUR_UNION);
            pPlayer->sendPacket(&gcGuildResponse);
        }
    }

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}
