#include "DB.h"
#include "repository/EffectSaveRepository.h"

namespace {

// MySQL implementation of EffectSaveRepository.
//  - The format strings are per-table data rather than one template
//    because the tables' spellings differ (spacing, "SELECt" in the
//    force-scroll loads — keywords are case-insensitive).
//  - The Turn_t (DWORD) year time and remain turn go through %u, the
//    time_t DayTime through %ld. A DWORD prints at most 4294967295,
//    exactly the int(10) unsigned columns' maximum, so no value is ever
//    clamped.
//  - The remain turn the force scrolls store is computed from
//    timediff(m_Deadline, now), and Timeval.cpp's timediff returns the
//    ABSOLUTE difference. A scroll saved after its deadline has passed
//    would store the time elapsed SINCE expiry as its "remaining" time;
//    the save paths run only while the effect is alive, so this is not
//    reached in practice.
//  - All eight tables are keyless (OwnerID index only) EXCEPT
//    EffectKillAftermath, whose OwnerID is the PRIMARY KEY: its insert
//    raises ER_DUP_ENTRY (1062) for an owner that already has a row,
//    where the other seven silently accumulate a duplicate.
//  - EnemyErase keys its DELETE on (OwnerID, EnemyName) but its UPDATE on
//    OwnerID alone: a save() of one enemy-erase effect rewrites EVERY
//    EnemyErase row the owner has to that effect's enemy.
//  - The loads have no ORDER BY; the deadline/enemy loaders consume every
//    row and the remain loaders only the first, so which duplicate a
//    remain load returns is the optimizer's choice.
//  - An UPDATE/DELETE with no matching row is a silent no-op; owner and
//    enemy names are interpolated raw.
//  - Every statement goes through executeQuery's 2048-byte format
//    buffer; OwnerID is a varchar(10) and the widest statement renders
//    under 200 bytes, so the cap is unreachable.
struct DeadlineSpec {
    const char* insert;
    const char* remove;
    const char* update;
    const char* select;
};

const DeadlineSpec DEADLINE_SPECS[DEADLINE_EFFECT_TABLE_MAX] = {
    // EFFECT_TABLE_AFTERMATH
    {"INSERT INTO EffectAftermath (OwnerID , YearTime, DayTime) VALUES('%s', %u, %ld)",
     "DELETE FROM EffectAftermath WHERE OwnerID = '%s'",
     "UPDATE EffectAftermath SET YearTime = %u, DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM EffectAftermath WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_KILL_AFTERMATH
    {"INSERT INTO EffectKillAftermath (OwnerID , YearTime, DayTime) VALUES('%s', %u, %ld)",
     "DELETE FROM EffectKillAftermath WHERE OwnerID = '%s'",
     "UPDATE EffectKillAftermath SET YearTime = %u, DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM EffectKillAftermath WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_MUTE
    {"INSERT INTO EffectMute (OwnerID , YearTime, DayTime) VALUES('%s', %u, %ld)",
     "DELETE FROM EffectMute WHERE OwnerID = '%s'", "UPDATE EffectMute SET YearTime=%u, DayTime=%ld WHERE OwnerID='%s'",
     "SELECT DayTime FROM EffectMute WHERE OwnerID='%s'"},
    // EFFECT_TABLE_CAN_ENTER_GDR_LAIR
    {"INSERT INTO CanEnterGDRLair (OwnerID , YearTime, DayTime) VALUES ('%s', %u, %ld)",
     "DELETE FROM CanEnterGDRLair WHERE OwnerID = '%s'",
     "UPDATE CanEnterGDRLair SET YearTime = %u, DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM CanEnterGDRLair WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_RESTORE: the same three columns, spaced its own way
    // (" VALUES('%s' , " and "YearTime = %u,DayTime").
    {"INSERT INTO EffectRestore (OwnerID, YearTime, DayTime) VALUES('%s' , %u , %ld)",
     "DELETE FROM EffectRestore WHERE OwnerID = '%s'",
     "UPDATE EffectRestore SET YearTime = %u,DayTime = %ld WHERE OwnerID = '%s'",
     "SELECT DayTime FROM EffectRestore WHERE OwnerID = '%s'"},
};

const DeadlineSpec REMAIN_SPECS[REMAIN_EFFECT_TABLE_MAX] = {
    // EFFECT_TABLE_SAFE_FORCE_SCROLL
    {"INSERT INTO EffectSafeForceScroll (OwnerID, RemainTime ) VALUES('%s',%u)",
     "DELETE FROM EffectSafeForceScroll WHERE OwnerID = '%s'",
     "UPDATE EffectSafeForceScroll SET RemainTime = %u WHERE OwnerID = '%s'",
     "SELECt RemainTime FROM EffectSafeForceScroll WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_BEHEMOTH_FORCE_SCROLL
    {"INSERT INTO EffectBehemothForceScroll (OwnerID, RemainTime ) VALUES('%s',%u)",
     "DELETE FROM EffectBehemothForceScroll WHERE OwnerID = '%s'",
     "UPDATE EffectBehemothForceScroll SET RemainTime = %u WHERE OwnerID = '%s'",
     "SELECt RemainTime FROM EffectBehemothForceScroll WHERE OwnerID = '%s'"},
    // EFFECT_TABLE_CARNELIAN_FORCE_SCROLL
    {"INSERT INTO EffectCarnelianForceScroll (OwnerID, RemainTime ) VALUES('%s',%u)",
     "DELETE FROM EffectCarnelianForceScroll WHERE OwnerID = '%s'",
     "UPDATE EffectCarnelianForceScroll SET RemainTime = %u WHERE OwnerID = '%s'",
     "SELECt RemainTime FROM EffectCarnelianForceScroll WHERE OwnerID = '%s'"},
};


// The per-creature tables. shape says which varargs the writes pass and
// which columns the read fetches; the four literals below are their
// classes' own, spacing quirks included.
enum CreatureEffectShape {
    SHAPE_LEVEL,      // EffectBloodDrain: (DayTime, Level), Level via getBYTE
    SHAPE_OLD_SIGHT,  // EffectFlare, EffectLight: (YearTime, DayTime, OldSight)
    SHAPE_LEVEL_SIGHT // EffectYellowPoisonToCreature: all four, Level via getInt
};

struct CreatureEffectSpec {
    const char* insert;
    const char* remove;
    const char* update;
    const char* select;
    CreatureEffectShape shape;
};

const CreatureEffectSpec CREATURE_EFFECT_SPECS[CREATURE_EFFECT_TABLE_MAX] = {
    // CREATURE_EFFECT_BLOOD_DRAIN: the only one of the four already
    // parameterized in its class, so these four literals are copied from
    // its live executeQuery calls rather than rebuilt from a StringStream.
    {"INSERT INTO EffectBloodDrain (OwnerID , YearTime, DayTime, Level) VALUES('%s', %u, %ld, %d)",
     "DELETE FROM EffectBloodDrain WHERE OwnerID = '%s'",
     "UPDATE EffectBloodDrain SET YearTime=%u, DayTime=%ld, Level=%d WHERE OwnerID='%s'",
     "SELECT DayTime, Level FROM EffectBloodDrain WHERE OwnerID='%s'", SHAPE_LEVEL},
    // CREATURE_EFFECT_FLARE
    {"INSERT INTO EffectFlare(OwnerID , YearTime, DayTime, OldSight) VALUES('%s' , %u , %ld,%d)",
     "DELETE FROM EffectFlare WHERE OwnerID = '%s'",
     "UPDATE EffectFlare SET YearTime = %u, DayTime = %ld, OldSight = %d WHERE OwnerID = '%s'",
     "SELECT YearTime, DayTime, OldSight FROM EffectFlare WHERE OwnerID = '%s'", SHAPE_OLD_SIGHT},
    // CREATURE_EFFECT_LIGHT: EffectFlare's four statements with the table
    // name swapped — the two classes' StringStream chains are identical
    // token for token.
    {"INSERT INTO EffectLight(OwnerID , YearTime, DayTime, OldSight) VALUES('%s' , %u , %ld,%d)",
     "DELETE FROM EffectLight WHERE OwnerID = '%s'",
     "UPDATE EffectLight SET YearTime = %u, DayTime = %ld, OldSight = %d WHERE OwnerID = '%s'",
     "SELECT YearTime, DayTime, OldSight FROM EffectLight WHERE OwnerID = '%s'", SHAPE_OLD_SIGHT},
    // CREATURE_EFFECT_YELLOW_POISON_TO_CREATURE: note the INSERT's single
    // " , " after the owner and bare "," between the three numbers, where
    // EffectFlare's spaces both separators.
    {"INSERT INTO EffectYellowPoisonToCreature(OwnerID , YearTime, DayTime, Level, OldSight) VALUES('%s' , "
     "%u,%ld,%d,%d)",
     "DELETE FROM EffectYellowPoisonToCreature WHERE OwnerID = '%s'",
     "UPDATE EffectYellowPoisonToCreature SET YearTime = %u, DayTime = %ld, Level = %d, OldSight = %d WHERE "
     "OwnerID = '%s'",
     "SELECT YearTime, DayTime, Level, OldSight FROM EffectYellowPoisonToCreature WHERE OwnerID = '%s'",
     SHAPE_LEVEL_SIGHT},
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
                "INSERT INTO EnemyErase (OwnerID , YearTime, DayTime, EnemyName) VALUES ('%s', %u, %ld, '%s')",
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
                "UPDATE EnemyErase SET YearTime = %u, DayTime = %ld, EnemyName = '%s' WHERE OwnerID = '%s'", yearTime,
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

    void insertCreatureEffect(CreatureEffectTable table, const string& ownerName, Turn_t yearTime, time_t dayTime,
                              int level, int oldSight) {
        const CreatureEffectSpec& spec = CREATURE_EFFECT_SPECS[table];
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            switch (spec.shape) {
            case SHAPE_LEVEL:
                pStmt->executeQuery(spec.insert, ownerName.c_str(), yearTime, dayTime, level);
                break;
            case SHAPE_OLD_SIGHT:
                pStmt->executeQuery(spec.insert, ownerName.c_str(), yearTime, dayTime, oldSight);
                break;
            case SHAPE_LEVEL_SIGHT:
                pStmt->executeQuery(spec.insert, ownerName.c_str(), yearTime, dayTime, level, oldSight);
                break;
            }
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteCreatureEffect(CreatureEffectTable table, const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(CREATURE_EFFECT_SPECS[table].remove, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateCreatureEffect(CreatureEffectTable table, const string& ownerName, Turn_t yearTime, time_t dayTime,
                              int level, int oldSight) {
        const CreatureEffectSpec& spec = CREATURE_EFFECT_SPECS[table];
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            switch (spec.shape) {
            case SHAPE_LEVEL:
                pStmt->executeQuery(spec.update, yearTime, dayTime, level, ownerName.c_str());
                break;
            case SHAPE_OLD_SIGHT:
                pStmt->executeQuery(spec.update, yearTime, dayTime, oldSight, ownerName.c_str());
                break;
            case SHAPE_LEVEL_SIGHT:
                pStmt->executeQuery(spec.update, yearTime, dayTime, level, oldSight, ownerName.c_str());
                break;
            }
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<CreatureEffectRow> loadCreatureEffects(CreatureEffectTable table, const string& ownerName) {
        const CreatureEffectSpec& spec = CREATURE_EFFECT_SPECS[table];
        vector<CreatureEffectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec.select, ownerName.c_str());

            while (pResult->next()) {
                CreatureEffectRow row;
                row.yearTime = 0;
                row.dayTime = 0;
                row.level = 0;

                switch (spec.shape) {
                case SHAPE_LEVEL:
                    row.dayTime = pResult->getDWORD(1);
                    row.level = pResult->getBYTE(2);
                    break;
                case SHAPE_OLD_SIGHT:
                    row.yearTime = pResult->getDWORD(1);
                    row.dayTime = pResult->getDWORD(2);
                    // OldSight is column 3 and no loader ever read it.
                    break;
                case SHAPE_LEVEL_SIGHT:
                    row.yearTime = pResult->getDWORD(1);
                    row.dayTime = pResult->getDWORD(2);
                    row.level = pResult->getInt(3);
                    // OldSight is column 4 and its loader never read it.
                    break;
                }

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
