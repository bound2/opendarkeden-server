#include "DB.h"
#include "repository/SkillSaveRepository.h"

namespace {

// MySQL implementation of SkillSaveRepository.
//  - The ousters INSERT names SkillLevel LAST (after NextTime) while the
//    other two tables' inserts follow the schema order.
//  - The tables are keyless (a non-unique (OwnerID, SkillType) index):
//    an INSERT never conflicts, an UPDATE or DELETE touches every row of
//    that type. The loads have no ORDER BY, so their row order is the
//    optimizer's choice: a near-empty table scans in insertion order, a
//    production-sized one may take the index and come back
//    SkillType-ascending. The vampire/ousters loaders keep the first row
//    of a duplicated type, so which duplicate wins is plan-dependent too.
//  - The DWORD-typed exp and delay members (Exp_t, Turn_t) go through %d:
//    a value at or above 2^31 would print negative and be clamped to 0 by
//    the UNSIGNED column under the non-strict sql_mode (unreachable for
//    skill data in practice). The WORD-typed type and level members
//    promote to int, so %d is exact for them; the time_t NextTime goes
//    through %ld.
//  - An UPDATE/DELETE for a row that does not exist matches zero rows,
//    silently; nothing checks it.
//  - Owner names are interpolated raw (no escaping).
class MySQLSkillSaveRepository : public SkillSaveRepository {
public:
    vector<SlayerSkillRow> loadSlayerSkills(const string& ownerName) {
        vector<SlayerSkillRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT SkillType, SkillLevel, SkillExp, Delay, CastingTime, NextTime FROM SkillSave WHERE OwnerID = "
                "'%s'",
                ownerName.c_str());

            while (pResult->next()) {
                int i = 0;
                SlayerSkillRow row;
                row.skillType = pResult->getInt(++i);
                row.skillLevel = pResult->getInt(++i);
                row.skillExp = pResult->getInt(++i);
                row.delay = pResult->getInt(++i);
                row.castingTime = pResult->getInt(++i);
                row.nextTime = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<VampireSkillRow> loadVampireSkills(const string& ownerName) {
        vector<VampireSkillRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT SkillType, Delay, CastingTime, NextTime FROM VampireSkillSave WHERE OwnerID = '%s'",
                ownerName.c_str());

            while (pResult->next()) {
                int i = 0;
                VampireSkillRow row;
                row.skillType = pResult->getInt(++i);
                row.delay = pResult->getInt(++i);
                row.castingTime = pResult->getInt(++i);
                row.nextTime = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<OustersSkillRow> loadOustersSkills(const string& ownerName) {
        vector<OustersSkillRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT SkillType, SkillLevel, Delay, CastingTime, NextTime FROM OustersSkillSave WHERE OwnerID = '%s'",
                ownerName.c_str());

            while (pResult->next()) {
                int i = 0;
                OustersSkillRow row;
                row.skillType = pResult->getInt(++i);
                row.skillLevel = pResult->getInt(++i);
                row.delay = pResult->getInt(++i);
                row.castingTime = pResult->getInt(++i);
                row.nextTime = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void insertSlayerSkill(const string& ownerName, const SlayerSkillRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO SkillSave (OwnerID , SkillType , SkillLevel , SkillExp , Delay , "
                                "CastingTime , NextTime) VALUES ( '%s', %d, %d, %d, %d, %d, %ld )",
                                ownerName.c_str(), record.skillType, record.skillLevel, record.skillExp, record.delay,
                                record.castingTime, record.nextTime);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertVampireSkill(const string& ownerName, const VampireSkillRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO VampireSkillSave (OwnerID, SkillType, Delay, CastingTime, NextTime) "
                                "VALUES ( '%s', %d, %d, %d, %ld )",
                                ownerName.c_str(), record.skillType, record.delay, record.castingTime, record.nextTime);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertOustersSkill(const string& ownerName, const OustersSkillRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            // SkillLevel comes LAST in this table's insert — the column
            // list and the value list agree, so it lands correctly.
            pStmt->executeQuery("INSERT INTO OustersSkillSave (OwnerID, SkillType, Delay, CastingTime, NextTime, "
                                "SkillLevel) VALUES ( '%s', %d, %d, %d, %ld, %d )",
                                ownerName.c_str(), record.skillType, record.delay, record.castingTime, record.nextTime,
                                record.skillLevel);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateSlayerSkill(const string& ownerName, SkillType_t skillType, ExpLevel_t skillLevel, Exp_t skillExp,
                           Turn_t delay) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE SkillSave SET SkillLevel=%d, SkillExp=%d, Delay=%d WHERE OwnerID='%s' AND SkillType=%d",
                skillLevel, skillExp, delay, ownerName.c_str(), skillType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateVampireSkill(const string& ownerName, SkillType_t skillType, Turn_t delay) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE VampireSkillSave SET Delay=%d WHERE OwnerID='%s' AND SkillType=%d", delay,
                                ownerName.c_str(), skillType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateOustersSkill(const string& ownerName, SkillType_t skillType, ExpLevel_t skillLevel, Turn_t delay) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE OustersSkillSave SET SkillLevel=%d, Delay=%d WHERE OwnerID='%s' AND SkillType=%d", skillLevel,
                delay, ownerName.c_str(), skillType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteOustersSkill(const string& ownerName, SkillType_t skillType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM OustersSkillSave WHERE OwnerID='%s' AND SkillType=%u", ownerName.c_str(),
                                skillType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

SkillSaveRepository& defaultSkillSaveRepository() {
    static MySQLSkillSaveRepository instance;
    return instance;
}
