//////////////////////////////////////////////////////////////////////////////
// Filename    : CLSelectServerHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLSelectServer.h"

#ifdef __LOGIN_SERVER__
#include <utility>

#include "Assert1.h"
#include "GlobalWorldTopology.h"
#include "LCPCList.h"
#include "LoginPlayer.h"
#include "WorldSelection.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// The client picks a server group; the login server puts the session on it
// and answers with the account's characters.
//////////////////////////////////////////////////////////////////////////////
void CLSelectServerHandler::execute(CLSelectServer* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    SelectServerRequest request;
    request.worldID = pLoginPlayer->getWorldID();
    request.serverGroupID = pPacket->getServerGroupID();

    GlobalWorldTopology topology;

    Outcome<SelectedServer, SelectServerRejection> outcome = decideSelectServer(request, topology);

    if (outcome.isRejected()) {
        filelog("errorLogin.txt", "Server Closed: %d", outcome.rejection().serverGroupID);
        throw DisconnectException("ServerClosed");
    }

    const SelectedServer selected = std::move(outcome).events();

    pLoginPlayer->setServerGroupID(selected.serverGroupID);

    //----------------------------------------------------------------------
    // Answer with the account's characters.
    //----------------------------------------------------------------------
    LCPCList lcPCList;
    pLoginPlayer->makePCList(lcPCList);

#ifdef __NETMARBLE_SERVER__
    // Netmarble asks whether the account accepted the terms.
    lcPCList.setAgree(pLoginPlayer->isAgree());
#endif

    pLoginPlayer->sendPacket(&lcPCList);
    pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);

    // The selected group is not written back here; CLChangeServerHandler
    // does that through LoginAccountRepository::setCurrentServerGroup.

#endif

    __END_DEBUG_EX __END_CATCH
}
