#include "DB.h"
#include "repository/SessionRepository.h"

namespace {

// MySQL implementation of the session seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, including the
//    boot sweep's lower-case "from", the mixed spacing of "LogOn='GAME'"
//    against "LogOn = 'LOGOFF'", the UserStatus INSERT's "Values" and its
//    INSERT IGNORE, and the lotto INSERT's positional VALUES list.
//  - Connections as before: Player, PCRoomUserInfo and PCRoomLottoObject
//    through the dist connection ("PLAYER_DB" — the name is ignored by
//    DatabaseManager::getDistConnection, which returns the thread's
//    second socket to the same DARKEDEN schema), UserStatus through the
//    USERINFO connection, GuildMember and UserIPInfo through DARKEDEN.
//  - The uint SpecialEventCount and the uint user count stream through
//    "%d", the DWORD PC-room id and BYTE race through "%u" — the callers'
//    conversions, kept.
//  - SpecialEventCount comes back through getDWORD, as before.
//  - Names and ids are interpolated raw, as before.
class MySQLSessionRepository : public SessionRepository {
public:
    void markGuildMemberLoggedOff(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET LogOn = 0 WHERE Name = '%s'", name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void markGuildMemberLoggedOn(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET LogOn = 1 WHERE Name = '%s'", name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadPlayerSession(const string& playerID, PlayerSessionRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();

#ifdef __THAILAND_SERVER__

            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID, CurrentServerGroupID, LogOn, SpecialEventCount, PayType, PayPlayDate, "
                "PayPlayHours, PayPlayFlag, BillingUserKey, FamilyPayPlayDate, Birthday FROM Player WHERE "
                "PlayerID = '%s'",
                playerID.c_str());

#else

            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID, CurrentServerGroupID, LogOn, SpecialEventCount, PayType, PayPlayDate, "
                "PayPlayHours, PayPlayFlag, BillingUserKey, FamilyPayPlayDate FROM Player WHERE PlayerID = '%s'",
                playerID.c_str());

#endif

            if (pResult->getRowCount() == 1) {
                pResult->next();

                int i = 0;
                row.playerID = pResult->getString(++i);
                row.serverGroupID = pResult->getInt(++i);
                row.logOn = pResult->getString(++i);
                row.specialEventCount = pResult->getDWORD(++i);
                row.payType = pResult->getInt(++i);
                row.payPlayDate = pResult->getString(++i);
                row.payPlayHours = pResult->getInt(++i);
                row.payPlayFlag = pResult->getInt(++i);
                row.billingUserKey = pResult->getInt(++i);
                row.familyPayPlayDate = pResult->getString(++i);
#ifdef __THAILAND_SERVER__
                row.birthday = pResult->getString(++i);
#endif
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool markPlayerLoggedOn(const string& playerID) {
        bool affected = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn='GAME' WHERE PlayerID = '%s' AND LogOn='LOGOFF'",
                                playerID.c_str());

            affected = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return affected;
    }

    bool loadPlayerLocation(const string& playerID, int& serverGroupID, string& logOn) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            // "USERINFO", not "PLAYER_DB": the name is ignored, and this
            // is what the call site wrote.
            pStmt = g_pDatabaseManager->getDistConnection("USERINFO")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT CurrentServerGroupID, LogOn FROM Player WHERE PlayerID='%s'",
                                                  playerID.c_str());

            if (pResult->next()) {
                serverGroupID = pResult->getInt(1);
                logOn = pResult->getString(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void markPlayerLoggedOff(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery(
                "UPDATE Player SET LogOn='LOGOFF', LastLogoutDate=now() WHERE PlayerID = '%s' AND LogOn='GAME'",
                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<string> loadPlayersInGame(int worldID, int serverGroupID) {
        vector<string> ids;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID from Player WHERE LogOn='GAME' AND CurrentWorldID=%d AND CurrentServerGroupID=%d",
                worldID, serverGroupID);

            while (pResult->next())
                ids.push_back(pResult->getString(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ids;
    }

    void logOffPlayersOfServer(int worldID, int serverGroupID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Player SET LogOn = 'LOGOFF' WHERE LogOn = 'GAME' AND CurrentWorldID=%d AND "
                                "CurrentServerGroupID=%d",
                                worldID, serverGroupID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadSpecialEventCount(const string& playerID, DWORD& count) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT SpecialEventCount FROM Player WHERE PlayerID='%s'", playerID.c_str());

            if (pResult->next()) {
                count = pResult->getDWORD(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void saveSpecialEventCount(uint count, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Player SET SpecialEventCount=%d WHERE PlayerID='%s'", count, playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deletePCRoomUser(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("DELETE FROM PCRoomUserInfo WHERE PlayerID='%s'", playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadPCRoomLottoAmount(const string& playerID, const string& name, uint dimensionID, uint worldID,
                               int& amount) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Amount FROM PCRoomLottoObject WHERE PlayerID = '%s' AND Name "
                                                  "= '%s' AND DimensionID = %u AND WorldID = %u",
                                                  playerID.c_str(), name.c_str(), dimensionID, worldID);

            if (pResult->next()) {
                amount = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void updatePCRoomLottoAmount(int amount, const string& playerID, const string& name, uint dimensionID,
                                 uint worldID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE PCRoomLottoObject SET Amount = %d WHERE PlayerID = '%s' AND Name = '%s' AND "
                                "DimensionID = %u AND WorldID = %u",
                                amount, playerID.c_str(), name.c_str(), dimensionID, worldID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertPCRoomLotto(ObjectID_t pcRoomID, const string& playerID, uint dimensionID, uint worldID,
                           const string& name, Race_t race) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("INSERT INTO PCRoomLottoObject VALUES ( 0, %u, '%s', %u, %u, '%s', %u, 1 )", pcRoomID,
                                playerID.c_str(), dimensionID, worldID, name.c_str(), race);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteUserIP(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM UserIPInfo WHERE Name = '%s'", name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteUserIPsOfServer(int serverID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM UserIPInfo WHERE ServerID = %d", serverID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool updateUserStatus(uint currentUser, int worldID, int serverID) {
        bool updated = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getUserInfoConnection()->createStatement();
            pStmt->executeQuery("UPDATE UserStatus SET CurrentUser=%d WHERE WorldID=%d AND ServerID=%d", currentUser,
                                worldID, serverID);

            updated = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return updated;
    }

    void insertUserStatus(int worldID, int serverID, uint currentUser) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getUserInfoConnection()->createStatement();
            pStmt->executeQuery("INSERT IGNORE INTO UserStatus (WorldID, ServerID, CurrentUser) Values (%d, %d, %d)",
                                worldID, serverID, currentUser);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

SessionRepository& defaultSessionRepository() {
    static MySQLSessionRepository instance;
    return instance;
}
