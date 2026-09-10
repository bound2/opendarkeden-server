#include "DB.h"
#include "repository/LoginAccountRepository.h"

namespace {

const char* const kCurrentLocationSelect[LOGIN_LOCATION_SQL_SPELLING_MAX] = {
    "SELECT CurrentWorldID, CurrentServerGroupID FROM Player where PlayerID='%s'",
    "SELECT CurrentWorldID, CurrentServerGroupID FROM Player WHERE PlayerID = '%s'",
};

// MySQL implementation of LoginAccountRepository. Everything runs on
// getConnection("DARKEDEN") except insertLoginRecord, which uses the
// USERINFO connection. Each method frees its Statement on every success
// path; a SQL failure is logged to DBError.log under the method's name and
// rethrown as END_DB's DatabaseError.
class MySQLLoginAccountRepository : public LoginAccountRepository {
public:
    void insertTestClientUser(const string& playerID, const string& ip) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO TestClientUser (PlayerID, IP, LoginDate) VALUES ('%s', '%s', now())",
                                playerID.c_str(), ip.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadAccount(const string& playerID, LoginAccountRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PlayerID, SSN, CurrentServerGroupID, LogOn, Access, ZipCode, "
                                                  "LoginIP, PayType, PayPlayDate, PayPlayHours, PayPlayFlag, "
                                                  "FamilyPayPlayDate FROM Player WHERE PlayerID = '%s'",
                                                  playerID.c_str());

            if (pResult->next()) {
                int i = 0;
                row.playerID = pResult->getString(++i);
                row.ssn = pResult->getString(++i);
                row.currentServerGroupID = pResult->getInt(++i);
                row.logOn = pResult->getString(++i);
                row.access = pResult->getString(++i);
                row.zipCode = pResult->getString(++i);
                row.loginIP = pResult->getString(++i);
                row.payType = pResult->getInt(++i);
                row.payPlayDate = pResult->getString(++i);
                row.payPlayHours = pResult->getInt(++i);
                row.payPlayFlag = pResult->getInt(++i);
                row.familyPayPlayDate = pResult->getString(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadAccountForWebLogin(const string& playerID, LoginAccountRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID, SSN, CurrentServerGroupID, LogOn, Access, LoginIP, PayType, PayPlayDate, "
                "PayPlayHours, PayPlayFlag, FamilyPayPlayDate FROM Player WHERE PlayerID = '%s'",
                playerID.c_str());

            if (pResult->next()) {
                int i = 0;
                row.playerID = pResult->getString(++i);
                row.ssn = pResult->getString(++i);
                row.currentServerGroupID = pResult->getInt(++i);
                row.logOn = pResult->getString(++i);
                row.access = pResult->getString(++i);
                row.zipCode = "";
                row.loginIP = pResult->getString(++i);
                row.payType = pResult->getInt(++i);
                row.payPlayDate = pResult->getString(++i);
                row.payPlayHours = pResult->getInt(++i);
                row.payPlayFlag = pResult->getInt(++i);
                row.familyPayPlayDate = pResult->getString(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadAccountForFreePass(const string& playerID, LoginAccountRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID, CurrentServerGroupID, LogOn, Access, LoginIP, PayType, PayPlayDate, PayPlayHours, "
                "PayPlayFlag, FamilyPayPlayDate FROM Player WHERE PlayerID = '%s'",
                playerID.c_str());

            if (pResult->next()) {
                int i = 0;
                row.playerID = pResult->getString(++i);
                row.ssn = "";
                row.currentServerGroupID = pResult->getInt(++i);
                row.logOn = pResult->getString(++i);
                row.access = pResult->getString(++i);
                row.zipCode = "";
                row.loginIP = pResult->getString(++i);
                row.payType = pResult->getInt(++i);
                row.payPlayDate = pResult->getString(++i);
                row.payPlayHours = pResult->getInt(++i);
                row.payPlayFlag = pResult->getInt(++i);
                row.familyPayPlayDate = pResult->getString(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadPasswordHash(const string& playerID, string& stored) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Password FROM Player WHERE PlayerID = '%s'", playerID.c_str());

            if (pResult->next()) {
                stored = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void updatePassword(const string& hashed, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET Password = '%s' WHERE PlayerID = '%s'", hashed.c_str(),
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool markLoggedOn(const string& ip, int loginServerID, const string& playerID) {
        bool changed = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGON', LoginIP = '%s', CurrentLoginServerID=%d, "
                                "LastLoginDate=now() WHERE PlayerID = '%s' AND LogOn='LOGOFF'",
                                ip.c_str(), loginServerID, playerID.c_str());

            changed = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return changed;
    }

    bool hasUnclaimedPremiumEvent(const string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID FROM Event200501Main WHERE PlayerID = '%s' AND RecvPremiumDate = '0000-00-00'",
                playerID.c_str());

            found = pResult->next();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void extendPayPlayByWeek(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET PayPlayDate = IF (PayPlayDate < NOW(), NOW() + INTERVAL "
                                "7 DAY, PayPlayDate + INTERVAL 7 DAY ) WHERE PlayerID = '%s'",
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void markPremiumEventReceived(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Event200501Main SET RecvPremiumDate = NOW() WHERE PlayerID = '%s'",
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool hasPrivateAgreementRemaining(const string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PlayerID FROM PrivateAgreementRemain WHERE PlayerID = '%s'",
                                                  playerID.c_str());

            found = pResult->next();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertNetMarbleAccount(const string& playerID, const string& hashedPassword) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT IGNORE INTO Player (PlayerID, Password, Name, SSN, SpecialEventCount, Event, "
                                "creation_date) Values ('%s', '%s', '%s', '123456-1122339', 2, 0, CURDATE())",
                                playerID.c_str(), hashedPassword.c_str(), playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<LoginIPBlockRow> loadIPBlocks(const string& classA, const string& classB, const string& classC) {
        vector<LoginIPBlockRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT class, first, last FROM IPBlockInfo WHERE	(IP = '%s' AND "
                                                  "class=1) OR (IP = '%s' AND class=2) OR (IP = '%s')",
                                                  classA.c_str(), classB.c_str(), classC.c_str());

            while (pResult->next()) {
                LoginIPBlockRow row;
                row.ipClass = pResult->getInt(1);
                row.first = pResult->getInt(2);
                row.last = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadWebLoginKey(const string& playerID, string& key, string& createTime, string& now) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT LoginKey, CreateTime, now() FROM WebLogin WHERE PlayerID = '%s'", playerID.c_str());

            if (pResult->next()) {
                key = pResult->getString(1);
                createTime = pResult->getString(2);
                now = pResult->getString(3);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void deleteWebLoginKey(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM WebLogin WHERE PlayerID = '%s'", playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertLoginRecord(const string& playerID, const string& ip, const string& date, const string& time) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getUserInfoConnection()->createStatement();
            pStmt->executeQuery("INSERT INTO LoginPlayerData (PlayerID,IP,Date,Time) VALUES ('%s','%s','%s','%s')",
                                playerID.c_str(), ip.c_str(), date.c_str(), time.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void markLoggedOff(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGOFF' WHERE PlayerID='%s' AND LogOn='LOGON'",
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadLastLocation(const string& playerID, int& worldID, int& serverGroupID, int& lastSlot) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT CurrentWorldID, CurrentServerGroupID, LastSlot FROM Player where PlayerID='%s'",
                playerID.c_str());

            if (pResult->next()) {
                worldID = pResult->getInt(1);
                serverGroupID = pResult->getInt(2);
                lastSlot = pResult->getInt(3);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool setLoggedOn(const string& playerID) {
        bool changed = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGON' WHERE PlayerID = '%s'", playerID.c_str());

            changed = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return changed;
    }

    void setLoginIP(const string& ip, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LoginIP = '%s' WHERE PlayerID = '%s'", ip.c_str(), playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<string> loadLoggedOnAccounts(int loginServerID) {
        vector<string> ids;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID from Player WHERE LogOn = 'LOGON' AND CurrentLoginServerID=%d", loginServerID);

            while (pResult->next()) {
                ids.push_back(pResult->getString(1));
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ids;
    }

    void deletePCRoomUser(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM PCRoomUserInfo WHERE PlayerID='%s'", playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void logOffAllOnServer(int loginServerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGOFF' WHERE LogOn = 'LOGON' AND CurrentLoginServerID=%d",
                                loginServerID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void setCurrentServerGroup(int serverGroupID, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player set CurrentServerGroupID = %d WHERE PlayerID = '%s'", serverGroupID,
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void setCurrentLocation(int worldID, int serverGroupID, int slot, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE Player Set CurrentWorldID = %d, CurrentServerGroupID = %d, LastSlot = %d WHERE PlayerID = '%s'",
                worldID, serverGroupID, slot, playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadCurrentLocation(LoginLocationSpelling spelling, const string& playerID, int& worldID, int& serverGroupID) {
        if (spelling >= LOGIN_LOCATION_SQL_SPELLING_MAX)
            return false;

        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(kCurrentLocationSelect[spelling], playerID.c_str());

            if (pResult->next()) {
                worldID = pResult->getInt(1);
                serverGroupID = pResult->getInt(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadCurrentWorld(const string& playerID, int& worldID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT CurrentWorldID FROM Player where PlayerID='%s'", playerID.c_str());

            if (pResult->next()) {
                worldID = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadCurrentServerGroup(const string& playerID, int& serverGroupID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT CurrentServerGroupID FROM Player where PlayerID='%s'", playerID.c_str());

            if (pResult->next()) {
                serverGroupID = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadAccountForReconnect(const string& playerID, LoginReconnectRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT CurrentWorldID, CurrentServerGroupID, LogOn, Access, PayType, "
                                    "PayPlayDate, PayPlayHours, PayPlayFlag FROM Player WHERE PlayerID = '%s'",
                                    playerID.c_str());

            if (pResult->next()) {
                row.currentWorldID = pResult->getInt(1);
                row.currentServerGroupID = pResult->getInt(2);
                row.logOn = pResult->getString(3);
                row.access = pResult->getString(4);
                row.payType = pResult->getInt(5);
                row.payPlayDate = pResult->getString(6);
                row.payPlayHours = pResult->getInt(7);
                row.payPlayFlag = pResult->getInt(8);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool markLoggedOnForReconnect(int loginServerID, const string& playerID) {
        bool changed = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE Player SET LogOn='LOGON', CurrentLoginServerID=%d WHERE PlayerID='%s' AND LogOn='LOGOFF'",
                loginServerID, playerID.c_str());

            changed = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return changed;
    }

    bool accountExists(const string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT PlayerID FROM Player WHERE PlayerID = '%s'", playerID.c_str());

            found = pResult->getRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool accountNameExists(const string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Name FROM Player WHERE PlayerID = '%s'", playerID.c_str());

            found = pResult->getRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertAccount(const LoginNewAccount& a) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO Player (PlayerID , Password , Name , Sex , SSN , Telephone , Cellular , Zipcode , Address "
                ", "
                "Nation , Email , Homepage , Profile , Pub) VALUES ('%s' , '%s' , '%s' , '%s' , '%s' , '%s' , "
                "'%s' , '%s' , '%s' , %d , '%s' , '%s' , '%s' , '%s')",
                a.playerID.c_str(), a.password.c_str(), a.name.c_str(), a.sex.c_str(), a.ssn.c_str(),
                a.telephone.c_str(), a.cellular.c_str(), a.zipCode.c_str(), a.address.c_str(), a.nation,
                a.email.c_str(), a.homepage.c_str(), a.profile.c_str(), a.pub.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void markLoggedOnAfterRegister(const string& ip, int loginServerID, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGON', LoginIP = '%s', CurrentLoginServerID=%d, "
                                "LastLoginDate=now() WHERE PlayerID = '%s'",
                                ip.c_str(), loginServerID, playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

LoginAccountRepository& defaultLoginAccountRepository() {
    static MySQLLoginAccountRepository instance;
    return instance;
}
