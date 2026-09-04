#include "DB.h"
#include "repository/MofusPointRepository.h"

namespace {

// MySQL implementation of the mofus point seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, including the
//    mixed-case "Update" and "Insert Into" keywords, the unspaced
//    "OwnerID='%s'" against the spaced "VALUES ('%s', now(), %u, %u)",
//    and the positional insert "Values ('%s',%d)" that names no columns.
//  - recvPoint and savePoint are ints marshalled through "%u". The
//    columns are smallint(5) SIGNED, so a negative point count formats
//    as a large unsigned number and then overflows the column; under
//    this project's non-strict sql_mode that clamps to 32767 with a
//    warning rather than failing. Kept as the call site wrote it.
//  - OwnerID is a CHARACTER NAME (see the header; an earlier version of
//    this note said account id), and it is interpolated raw, as before.
//  - THE INSERT CAN FAIL ON AN ORDINARY PATH, which matters because
//    END_DB writes DBError.log every time it does. A save of ZERO
//    points is reachable — PKTPowerPointHandler clamps with min(points,
//    40) and points can be 0 — and the driver connects without
//    CLIENT_FOUND_ROWS, so mysql_affected_rows reports rows CHANGED.
//    "Point = Point + 0" changes nothing, the caller reads that as "no
//    row" and inserts, and OwnerID is the PRIMARY KEY, so the insert
//    raises on a duplicate key. That throw is pre-existing and
//    preserved; the per-occurrence log append is new.
//  - THE STATEMENT LEAKS ON A NON-SQLQueryException PATH. SAFE_DELETE
//    sits inside the try, so success and SQLQueryException free it and
//    nothing else does. The inline code had a catch (...) that freed it
//    unconditionally; that arm was removed from the call site and its
//    cleanup was not reproduced here. Every other MySQL*Repository in
//    the tree has the same shape, and the candidates — the 2048-byte
//    statement guard, getField before next(), bad_alloc — are
//    unreachable for these four short statements. Recorded rather than
//    fixed, because fixing it belongs in every seam at once.
class MySQLMofusPointRepository : public MofusPointRepository {
public:
    bool loadPowerPoint(const string& ownerID, int& point) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Point FROM MofusPowerPoint WHERE OwnerID='%s'", ownerID.c_str());

            if (pResult->next()) {
                point = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool increasePowerPoint(const string& ownerID, int amount) {
        bool affected = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("Update MofusPowerPoint SET Point = Point + %d WHERE OwnerID='%s'", amount,
                                ownerID.c_str());

            affected = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return affected;
    }

    void insertPowerPoint(const string& ownerID, int amount) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("Insert Into MofusPowerPoint Values ('%s',%d)", ownerID.c_str(), amount);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void logPowerPoint(const string& ownerID, int recvPoint, int savePoint) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO MofusLog (OwnerID, SaveTime, RecvPoint, SavePoint) VALUES ('%s', now(), %u, %u)",
                ownerID.c_str(), recvPoint, savePoint);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

MofusPointRepository& defaultMofusPointRepository() {
    static MySQLMofusPointRepository instance;
    return instance;
}
