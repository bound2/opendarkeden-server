//////////////////////////////////////////////////////////////////////////////
// Filename    : ReconnectDecision.cpp
// Description : the reconnect decision behind CLReconnectLoginHandler.
//////////////////////////////////////////////////////////////////////////////

#include "ReconnectDecision.h"

namespace {

// The id an account already in a game is left under.
const char* const kNoAccount = "NONE";

ReconnectRejection refusal(ReconnectRejectReason reason) {
    ReconnectRejection rejection;
    rejection.reason = reason;
    return rejection;
}

} // namespace

Outcome<ReconnectAccepted, ReconnectRejection>
decideReconnectLogin(const ReconnectRequest& request, LoginAccountRepository& repository, ReconnectSession& session) {
    typedef Outcome<ReconnectAccepted, ReconnectRejection> Result;

    LoginReconnectRow account;
    if (!repository.loadAccountForReconnect(request.playerID, account))
        return Result::Rejected(refusal(ReconnectRejectReason::NoSuchAccount));

    ReconnectAccepted accepted;
    accepted.worldID = account.currentWorldID;
    accepted.serverGroupID = account.currentServerGroupID;

    // Where the account is, whatever the checks below decide.
    session.setWorldID(accepted.worldID);
    session.setServerGroupID(accepted.serverGroupID);

    if (account.logOn == "GAME" || account.logOn == "LOGON") {
        session.setID(kNoAccount);

        ReconnectRejection rejection = refusal(ReconnectRejectReason::AlreadyInGame);
        rejection.logOn = account.logOn;
        return Result::Rejected(rejection);
    }

    if (account.logOn == "LOGOFF") {
        // LogOn flips to LOGON for a LOGOFF row; a row that did not change
        // belongs to a session already logged on.
        if (!repository.markLoggedOnForReconnect(request.loginServerID, request.playerID))
            return Result::Rejected(refusal(ReconnectRejectReason::AlreadyLoggedOnElsewhere));
    }

    // The account is marked logged on before this, so a refused account
    // leaves its row LOGON for the logout sweep to clear.
    if (account.access != "ALLOW")
        return Result::Rejected(refusal(ReconnectRejectReason::AccessNotAllowed));

#ifdef __PAY_SYSTEM_LOGIN__
    if (!session.loginPayPlay(account.payType, account.payPlayDate, account.payPlayHours, account.payPlayFlag,
                              request.connectIP, request.playerID)) {
        return Result::Rejected(refusal(ReconnectRejectReason::NotPayAccount));
    }
#elif defined(__PAY_SYSTEM_FREE_LIMIT__)
    if (session.loginPayPlay(account.payType, account.payPlayDate, account.payPlayHours, account.payPlayFlag,
                             request.connectIP, request.playerID)) {
        // Admitted either way.
    }
#else
    session.setPayPlayValue(account.payType, account.payPlayDate, account.payPlayHours, account.payPlayFlag);
#endif

    return Result::Ok(accepted);
}
