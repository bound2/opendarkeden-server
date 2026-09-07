//----------------------------------------------------------------------
//
// Filename    : GSQuitGuildHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "Assert1.h"
#include "GSQuitGuild.h"

#ifdef __SHARED_SERVER__

#include "GameServerManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Properties.h"
#include "SGDeleteGuildOK.h"
#include "SGQuitGuildOK.h"
#include "StringPool.h"
#include "repository/SharedGuildRepository.h"

#endif

//----------------------------------------------------------------------
//
// GSQuitGuildHandler::execute()
//
//----------------------------------------------------------------------
void GSQuitGuildHandler::execute(GSQuitGuild* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __SHARED_SERVER__

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
    GuildMemberRank_t rank = pGuildMember->getRank();
    if (rank == GuildMember::GUILDMEMBER_RANK_NORMAL || rank == GuildMember::GUILDMEMBER_RANK_MASTER ||
        rank == GuildMember::GUILDMEMBER_RANK_SUBMASTER)
        filelog("GuildExit.log", "GuildID: %d, GuildName: %s, Quit: %s", pGuild->getID(), pGuild->getName().c_str(),
                pPacket->getName().c_str());

    //////////////////////////////////////////////////////////////////////////////
    // 길드가 active 라면 걍 탈퇴하는 거다
    // wait 라면 돈도 돌려주고 탈퇴하는 사람이 마스터라면 길드를 취소시킨다.
    //////////////////////////////////////////////////////////////////////////////
    if (pGuild->getState() == Guild::GUILD_STATE_ACTIVE) {
        // 길드 마스터일 경우 무시
        if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_MASTER)
            return;

        // The character row loses its guild id.
        if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER) {
            defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 99, pGuildMember->getName());
        } else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE) {
            defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 0, pGuildMember->getName());
        } else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS) {
            defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 66, pGuildMember->getName());
        }

        // Guild Member 를 leave 시킨다.
        pGuildMember->leave();

        // Guild 에서 삭제한다.
        pGuild->deleteMember(pGuildMember->getName());

        // 게임 서버로 보낼 패킷을 만든다.
        SGQuitGuildOK sgQuitGuildOK;
        sgQuitGuildOK.setGuildID(pGuild->getID());
        sgQuitGuildOK.setName(pPacket->getName());

        // 게임 서버로 패킷을 보낸다.
        g_pGameServerManager->broadcast(&sgQuitGuildOK);

        // 길드 인원이 5명 미만이 될 경우
        if (pGuild->getState() == Guild::GUILD_STATE_ACTIVE && pGuild->getActiveMemberCount() < MIN_GUILDMEMBER_COUNT) {
            // 기록을 남긴다.
            filelog("GuildBroken.log", "GuildID: %d, GuildName: %s, MemberCount: %d, Quit: %s", pGuild->getID(),
                    pGuild->getName().c_str(), pGuild->getActiveMemberCount(), pPacket->getName().c_str());

            // The guild breaks up: every remaining member's character row
            // loses its guild id and gets a message, the roster row is
            // expired. The Ousters row is set to 0 here, not 66.
            HashMapGuildMember& Members = pGuild->getMembers();
            HashMapGuildMemberItor itr = Members.begin();

            for (; itr != Members.end(); itr++) {
                GuildMember* pGuildMember = itr->second;

                if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER) {
                    defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 99, pGuildMember->getName());
                    defaultSharedGuildRepository().insertMessage(SHARED_MESSAGE_SQL_COMPACT, pGuildMember->getName(),
                                                                 g_pStringPool->c_str(STRID_TEAM_BROKEN));
                } else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE) {
                    defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 0, pGuildMember->getName());
                    defaultSharedGuildRepository().insertMessage(SHARED_MESSAGE_SQL_COMPACT, pGuildMember->getName(),
                                                                 g_pStringPool->c_str(STRID_CLAN_BROKEN));
                } else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS) {
                    defaultSharedGuildRepository().setCharacterGuildID(pGuild->getRace(), 0, pGuildMember->getName());
                    defaultSharedGuildRepository().insertMessage(SHARED_MESSAGE_SQL_COMPACT, pGuildMember->getName(),
                                                                 g_pStringPool->c_str(STRID_CLAN_BROKEN));
                }

                pGuildMember->expire();

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
    } else if (pGuild->getState() == Guild::GUILD_STATE_WAIT) {
        if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_MASTER) {
            // The guild registration is cancelled: every member is expired,
            // the master and the submaster get their fee back with a
            // message.
            HashMapGuildMember& Members = pGuild->getMembers();
            HashMapGuildMemberItor itr = Members.begin();

            for (; itr != Members.end(); itr++) {
                GuildMember* pGuildMember = itr->second;

                bool bKnownRace = false;
                string Message = "";
                Gold_t Gold = 0;

                if (pGuild->getRace() == Guild::GUILD_RACE_SLAYER) {
                    bKnownRace = true;
                    Message = g_pStringPool->getString(STRID_TEAM_CANCEL);
                } else if (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE) {
                    bKnownRace = true;
                    Message = g_pStringPool->getString(STRID_CLAN_CANCEL);
                } else if (pGuild->getRace() == Guild::GUILD_RACE_OUSTERS) {
                    bKnownRace = true;
                    Message = g_pStringPool->getString(STRID_CLAN_CANCEL);
                }

                if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_MASTER)
                    Gold = RETURN_SLAYER_MASTER_GOLD;
                else if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_SUBMASTER)
                    Gold = RETURN_SLAYER_SUBMASTER_GOLD;

                if (bKnownRace && !Message.empty() && Gold != 0) {
                    defaultSharedGuildRepository().insertMessage(SHARED_MESSAGE_SQL_COMPACT, pGuildMember->getName(),
                                                                 Message);
                    // The refund goes to the row in the database; a character
                    // in play carries its own gold in the gameserver, which
                    // writes it back later.
                    defaultSharedGuildRepository().addCharacterGold(pGuild->getRace(), (int)Gold,
                                                                    pGuildMember->getName());
                }

                pGuildMember->expire();

                SAFE_DELETE(pGuildMember);
            }

            Members.clear();

            // 길드를 삭제한다
            pGuild->setState(Guild::GUILD_STATE_CANCEL);
            pGuild->save();

            SAFE_DELETE(pGuild);

            g_pGuildManager->deleteGuild(pPacket->getGuildID());

            // 길드를 삭제하도록 패킷을 보낸다.
            SGDeleteGuildOK sgDeleteGuildOK;
            sgDeleteGuildOK.setGuildID(pPacket->getGuildID());

            g_pGameServerManager->broadcast(&sgDeleteGuildOK);
        } else if (pGuildMember->getRank() == GuildMember::GUILDMEMBER_RANK_SUBMASTER) {
            //////////////////////////////////////////////////////////
            // 스타팅 멤버 가입을 취소 시킨다.
            // Guild Member 를 expire 시킨다.
            //////////////////////////////////////////////////////////
            pGuildMember->expire();

            // Guild 에서 삭제한다.
            pGuild->deleteMember(pGuildMember->getName());

            // 게임 서버로 보낼 패킷을 만든다.
            SGQuitGuildOK sgQuitGuildOK;
            sgQuitGuildOK.setGuildID(pGuild->getID());
            sgQuitGuildOK.setName(pPacket->getName());

            // 게임 서버로 패킷을 보낸다.
            g_pGameServerManager->broadcast(&sgQuitGuildOK);
        }
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
