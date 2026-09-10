//////////////////////////////////////////////////////////////////////////////
// Filename    : CLReconnectLoginHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLReconnectLogin.h"

#ifdef __LOGIN_SERVER__
#include <utility>

#include "Assert1.h"
#include "DatabaseError.h"
#include "GCDisconnect.h"
#include "GameServerInfoManager.h"
#include "LCPCList.h"
#include "LoginPlayer.h"
#include "OptionInfo.h"
#include "Properties.h"
#include "ReconnectDecision.h"
#include "ReconnectLoginInfoManager.h"
#include "repository/LoginAccountRepository.h"

#endif

#ifdef __LOGIN_SERVER__
namespace {

// ReconnectSession over the LoginPlayer the reconnect is running for.
class ReconnectPlayerSession : public ReconnectSession {
public:
    explicit ReconnectPlayerSession(LoginPlayer* pLoginPlayer) : m_pLoginPlayer(pLoginPlayer) {}

    void setWorldID(int worldID) override {
        m_pLoginPlayer->setWorldID((WorldID_t)worldID);
    }

    void setServerGroupID(int serverGroupID) override {
        m_pLoginPlayer->setServerGroupID((ServerGroupID_t)serverGroupID);
    }

    void setID(const string& playerID) override {
        m_pLoginPlayer->setID(playerID);
    }

    void setPayPlayValue(int payType, const string& payPlayDate, int payPlayHours, uint payPlayFlag) override {
        m_pLoginPlayer->setPayPlayValue((PayType)payType, payPlayDate, payPlayHours, payPlayFlag);
    }

    bool loginPayPlay(int payType, const string& payPlayDate, int payPlayHours, uint payPlayFlag, const string& ip,
                      const string& playerID) override {
        return m_pLoginPlayer->loginPayPlay((PayType)payType, payPlayDate, payPlayHours, payPlayFlag, ip, playerID);
    }

private:
    LoginPlayer* m_pLoginPlayer;
};

} // namespace
#endif

//////////////////////////////////////////////////////////////////////////////
// CLReconnectLoginHandler::execute()
//
// This is the first packet a client sends when it moves from the login
// server to a game server, or from one game server to another. The player
// object has just been created and is held by the IncomingPlayerManager.
//
// Any other packet arriving first is taken to be an intrusion attempt, so
// this has to be the first one: the player object keeps the previous packet,
// and it is enough to check that it is NULL.
//
// A packet that does not verify ends the connection.
//////////////////////////////////////////////////////////////////////////////
void CLReconnectLoginHandler::execute(CLReconnectLogin* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    // cout << "CLReconnectLogin : STARTING HANDLING PROCESS" << endl;

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);
    pLoginPlayer->setWorldID(pPacket->isWebLogin());
    // pLoginPlayer->setWebLogin(g_pConfig->getPropertyInt("WebLogin") != 0);

    string PlayerID;

    //----------------------------------------------------------------------
    // Find the ReconnectLoginInfo this packet belongs to. An intruder has to
    // guess both the key and the character name inside a time window.
    //----------------------------------------------------------------------
    try {
        ReconnectLoginInfo* pReconnectLoginInfo =
            g_pReconnectLoginInfoManager->getReconnectLoginInfo(pLoginPlayer->getSocket()->getHost());

        PlayerID = pReconnectLoginInfo->getPlayerID();

        // Keep the account id on the session.
        pLoginPlayer->setID(PlayerID);

        // Verify the key.
        if (pPacket->getKey() != pReconnectLoginInfo->getKey())
            throw InvalidProtocolException("invalid key");

        // Compare the current time against the expiry time.
        Timeval currentTime;
        getCurrentTime(currentTime);
        if (pReconnectLoginInfo->getExpireTime() < currentTime) {
            g_pReconnectLoginInfoManager->deleteReconnectLoginInfo(pReconnectLoginInfo->getClientIP());
            throw InvalidProtocolException("session already expired");
        }

        // Verified, so the ReconnectLoginInfo is spent.
        g_pReconnectLoginInfoManager->deleteReconnectLoginInfo(pReconnectLoginInfo->getClientIP());

    } catch (NoSuchElementException& nsee) // no ReconnectLoginInfo for that address
    {
        // A client that takes too long between connecting and sending
        // CLReconnectLogin finds its session expired, and is dropped as
        // well. (Stopping in a debugger between the two is enough.)
        GCDisconnect gcDisconnect;
        gcDisconnect.setMessage(nsee.toString());

        pLoginPlayer->sendPacket(&gcDisconnect);

        // Thrown so that IPM::processCommands() above disconnects.
        throw InvalidProtocolException("reconnect login info not found");
    } catch (InvalidProtocolException& ipe) {
        cout << endl
             << "+-----------------------+" << endl
             << "| Level 2 Access Denied |" << endl
             << "+-----------------------+" << endl
             << endl;

        GCDisconnect gcDisconnect;
        gcDisconnect.setMessage(ipe.toString());

        pLoginPlayer->sendPacket(&gcDisconnect);

        // Thrown so that IPM::processCommands() above disconnects.
        throw;
    }

    ServerGroupID_t CurrentServerGroupID = 0;

    try {
        LoginAccountRepository& repo = defaultLoginAccountRepository();

        ReconnectRequest request;
        request.playerID = PlayerID;
        request.connectIP = pLoginPlayer->getSocket()->getHost();
        request.loginServerID = g_pConfig->getPropertyInt("LoginServerID");

        ReconnectPlayerSession session(pLoginPlayer);

        Outcome<ReconnectAccepted, ReconnectRejection> outcome = decideReconnectLogin(request, repo, session);

        if (outcome.isRejected()) {
            const ReconnectRejection rejection = std::move(outcome).rejection();

            switch (rejection.reason) {
            case ReconnectRejectReason::NoSuchAccount:
                throw DisconnectException("ReconnectLogin verify failed: no such player");

            case ReconnectRejectReason::AlreadyInGame: {
                string msg = "ReconnectLogin verify failed: LogOn = ";
                msg += rejection.logOn;
                throw DisconnectException(msg);
            }

            case ReconnectRejectReason::AlreadyLoggedOnElsewhere:
                throw DisconnectException("Deny MultiLogin");

            case ReconnectRejectReason::AccessNotAllowed:
                throw DisconnectException("ReconnectLogin verify failed ");

            case ReconnectRejectReason::NotPayAccount:
                // Only a build with the pay system applied at login
                // produces this, and it leaves as a protocol error rather
                // than a disconnect.
                throw InvalidProtocolException("Pay First!");
            }
        }

        CurrentServerGroupID = (ServerGroupID_t)std::move(outcome).events().serverGroupID;

        // The Thailand build refuses an unapproved account inside the
        // guarded hours. Not compiled here, and its permission variable is
        // never read from the account row, so it does not build either.
#ifdef __THAILAND_SERVER__
        if (strPermission != "ALLOW" && onChildGuardTimeArea(g_pConfig->getPropertyInt("CHILDGUARD_START_TIME"),
                                                             g_pConfig->getPropertyInt("CHILDGUARD_END_TIME"),
                                                             g_pConfig->getProperty("CHILDGUARD"))) {
            throw DisconnectException("Player Permission is DENY (child guard) running. ");
        }
#endif
    } catch (const DatabaseError& error) {
        // A SQL failure arrives as END_DB's DatabaseError carrying the line
        // it wrote to DBError.log; the reason travels with the disconnect.
        throw DisconnectException("CLReconnectLoginHandler : " + error.message());
    }

    // cout << "CLReconnectLogin : ReconnectLoginInfo verified" << endl;

    pLoginPlayer->setServerGroupID(CurrentServerGroupID);

    pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);


    //----------------------------------------------------------------------
    // Answer with the PC list.
    //----------------------------------------------------------------------
    LCPCList lcPCList;

    pLoginPlayer->makePCList(lcPCList);
    pLoginPlayer->sendPacket(&lcPCList);
    pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);


#endif

    __END_DEBUG_EX __END_CATCH
}
#ifdef __THAILAND_SERVER__
bool CLReconnectLoginHandler::onChildGuardTimeArea(int pm, int am, string enable) {
    bool returnValue = false;
    tm Timem;
    time_t daytime = time(0);
    localtime_r(&daytime, &Timem);

    int Hour = Timem.tm_hour;
    int Min = Timem.tm_min;

    int timeValue = (Hour * 100) + Min;
    bool bSwitch = (enable == "ENABLE" || enable == "enable" || enable == "Enable");

    if ((timeValue >= pm && timeValue <= am) && bSwitch) {
        returnValue = true;
    } else if ((timeValue <= pm && timeValue <= am) && bSwitch) {
        if (am > 1200)
            returnValue = false;
        else
            returnValue = true;
    } else if ((timeValue <= pm && timeValue <= am) && bSwitch) {
        returnValue = false;
    } else if ((timeValue >= pm && timeValue >= am) && bSwitch) {
        if (am > 1200)
            returnValue = false;
        else
            returnValue = true;
    }


    return returnValue;
}
#endif
