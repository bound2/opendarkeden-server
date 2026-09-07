//----------------------------------------------------------------------
//
// Filename    : GSExpelGuildMemberHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSExpelGuildMember.h"

#ifdef __SHARED_SERVER__

#include "GameServerManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Properties.h"
#include "SGDeleteGuildOK.h"
#include "SGExpelGuildMemberOK.h"
#include "repository/SharedGuildRepository.h"

#endif

//----------------------------------------------------------------------
//
// GSExpelGuildMemberHandler::execute()
//
//----------------------------------------------------------------------
void GSExpelGuildMemberHandler::execute(GSExpelGuildMember* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__
        // cout << "GSExpelGuildMember received" << endl;

        Assert(pPacket != NULL);

    // 플레이어가 속한 길드를 가져온다.
    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    // try { Assert(pGuild != NULL); } catch (Throwable& ) { return; }
    if (pGuild == NULL)
        return;

    // 플레이어가 길드의 멤버인지 확인한다.
    GuildMember* pGuildMember = pGuild->getMember(pPacket->getName());
    // try { Assert(pGuildMember != NULL); } catch (Throwable& ) { return; }
    if (pGuildMember == NULL)
        return;

    // 길드 탈퇴 로그를 남긴다.
    filelog("GuildExit.log", "GuildID: %d, GuildName: %s, Expel: %s, By: %s", pGuild->getID(),
            pGuild->getName().c_str(), pPacket->getName().c_str(), pPacket->getSender().c_str());

    // The character row loses its guild id.
    if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER) {
        defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 99, pGuildMember->getName());
    } else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE) {
        defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 0, pGuildMember->getName());
    } else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS) {
        defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 66, pGuildMember->getName());
    }

    // Guild Member 를 expire 시킨다.
    pGuildMember->expire();

    // Guild 에서 삭제한다.
    pGuild->deleteMember(pGuildMember->getName());

    // 게임 서버로 보낼 패킷을 만든다.
    SGExpelGuildMemberOK sgExpelGuildMemberOK;
    sgExpelGuildMemberOK.setGuildID(pGuild->getID());
    sgExpelGuildMemberOK.setName(pPacket->getName());
    sgExpelGuildMemberOK.setSender(pPacket->getSender());

    // 게임 서버로 패킷을 보낸다.
    g_pGameServerManager->broadcast(&sgExpelGuildMemberOK);

    // 길드 인원이 5명 미만이 될 경우 길드를 삭제한다.
    if (pGuild->getState() == Guild::GUILD_STATE_ACTIVE && pGuild->getActiveMemberCount() < MIN_GUILDMEMBER_COUNT) {
        // 길드 삭제 로그를 남긴다.
        filelog("GuildBroken.log", "GuildID: %d, GuildName: %s, MemberCount: %d, Expel: %s", pGuild->getID(),
                pGuild->getName().c_str(), pGuild->getActiveMemberCount(), pPacket->getName().c_str());

        // Every remaining member is expelled: the character row loses its
        // guild id, the roster row is expired, the object is freed.
        HashMapGuildMember& Members = pGuild->getMembers();
        HashMapGuildMemberItor itr = Members.begin();

        for (; itr != Members.end(); itr++) {
            GuildMember* pGuildMember = itr->second;

            if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER) {
                defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 99, pGuildMember->getName());
            } else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE) {
                defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 0, pGuildMember->getName());
            } else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS) {
                defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 66, pGuildMember->getName());
            }

            pGuildMember->expire();
            // pGuildMember->destroy();

            SAFE_DELETE(pGuildMember);
        }

        Members.clear();

        // 길드를 삭제한다
        pGuild->setState(Guild::GUILD_STATE_BROKEN);
        pGuild->save();

        SAFE_DELETE(pGuild);
        g_pGuildManager->deleteGuild(pPacket->getGuildID());

        // 길드를 삭제하도록 패킷을 보낸다.
        SGDeleteGuildOK sgDeleteGuildOK;
        sgDeleteGuildOK.setGuildID(pPacket->getGuildID());

        g_pGameServerManager->broadcast(&sgDeleteGuildOK);
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
