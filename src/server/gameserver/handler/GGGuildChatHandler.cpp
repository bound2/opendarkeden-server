//----------------------------------------------------------------------
//
// Filename    : GGGuildChatHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GGGuildChat.h"

#ifdef __GAME_SERVER__

#include "Creature.h"
#include "GCGuildChat.h"
#include "Guild.h"
#include "GuildManager.h"
#include "GuildUnion.h"
#include "PCFinder.h"
#include "Player.h"

static void broadcastGuild(Guild* pGuild, Packet* pPacket) {
    if (pGuild == NULL || pPacket == NULL)
        return;

    list<string> currentMembers = pGuild->getCurrentMembers();
    list<string>::const_iterator itr = currentMembers.begin();
    for (; itr != currentMembers.end(); itr++) {
        __ENTER_CRITICAL_SECTION((*g_pPCFinder))

        Creature* pCreature = g_pPCFinder->getCreature_LOCKED((*itr));
        if (pCreature != NULL) {
            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);

            if (pPlayer->getSocket() != NULL)
                pPlayer->sendPacket(pPacket);
        }

        __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
    }
}

#endif

//----------------------------------------------------------------------
//
// GGGuildChatHander::execute()
//
//----------------------------------------------------------------------
void GGGuildChatHandler::execute(GGGuildChat* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX __BEGIN_DEBUG

#ifdef __GAME_SERVER__

        // Get the guild's currently connected members.
        Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());

    if (pGuild == NULL) {
        filelog("GuildMissing.log", "[NoSuchGuild] GuildID : %d", (int)pPacket->getGuildID());
        return;
    }

    // Build the guild chat packet.
    GCGuildChat gcGuildChat;
    gcGuildChat.setType(pPacket->getType());
    gcGuildChat.setSendGuildName(pGuild->getName());
    gcGuildChat.setSender(pPacket->getSender());
    gcGuildChat.setColor(pPacket->getColor());
    gcGuildChat.setMessage(pPacket->getMessage());

    if (pPacket->getType() == 0) {
        broadcastGuild(pGuild, &gcGuildChat);
    } else {
        // Union chat
        GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(pGuild->getID());
        if (pUnion == NULL) {
            broadcastGuild(pGuild, &gcGuildChat);
        } else {
            list<GuildID_t> gList = pUnion->getGuildList();
            list<GuildID_t>::iterator itr = gList.begin();

            for (; itr != gList.end(); ++itr) {
                pGuild = g_pGuildManager->getGuild(*itr);
                broadcastGuild(pGuild, &gcGuildChat);
            }

            pGuild = g_pGuildManager->getGuild(pUnion->getMasterGuildID());
            broadcastGuild(pGuild, &gcGuildChat);
        }
    }

#endif

    __END_DEBUG
    __END_DEBUG_EX __END_CATCH
}
