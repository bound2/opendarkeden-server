#include "DB.h"
#include "repository/GoodsRepository.h"

namespace {

// MySQL implementation of the GoodsListObject persistence seam. The legacy
// schema quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - getDistConnection("PLAYER_DB") IGNORES its name argument
//    (DatabaseManager.cpp: a bare per-thread lookup, name-based routing
//    commented out): it returns the thread's second connection, built
//    from the UI_DB_* config keys with the schema hard-coded to
//    "DARKEDEN" (ZoneGroupThread.cpp), and silently falls back to the
//    world default connection on a thread that never registered one. In
//    the shipped stack both connections point at the same server and
//    schema — the "dist" connection is just a second socket.
//  - Status is an enum('NOT','GET'): 'NOT' = still waiting for pickup.
//  - takeOne()'s single UPDATE decrements Num and sets Status in one
//    statement, relying on MySQL's non-standard left-to-right SET
//    evaluation: the IF() reads the ALREADY-DECREMENTED Num, so a row at
//    Num=1 flips to 'GET' in the same statement that takes its last unit.
//  - A pending row at Num=0 is reachable (the loader delivers one item
//    for it: its loop runs max(1, min(50, num)) times) and taking it
//    FAILS: Num - 1 on the UNSIGNED column raises ER_DATA_OUT_OF_RANGE
//    (1690) — an expression error independent of strict mode — leaving
//    the row untouched. The SQLQueryException escapes through END_DB
//    (as a const char*), GoodsInventory::popItem never erases the entry,
//    and the purchase is re-delivered on the next load: a pre-existing
//    stuck-item bug this seam documents rather than silently fixes.
//  - The row id is a bigint but is carried and interpolated as a string,
//    unquoted (%s straight into the numeric comparison), exactly as the
//    call site always did.
//  - PlayerID and character names are interpolated raw (no escaping), as
//    the call site always did.
class MySQLGoodsRepository : public GoodsRepository {
public:
    vector<GoodsRecord> loadPending(int world, const string& playerID, const string& characterName) {
        vector<GoodsRecord> records;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();

            Result* pResult =
                pStmt->executeQuery("SELECT ID, GoodsID, Num FROM GoodsListObject WHERE World = %d AND PlayerID = '%s' "
                                    "AND Name = '%s' AND Status = 'NOT'",
                                    world, playerID.c_str(), characterName.c_str());

            while (pResult->next()) {
                GoodsRecord record;
                record.id = pResult->getString(1);
                record.goodsID = pResult->getInt(2);
                record.num = pResult->getInt(3);
                records.push_back(record);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return records;
    }

    bool takeOne(const string& id) {
        bool updated = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE GoodsListObject SET Num = Num - 1, Status = IF( NUM < 1, 'GET', 'NOT' ) "
                                "WHERE ID=%s",
                                id.c_str());

            updated = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return updated;
    }
};

} // namespace

GoodsRepository& defaultGoodsRepository() {
    static MySQLGoodsRepository instance;
    return instance;
}
