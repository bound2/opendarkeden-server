#include "DB.h"
#include "repository/LoginConfigRepository.h"

namespace {

// MAX() over an empty table is one row whose field is NULL; that answers
// false instead of reaching atoi.
bool readMax(Result* pResult, int& maxValue) {
    if (!pResult->next())
        return false;

    const char* field = pResult->getField(1);
    if (field == NULL)
        return false;

    maxValue = atoi(field);
    return true;
}

// MySQL implementation of LoginConfigRepository. Everything runs on
// getConnection("DARKEDEN").
class MySQLLoginConfigRepository : public LoginConfigRepository {
public:
    bool loadMaxGameServerGroupWorldID(int& maxWorldID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MAX(WorldID) FROM GameServerGroupInfo");

            found = readMax(pResult, maxWorldID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<LoginGameServerGroupRow> loadGameServerGroups() {
        vector<LoginGameServerGroupRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT WorldID, GroupID, GroupName, Stat FROM GameServerGroupInfo");

            while (pResult->next()) {
                LoginGameServerGroupRow row;
                row.worldID = pResult->getInt(1);
                row.groupID = pResult->getInt(2);
                row.groupName = pResult->getString(3);
                row.stat = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LoginGameServerGroupIDRow> loadGameServerGroupIDs() {
        vector<LoginGameServerGroupIDRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT WorldID, GroupID FROM GameServerGroupInfo");

            while (pResult->next()) {
                LoginGameServerGroupIDRow row;
                row.worldID = pResult->getInt(1);
                row.groupID = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LoginZoneGroupRow> loadZoneGroups() {
        vector<LoginZoneGroupRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ZoneGroupID , ServerID FROM ZoneGroupInfo");

            while (pResult->next()) {
                LoginZoneGroupRow row;
                row.zoneGroupID = pResult->getWORD(1);
                row.serverID = pResult->getWORD(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LoginZoneRow> loadZones() {
        vector<LoginZoneRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ZoneID , ZoneGroupID FROM ZoneInfo");

            while (pResult->next()) {
                LoginZoneRow row;
                row.zoneID = pResult->getWORD(1);
                row.zoneGroupID = pResult->getWORD(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadClientVersion(int& version) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Version FROM ClientVersion");

            if (pResult->next()) {
                version = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

LoginConfigRepository& defaultLoginConfigRepository() {
    static MySQLLoginConfigRepository instance;
    return instance;
}
