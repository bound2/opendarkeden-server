//////////////////////////////////////////////////////////////////////////////
// Filename    : LoginPacketDispatch.cpp
// Description : the loginserver composition root (docs/RESTRUCTURING.md
//               task 2.3): every packet id the loginserver receives is
//               bound to its handler here. CL rides the client TCP
//               connection (LoginPlayer), GL rides the GameServerManager
//               datagram socket. (CLAgreement turned out to be a dead
//               packet - its id enum never existed - and was deleted
//               in 2.4.)
//////////////////////////////////////////////////////////////////////////////

#include "LoginPacketDispatch.h"

#include "CGConnectSetKey.h"
#include "CLChangeServer.h"
#include "CLCreatePC.h"
#include "CLDeletePC.h"
#include "CLGetPCList.h"
#include "CLGetServerList.h"
#include "CLGetWorldList.h"
#include "CLLogin.h"
#include "CLLogout.h"
#include "CLQueryCharacterName.h"
#include "CLQueryPlayerID.h"
#include "CLReconnectLogin.h"
#include "CLRegisterPlayer.h"
#include "CLSelectPC.h"
#include "CLSelectServer.h"
#include "CLSelectWorld.h"
#include "CLVersionCheck.h"
#include "GLIncomingConnection.h"
#include "GLIncomingConnectionError.h"
#include "GLIncomingConnectionOK.h"
#include "GLKickVerify.h"
#include "GMServerInfo.h"
#include "PacketDispatcher.h"

void registerLoginServerPacketHandlers() {
    // CG by name, but the client opens EVERY fresh connection with it -
    // including the login connection - to install the socket encrypt/hash
    // keys before CLVersionCheck/CLLogin. See handler/CGConnectSetKeyHandler.
    DE_REGISTER_PACKET_HANDLER(CGConnectSetKey);

    DE_REGISTER_PACKET_HANDLER(CLChangeServer);
    DE_REGISTER_PACKET_HANDLER(CLCreatePC);
    DE_REGISTER_PACKET_HANDLER(CLDeletePC);
    DE_REGISTER_PACKET_HANDLER(CLGetPCList);
    DE_REGISTER_PACKET_HANDLER(CLGetServerList);
    DE_REGISTER_PACKET_HANDLER(CLGetWorldList);
    DE_REGISTER_PACKET_HANDLER(CLLogin);
    DE_REGISTER_PACKET_HANDLER(CLLogout);
    DE_REGISTER_PACKET_HANDLER(CLQueryCharacterName);
    DE_REGISTER_PACKET_HANDLER(CLQueryPlayerID);
    DE_REGISTER_PACKET_HANDLER(CLReconnectLogin);
    DE_REGISTER_PACKET_HANDLER(CLRegisterPlayer);
    DE_REGISTER_PACKET_HANDLER(CLSelectPC);
    DE_REGISTER_PACKET_HANDLER(CLSelectServer);
    DE_REGISTER_PACKET_HANDLER(CLSelectWorld);
    DE_REGISTER_PACKET_HANDLER(CLVersionCheck);

    // GL (game -> login) datagrams.
    DE_REGISTER_PACKET_HANDLER_NOPLAYER(GLIncomingConnection);
    DE_REGISTER_PACKET_HANDLER_NOPLAYER(GLIncomingConnectionError);
    DE_REGISTER_PACKET_HANDLER_NOPLAYER(GLIncomingConnectionOK);
    DE_REGISTER_PACKET_HANDLER_NOPLAYER(GLKickVerify);

    // GMServerInfo rides the same datagram socket: each gameserver
    // reports its per-zone user counts to the loginserver with it.
    DE_REGISTER_PACKET_HANDLER_NOPLAYER(GMServerInfo);
}
