//----------------------------------------------------------------------
//
// Filename    : SGAddGuildMemberOKHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Properties.h"
#include "SGAddGuildMemberOK.h"

#ifdef __GAME_SERVER__

#include <stdio.h>

#include "DB.h"
#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCSystemMessage.h"
#include "Guild.h"
#include "GuildManager.h"
#include "PCFinder.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "PlayerMailbox.h"
#include "Properties.h"
#include "StringPool.h"
#include "StringStream.h"
#include "Zone.h"
#include "repository/GoldRepository.h"

#endif

//----------------------------------------------------------------------
//
// SGAddGuildMemberOKHandler::execute()
//
//----------------------------------------------------------------------
void SGAddGuildMemberOKHandler::execute(SGAddGuildMemberOK* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        // 길드 멤버 object 를 만든다.
        GuildMember* pGuildMember = new GuildMember();
    pGuildMember->setGuildID(pPacket->getGuildID());
    pGuildMember->setName(pPacket->getName());
    pGuildMember->setRank(pPacket->getGuildMemberRank());

    // 길드에 추가한다.
    Guild* pGuild = g_pGuildManager->getGuild(pGuildMember->getGuildID());
    pGuild->addMember(pGuildMember);

    // 멤버에게 메세지를 보낸다.
    //
    // A master/submaster pays the fee: from the in-memory gold if the
    // member is online, from the database row if not. The in-memory path
    // mutates the creature, so it runs on the owning zone thread
    // (PlayerMailbox.h) with the guild facts captured by value; the
    // database path doubles as the ifGone fallback, so a member who logs
    // out between the post and the tick is still charged.
    const string memberName = pGuildMember->getName();
    const GuildMemberRank_t rank = pGuildMember->getRank();
    const GuildRace_t guildRace = pGuild->getRace();

    Gold_t Fee;
    if (rank == GuildMember::GUILDMEMBER_RANK_MASTER)
        Fee = REQUIRE_SLAYER_MASTER_GOLD;
    else if (rank == GuildMember::GUILDMEMBER_RANK_SUBMASTER)
        Fee = REQUIRE_SLAYER_SUBMASTER_GOLD;
    else
        Fee = 0;

    // 접속이 안되어 있다: 마스터나 서브마스터일 경우 DB 에서 돈을 까도록 한다.
    const bool addedHere = pPacket->getServerGroupID() == g_pConfig->getPropertyInt("ServerID");
    de::GoneCommand chargeInDatabase = [=] {
        if ((rank == GuildMember::GUILDMEMBER_RANK_MASTER ||
             rank == GuildMember::GUILDMEMBER_RANK_SUBMASTER) // 길드마스터나 서브마스터일 경우
            && addedHere)                                     // 이 게임 서버에서 추가한 길드원인가?
        {
            // The race decides which table the row is in. A guild whose
            // race is none of the three named none, and the write was
            // skipped; hasRaceTable keeps that guard, since CharacterRace
            // has no "no table" value.
            CharacterRace race = CHARACTER_RACE_SLAYER;
            bool hasRaceTable = false;
            if (guildRace == Guild::GUILD_RACE_SLAYER) {
                race = CHARACTER_RACE_SLAYER;
                hasRaceTable = true;
            } else if (guildRace == Guild::GUILD_RACE_VAMPIRE) {
                race = CHARACTER_RACE_VAMPIRE;
                hasRaceTable = true;
            } else if (guildRace == Guild::GUILD_RACE_OUSTERS) {
                race = CHARACTER_RACE_OUSTERS;
                hasRaceTable = true;
            }

            if (hasRaceTable && Fee != 0) {
                defaultGoldRepository().decreaseGoldClamped(memberName, race, Fee);
            }
        }
    };

    const bool online = de::postToPlayer(
        memberName,
        [=](PlayerCreature& pc, Player& player) {
            if (rank == GuildMember::GUILDMEMBER_RANK_MASTER ||
                rank == GuildMember::GUILDMEMBER_RANK_SUBMASTER) // 길드마스터나 서브마스터일 경우
            {
                Gold_t CurMoney = pc.getGold();
                if (CurMoney < Fee) {
                    // 큰일났군
                    CurMoney = 0;
                } else
                    CurMoney -= Fee;

                pc.setGoldEx(CurMoney);

                if (Fee != 0) {
                    GCModifyInformation gcModifyInformation;
                    gcModifyInformation.addLongData(MODIFY_GOLD, CurMoney);

                    // 바뀐정보를 클라이언트에 보내준다.
                    player.sendPacket(&gcModifyInformation);
                }

                // 길드 가입 메시지를 보여준다.
                GCSystemMessage gcSystemMessage;
                if (guildRace == Guild::GUILD_RACE_SLAYER)
                    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_TEAM_JOIN_ACCEPTED));
                else if (guildRace == Guild::GUILD_RACE_VAMPIRE)
                    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_CLAN_JOIN_ACCEPTED));
                else if (guildRace == Guild::GUILD_RACE_OUSTERS)
                    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_CLAN_JOIN_ACCEPTED));
                player.sendPacket(&gcSystemMessage);

            } else if (rank == GuildMember::GUILDMEMBER_RANK_WAIT) {
                // 길드 가입 신청 메시지를 보낸다.
                GCSystemMessage gcSystemMessage;
                if (guildRace == Guild::GUILD_RACE_SLAYER)
                    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_TEAM_JOIN_TRY));
                else if (guildRace == Guild::GUILD_RACE_VAMPIRE)
                    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_CLAN_JOIN_TRY));
                else if (guildRace == Guild::GUILD_RACE_OUSTERS)
                    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_CLAN_JOIN_TRY));

                player.sendPacket(&gcSystemMessage);
            }
        },
        chargeInDatabase);

    if (!online)
        chargeInDatabase();

    // 길드 마스터에게 메시지를 보낸다. (send only: fine from this thread)
    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    Creature* pCreature = g_pPCFinder->getCreature_LOCKED(pGuild->getMaster());
    if (pCreature != NULL && pCreature->isPC() && pGuildMember->getRank() != GuildMember::GUILDMEMBER_RANK_MASTER) {
        Player* pPlayer = pCreature->getPlayer();
        Assert(pPlayer != NULL);

        char msg[100];

        if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_SUBMASTER) {
            if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER)
                sprintf(msg, g_pStringPool->c_str(STRID_TEAM_JOIN_ACCEPTED_2), pGuildMember->getName().c_str());
            else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE)
                sprintf(msg, g_pStringPool->c_str(STRID_CLAN_JOIN_ACCEPTED_2), pGuildMember->getName().c_str());
            else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS)
                sprintf(msg, g_pStringPool->c_str(STRID_CLAN_JOIN_ACCEPTED_2), pGuildMember->getName().c_str());
        } else if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_WAIT) {
            if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER)
                sprintf(msg, g_pStringPool->c_str(STRID_TEAM_JOIN_TRY_2), pGuildMember->getName().c_str());
            else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE)
                sprintf(msg, g_pStringPool->c_str(STRID_CLAN_JOIN_TRY_2), pGuildMember->getName().c_str());
            else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS)
                sprintf(msg, g_pStringPool->c_str(STRID_CLAN_JOIN_TRY_2), pGuildMember->getName().c_str());
        }

        GCSystemMessage gcSystemMessage;
        gcSystemMessage.setMessage(msg);
        pPlayer->sendPacket(&gcSystemMessage);
    }

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))

#endif

    __END_DEBUG_EX __END_CATCH
}
