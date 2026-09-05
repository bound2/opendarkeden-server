//----------------------------------------------------------------------
//
// Filename    : SGModifyGuildOKHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Properties.h"
#include "SGModifyGuildOK.h"

#ifdef __GAME_SERVER__

#include "DB.h"
#include "GCModifyGuildMemberInfo.h"
#include "GCOtherModifyInfo.h"
#include "GCSystemMessage.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "PlayerMailbox.h"
#include "Properties.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "repository/MessageRepository.h"

#endif

//----------------------------------------------------------------------
//
// SGModifyGuildOKHandler::execute()
//
//----------------------------------------------------------------------
void SGModifyGuildOKHandler::execute(SGModifyGuildOK* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    Assert(pGuild != NULL);

    if (pGuild->getState() == Guild::GUILD_STATE_WAIT && pPacket->getGuildState() == Guild::GUILD_STATE_ACTIVE) {
        /////////////////////////////////////////////////////////////
        // 존 추가
        /////////////////////////////////////////////////////////////
        /*		if (pGuild->getServerGroupID() == g_pConfig->getPropertyInt("ServerID" ) )
                {
                    // 이 게임 서버에 길드 아지트를 만든다.

                    //////////////
                    // Zone Info
                    //////////////
                    ZoneInfo* pZoneInfo = new ZoneInfo();
                    pZoneInfo->setZoneID(pGuild->getZoneID());
                    pZoneInfo->setZoneGroupID(2);
                    pZoneInfo->setZoneType("NPC_SHOP");
                    pZoneInfo->setZoneLevel(0);
                    pZoneInfo->setZoneAccessMode("PUBLIC");
                    pZoneInfo->setZoneOwnerID("");
                    pZoneInfo->setPayPlay("");
                    if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER )
                    {
                        pZoneInfo->setSMPFilename("team_hdqrs.smp");
                        pZoneInfo->setSSIFilename("team_hdqrs.ssi");
                        string Name = "team - " + pGuild->getName();
                        pZoneInfo->setFullName(Name);
                        pZoneInfo->setShortName(Name);
                    }
                    else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE )
                    {
                        pZoneInfo->setSMPFilename("clan_hdqrs.smp");
                        pZoneInfo->setSSIFilename("clan_hdqrs.ssi");
                        string Name = "clan - " + pGuild->getName();
                        pZoneInfo->setFullName(Name);
                        pZoneInfo->setShortName(Name);
                    }

                    g_pZoneInfoManager->addZoneInfo(pZoneInfo);

                    /////////
                    // Zone
                    /////////
                    Zone* pZone = new Zone(pGuild->getZoneID());
                    Assert(pZone != NULL);

                    ZoneGroup* pZoneGroup = g_pZoneGroupManager->getZoneGroup(2);
                    Assert(pZoneGroup != NULL);

                    pZone->setZoneGroup(pZoneGroup);
                    pZoneGroup->addZone(pZone);
                    pZone->init();
                }
        */

        // 정식 길드로 변경
        pGuild->setState(Guild::GUILD_STATE_ACTIVE);

        HashMapGuildMember& Members = pGuild->getMembers();
        HashMapGuildMemberItor itr = Members.begin();
        for (; itr != Members.end(); itr++) {
            GuildMember* pGuildMember = itr->second;

            // If the member is online, apply the guild id on the owning zone
            // thread (PlayerMailbox.h), with the guild facts captured by
            // value: the member map is walked here and now, the command
            // runs a tick later.
            const string memberName = pGuildMember->getName();
            const GuildID_t guildID = pGuild->getID();
            const string guildName = pGuild->getName();
            const GuildMemberRank_t rank = pGuildMember->getRank();
            de::postToPlayer(memberName, [=](PlayerCreature& pc, Player& player) {
                // 길드 아이디를 바꿔준다.
                pc.setGuildID(guildID);

                // 클라이언트에 길드 아이디가 바꼈음을 알려준다.
                GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                gcModifyGuildMemberInfo.setGuildID(guildID);
                gcModifyGuildMemberInfo.setGuildName(guildName);
                gcModifyGuildMemberInfo.setGuildMemberRank(rank);
                player.sendPacket(&gcModifyGuildMemberInfo);

                // 주위에 알린다.
                Zone* pZone = pc.getZone();
                Assert(pZone != NULL);

                GCOtherModifyInfo gcOtherModifyInfo;
                gcOtherModifyInfo.setObjectID(pc.getObjectID());
                gcOtherModifyInfo.addShortData(MODIFY_GUILDID, pc.getGuildID());

                pZone->broadcastPacket(pc.getX(), pc.getY(), &gcOtherModifyInfo, &pc);

                // 정식 길드가 되었음을 알림
                MessageRepository& messages = defaultMessageRepository();
                vector<string> queued = messages.loadMessages(memberName);

                for (size_t m = 0; m < queued.size(); m++) {
                    GCSystemMessage gcSystemMessage;
                    gcSystemMessage.setMessage(queued[m]);
                    player.sendPacket(&gcSystemMessage);
                }

                messages.deleteMessages(memberName);
            });
        }
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
