#include "DB.h"
#include "repository/SharedConfigRepository.h"

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

// MySQL implementation of SharedConfigRepository. Everything runs on
// getConnection("DARKEDEN").
class MySQLSharedConfigRepository : public SharedConfigRepository {
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

    vector<SharedGameServerGroupRow> loadGameServerGroups() {
        vector<SharedGameServerGroupRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT WorldID, GroupID, GroupName FROM GameServerGroupInfo");

            while (pResult->next()) {
                SharedGameServerGroupRow row;
                row.worldID = pResult->getInt(1);
                row.groupID = pResult->getInt(2);
                row.groupName = pResult->getString(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxGameServerGroupID(int worldID, int& maxGroupID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT MAX(GroupID) FROM GameServerInfo WHERE WorldID = %d", worldID);

            found = readMax(pResult, maxGroupID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<SharedGameServerRow> loadGameServers() {
        vector<SharedGameServerRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ServerID, Nickname , IP , TCPPort , UDPPort, GroupID, Stat, WorldID FROM GameServerInfo");

            while (pResult->next()) {
                int i = 0;
                SharedGameServerRow row;
                row.serverID = pResult->getInt(++i);
                row.nickname = pResult->getString(++i);
                row.ip = pResult->getString(++i);
                row.tcpPort = pResult->getInt(++i);
                row.udpPort = pResult->getInt(++i);
                row.groupID = pResult->getInt(++i);
                row.stat = pResult->getInt(++i);
                row.worldID = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SharedResurrectLocationRow> loadResurrectLocations() {
        vector<SharedResurrectLocationRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ZoneID, SResurrectZoneID, SResurrectX, SResurrectY, VResurrectZoneID, "
                                    "VResurrectX, VResurrectY FROM ZoneInfo");

            while (pResult->next()) {
                SharedResurrectLocationRow row;
                row.zoneID = pResult->getInt(1);
                row.slayerZoneID = pResult->getInt(2);
                row.slayerX = pResult->getInt(3);
                row.slayerY = pResult->getInt(4);
                row.vampireZoneID = pResult->getInt(5);
                row.vampireX = pResult->getInt(6);
                row.vampireY = pResult->getInt(7);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SharedStringRow> loadStrings() {
        vector<SharedStringRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, String FROM SSStringPool");

            while (pResult->next()) {
                int i = 0;
                SharedStringRow row;
                row.id = pResult->getInt(++i);
                row.text = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

SharedConfigRepository& defaultSharedConfigRepository() {
    static MySQLSharedConfigRepository instance;
    return instance;
}
