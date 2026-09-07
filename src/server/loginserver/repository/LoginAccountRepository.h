#ifndef __LOGIN_ACCOUNT_REPOSITORY_H__
#define __LOGIN_ACCOUNT_REPOSITORY_H__

#include <string>
#include <vector>

// The loginserver's account persistence: the Player row through a
// session (the login reads and the LogOn / LoginIP / CurrentLoginServerID
// / LastLoginDate writes, the current world, group and slot, the stored
// password hash, registration), the login-side tables around it
// (TestClientUser, WebLogin, IPBlockInfo, Event200501Main,
// PrivateAgreementRemain, PCRoomUserInfo) and the per-login record in
// USERINFO.LoginPlayerData.
//
// Every method runs on the thread's DARKEDEN connection except
// insertLoginRecord, which uses g_pDatabaseManager->getUserInfoConnection()
// (the USERINFO database). In the loginserver the main thread runs every
// handler, so the DARKEDEN name is the process default connection.
//
// Reads are typed to the driver getter used: getInt → int, getString →
// std::string. The callers cast to their own types (PayType, WorldID_t,
// ServerGroupID_t, uint). Every argument is interpolated raw; the callers
// reject quotes and backslashes in the account id and the registration
// fields before calling, and nothing else is escaped.
//
// Behaviour the callers depend on:
//  - markLoggedOn / markLoggedOnForReconnect / setLoggedOn answer whether
//    the UPDATE changed a row. The connection has no CLIENT_FOUND_ROWS,
//    so a row already in the target state answers false; the callers
//    read that as another session holding the account.
//  - The two current-location SELECTs (CLGetServerListHandler and
//    CLRegisterPlayerHandler) differ only in the case of WHERE and the
//    spaces around '='; they sit behind LoginLocationSpelling so each
//    caller keeps its bytes.
//  - PrivateAgreementRemain is not in initdb/. Its only caller is under
//    __NETMARBLE_SERVER__; against the shipped schema the SELECT fails
//    as a SQL error crossing as END_DB's const char*.
//
// Not enclosed (SQL on the same tables elsewhere in the tree):
//  - Player: the gameserver's MySQLSessionRepository.cpp (the connect-time
//    read and the LogOn / LastLogoutDate / event-count writes) and
//    ServerCore's PaySystem.cpp (the pay-play columns, compiled into the
//    gameserver); the loginserver's LoginCharacterPurgeRepository does
//    not touch it. src/server/database/testdb.cpp is in no CMakeLists.
//  - PCRoomUserInfo: MySQLSessionRepository.cpp and PaySystem.cpp.
//  - Event200501Main: the gameserver's MySQLComebackEventRepository.cpp.
//  - TestClientUser, WebLogin, IPBlockInfo, LoginPlayerData: nothing else.

// The account row as the login reads it. loadAccount fills every field;
// loadAccountForWebLogin leaves zipCode empty (not selected);
// loadAccountForFreePass leaves ssn and zipCode empty (not selected).
struct LoginAccountRow {
    std::string playerID;
    std::string ssn;
    int currentServerGroupID;
    std::string logOn;
    std::string access;
    std::string zipCode;
    std::string loginIP;
    int payType;
    std::string payPlayDate;
    int payPlayHours;
    int payPlayFlag;
    std::string familyPayPlayDate;
};

// CLReconnectLoginHandler's projection.
struct LoginReconnectRow {
    int currentWorldID;
    int currentServerGroupID;
    std::string logOn;
    std::string access;
    int payType;
    std::string payPlayDate;
    int payPlayHours;
    int payPlayFlag;
};

// One IPBlockInfo row: the class the entry blocks (0 = a range of the
// last octet under a class C prefix, 1 = second octet under a class A
// prefix, 2 = third octet under a class B prefix) and its bounds.
struct LoginIPBlockRow {
    int ipClass;
    int first;
    int last;
};

// CLRegisterPlayerHandler's INSERT. password is the argon2id hash; pub is
// the text 'PUBLIC' or 'PRIVATE'; sex is the Sex2String text.
struct LoginNewAccount {
    std::string playerID;
    std::string password;
    std::string name;
    std::string sex;
    std::string ssn;
    std::string telephone;
    std::string cellular;
    std::string zipCode;
    std::string address;
    int nation;
    std::string email;
    std::string homepage;
    std::string profile;
    std::string pub;
};

enum LoginLocationSpelling {
    // "... FROM Player where PlayerID='%s'" — CLGetServerListHandler.
    LOGIN_LOCATION_SQL_LOWER,
    // "... FROM Player WHERE PlayerID = '%s'" — CLRegisterPlayerHandler.
    LOGIN_LOCATION_SQL_UPPER,
    LOGIN_LOCATION_SQL_SPELLING_MAX
};

class LoginAccountRepository {
public:
    virtual ~LoginAccountRepository() {}

    // --- the login (CLLoginHandler) ------------------------------------------
    // One TestClientUser row (PlayerID, IP, LoginDate = now()) for a
    // connection whose id carried the test-client marker.
    virtual void insertTestClientUser(const std::string& playerID, const std::string& ip) = 0;
    // The twelve-column read (with SSN and ZipCode). False when there is
    // no row; row is untouched then.
    virtual bool loadAccount(const std::string& playerID, LoginAccountRow& row) = 0;
    // Eleven columns: no ZipCode.
    virtual bool loadAccountForWebLogin(const std::string& playerID, LoginAccountRow& row) = 0;
    // Ten columns: no SSN, no ZipCode.
    virtual bool loadAccountForFreePass(const std::string& playerID, LoginAccountRow& row) = 0;
    // The stored Password column; false when there is no row.
    virtual bool loadPasswordHash(const std::string& playerID, std::string& stored) = 0;
    virtual void updatePassword(const std::string& hashed, const std::string& playerID) = 0;
    // LogOn = 'LOGON', LoginIP, CurrentLoginServerID, LastLoginDate = now()
    // for a row that is LOGOFF. True when a row changed.
    virtual bool markLoggedOn(const std::string& ip, int loginServerID, const std::string& playerID) = 0;
    // Event200501Main: does the account still hold an unclaimed premium
    // week (RecvPremiumDate = '0000-00-00')?
    virtual bool hasUnclaimedPremiumEvent(const std::string& playerID) = 0;
    // PayPlayDate moves a week ahead of now() or of itself, whichever is
    // later.
    virtual void extendPayPlayByWeek(const std::string& playerID) = 0;
    virtual void markPremiumEventReceived(const std::string& playerID) = 0;
    // PrivateAgreementRemain: is the account still waiting on the
    // agreement? (Table not in initdb/, see above.)
    virtual bool hasPrivateAgreementRemaining(const std::string& playerID) = 0;
    // The NetMarble auto-created account: INSERT IGNORE with the hashed
    // password, Name = the id, a fixed SSN, SpecialEventCount 2, Event 0,
    // creation_date = CURDATE().
    virtual void insertNetMarbleAccount(const std::string& playerID, const std::string& hashedPassword) = 0;
    // Every IPBlockInfo row matching the three prefixes: (IP = classA AND
    // class=1) OR (IP = classB AND class=2) OR (IP = classC).
    virtual std::vector<LoginIPBlockRow> loadIPBlocks(const std::string& classA, const std::string& classB,
                                                      const std::string& classC) = 0;
    // WebLogin: the key, its CreateTime and the database's now() as text.
    // False when there is no row; the out-parameters are untouched then.
    virtual bool loadWebLoginKey(const std::string& playerID, std::string& key, std::string& createTime,
                                 std::string& now) = 0;
    virtual void deleteWebLoginKey(const std::string& playerID) = 0;
    // One USERINFO.LoginPlayerData row (PlayerID, IP, Date, Time), on the
    // USERINFO connection.
    virtual void insertLoginRecord(const std::string& playerID, const std::string& ip, const std::string& date,
                                   const std::string& time) = 0;

    // --- the session (LoginPlayer, LoginPlayerManager) ---------------------------
    // LogOn = 'LOGOFF' for a row that is LOGON.
    virtual void markLoggedOff(const std::string& playerID) = 0;
    // CurrentWorldID, CurrentServerGroupID, LastSlot. False when there is
    // no row.
    virtual bool loadLastLocation(const std::string& playerID, int& worldID, int& serverGroupID, int& lastSlot) = 0;
    // LogOn = 'LOGON' whatever the row holds. True when a row changed.
    virtual bool setLoggedOn(const std::string& playerID) = 0;
    virtual void setLoginIP(const std::string& ip, const std::string& playerID) = 0;
    // Every PlayerID left LOGON on this login server.
    virtual std::vector<std::string> loadLoggedOnAccounts(int loginServerID) = 0;
    virtual void deletePCRoomUser(const std::string& playerID) = 0;
    // LogOn = 'LOGOFF' for every LOGON row on this login server.
    virtual void logOffAllOnServer(int loginServerID) = 0;

    // --- the current world, group and slot (the CL handlers) -----------------
    virtual void setCurrentServerGroup(int serverGroupID, const std::string& playerID) = 0;
    virtual void setCurrentLocation(int worldID, int serverGroupID, int slot, const std::string& playerID) = 0;
    // CurrentWorldID and CurrentServerGroupID. False when there is no row.
    virtual bool loadCurrentLocation(LoginLocationSpelling spelling, const std::string& playerID, int& worldID,
                                     int& serverGroupID) = 0;
    virtual bool loadCurrentWorld(const std::string& playerID, int& worldID) = 0;
    virtual bool loadCurrentServerGroup(const std::string& playerID, int& serverGroupID) = 0;

    // --- reconnect (CLReconnectLoginHandler) ---------------------------------
    virtual bool loadAccountForReconnect(const std::string& playerID, LoginReconnectRow& row) = 0;
    // LogOn = 'LOGON', CurrentLoginServerID for a row that is LOGOFF. True
    // when a row changed.
    virtual bool markLoggedOnForReconnect(int loginServerID, const std::string& playerID) = 0;

    // --- registration (CLRegisterPlayerHandler, CLQueryPlayerIDHandler) --------
    // SELECT PlayerID: does the row exist?
    virtual bool accountExists(const std::string& playerID) = 0;
    // The same probe through SELECT Name (CLQueryPlayerIDHandler's).
    virtual bool accountNameExists(const std::string& playerID) = 0;
    virtual void insertAccount(const LoginNewAccount& account) = 0;
    // LogOn = 'LOGON', LoginIP, CurrentLoginServerID, LastLoginDate = now()
    // whatever the row holds.
    virtual void markLoggedOnAfterRegister(const std::string& ip, int loginServerID, const std::string& playerID) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLLoginAccountRepository.cpp.
LoginAccountRepository& defaultLoginAccountRepository();

#endif
