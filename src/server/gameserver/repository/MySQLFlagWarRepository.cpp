#include "DB.h"
#include "repository/FlagWarRepository.h"

namespace {

// MySQL implementation of the capture-the-flag seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, including the
//    "Race-1" projection (FlagPolePosition.Race is an ENUM, so the
//    arithmetic yields its 1-based index less one), the unspaced
//    "VALUES ('%s','%s',%d,%d,%d)" lists, and the GROUP BY on
//    "Name, ServerID" while PlayerID and Race are selected bare.
//  - THAT GROUP BY IS BROKEN under the sql_mode this project requires.
//    CLAUDE.md's setting removes NO_ZERO_DATE and STRICT_TRANS_TABLES
//    but KEEPS ONLY_FULL_GROUP_BY, and PlayerID is not functionally
//    dependent on (Name, ServerID) — FlagWarStat has no unique key at
//    all — so MySQL raises error 1055 every time the statement is
//    called. It is called only when the flag war is switched on, which
//    ActiveFlagWar : 0 in both shipped configs prevents; and when it is
//    called, the const char* END_DB rethrows escapes every
//    Throwable-only catch up to main.cpp's catch (...), so the process
//    exits. The statement is kept
//    byte-for-byte anyway, because task 3.2 moves statements without
//    fixing them; the integration tier pins the throw so the bug is
//    recorded rather than rediscovered. Fixing it (adding PlayerID and
//    Race to the GROUP BY, or wrapping them in ANY_VALUE) changes
//    behaviour and belongs to its own round.
//  - ItemID reaches its two statements through "%d" although the column
//    is bigint(20) unsigned and ItemID_t is a DWORD, exactly as the call
//    sites always wrote it. An id above INT_MAX formats negative: the
//    SELECT then matches nothing, and the INSERT writes ItemID = 0,
//    clamped, because this project's sql_mode drops
//    STRICT_TRANS_TABLES. Unreachable in practice — ids step by the
//    server count from a per-class MAX(ItemID), so 2^31 rows in one
//    item table would be needed — and the inline behaviour either way.
//  - Names, player ids and the FlagWarID date text are interpolated raw
//    (no escaping), as before.
class MySQLFlagWarRepository : public FlagWarRepository {
public:
    vector<FlagPoleRow> loadFlagPoles() {
        vector<FlagPoleRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ZoneID, CenterX, CenterY, Width, Height, Race-1, MonsterType FROM FlagPolePosition");

            while (pResult->next()) {
                FlagPoleRow row;
                row.zoneID = pResult->getInt(1);
                row.centerX = pResult->getInt(2);
                row.centerY = pResult->getInt(3);
                row.width = pResult->getInt(4);
                row.height = pResult->getInt(5);
                row.race = pResult->getInt(6);
                row.monsterType = pResult->getInt(7);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void deleteAllFlagWarStats() {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM FlagWarStat");

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool flagStatExists(const string& name, ItemID_t itemID) {
        bool exists = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Name FROM FlagWarStat WHERE Name = '%s' AND ItemID = %d",
                                                  name.c_str(), itemID);

            exists = pResult->next();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return exists;
    }

    void insertFlagStat(const string& playerID, const string& name, int race, int serverID, ItemID_t itemID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO FlagWarStat (PlayerID, Name, Race, ServerID, ItemID) VALUES ('%s','%s',%d,%d,%d)",
                playerID.c_str(), name.c_str(), race, serverID, itemID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<FlagWarStatTotalRow> loadFlagWarStatTotals() {
        vector<FlagWarStatTotalRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PlayerID, Name, Race, ServerID, count(*) FROM FlagWarStat GROUP BY Name, ServerID");

            while (pResult->next()) {
                FlagWarStatTotalRow row;
                row.playerID = pResult->getString(1);
                row.name = pResult->getString(2);
                row.race = pResult->getInt(3);
                row.serverID = pResult->getInt(4);
                row.flagNum = pResult->getInt(5);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void insertFlagWarHistory(const string& flagWarID, const string& playerID, const string& name, int race,
                              int serverID, int flagNum) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO FlagWarHistory (FlagWarID, PlayerID, Name, Race, ServerID, FlagNum) "
                                "VALUES ('%s','%s','%s',%d,%d,%d)",
                                flagWarID.c_str(), playerID.c_str(), name.c_str(), race, serverID, flagNum);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

FlagWarRepository& defaultFlagWarRepository() {
    static MySQLFlagWarRepository instance;
    return instance;
}
