//----------------------------------------------------------------------
//
// Filename    : SGDeleteGuildOKHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "SGDeleteGuildOK.h"

#ifdef __GAME_SERVER__

#include <string>
#include <utility>
#include <vector>

#include "Assert1.h"
#include "DB.h"
#include "GCModifyGuildMemberInfo.h"
#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCSystemMessage.h"
#include "Guild.h"
#include "GuildManager.h"
#include "GuildUnion.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "PlayerMailbox.h"
#include "Properties.h"
#include "ResurrectLocationManager.h"
#include "Zone.h"
#include "repository/MessageRepository.h"
#endif

//----------------------------------------------------------------------
//
// SGDeleteGuildOKHandler::execute()
//
//----------------------------------------------------------------------
void SGDeleteGuildOKHandler::execute(SGDeleteGuildOK* pPacket)

{
    __BEGIN_TRY

#ifdef __GAME_SERVER__

    // 길드 아지트에 있는 멤버를 warp 시킨다.
    // 길드 아지트를 삭제한다.
    // 멤버 warp와 길드 아지트 삭제 시 다른 쓰레드에서 ZoneGroup Thread 내부에서 일어나게 해야 별탈이 없을 듯 하다.
    // 일단은 걍 둔다. Portal 이 막히므로 다시 들어갈 수 없을 것이다.

    Assert(pPacket != NULL);

    // 길드를 가져온다.
    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    try {
        Assert(pGuild != NULL);
    } catch (Throwable&) {
        return;
    }


    // 길드 활동 중인 상태에서의 해체인지 대기 중인 상태에서의 해체인지 구별한다.
    if (pGuild->getState() == Guild::GUILD_STATE_ACTIVE) {
        // Take the members out under the guild mutex -- a zone thread may
        // be copying the member list at this moment (CGSelectGuild) -- and
        // work from the returned names. The GuildMember objects and the
        // Guild itself are retired, never freed here: readers hold raw
        // pointers to both across the lock (see Guild::m_RetiredMembers,
        // GuildManager::m_RetiredGuilds).
        const std::vector<std::pair<std::string, GuildMemberRank_t>> members = pGuild->retireAllMembers();

        for (size_t i = 0; i < members.size(); i++) {
            const std::string& memberName = members[i].first;

            // If the member is online, reset its guild id on the owning zone
            // thread (PlayerMailbox.h). Nothing from this handler is
            // captured by pointer: the member and the guild are retired
            // below, before the command runs.
            // That also means the guild is unregistered one tick before the
            // member's guild id is reset (the old code reset it first); the
            // readers of a creature's guild id all null-check the lookup
            // (GuildMissing.log), so the window shows as a stale badge, not
            // a crash.
            de::postToPlayer(memberName, [](PlayerCreature& pc, Player& player) {
                // Slayer, Vampire 의 길드 아이디를 바꾼다.
                if (pc.isSlayer()) {
                    pc.setGuildID(99); // 슬레이어 가입안한 상태의 길드 ID

                    // 클라이언트에 길드 아이디가 바꼈음을 알린다.
                    GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                    gcModifyGuildMemberInfo.setGuildID(pc.getGuildID());
                    gcModifyGuildMemberInfo.setGuildName("");
                    gcModifyGuildMemberInfo.setGuildMemberRank(GuildMember::GUILDMEMBER_RANK_DENY);
                    player.sendPacket(&gcModifyGuildMemberInfo);
                } else if (pc.isVampire()) {
                    pc.setGuildID(0); // 뱀파이어 가입안한 상태의 길드 ID

                    // 클라이언트에 길드 아이디가 바꼈음을 알린다.
                    GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                    gcModifyGuildMemberInfo.setGuildID(pc.getGuildID());
                    gcModifyGuildMemberInfo.setGuildName("");
                    gcModifyGuildMemberInfo.setGuildMemberRank(GuildMember::GUILDMEMBER_RANK_DENY);
                    player.sendPacket(&gcModifyGuildMemberInfo);
                } else if (pc.isOusters()) {
                    pc.setGuildID(66); // 아우스터즈 가입안한 상태의 길드 ID

                    // 클라이언트에 길드 아이디가 바꼈음을 알린다.
                    GCModifyGuildMemberInfo gcModifyGuildMemberInfo;
                    gcModifyGuildMemberInfo.setGuildID(pc.getGuildID());
                    gcModifyGuildMemberInfo.setGuildName("");
                    gcModifyGuildMemberInfo.setGuildMemberRank(GuildMember::GUILDMEMBER_RANK_DENY);
                    player.sendPacket(&gcModifyGuildMemberInfo);
                }

                // 주위에 클라이언트에 길드 아이디가 바꼈음을 알린다.
                GCOtherModifyInfo gcOtherModifyInfo;
                gcOtherModifyInfo.setObjectID(pc.getObjectID());
                gcOtherModifyInfo.addShortData(MODIFY_GUILDID, pc.getGuildID());

                Zone* pZone = pc.getZone();
                Assert(pZone != NULL);

                pZone->broadcastPacket(pc.getX(), pc.getY(), &gcOtherModifyInfo, &pc);
            });
        }

        // 길드 매니저에서 길드를 삭제한다 (retired, not freed).
        g_pGuildManager->deleteGuild(pGuild->getID());
    } else if (pGuild->getState() == Guild::GUILD_STATE_WAIT) {
        const std::vector<std::pair<std::string, GuildMemberRank_t>> members = pGuild->retireAllMembers();

        for (size_t i = 0; i < members.size(); i++) {
            const std::string& memberName = members[i].first;

            // If the member is online, refund the fee on the owning zone
            // thread (PlayerMailbox.h); the rank is captured by value
            // taken from the returned list, not a live member. The
            // message repository is looked up inside the command so the
            // SQL runs on the zone thread's own connection. A SQL failure
            // there surfaces as the const char* END_DB rethrows -- not a
            // Throwable -- which the mailbox drain logs without stopping
            // the tick; the dangling const char* itself (END_DB throws
            // msg.c_str() from a local string) is a separate open defect.
            const GuildMemberRank_t rank = members[i].second;
            de::postToPlayer(memberName, [rank](PlayerCreature& pc, Player& player) {
                // 등록비를 환불한다.
                Gold_t Gold = pc.getGold();
                if (rank == GuildMember::GUILDMEMBER_RANK_MASTER) {
                    Gold = min((uint64_t)(Gold + RETURN_SLAYER_MASTER_GOLD), (uint64_t)2000000000);
                } else if (rank == GuildMember::GUILDMEMBER_RANK_SUBMASTER) {
                    Gold = min((uint64_t)(Gold + RETURN_SLAYER_SUBMASTER_GOLD), (uint64_t)2000000000);
                }

                pc.setGoldEx(Gold);

                GCModifyInformation gcModifyInformation;
                gcModifyInformation.addLongData(MODIFY_GOLD, Gold);
                player.sendPacket(&gcModifyInformation);

                // 메시지를 보낸다.
                MessageRepository& messages = defaultMessageRepository();
                vector<string> queued = messages.loadMessages(pc.getName());

                for (size_t m = 0; m < queued.size(); m++) {
                    GCSystemMessage message;
                    message.setMessage(queued[m]);
                    player.sendPacket(&message);
                }

                messages.deleteMessages(pc.getName());
            });
        }

        // 길드 매니저에서 길드를 삭제한다 (retired, not freed).
        g_pGuildManager->deleteGuild(pGuild->getID());
        GuildUnionManager::Instance().removeMasterGuild(pGuild->getID());
    }

#endif

    __END_CATCH
}
