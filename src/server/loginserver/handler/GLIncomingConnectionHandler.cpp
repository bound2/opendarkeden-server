//----------------------------------------------------------------------
//
// Filename    : GLIncomingConnectionHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GLIncomingConnection.h"
#include "Properties.h"

#ifdef __LOGIN_SERVER__

#include "GameServerManager.h"
#include "KernelContext.h"
#include "LGIncomingConnectionError.h"
#include "LGIncomingConnectionOK.h"
#include "LoginContext.h"
#include "ReconnectLoginInfo.h"
#include "ReconnectLoginInfoManager.h"

#endif

//----------------------------------------------------------------------
//
// GLIncomingConnectionHander::execute()
//
// When the game server receives a GLIncomingConnection packet from the login server,
// a new ReconnectLoginInfo is added.
//
//----------------------------------------------------------------------
void GLIncomingConnectionHandler::execute(GLIncomingConnection* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX __BEGIN_DEBUG

#ifdef __LOGIN_SERVER__

        //--------------------------------------------------------------------------------
        //
        // Generate the authentication key.
        //
        // *NOTE*
        //
        // The old scheme generated the key on the login server, sent it to the game server
        // and on to the client. Done that way, CLSelectPCHandler::execute() generates the
        // key and GLIncomingConnectionOKHandler::execute() sends it to the client,
        // so with two different methods the key has to be kept somewhere. The simplest
        // place is the login player object, which is a bit ugly. Another way is for the
        // game server to hand the key back to the login server, which is unnecessary
        // since the key then crosses the network twice.
        //
        // So generating it on the login server and sending it to the game server is much cleaner.
        //
        // *TODO*
        //
        // In the worst case the local network is sniffed and the key leaks. (Then again, the
        // root password could leak too.. that is what SSL is for..)
        // To guard against that the GLIncomingConnectionOK packet must be encrypted.
        //
        // The key must also be unpredictable. (It becomes predictable once the code is read.)
        //
        //--------------------------------------------------------------------------------

        DWORD authKey = rand() << (time(0) % 10) + rand() >> (time(0) % 10);

    // Create the CI object.
    ReconnectLoginInfo* pReconnectLoginInfo = new ReconnectLoginInfo();
    pReconnectLoginInfo->setClientIP(pPacket->getClientIP());
    pReconnectLoginInfo->setPlayerID(pPacket->getPlayerID());
    pReconnectLoginInfo->setKey(authKey);

    //--------------------------------------------------------------------------------
    //
    // Set the expire time to the current time + 30 seconds.
    //
    // *TODO*
    //
    // The expire period should be configurable too.
    //
    //--------------------------------------------------------------------------------
    Timeval currentTime;
    getCurrentTime(currentTime);
    currentTime.tv_sec += 30;
    pReconnectLoginInfo->setExpireTime(currentTime);


    try {
        // Add it to the RLIM.
        de::loginContext().reconnectLogins().addReconnectLoginInfo(pReconnectLoginInfo);

        // Tell the login server about it again.
        LGIncomingConnectionOK lgIncomingConnectionOK;
        lgIncomingConnectionOK.setPlayerID(pPacket->getPlayerID());
        lgIncomingConnectionOK.setTCPPort(de::kernelContext().config().getPropertyInt("LoginServerPort"));
        lgIncomingConnectionOK.setKey(authKey);

        de::loginContext().gameServers().sendPacket(pPacket->getHost(), pPacket->getPort(), &lgIncomingConnectionOK);
    } catch (DuplicatedException&) {
        // On failure, discard the CI and report the rejection to the peer.
        SAFE_DELETE(pReconnectLoginInfo);

        // LGIncomingConnectionError::write() rejects a message of 128 bytes
        // or more, part way through building the datagram, so the message is
        // a fixed short constant. The receiving handler identifies the
        // rejected login by the player id and never looks at the text.
        LGIncomingConnectionError lgIncomingConnectionError;
        lgIncomingConnectionError.setMessage("duplicated incoming connection");
        lgIncomingConnectionError.setPlayerID(pPacket->getPlayerID());

        de::loginContext().gameServers().sendPacket(pPacket->getHost(), pPacket->getPort(), &lgIncomingConnectionError);
    }

#endif

    __END_DEBUG
    __END_DEBUG_EX __END_CATCH
}
