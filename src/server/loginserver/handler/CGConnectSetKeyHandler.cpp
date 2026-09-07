//////////////////////////////////////////////////////////////////////////////
// Filename    : CGConnectSetKeyHandler.cpp
// Description : loginserver binding for the connection-key handshake. The
//               client sends CGConnectSetKey as the first packet of every
//               fresh connection - to the loginserver as well as to the
//               gameserver - and expects the receiving side to install the
//               same encrypt/hash key pair on its socket. Without this
//               binding the loginserver rejects the packet with "no
//               registered handler" and the login flow dies. The
//               gameserver has its own copy of this definition; both are
//               one line and bind to the shared Player base class.
//////////////////////////////////////////////////////////////////////////////

#include "Assert.h"
#include "CGConnectSetKey.h"
#include "Player.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGConnectSetKeyHandler::execute(CGConnectSetKey* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        pPlayer->setKey(pPacket->getEncryptKey(), pPacket->getHashKey());

    __END_DEBUG_EX __END_CATCH
}
