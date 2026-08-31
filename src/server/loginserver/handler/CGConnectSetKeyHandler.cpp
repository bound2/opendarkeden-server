//////////////////////////////////////////////////////////////////////////////
// Filename    : CGConnectSetKeyHandler.cpp
// Description : loginserver binding for the connection-key handshake. The
//               client sends CGConnectSetKey as the first packet of every
//               fresh connection - to the loginserver as well as to the
//               gameserver - and expects the receiving side to install the
//               same encrypt/hash key pair on its socket. Before the
//               dispatch-table migration (task 2.3/2.4) the handler lived
//               in Core and was linked into every server; the migration
//               moved it into the gameserver only, which left the
//               loginserver rejecting the packet with "no registered
//               handler" and killed the login flow. The gameserver has its
//               own copy of this definition; both are one line and bind to
//               the shared Player base class.
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
