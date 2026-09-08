#ifndef __FAKE_LOGIN_ACCOUNT_REPOSITORY_H__
#define __FAKE_LOGIN_ACCOUNT_REPOSITORY_H__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "repository/LoginAccountRepository.h"

// In-memory LoginAccountRepository for the login-decision tests. Only the
// reads and the one compare-and-set the decision makes carry behaviour; the
// rest of the interface is present to satisfy it, records what it was asked
// to write, and answers "no row".
//
// The account map starts empty, which is the real "no such account" case:
// the loaders leave the caller's row untouched and return false.
class FakeLoginAccountRepository : public LoginAccountRepository {
public:
    // --- what the reads see -----------------------------------------------
    std::map<std::string, LoginAccountRow> accounts;
    std::map<std::string, std::string> storedPasswords;
    std::vector<LoginIPBlockRow> ipBlocks;
    std::set<std::string> unclaimedPremiumEvents;
    std::set<std::string> privateAgreementsRemaining;
    // The ids and names registration's two probes find.
    std::set<std::string> registeredIDs;
    std::set<std::string> registeredNames;

    struct WebLoginKeyRow {
        std::string key;
        std::string createTime;
        std::string now;
    };
    std::map<std::string, WebLoginKeyRow> webLoginKeys;

    // The reconnect projection of the same rows, kept apart because it
    // selects different columns.
    std::map<std::string, LoginReconnectRow> reconnectAccounts;

    // markLoggedOn changes a row only while its LogOn column reads LOGOFF,
    // which is what tells the callers that another session holds it.
    bool markLoggedOnSucceeds = true;
    bool markLoggedOnForReconnectSucceeds = true;

    // --- what was written --------------------------------------------------
    struct LoggedOn {
        std::string ip;
        int loginServerID;
        std::string playerID;
    };
    std::vector<LoggedOn> markedLoggedOn;
    std::vector<std::pair<int, std::string>> markedLoggedOnForReconnect; // (loginServerID, playerID)
    std::vector<std::pair<std::string, std::string>> updatedPasswords;   // (hash, playerID)
    std::vector<std::string> extendedPayPlay;
    std::vector<std::string> premiumEventsReceived;
    std::vector<std::string> deletedWebLoginKeys;
    std::vector<std::pair<std::string, std::string>> testClientUsers; // (playerID, ip)
    std::vector<std::string> loginRecords;
    std::vector<std::pair<std::string, std::string>> netMarbleAccounts; // (playerID, hash)
    std::vector<LoginNewAccount> insertedAccounts;
    std::vector<LoggedOn> markedLoggedOnAfterRegister;

    // --- how often the reads were made -------------------------------------
    int loadAccountCalls = 0;
    int loadAccountForWebLoginCalls = 0;
    int loadAccountForFreePassCalls = 0;
    int loadPasswordHashCalls = 0;
    int markLoggedOnCalls = 0;
    int loadIPBlocksCalls = 0;
    int hasUnclaimedPremiumEventCalls = 0;
    int loadAccountForReconnectCalls = 0;
    int markLoggedOnForReconnectCalls = 0;
    int accountExistsCalls = 0;

    // --- helpers -----------------------------------------------------------
    // An account that logs in cleanly: allowed, logged off, no pay plan.
    static LoginAccountRow allowedAccount(const std::string& playerID) {
        LoginAccountRow row;
        row.playerID = playerID;
        row.ssn = "";
        row.currentServerGroupID = 3;
        row.logOn = "LOGOFF";
        row.access = "ALLOW";
        row.zipCode = "123-456";
        row.loginIP = "10.0.0.1";
        row.payType = 0;
        row.payPlayDate = "";
        row.payPlayHours = 0;
        row.payPlayFlag = 0;
        row.familyPayPlayDate = "";
        return row;
    }

    // The same account through the reconnect projection: allowed, logged
    // off, no pay plan.
    static LoginReconnectRow allowedReconnectAccount() {
        LoginReconnectRow row;
        row.currentWorldID = 1;
        row.currentServerGroupID = 3;
        row.logOn = "LOGOFF";
        row.access = "ALLOW";
        row.payType = 0;
        row.payPlayDate = "";
        row.payPlayHours = 0;
        row.payPlayFlag = 0;
        return row;
    }

    // --- the login ---------------------------------------------------------
    void insertTestClientUser(const std::string& playerID, const std::string& ip) {
        testClientUsers.push_back(std::make_pair(playerID, ip));
    }

    bool loadAccount(const std::string& playerID, LoginAccountRow& row) {
        loadAccountCalls++;
        return loadInto(playerID, row);
    }

    bool loadAccountForWebLogin(const std::string& playerID, LoginAccountRow& row) {
        loadAccountForWebLoginCalls++;
        if (!loadInto(playerID, row))
            return false;
        // The projection does not select ZipCode.
        row.zipCode = "";
        return true;
    }

    bool loadAccountForFreePass(const std::string& playerID, LoginAccountRow& row) {
        loadAccountForFreePassCalls++;
        if (!loadInto(playerID, row))
            return false;
        // The projection selects neither SSN nor ZipCode.
        row.ssn = "";
        row.zipCode = "";
        return true;
    }

    bool loadPasswordHash(const std::string& playerID, std::string& stored) {
        loadPasswordHashCalls++;
        std::map<std::string, std::string>::const_iterator itr = storedPasswords.find(playerID);
        if (itr == storedPasswords.end())
            return false;
        stored = itr->second;
        return true;
    }

    void updatePassword(const std::string& hashed, const std::string& playerID) {
        updatedPasswords.push_back(std::make_pair(hashed, playerID));
        storedPasswords[playerID] = hashed;
    }

    bool markLoggedOn(const std::string& ip, int loginServerID, const std::string& playerID) {
        markLoggedOnCalls++;
        LoggedOn call;
        call.ip = ip;
        call.loginServerID = loginServerID;
        call.playerID = playerID;
        markedLoggedOn.push_back(call);
        return markLoggedOnSucceeds;
    }

    bool hasUnclaimedPremiumEvent(const std::string& playerID) {
        hasUnclaimedPremiumEventCalls++;
        return unclaimedPremiumEvents.count(playerID) != 0;
    }

    void extendPayPlayByWeek(const std::string& playerID) {
        extendedPayPlay.push_back(playerID);
    }

    void markPremiumEventReceived(const std::string& playerID) {
        premiumEventsReceived.push_back(playerID);
    }

    bool hasPrivateAgreementRemaining(const std::string& playerID) {
        return privateAgreementsRemaining.count(playerID) != 0;
    }

    void insertNetMarbleAccount(const std::string& playerID, const std::string& hashedPassword) {
        netMarbleAccounts.push_back(std::make_pair(playerID, hashedPassword));
    }

    std::vector<LoginIPBlockRow> loadIPBlocks(const std::string&, const std::string&, const std::string&) {
        loadIPBlocksCalls++;
        return ipBlocks;
    }

    bool loadWebLoginKey(const std::string& playerID, std::string& key, std::string& createTime, std::string& now) {
        std::map<std::string, WebLoginKeyRow>::const_iterator itr = webLoginKeys.find(playerID);
        if (itr == webLoginKeys.end())
            return false;
        key = itr->second.key;
        createTime = itr->second.createTime;
        now = itr->second.now;
        return true;
    }

    void deleteWebLoginKey(const std::string& playerID) {
        deletedWebLoginKeys.push_back(playerID);
    }

    void insertLoginRecord(const std::string& playerID, const std::string&, const std::string&, const std::string&) {
        loginRecords.push_back(playerID);
    }

    // --- the rest of the interface ------------------------------------------
    void markLoggedOff(const std::string&) {}

    bool loadLastLocation(const std::string&, int&, int&, int&) {
        return false;
    }

    bool setLoggedOn(const std::string&) {
        return false;
    }

    void setLoginIP(const std::string&, const std::string&) {}

    std::vector<std::string> loadLoggedOnAccounts(int) {
        return std::vector<std::string>();
    }

    void deletePCRoomUser(const std::string&) {}

    void logOffAllOnServer(int) {}

    void setCurrentServerGroup(int, const std::string&) {}

    void setCurrentLocation(int, int, int, const std::string&) {}

    bool loadCurrentLocation(LoginLocationSpelling, const std::string&, int&, int&) {
        return false;
    }

    bool loadCurrentWorld(const std::string&, int&) {
        return false;
    }

    bool loadCurrentServerGroup(const std::string&, int&) {
        return false;
    }

    // --- reconnect ----------------------------------------------------------
    bool loadAccountForReconnect(const std::string& playerID, LoginReconnectRow& row) {
        loadAccountForReconnectCalls++;
        std::map<std::string, LoginReconnectRow>::const_iterator itr = reconnectAccounts.find(playerID);
        if (itr == reconnectAccounts.end())
            return false;
        row = itr->second;
        return true;
    }

    bool markLoggedOnForReconnect(int loginServerID, const std::string& playerID) {
        markLoggedOnForReconnectCalls++;
        markedLoggedOnForReconnect.push_back(std::make_pair(loginServerID, playerID));
        return markLoggedOnForReconnectSucceeds;
    }

    // --- registration --------------------------------------------------------
    bool accountExists(const std::string& playerID) {
        accountExistsCalls++;
        return registeredIDs.count(playerID) != 0;
    }

    bool accountNameExists(const std::string& playerID) {
        return registeredNames.count(playerID) != 0;
    }

    void insertAccount(const LoginNewAccount& account) {
        insertedAccounts.push_back(account);
        registeredIDs.insert(account.playerID);
    }

    void markLoggedOnAfterRegister(const std::string& ip, int loginServerID, const std::string& playerID) {
        LoggedOn call;
        call.ip = ip;
        call.loginServerID = loginServerID;
        call.playerID = playerID;
        markedLoggedOnAfterRegister.push_back(call);
    }

private:
    bool loadInto(const std::string& playerID, LoginAccountRow& row) {
        std::map<std::string, LoginAccountRow>::const_iterator itr = accounts.find(playerID);
        if (itr == accounts.end())
            return false;
        row = itr->second;
        return true;
    }
};

#endif
