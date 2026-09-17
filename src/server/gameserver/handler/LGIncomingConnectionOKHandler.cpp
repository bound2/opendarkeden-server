//--------------------------------------------------------------------------------
//
// Filename    : LGIncomingConnectionOKHandler.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "LGIncomingConnectionOK.h"

#ifdef __GAME_SERVER__

#include "Assert1.h"
#include "DatabaseManager.h"
#include "GCReconnectLogin.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "LogDef.h"
#include "Statement.h"

#endif

//--------------------------------------------------------------------------------
//
// LGIncomingConnectionOKHander::execute()
//
// When an LGIncomingConnectionOK packet arrives from the login server, the game server has
// to find out which player this permission is for. It then has to throw an LCReconnectLogin
// packet to that player.
//
//--------------------------------------------------------------------------------
void LGIncomingConnectionOKHandler::execute(LGIncomingConnectionOK* pPacket) {
    __BEGIN_TRY __BEGIN_DEBUG_EX
#ifdef __GAME_SERVER__

        try {
        // Reach the player object by player id.
        GamePlayer* pGamePlayer = NULL;

        try {
            pGamePlayer = g_pIncomingPlayerManager->getPlayer(pPacket->getPlayerID());
        } catch (NoSuchElementException) {
            pGamePlayer = g_pIncomingPlayerManager->getReadyPlayer(pPacket->getPlayerID());
        }

        int fd = -1;
        Socket* pSocket = pGamePlayer->getSocket();
        if (pSocket != NULL)
            fd = (int)pSocket->getSOCKET();


        if (pGamePlayer->getPlayerStatus() == GPS_AFTER_SENDING_GL_INCOMING_CONNECTION) {
            FILELOG_INCOMING_CONNECTION("incomingDisconnect.log", "OK FD : %d, %s", fd,
                                        (pSocket == NULL ? "NULL" : pSocket->getHost().c_str()));


            // Originally the packet was simply sent at this moment..
            // but the disconnect did not go through properly because of timing while the client was
            // already asking to connect, so it was cut off..
            // It is stored on the GamePlayer and, when the IncomingPlayerManager disconnects,
            // the stored packet is sent to the client.
            GCReconnectLogin* gcReconnectLogin = new GCReconnectLogin;
            gcReconnectLogin->setLoginServerIP(pPacket->getHost());
            gcReconnectLogin->setLoginServerPort(pPacket->getTCPPort());
            gcReconnectLogin->setKey(pPacket->getKey());

            if (pGamePlayer != NULL) {
                pGamePlayer->setReconnectPacket(gcReconnectLogin);
            }
        } else {
            FILELOG_INCOMING_CONNECTION("incomingDisconnect.log", "Invalid FD : %d, %s, ps=%d", fd,
                                        (pSocket == NULL ? "NULL" : pSocket->getHost().c_str()),
                                        (int)pGamePlayer->getPlayerStatus());
        }


        // Done this way,
        // the isPenaltyFlag() check in GamePlayer->processCommand() catches it and
        // the next turn's IncomingPlayer->processCommands() cuts it off.
        pGamePlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
        pGamePlayer->setItemRatioBonusPoint(3);
    } catch (NoSuchElementException& nsee) {
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
