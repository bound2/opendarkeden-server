//----------------------------------------------------------------------
//
// Filename    : SGQuitGuildOKHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Properties.h"
#include "SGQuitGuildOK.h"

#ifdef __GAME_SERVER__

#include <stdio.h>

#include "GCModifyGuildMemberInfo.h"
#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCSystemMessage.h"
#include "Guild.h"
#include "GuildManager.h"
#include "PCFinder.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "PlayerMailbox.h"
#include "StringPool.h"
#include "Zone.h"

#endif

//----------------------------------------------------------------------
//
// SGQuitGuildOKHandler::execute()
//
//----------------------------------------------------------------------
void SGQuitGuildOKHandler::execute(SGQuitGuildOK* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);

    // 길드를 가져온다.
    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    // try { Assert(pGuild != NULL); } catch (Throwable& ) { return; }
    if (pGuild == NULL)
        return;

    // 길드 멤버인지 확인한다.
    GuildMember* pGuildMember = pGuild->getMember(pPacket->getName());
    // try { Assert(pGuildMember != NULL); } catch (Throwable& ) { return; }
    if (pGuildMember == NULL)
        return;

    string memberName = pGuildMember->getName();

    // If the member is online, reset its guild id / refund the fee and tell
    // the zone. That mutates creature and zone state, so it runs on the
    // owning zone thread (PlayerMailbox.h) with everything captured by
    // value: pGuildMember is deleted below, before the command can run.
    {
        const GuildState_t guildState = pGuild->getState();
        const GuildID_t guildID = pGuild->getID();
        const string guildName = pGuild->getName();
        const GuildRace_t guildRace = pGuild->getRace();
        const GuildMemberRank_t rank = pGuildMember->getRank();
        de::postToPlayer(memberName, [=](PlayerCreature& pc, Player& player) {
            if (guildState == Guild::GUILD_STATE_ACTIVE) {
                ////////////////////////////////////////////////////////////////////////////////
                // 활동 중인 길드 였다면 Slayer, Vampire 길드 아이디를 가입 안한 상태로 바꾼다.
                ////////////////////////////////////////////////////////////////////////////////
                if (pc.isSlayer()) {
                    pc.setGuildID(99); // 슬레이어의 가입안한 상태의 길드 ID

                    // 클라이언트로 메시지를 보낸다.
                    GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                    gcModifyGuildMemberInfo.setGuildID(guildID);
                    gcModifyGuildMemberInfo.setGuildName(guildName);
                    gcModifyGuildMemberInfo.setGuildMemberRank(rank);
                    player.sendPacket(&gcModifyGuildMemberInfo);
                } else if (pc.isVampire()) {
                    pc.setGuildID(0); // 뱀파이어의 가입안한 상태의 길드 ID

                    // 클라이언트로 메시지를 보낸다.
                    GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                    gcModifyGuildMemberInfo.setGuildID(guildID);
                    gcModifyGuildMemberInfo.setGuildName(guildName);
                    gcModifyGuildMemberInfo.setGuildMemberRank(rank);
                    player.sendPacket(&gcModifyGuildMemberInfo);
                } else if (pc.isOusters()) {
                    pc.setGuildID(66); // 아우스터즈의 가입안한 상태의 길드 ID

                    // 클라이언트로 메시지를 보낸다.
                    GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                    gcModifyGuildMemberInfo.setGuildID(guildID);
                    gcModifyGuildMemberInfo.setGuildName(guildName);
                    gcModifyGuildMemberInfo.setGuildMemberRank(rank);
                    player.sendPacket(&gcModifyGuildMemberInfo);
                }
            }

            if (guildState == Guild::GUILD_STATE_WAIT && rank == GuildMember::GUILDMEMBER_RANK_SUBMASTER) {
                ///////////////////////////////////////////////////////////
                // 대기 중인 길드의 서브 마스터라면 등록비를 환불한다.
                ///////////////////////////////////////////////////////////
                Gold_t Gold = pc.getGold();
                Gold = min((uint64_t)(Gold + RETURN_SLAYER_SUBMASTER_GOLD), (uint64_t)2000000000);
                pc.setGoldEx(Gold);

                GCModifyInformation gcModifyInformation;
                gcModifyInformation.addLongData(MODIFY_GOLD, Gold);
                player.sendPacket(&gcModifyInformation);
            }

            // 길드 탈퇴 메시지를 보낸다.
            GCSystemMessage gcSystemMessage;
            if (guildRace == Guild::GUILD_RACE_SLAYER)
                gcSystemMessage.setMessage(g_pStringPool->getString(STRID_QUIT_TEAM));
            else if (guildRace == Guild::GUILD_RACE_VAMPIRE)
                gcSystemMessage.setMessage(g_pStringPool->getString(STRID_QUIT_CLAN));
            else if (guildRace == Guild::GUILD_RACE_OUSTERS)
                gcSystemMessage.setMessage(g_pStringPool->getString(STRID_QUIT_CLAN));
            player.sendPacket(&gcSystemMessage);

            if (guildState == Guild::GUILD_STATE_ACTIVE) {
                // 주위에 알린다.
                Zone* pZone = pc.getZone();
                Assert(pZone != NULL);

                GCOtherModifyInfo gcOtherModifyInfo;
                gcOtherModifyInfo.setObjectID(pc.getObjectID());
                gcOtherModifyInfo.addShortData(MODIFY_GUILDID, pc.getGuildID());

                pZone->broadcastPacket(pc.getX(), pc.getY(), &gcOtherModifyInfo, &pc);
            }
        });
    }

    // 길드에서 삭제한다.
    pGuild->deleteMember(memberName);

    // 길드 마스터에게 메시지를 보낸다. (send only: fine from this thread)
    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    Creature* pCreature = g_pPCFinder->getCreature_LOCKED(pGuild->getMaster());
    if (pCreature != NULL && pCreature->isPC()) {
        Player* pPlayer = pCreature->getPlayer();
        Assert(pPlayer != NULL);

        //		StringStream msg;
        //		msg << memberName << "님이 길드를 탈퇴하였습니다.";

        char msg[100];
        if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER)
            sprintf(msg, g_pStringPool->c_str(STRID_QUIT_TEAM_2), memberName.c_str());
        else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE)
            sprintf(msg, g_pStringPool->c_str(STRID_QUIT_CLAN_2), memberName.c_str());
        else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS)
            sprintf(msg, g_pStringPool->c_str(STRID_QUIT_CLAN_2), memberName.c_str());

        GCSystemMessage gcSystemMessage;
        gcSystemMessage.setMessage(msg);
        pPlayer->sendPacket(&gcSystemMessage);
    } else {
        // 같은 서버에 길드 마스터가 없는 경우. how?
    }

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))


#endif

    __END_DEBUG_EX __END_CATCH
}
