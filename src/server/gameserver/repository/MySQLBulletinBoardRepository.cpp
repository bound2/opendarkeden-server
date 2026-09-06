#include "DB.h"
#include "repository/BulletinBoardRepository.h"

namespace {

// MySQL implementation of BulletinBoardRepository. Quirks:
//  - The INSERT lists no columns — "VALUES (0, ...)" — so it depends on
//    the table's column ORDER (ID, ServerID, ZoneID, X, Y, Message, Type,
//    TimeLimit) and on the literal 0 letting the auto-increment ID
//    assign itself.
//  - insert() returns getAffectedRowCount() for the caller's "0 rows"
//    log line; nothing else reads it.
//  - Message text arrives already run through ZoneUtil.cpp's free
//    function correctString by the caller and is interpolated as handed
//    over.
class MySQLBulletinBoardRepository : public BulletinBoardRepository {
public:
    uint insert(int serverID, ZoneID_t zoneID, int x, int y, const string& message, uint type,
                const string& timeLimit) {
        uint affected = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO BulletinBoardObject VALUES (0, %u, %u, %u, %u, '%s', %u, '%s')", serverID,
                                zoneID, x, y, message.c_str(), type, timeLimit.c_str());

            // an UPDATE/INSERT has no Result; the affected-row count is the answer
            affected = pStmt->getAffectedRowCount();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return affected;
    }

    vector<BulletinBoardRow> loadForZone(int serverID, ZoneID_t zoneID) {
        vector<BulletinBoardRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, X, Y, Message, Type, TimeLimit FROM BulletinBoardObject "
                                                  "WHERE ServerID = %u AND ZoneID = %u",
                                                  serverID, zoneID);

            while (pResult->next()) {
                BulletinBoardRow row;
                row.id = pResult->getInt(1);
                row.x = pResult->getInt(2);
                row.y = pResult->getInt(3);
                row.message = pResult->getString(4);
                row.type = pResult->getInt(5);
                row.timeLimit = pResult->getString(6);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void remove(uint id) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM BulletinBoardObject WHERE ID = %u", id);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

BulletinBoardRepository& defaultBulletinBoardRepository() {
    static MySQLBulletinBoardRepository instance;
    return instance;
}
