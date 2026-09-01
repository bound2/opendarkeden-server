#include "DB.h"
#include "repository/BulletinBoardRepository.h"

namespace {

// MySQL implementation of the bulletin-board seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The SQL is byte-for-byte the ZoneUtil.cpp originals. The INSERT
//    lists no columns — "VALUES (0, ...)" — so it depends on the
//    table's column ORDER (ID, ServerID, ZoneID, X, Y, Message, Type,
//    TimeLimit) and on the literal 0 letting the auto-increment ID
//    assign itself. Kept verbatim.
//  - The varargs conversions are the originals': the config's int
//    ServerID, the ZoneID_t and the TPOINT ints through %u (each
//    promotes to int/unsigned with the same bytes), the (uint)-cast
//    monster type through %u, the datetime text through '%s'.
//  - insert() returns getAffectedRowCount() so the caller can keep its
//    "0 rows" log line; nothing checked it beyond that.
//  - loadBulletinBoard's expired-row DELETE used a SECOND statement it
//    never freed (a leak per expired notice); remove() frees it — fixed
//    knowingly, as in earlier rounds.
//  - Message text arrives already run through Guild::correctString by
//    the caller; the seam interpolates it as it was handed over.
class MySQLBulletinBoardRepository : public BulletinBoardRepository {
public:
    int insert(int serverID, ZoneID_t zoneID, int x, int y, const string& message, uint type, const string& timeLimit) {
        int affected = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO BulletinBoardObject VALUES (0, %u, %u, %u, %u, '%s', %u, '%s')", serverID,
                                zoneID, x, y, message.c_str(), type, timeLimit.c_str());

            // UPDATE인 경우는 Result* 대신에.. pStmt->getAffectedRowCount()
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
