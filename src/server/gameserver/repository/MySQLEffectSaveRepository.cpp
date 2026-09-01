#include "DB.h"
#include "repository/EffectSaveRepository.h"

namespace {

// MySQL implementation of the persisted-effect seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte what its Effect*.cpp emitted, and
//    the eight classes did NOT agree on spacing, so the format strings
//    are per-table data rather than one template: "VALUES('%s', ..." vs
//    the "VALUES ('%s', ..." of CanEnterGDRLair and EnemyErase;
//    EffectMute's "YearTime=%ld" and "OwnerID='%s'" where the others
//    space around "="; the force scrolls' "(OwnerID, RemainTime )" and
//    "VALUES('%s',%lu)"; and the force-scroll loads' "SELECt" (a typo
//    MySQL accepts — keywords are case-insensitive).
//  - The varargs conversions are the originals': the Turn_t (DWORD)
//    year time and remain turn through %ld/%lu — the 4-byte-through-
//    8-byte-conversion latent bug documented in
//    MySQLCharacterRepository.cpp; every call in this seam has at most
//    four varargs, so they are all register-passed and GCC's
//    zero-extension keeps it benign — and the time_t DayTime through
//    %ld, which is exact. Preserved, not fixed. A DWORD can never print
//    more than 4294967295 this way, which is exactly the int(10)
//    unsigned columns' maximum: no value from this seam is ever clamped.
//  - The remain turn the force scrolls store is computed from
//    timediff(m_Deadline, now), and Timeval.cpp's timediff returns the
//    ABSOLUTE difference. A scroll saved after its deadline has passed
//    therefore stores the time elapsed SINCE expiry as its "remaining"
//    time, and the loader would resurrect it with that much left. The
//    save paths run only while the effect is alive, so this is not
//    reached in practice; documented, not pinned.
//  - All eight tables are keyless (OwnerID index only) EXCEPT
//    EffectKillAftermath, whose OwnerID is the PRIMARY KEY: its insert
//    raises ER_DUP_ENTRY (1062) for an owner that already has a row,
//    where the other seven silently accumulate a duplicate. Pinned by
//    the integration tier.
//  - EnemyErase keys its DELETE on (OwnerID, EnemyName) but its UPDATE on
//    OwnerID alone: a save() of one enemy-erase effect rewrites EVERY
//    EnemyErase row the owner has to that effect's enemy. Pinned.
//  - The loads have no ORDER BY; the deadline/enemy loaders consume every
//    row and the remain loaders only the first, so order is immaterial
//    except for which duplicate a remain load returns — the optimizer's
//    choice (see MySQLSkillSaveRepository.cpp), not a contract.
//  - An UPDATE/DELETE with no matching row is a silent no-op; owner and
//    enemy names are interpolated raw. As before.
struct DeadlineSpec {
    const char* insert;
    const char* remove;
    const char* update;
    const char* select;
};

const DeadlineSpec DEADLINE_SPECS[DEADLINE_EFFECT_TABLE_MAX] = {
    // EFFECT_TABLE_AFTERMATH
    {"INSERT INTO EffectAftermath (OwnerID , YearTime, DayTime) VALUES('%s', %ld, %ld)",
     "DELETE FROM EffectAftermath WHERE OwnerID = '%s'",
     "UPDATE EffectAftermath SET YearTime = %ld, DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM EffectAftermath WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_KILL_AFTERMATH
    {"INSERT INTO EffectKillAftermath (OwnerID , YearTime, DayTime) VALUES('%s', %ld, %ld)",
     "DELETE FROM EffectKillAftermath WHERE OwnerID = '%s'",
     "UPDATE EffectKillAftermath SET YearTime = %ld, DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM EffectKillAftermath WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_MUTE
    {"INSERT INTO EffectMute (OwnerID , YearTime, DayTime) VALUES('%s', %ld, %ld)",
     "DELETE FROM EffectMute WHERE OwnerID = '%s'",
     "UPDATE EffectMute SET YearTime=%ld, DayTime=%ld WHERE OwnerID='%s'",
     "SELECT DayTime FROM EffectMute WHERE OwnerID='%s'"},
    // EFFECT_TABLE_CAN_ENTER_GDR_LAIR
    {"INSERT INTO CanEnterGDRLair (OwnerID , YearTime, DayTime) VALUES ('%s', %ld, %ld)",
     "DELETE FROM CanEnterGDRLair WHERE OwnerID = '%s'",
     "UPDATE CanEnterGDRLair SET YearTime = %ld, DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM CanEnterGDRLair WHERE OwnerID = '%s'"},
};

const DeadlineSpec REMAIN_SPECS[REMAIN_EFFECT_TABLE_MAX] = {
    // EFFECT_TABLE_SAFE_FORCE_SCROLL
    {"INSERT INTO EffectSafeForceScroll (OwnerID, RemainTime ) VALUES('%s',%lu)",
     "DELETE FROM EffectSafeForceScroll WHERE OwnerID = '%s'",
     "UPDATE EffectSafeForceScroll SET RemainTime = %lu WHERE OwnerID = '%s'",
     "SELECt RemainTime FROM EffectSafeForceScroll WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_BEHEMOTH_FORCE_SCROLL
    {"INSERT INTO EffectBehemothForceScroll (OwnerID, RemainTime ) VALUES('%s',%lu)",
     "DELETE FROM EffectBehemothForceScroll WHERE OwnerID = '%s'",
     "UPDATE EffectBehemothForceScroll SET RemainTime = %lu WHERE OwnerID = '%s'",
     "SELECt RemainTime FROM EffectBehemothForceScroll WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_CARNELIAN_FORCE_SCROLL
    {"INSERT INTO EffectCarnelianForceScroll (OwnerID, RemainTime ) VALUES('%s',%lu)",
     "DELETE FROM EffectCarnelianForceScroll WHERE OwnerID = '%s'",
     "UPDATE EffectCarnelianForceScroll SET RemainTime = %lu WHERE OwnerID = '%s'",
     "SELECt RemainTime FROM EffectCarnelianForceScroll WHERE OwnerID = '%s'"},
};

class MySQLEffectSaveRepository : public EffectSaveRepository {
public:
    void insertDeadline(DeadlineEffectTable table, const string& ownerName, Turn_t yearTime, time_t dayTime) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(DEADLINE_SPECS[table].insert, ownerName.c_str(), yearTime, dayTime);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteDeadline(DeadlineEffectTable table, const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(DEADLINE_SPECS[table].remove, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateDeadline(DeadlineEffectTable table, const string& ownerName, Turn_t yearTime, time_t dayTime) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(DEADLINE_SPECS[table].update, yearTime, dayTime, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<DWORD> loadDeadlines(DeadlineEffectTable table, const string& ownerName) {
        vector<DWORD> dayTimes;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(DEADLINE_SPECS[table].select, ownerName.c_str());

            while (pResult->next())
                dayTimes.push_back(pResult->getDWORD(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return dayTimes;
    }

    void insertRemain(RemainEffectTable table, const string& ownerName, Turn_t remainTurn) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(REMAIN_SPECS[table].insert, ownerName.c_str(), remainTurn);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteRemain(RemainEffectTable table, const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(REMAIN_SPECS[table].remove, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateRemain(RemainEffectTable table, const string& ownerName, Turn_t remainTurn) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(REMAIN_SPECS[table].update, remainTurn, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadRemain(RemainEffectTable table, const string& ownerName, DWORD& remainTurn) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(REMAIN_SPECS[table].select, ownerName.c_str());

            if (pResult->next()) {
                remainTurn = pResult->getDWORD(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertEnemyErase(const string& ownerName, Turn_t yearTime, time_t dayTime, const string& enemyName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO EnemyErase (OwnerID , YearTime, DayTime, EnemyName) VALUES ('%s', %ld, %ld, '%s')",
                ownerName.c_str(), yearTime, dayTime, enemyName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteEnemyErase(const string& ownerName, const string& enemyName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM EnemyErase WHERE OwnerID = '%s' AND EnemyName = '%s'", ownerName.c_str(),
                                enemyName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateEnemyErase(const string& ownerName, Turn_t yearTime, time_t dayTime, const string& enemyName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            // keyed on OwnerID alone — rewrites every EnemyErase row the
            // owner has (see the quirk notes above)
            pStmt->executeQuery(
                "UPDATE EnemyErase SET YearTime = %ld, DayTime = %ld, EnemyName = '%s' WHERE OwnerID = '%s'", yearTime,
                dayTime, enemyName.c_str(), ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<EnemyEraseRow> loadEnemyErases(const string& ownerName) {
        vector<EnemyEraseRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT DayTime, EnemyName FROM EnemyErase WHERE OwnerID = '%s'",
                                                  ownerName.c_str());

            while (pResult->next()) {
                EnemyEraseRow row;
                row.dayTime = pResult->getDWORD(1);
                row.enemyName = pResult->getString(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

EffectSaveRepository& defaultEffectSaveRepository() {
    static MySQLEffectSaveRepository instance;
    return instance;
}
