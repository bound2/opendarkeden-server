//----------------------------------------------------------------------
//
// Filename    : GSAddGuildMemberHandler.cpp
// Written By  :
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert.h"
#include "GSAddGuildMember.h"

#ifdef __SHARED_SERVER__

#include "GameServerManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "SGAddGuildMemberOK.h"
#include "SGModifyGuildOK.h"
#include "StringPool.h"
#include "repository/SharedGuildRepository.h"

#endif

//----------------------------------------------------------------------
//
// GSAddGuildHandler::execute()
//
//----------------------------------------------------------------------
void GSAddGuildMemberHandler::execute(GSAddGuildMember* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

        Assert(pPacket != NULL);

    GuildMember* pGuildMember = new GuildMember();
    pGuildMember->setGuildID(pPacket->getGuildID());
    pGuildMember->setName(pPacket->getName());
    pGuildMember->setRank(pPacket->getGuildMemberRank());

    if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_WAIT) {
        pGuildMember->setRequestDateTime(VSDateTime::currentDateTime());
    }

    // DB 에 Guild Member 를 저장한다.
    pGuildMember->create();

    // Guild Member Intro 를 DB에 저장한다.
    pGuildMember->saveIntro(pPacket->getGuildMemberIntro());

    // 길드에 추가한다.
    Guild* pGuild = g_pGuildManager->getGuild(pPacket->getGuildID());
    pGuild->addMember(pGuildMember);

    // 게임 서버로 보낼 패킷을 만든다.
    SGAddGuildMemberOK sgAddGuildMemberOK;
    sgAddGuildMemberOK.setGuildID(pGuildMember->getGuildID());
    sgAddGuildMemberOK.setName(pGuildMember->getName());
    sgAddGuildMemberOK.setGuildMemberRank(pGuildMember->getRank());
    sgAddGuildMemberOK.setServerGroupID(pPacket->getServerGroupID());

    // 게임 서버로 패킷을 보낸다.
    g_pGameServerManager->broadcast(&sgAddGuildMemberOK);

    // 등록 대기 길드이고 길드원이 5명 이상이 되면 정식 길드가 된다.
    if (pGuild->getState() == Guild::GUILD_STATE_WAIT && pGuild->getActiveMemberCount() > 4) {
        HashMapGuildMember& Members = pGuild->getMembers();
        HashMapGuildMemberItor itr = Members.begin();
        for (; itr != Members.end(); itr++) {
            pGuildMember = itr->second;

            // The member's character row is pointed at the guild and a
            // message tells the character the guild was accepted; the
            // master gets the master's wording.
            SharedGuildRepository& repo = defaultSharedGuildRepository();

            repo.stampMemberRequestDateTime(pGuildMember->getName());

            if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER) {
                repo.setCharacterGuildID(pGuild->getRace(), pGuild->getID(), pGuildMember->getName());
                if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_MASTER)
                    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, pGuildMember->getName(),
                                       g_pStringPool->c_str(STRID_TEAM_REGISTRATION_ACCEPT));
                else {
                    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, pGuildMember->getName(),
                                       g_pStringPool->c_str(STRID_TEAM_REGISTRATION_ACCEPT_2));
                }
            } else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE) {
                repo.setCharacterGuildID(pGuild->getRace(), pGuild->getID(), pGuildMember->getName());
                if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_MASTER)
                    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, pGuildMember->getName(),
                                       g_pStringPool->c_str(STRID_CLAN_REGISTRATION_ACCEPT));
                else {
                    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, pGuildMember->getName(),
                                       g_pStringPool->c_str(STRID_CLAN_REGISTRATION_ACCEPT_2));
                }
            } else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS) {
                repo.setCharacterGuildID(pGuild->getRace(), pGuild->getID(), pGuildMember->getName());
                if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_MASTER)
                    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, pGuildMember->getName(),
                                       g_pStringPool->c_str(STRID_CLAN_REGISTRATION_ACCEPT));
                else {
                    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, pGuildMember->getName(),
                                       g_pStringPool->c_str(STRID_CLAN_REGISTRATION_ACCEPT_2));
                }
            }
        }

        pGuild->setState(Guild::GUILD_STATE_ACTIVE);
        pGuild->save();

        SGModifyGuildOK sgModifyGuildOK;
        sgModifyGuildOK.setGuildID(pGuild->getID());
        sgModifyGuildOK.setGuildState(pGuild->getState());

        g_pGameServerManager->broadcast(&sgModifyGuildOK);
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
