//////////////////////////////////////////////////////////////////////////////
// Filename    : CLRegisterPlayerHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLRegisterPlayer.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "DB.h"
#include "GameServerGroupInfoManager.h"
#include "LCRegisterPlayerError.h"
#include "LCRegisterPlayerOK.h"
#include "LoginPlayer.h"
#include "Properties.h"
#endif

#ifdef __LOGIN_SERVER__
namespace {

// Every string in the registration packet is interpolated into SQL text
// verbatim, so anything that could break out of a quoted literal is refused.
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

        if (containsSqlMetaCharacter(pPacket->getPassword())) {
            lcRegisterPlayerError.setErrorID(ETC_ERROR);
            throw string("Invalid Password");
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

    Statement* pStmt = NULL;
    Result* pResult = NULL;

    try {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

        //--------------------------------------------------------------------------------
        // Check for duplicate PlayerID.
        //--------------------------------------------------------------------------------
        pResult = pStmt->executeQuery("SELECT PlayerID FROM Player WHERE PlayerID = '%s'", pPacket->getID().c_str());

        if (pResult->getRowCount() != 0) {
            lcRegisterPlayerError.setErrorID(ALREADY_REGISTER_ID);
            throw DuplicatedException("that ID already exists");
        }

        //--------------------------------------------------------------------------------
        // The SSN used to be checked for uniqueness here. The client no longer
        // collects a national ID number and sends a fixed placeholder, so a
        // uniqueness check would reject every account after the first.
        //--------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------
        // Insert the new player row.
        //
        // The password must be stored the way CLLoginHandler compares it:
        // OLD_PASSWORD() when DB_VERSION starts with '4', otherwise plain text
        // (the Password column is varchar(16), which cannot hold a PASSWORD()
        // hash anyway).
        //--------------------------------------------------------------------------------
        const bool bOldPasswordHash = g_pConfig->hasKey("DB_VERSION") && g_pConfig->getProperty("DB_VERSION")[0] == '4';

        pResult = pStmt->executeQuery(
            "INSERT INTO Player (PlayerID , Password , Name , Sex , SSN , Telephone , Cellular , Zipcode , Address , "
            "Nation , Email , Homepage , Profile , Pub) VALUES ('%s' , %s('%s') , '%s' , '%s' , '%s' , '%s' , "
            "'%s' , '%s' , '%s' , %d , '%s' , '%s' , '%s' , '%s')",
            pPacket->getID().c_str(), bOldPasswordHash ? "OLD_PASSWORD" : "", pPacket->getPassword().c_str(),
            pPacket->getName().c_str(), Sex2String[pPacket->getSex()].c_str(), pPacket->getSSN().c_str(),
            pPacket->getTelephone().c_str(), pPacket->getCellular().c_str(), pPacket->getZipCode().c_str(),
            pPacket->getAddress().c_str(), (int)pPacket->getNation(), pPacket->getEmail().c_str(),
            pPacket->getHomepage().c_str(), pPacket->getProfile().c_str(),
            (pPacket->getPublic() == true) ? "PUBLIC" : "PRIVATE");

        // After successful insert, send LCRegisterPlayerOK to the client.
        Assert(pResult == NULL);
        Assert(pStmt->getAffectedRowCount() == 1);

        // The client goes on to the world list exactly as after CLLogin, so
        // mark the account logged on the way CLLoginHandler does; the
        // disconnect path only clears LogOn when it is 'LOGON'.
        pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGON', LoginIP = '%s', CurrentLoginServerID=%d, "
                            "LastLoginDate=now() WHERE PlayerID = '%s'",
                            pLoginPlayer->getSocket()->getHost().c_str(), g_pConfig->getPropertyInt("LoginServerID"),
                            pPacket->getID().c_str());

        // Retrieve current world/group IDs for the new user.
        pResult = pStmt->executeQuery("SELECT CurrentWorldID, CurrentServerGroupID FROM Player WHERE PlayerID = '%s'",
                                      pPacket->getID().c_str());

        if (pResult->getRowCount() == 0) {
            lcRegisterPlayerError.setErrorID(ETC_ERROR);
            throw SQLQueryException("the new player row could not be read back after the insert");
        }

        WorldID_t WorldID = 0;
        ServerGroupID_t ServerGroupID = 0;

        // next() is true when a row was fetched; the old check had this
        // inverted and turned every successful registration into ETC_ERROR.
        if (!pResult->next())
            throw SQLQueryException("the new player row could not be fetched after the insert");

        WorldID = pResult->getInt(1);
        ServerGroupID = pResult->getInt(2);

        pLoginPlayer->setServerGroupID(ServerGroupID);

        LCRegisterPlayerOK lcRegisterPlayerOK;

        lcRegisterPlayerOK.setGroupName(
            g_pGameServerGroupInfoManager->getGameServerGroupInfo(ServerGroupID, WorldID)->getGroupName());

        // The client does not collect a real national ID any more, so the
        // adult flag cannot be derived from the SSN. CLLoginHandler's client
        // side already ignores the flag and decides gore level from the
        // teen-version option alone; report the same thing here.
        lcRegisterPlayerOK.setAdult(true);

        pLoginPlayer->sendPacket(&lcRegisterPlayerOK);

        // Remember the ID: the session is now logged in as the new account.
        pLoginPlayer->setID(pPacket->getID());

        // Registration succeeded; wait for the world/PC list requests.
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);

        SAFE_DELETE(pStmt);
    } catch (DuplicatedException& de) {
        SAFE_DELETE(pStmt);

        //--------------------------------------------------------------------------------
        // Report the registration failure.
        //--------------------------------------------------------------------------------
        pLoginPlayer->sendPacket(&lcRegisterPlayerError);

        //--------------------------------------------------------------------------------
        // Count the failure; too many failures end the session.
        //--------------------------------------------------------------------------------
        uint nFailed = pLoginPlayer->getFailureCount() + 1;

        if (nFailed > 3)
            throw DisconnectException("too many failure");

        pLoginPlayer->setFailureCount(nFailed);

        // Registration failed; wait for another CLRegisterPlayer.
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_REGISTER_PLAYER);

    } catch (SQLQueryException& sqe) {
        SAFE_DELETE(pStmt);

        //--------------------------------------------------------------------------------
        // Report the registration failure.
        //--------------------------------------------------------------------------------
        lcRegisterPlayerError.setErrorID(ETC_ERROR);

        pLoginPlayer->sendPacket(&lcRegisterPlayerError);

        //--------------------------------------------------------------------------------
        // Count the failure; too many failures end the session.
        //--------------------------------------------------------------------------------
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
