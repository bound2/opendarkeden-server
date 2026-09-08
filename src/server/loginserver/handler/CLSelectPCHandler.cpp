//////////////////////////////////////////////////////////////////////////////
// Filename    : CLSelectPCHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLSelectPC.h"

#ifdef __LOGIN_SERVER__
#include <utility>

#include "Assert1.h"
#include "CharacterSelection.h"
#include "GameServerInfo.h"
#include "GameServerInfoManager.h"
#include "GameServerManager.h"
#include "LCReconnect.h"
#include "LCSelectPCError.h"
#include "LGIncomingConnection.h"
#include "LoginPlayer.h"
#include "Properties.h"
#include "ZoneGroupInfoManager.h"
#include "ZoneInfoManager.h"
#include "gameserver/billing/BillingInfo.h"
#include "repository/LoginAccountRepository.h"
#include "repository/LoginCharacterRepository.h"

namespace {

// The decision's view of the server tables the login server loaded at
// startup.
class GlobalSelectPCTopology : public SelectPCTopology {
public:
    bool isNonPKServer(WorldID_t worldID, ServerGroupID_t serverGroupID) override {
        return g_pGameServerInfoManager->getGameServerInfo(1, serverGroupID, worldID)->isNonPKServer();
    }

    ServerID_t zoneServerID(ZoneID_t zoneID) override {
        ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo(zoneID);
        ZoneGroupInfo* pZoneGroupInfo = g_pZoneGroupInfoManager->getZoneGroupInfo(pZoneInfo->getZoneGroupID());
        return pZoneGroupInfo->getServerID();
    }
};

} // namespace
#endif

//////////////////////////////////////////////////////////////////////////////
// CLSelectPCHandler::execute()
//
// Loads the PC named by the packet, checks that the account may bring it
// into the game, and tells the game server that runs its zone to expect
// the incoming connection.
//////////////////////////////////////////////////////////////////////////////
void CLSelectPCHandler::execute(CLSelectPC* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    WorldID_t WorldID = pLoginPlayer->getWorldID();

    SelectPCRequest request;
    request.worldID = WorldID;
    request.serverGroupID = pLoginPlayer->getServerGroupID();
    request.playerID = pLoginPlayer->getID();
    request.pcName = pPacket->getPCName();
    request.pcType = pPacket->getPCType();
    request.inCharacterManagement = (pLoginPlayer->getPlayerStatus() == LPS_PC_MANAGEMENT);

#ifdef __NETMARBLE_SERVER__
    request.agreedToTerms = pLoginPlayer->isAgree();
#endif

    // The external billing gate that used to answer SELECT_PC_CANNOT_PLAY and
    // SELECT_PC_NOT_BILLING_CHECK is switched off, so nothing produces those
    // two codes and the free-play cap below is the only account-state check
    // left.
#ifdef __PAY_SYSTEM_FREE_LIMIT__
    // An account that is not paying plays under the level caps.
    if (!pLoginPlayer->isPayPlaying()) {
        static int slayerSum = g_pConfig->getPropertyInt("FreePlaySlayerDomainSum");
        static int vampireLevel = g_pConfig->getPropertyInt("FreePlayVampireLevel");

        request.checkFreePlayLimit = true;
        request.freePlaySlayerDomainSum = slayerSum;
        request.freePlayVampireLevel = vampireLevel;
    }
#endif

    GlobalSelectPCTopology topology;

    try {
        Outcome<SelectedCharacter, SelectPCRejection> outcome =
            decideSelectPC(request, defaultLoginCharacterRepository(), topology);

        if (outcome.isRejected()) {
            LCSelectPCError lcSelectPCError;

            switch (outcome.rejection()) {
            case SelectPCRejection::DidNotAgree:
                lcSelectPCError.setCode(SELECT_PC_DIDNOT_AGREE);
                break;

            case SelectPCRejection::FreePlayLimit:
            case SelectPCRejection::NonPKServerLimit:
                lcSelectPCError.setCode(SELECT_PC_CANNOT_PLAY_BY_ATTR);
                break;

            // The three below cannot be produced by a client that speaks
            // the protocol, so the connection is dropped rather than
            // answered with an error packet.
            case SelectPCRejection::InvalidStatus:
                throw DisconnectException("invalid player status");

            case SelectPCRejection::NoSuchCharacter:
                throw InvalidProtocolException("no such PC exist.");

            case SelectPCRejection::NoSlot:
                throw InvalidProtocolException("no slot exist.");
            }

            pLoginPlayer->sendPacket(&lcSelectPCError);
            return;
        }

        const SelectedCharacter selected = std::move(outcome).events();

        GameServerInfo* pGameServerInfo =
            g_pGameServerInfoManager->getGameServerInfo(selected.serverID, pLoginPlayer->getServerGroupID(), WorldID);

        //----------------------------------------------------------------------
        // Tell the game server to expect this incoming connection.
        //----------------------------------------------------------------------
        LGIncomingConnection lgIncomingConnection;
        lgIncomingConnection.setClientIP(pLoginPlayer->getSocket()->getHost());
        lgIncomingConnection.setPlayerID(pLoginPlayer->getID());
        lgIncomingConnection.setPCName(pPacket->getPCName());

        //--------------------------------------------------------------------------------
        //
        // *CAUTION*
        //
        // Mind the order of LoginPlayer::setPlayerStatus() and
        // GameServerManager::sendPacket(). Calling setPlayerStatus() after
        // sendPacket() would read more naturally, but then the game server's
        // GLIncomingConnectionXXX packet can come back and run its handler
        // before setPlayerStatus() is reached. So the status is set first and
        // the UDP packet sent afterwards.
        //
        //--------------------------------------------------------------------------------
        pLoginPlayer->setPlayerStatus(LPS_AFTER_SENDING_LG_INCOMING_CONNECTION);

        // by tiancaiamao: when gameserver is behind docker, it may have a docker internal IP 172.20.0.1 and a outside
        // IP in database GameServerInfo table. The outside IP should be used.
        pLoginPlayer->setGameServerIP(pGameServerInfo->getIP());

        if (g_pConfig->getProperty("User") == "excel96")
            g_pGameServerManager->sendPacket(pGameServerInfo->getIP(), pGameServerInfo->getUDPPort(),
                                             &lgIncomingConnection);
        else if (g_pConfig->getProperty("User") == "beowulf")
            g_pGameServerManager->sendPacket(pGameServerInfo->getIP(), g_pConfig->getPropertyInt("GameServerUDPPort"),
                                             &lgIncomingConnection);
        else if (g_pConfig->getProperty("User") == "crazydog")
            g_pGameServerManager->sendPacket(pGameServerInfo->getIP(), g_pConfig->getPropertyInt("GameServerUDPPort"),
                                             &lgIncomingConnection);
        else if (g_pConfig->getProperty("User") == "elcastle") {
            cout << "gameserver ip: " << pGameServerInfo->getIP()
                 << ", port: " << g_pConfig->getPropertyInt("GameServerUDPPort") << endl;
            g_pGameServerManager->sendPacket(pGameServerInfo->getIP(), g_pConfig->getPropertyInt("GameServerUDPPort"),
                                             &lgIncomingConnection);
        } else if (g_pConfig->getProperty("User") == "elca")
            g_pGameServerManager->sendPacket(pGameServerInfo->getIP(), g_pConfig->getPropertyInt("GameServerUDPPort"),
                                             &lgIncomingConnection);

        // The slot the account played last, on the account row; the group
        // on all three race rows of the name.
        defaultLoginAccountRepository().setCurrentLocation(WorldID, pLoginPlayer->getServerGroupID(), selected.slot,
                                                           pLoginPlayer->getID());

        defaultLoginCharacterRepository().setCharacterServerGroup(WorldID, pLoginPlayer->getServerGroupID(),
                                                                  pPacket->getPCName());
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); the client is dropped.
        throw DisconnectException("CLSelectPCHandler : SQL error, see DBError.log");
    } catch (NoSuchElementException& nsee) {
        StringStream msg;

        msg << "Critical Error : data intergrity broken at ZoneInfo - ZoneGroupInfo - GameServerInfo : "
            << nsee.toString();

        throw Error(msg.toString());
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
