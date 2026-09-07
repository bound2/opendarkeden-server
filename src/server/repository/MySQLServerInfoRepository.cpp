#include "database/DB.h"
#include "repository/ServerInfoRepository.h"

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

// MySQL implementation of ServerInfoRepository (see the header for the
// two connections).
class MySQLServerInfoRepository : public ServerInfoRepository {
public:
    bool loadMaxServerGroupID(int& maxGroupID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString("SELECT MAX(GroupID) FROM GameServerInfo");

            found = readMax(pResult, maxGroupID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadMaxWorldID(int& maxWorldID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString("SELECT MAX(WorldID) FROM GameServerInfo");

            found = readMax(pResult, maxWorldID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<ServerInfoRow> loadServers() {
        vector<ServerInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString(
                "SELECT ServerID, Nickname , IP , TCPPort , UDPPort, WorldID, GroupID, Stat FROM GameServerInfo");

            while (pResult->next()) {
                ServerInfoRow row;
                row.serverID = pResult->getInt(1);
                row.nickname = pResult->getString(2);
                row.ip = pResult->getString(3);
                row.tcpPort = pResult->getInt(4);
                row.udpPort = pResult->getInt(5);
                row.worldID = pResult->getInt(6);
                row.groupID = pResult->getInt(7);
                row.stat = pResult->getInt(8);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ServerInfoNonPKRow> loadNonPKServers() {
        vector<ServerInfoNonPKRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString("SELECT WorldID, ServerGroupID FROM NonPKServerList");

            while (pResult->next()) {
                ServerInfoNonPKRow row;
                row.worldID = pResult->getInt(1);
                row.serverGroupID = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ServerInfoCastleStatRow> loadCastleStats() {
        vector<ServerInfoCastleStatRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQueryString("SELECT WorldID, ServerGroupID, FollowServerID FROM CastleStatInfo");

            while (pResult->next()) {
                ServerInfoCastleStatRow row;
                row.worldID = pResult->getInt(1);
                row.serverGroupID = pResult->getInt(2);
                row.followServerID = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ServerInfoWorldRow> loadWorlds() {
        vector<ServerInfoWorldRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString("SELECT ID, Name, Stat FROM WorldInfo");

            while (pResult->next()) {
                ServerInfoWorldRow row;
                row.id = pResult->getInt(1);
                row.name = pResult->getString(2);
                row.stat = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

ServerInfoRepository& defaultServerInfoRepository() {
    static MySQLServerInfoRepository instance;
    return instance;
}
