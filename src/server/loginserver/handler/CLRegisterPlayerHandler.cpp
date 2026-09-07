//////////////////////////////////////////////////////////////////////////////
// Filename    : CLRegisterPlayerHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLRegisterPlayer.h"

#ifdef __LOGIN_SERVER__
#include <exception>

#include "Assert1.h"
#include "GameServerGroupInfoManager.h"
#include "LCRegisterPlayerError.h"
#include "LCRegisterPlayerOK.h"
#include "LoginPlayer.h"
#include "PasswordHash.h"
#include "Properties.h"
#include "repository/LoginAccountRepository.h"
#endif

#ifdef __LOGIN_SERVER__
namespace {

// Every string in the registration packet except the password (only its
// argon2 hash reaches SQL) is interpolated into SQL text verbatim, so
// anything that could break out of a quoted literal is refused.
bool containsSqlMetaCharacter(const string& s) {
    return s.find_first_of("'\\\";") != string::npos;
}

} // namespace
#endif

//////////////////////////////////////////////////////////////////////////////
// A fresh connection (LPS_BEGIN_SESSION) may register an account: validate
// the packet, insert the Player row, and answer with LCRegisterPlayerOK or
// LCRegisterPlayerError. On success the session is logged in as the new
// account and continues like a normal login (world list, PC list, ...).
//////////////////////////////////////////////////////////////////////////////
void CLRegisterPlayerHandler::execute(CLRegisterPlayer* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    __BEGIN_DEBUG

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    // cout << "Registering Player... " << endl;

    //----------------------------------------------------------------------
    // Ensure the login user ID is "guest".
    //----------------------------------------------------------------------
    //	if (pLoginPlayer->getID() != "guest")
    //		throw InvalidProtocolException("must be guest user");

    //----------------------------------------------------------------------
    // Validate player profile fields; use NULL checks for each string.
    //----------------------------------------------------------------------
    LCRegisterPlayerError lcRegisterPlayerError;

    try {
        // cout << "Player registration : " << pPacket->toString() << endl;

        if (pPacket->getID() == "") {
            lcRegisterPlayerError.setErrorID(EMPTY_ID);
            throw string("ID field is empty");
        }

        if (pPacket->getID().size() < 4) {
            lcRegisterPlayerError.setErrorID(SMALL_ID_LENGTH);
            throw string("too small ID length");
        }

        if (containsSqlMetaCharacter(pPacket->getID())) {
            lcRegisterPlayerError.setErrorID(ETC_ERROR);
            throw string("Invalid ID");
        }

        if (pPacket->getPassword() == "") {
            lcRegisterPlayerError.setErrorID(EMPTY_PASSWORD);
            throw string("Password field is empty");
        }

        if (pPacket->getPassword().size() < 6) {
            lcRegisterPlayerError.setErrorID(SMALL_PASSWORD_LENGTH);
            throw string("too small password length");
        }

        if (pPacket->getName() == "") {
            lcRegisterPlayerError.setErrorID(EMPTY_NAME);
            throw string("Name field is empty");
        }

        if (pPacket->getSSN() == "") {
            lcRegisterPlayerError.setErrorID(EMPTY_SSN);
            throw string("SSN field is empty");
        }

        if (containsSqlMetaCharacter(pPacket->getName()) || containsSqlMetaCharacter(pPacket->getSSN()) ||
            containsSqlMetaCharacter(pPacket->getTelephone()) || containsSqlMetaCharacter(pPacket->getCellular()) ||
            containsSqlMetaCharacter(pPacket->getZipCode()) || containsSqlMetaCharacter(pPacket->getAddress()) ||
            containsSqlMetaCharacter(pPacket->getEmail()) || containsSqlMetaCharacter(pPacket->getHomepage()) ||
            containsSqlMetaCharacter(pPacket->getProfile())) {
            lcRegisterPlayerError.setErrorID(ETC_ERROR);
            throw string("Invalid profile field");
        }

    } catch (string& errstr) {
        pLoginPlayer->sendPacket(&lcRegisterPlayerError);

        // cout << lcRegisterPlayerError.toString() << endl;

        // For now disconnect the client on validation failure.
        // *TODO* Allow guest to retry without full disconnect.
        throw DisconnectException(lcRegisterPlayerError.toString());
    }


    //----------------------------------------------------------------------
    // Insert into the database.
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    // Hash the password before touching the database. Only the hash is
    // stored; CLLoginHandler verifies logins against it in C++.
    //----------------------------------------------------------------------
    string hashedPassword;
    try {
        hashedPassword = de::password::hash(pPacket->getPassword());
    } catch (const std::exception& e) {
        lcRegisterPlayerError.setErrorID(ETC_ERROR);
        pLoginPlayer->sendPacket(&lcRegisterPlayerError);
        filelog("loginfail.txt", "Password hashing failed, PlayerID : %s : %s", pPacket->getID().c_str(), e.what());
        throw DisconnectException("password hashing failed");
    }

    LoginAccountRepository& repo = defaultLoginAccountRepository();

    try {
        if (repo.accountExists(pPacket->getID())) {
            lcRegisterPlayerError.setErrorID(ALREADY_REGISTER_ID);
            throw DuplicatedException("that ID already exists");
        }

        LoginNewAccount account;
        account.playerID = pPacket->getID();
        account.password = hashedPassword;
        account.name = pPacket->getName();
        account.sex = Sex2String[pPacket->getSex()];
        account.ssn = pPacket->getSSN();
        account.telephone = pPacket->getTelephone();
        account.cellular = pPacket->getCellular();
        account.zipCode = pPacket->getZipCode();
        account.address = pPacket->getAddress();
        account.nation = (int)pPacket->getNation();
        account.email = pPacket->getEmail();
        account.homepage = pPacket->getHomepage();
        account.profile = pPacket->getProfile();
        account.pub = (pPacket->getPublic() == true) ? "PUBLIC" : "PRIVATE";

        repo.insertAccount(account);

        // The new account is logged on at once.
        repo.markLoggedOnAfterRegister(pLoginPlayer->getSocket()->getHost(), g_pConfig->getPropertyInt("LoginServerID"),
                                       pPacket->getID());

        int currentWorldID = 0;
        int currentServerGroupID = 0;
        if (!repo.loadCurrentLocation(LOGIN_LOCATION_SQL_UPPER, pPacket->getID(), currentWorldID,
                                      currentServerGroupID)) {
            lcRegisterPlayerError.setErrorID(ETC_ERROR);
            throw SQLQueryException("the new player row could not be read back after the insert");
        }

        WorldID_t WorldID = currentWorldID;
        ServerGroupID_t ServerGroupID = currentServerGroupID;

        pLoginPlayer->setServerGroupID(ServerGroupID);

        LCRegisterPlayerOK lcRegisterPlayerOK;
        lcRegisterPlayerOK.setGroupName(
            g_pGameServerGroupInfoManager->getGameServerGroupInfo(ServerGroupID, WorldID)->getGroupName());
        lcRegisterPlayerOK.setAdult(true);
        pLoginPlayer->sendPacket(&lcRegisterPlayerOK);

        pLoginPlayer->setID(pPacket->getID());
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);
    } catch (DuplicatedException& de) {
        pLoginPlayer->sendPacket(&lcRegisterPlayerError);

        uint nFailed = pLoginPlayer->getFailureCount() + 1;
        if (nFailed > 3)
            throw DisconnectException("too many failure");
        pLoginPlayer->setFailureCount(nFailed);

        // Registration failed; wait for another CLRegisterPlayer.
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_REGISTER_PLAYER);
    } catch (SQLQueryException& sqe) {
        // The handler's own throw above: the row could not be read back.
        lcRegisterPlayerError.setErrorID(ETC_ERROR);
        pLoginPlayer->sendPacket(&lcRegisterPlayerError);

        uint nFailed = pLoginPlayer->getFailureCount() + 1;
        if (nFailed > 3)
            throw DisconnectException("too many failure");
        pLoginPlayer->setFailureCount(nFailed);

        // Registration failed; wait for another CLRegisterPlayer.
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_REGISTER_PLAYER);
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); answered like the case
        // above.
        lcRegisterPlayerError.setErrorID(ETC_ERROR);
        pLoginPlayer->sendPacket(&lcRegisterPlayerError);

        uint nFailed = pLoginPlayer->getFailureCount() + 1;
        if (nFailed > 3)
            throw DisconnectException("too many failure");
        pLoginPlayer->setFailureCount(nFailed);

        // Registration failed; wait for another CLRegisterPlayer.
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_REGISTER_PLAYER);
    }
    __END_DEBUG

#endif

    __END_DEBUG_EX __END_CATCH
}
