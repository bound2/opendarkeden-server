//----------------------------------------------------------------------
//
// Filename    : LGKickCharacterHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "LGKickCharacter.h"

#ifdef __GAME_SERVER__

#include "Creature.h"
#include "GLKickVerify.h"
#include "GamePlayer.h"
#include "LogDef.h"
#include "LoginServerManager.h"
#include "PlayerMailbox.h"

#endif

//----------------------------------------------------------------------
//
// LGKickCharacterHander::execute()
//
// 게임 서버가 로그인 서버로부터 LGKickCharacter 패킷을 받게 되면,
// ConnectionInfo를 새로 추가하게 된다.
//
//----------------------------------------------------------------------
void LGKickCharacterHandler::execute(LGKickCharacter* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX
#ifdef __GAME_SERVER__

        // 냐햐햐
        /*
        if (!g_pPCFinder->setKickCharacter(pPacket->getPCName(), pPacket->getHost(), pPacket->getPort() ))
        {
            GLKickVerify glKickVerify;
            glKickVerify.setKicked(false);
            glKickVerify.setID(pPacket->getID());
            glKickVerify.setPCName(pPacket->getPCName());

            g_pLoginServerManager->sendPacket(pPacket->getHost() , pPacket->getPort() , &glKickVerify);

            //cout << "LGKickVerify Send Packet to ServerIP : " << pPacket->getHost() << endl;
            //cout << "LGKickVerify Send Packet to ServerPort : " << pPacket->getPort() << endl;

            return;
        }
        */

        try {

        const string pcName = pPacket->getPCName();
        const string host = pPacket->getHost();
        const uint port = pPacket->getPort();
        const uint requestID = pPacket->getID();

        // 캐릭터가 없는 경우에는 GLKickVerify(false)를 보낸다.
        de::GoneCommand notHere = [=] {
            GLKickVerify glKickVerify;
            glKickVerify.setKicked(false);
            glKickVerify.setID(requestID);
            glKickVerify.setPCName(pcName);

            g_pLoginServerManager->sendPacket(host, port, &glKickVerify);

            // cout << "LGKickVerify Send Packet to ServerIP : " << host << endl;
            // cout << "LGKickVerify Send Packet to ServerPort : " << port << endl;
        };

        // The kick flags are read by the player manager that owns the
        // player (the zone group's, once the player is in a zone), so they
        // are set on that thread (PlayerMailbox.h). A player that logs out
        // between the post and the tick gets the same "not here" verify a
        // player who was never found gets, so the loginserver is answered
        // either way.
        const bool found = de::postToPlayer(
            pcName,
            [=](PlayerCreature&, Player& player) {
                // cout << "KickCharacter : " << pcName.c_str() << endl;

                GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(&player);

                // Assert(pGamePlayer!=NULL);
                if (pGamePlayer == NULL) // 어떻게 가능할까? -_-;
                {
                    return;
                }

                int fd = -1;
                Socket* pSocket = pGamePlayer->getSocket();
                if (pSocket != NULL)
                    fd = (int)pSocket->getSOCKET();

                FILELOG_INCOMING_CONNECTION("incomingDisconnect.log", "Kick FD : %d, %s", fd,
                                            (pSocket == NULL ? "NULL" : pSocket->getHost().c_str()));

                // 강제 종료 시킨다.
                pGamePlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                pGamePlayer->setItemRatioBonusPoint(4);
                pGamePlayer->setKickForLogin(true);

                // 접속 해제 후, 응답을 보내줄 곳..
                pGamePlayer->setKickRequestHost(host);
                pGamePlayer->setKickRequestPort(port);
            },
            notHere);

        if (!found)
            notHere();

    } catch (NoSuchElementException&) {
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
