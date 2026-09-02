#include "DB.h"
#include "repository/BalanceInfoRepository.h"

namespace {

// MySQL implementation of the balance-table seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, and the
//    originals did not agree on spelling, so the ladder statements are
//    per-table data: "Select ... from" in mixed case for the five
//    ladders and the rank/domain ones, "SELECT ... FROM" for the fame
//    limits and the pet tables; a TRAILING SPACE after the table name
//    on the three attribute ladders and none on the vampire/ousters
//    ones; "WHERE RankType=%d" unspaced vs "WHERE DomainType = %d"
//    spaced (both the domain MAX and its rows) but "WHERE DomainType=%d"
//    unspaced for the fame MAX. Immaterial to the parser, kept for
//    fidelity.
//  - The MAX probes: MySQL answers MAX() over an EMPTY table (or a
//    WHERE that matches nothing) with ONE row whose value is NULL.
//    The inline loaders tested getRowCount()==0 or !next() — never
//    true — and then called getInt(1), i.e. atoi(NULL), on the NULL:
//    an empty table crashed the boot instead of raising the "There is
//    no data" Error the code intended. The seam reads the field raw and
//    returns false on NULL, so the callers' throws now fire. A
//    behavior change, made knowingly, on an unreachable-in-practice
//    path (every one of these tables ships populated); pinned by the
//    integration tier through a RankType no ladder has.
//  - MAX(Level) is read through getInt like the rows, so a ladder whose
//    top level does not fit an int would size its array from a
//    truncated value — as before; the shipped ladders top out at 315.
//  - The rank/domain/fame filters take the caller's int (the enum or
//    loop index, exactly the expression the inline code streamed
//    through %d).
//  - AccumExp is bigint on all five ladders and is read through
//    getInt (atoi), as before. The shipped ladders EXCEED int range
//    (STRBalanceInfo tops out at 2431521747, the vampire and ousters
//    ladders at 3344798380): atoi truncates the 64-bit strtol result
//    to int, i.e. a negative value, which the caller's DWORD Exp_t
//    turns back into the original number — lossless below 2^32 and
//    exactly what the inline code did, so nothing changes; documented
//    because the first draft claimed the data stayed within range.
struct LadderSpec {
    const char* max;
    const char* rows;
};

const LadderSpec LADDER_SPECS[LEVEL_EXP_TABLE_MAX] = {
    // LEVEL_EXP_TABLE_STR
    {"SELECT MAX(Level) FROM STRBalanceInfo", "Select Level, GoalExp, AccumExp from STRBalanceInfo "},
    // LEVEL_EXP_TABLE_DEX
    {"SELECT MAX(Level) FROM DEXBalanceInfo", "Select Level, GoalExp, AccumExp from DEXBalanceInfo "},
    // LEVEL_EXP_TABLE_INT
    {"SELECT MAX(Level) FROM INTBalanceInfo", "Select Level, GoalExp, AccumExp from INTBalanceInfo "},
    // LEVEL_EXP_TABLE_VAMP_EXP
    {"SELECT MAX(Level) FROM VampEXPBalanceInfo", "Select Level, GoalExp, AccumExp from VampEXPBalanceInfo"},
    // LEVEL_EXP_TABLE_OUSTERS_EXP
    {"SELECT MAX(Level) FROM OustersEXPBalanceInfo",
     "Select Level, GoalExp, AccumExp, SkillPointBonus from OustersEXPBalanceInfo"},
};

// Reads the single MAX() row; false when the aggregate is NULL (no
// matching rows), which is what the drivers' getInt would have
// atoi(NULL)'d on.
bool readMax(Result* pResult, int& maxValue) {
    if (!pResult->next())
        return false;

    const char* field = pResult->getField(1);
    if (field == NULL)
        return false;

    maxValue = atoi(field);
    return true;
}

class MySQLBalanceInfoRepository : public BalanceInfoRepository {
public:
    bool loadMaxLevel(LevelExpTable table, int& maxLevel) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(LADDER_SPECS[table].max);
            found = readMax(pResult, maxLevel);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<LevelExpRow> loadLevels(LevelExpTable table) {
        vector<LevelExpRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(LADDER_SPECS[table].rows);

            while (pResult->next()) {
                int i = 0;
                LevelExpRow row;
                row.level = pResult->getInt(++i);
                row.goalExp = pResult->getInt(++i);
                row.accumExp = pResult->getInt(++i);
                row.skillPointBonus = table == LEVEL_EXP_TABLE_OUSTERS_EXP ? pResult->getInt(++i) : 0;
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxRankLevel(int rankType, int& maxLevel) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MAX(Level) FROM RankEXPInfo WHERE RankType=%d", rankType);
            found = readMax(pResult, maxLevel);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<LevelExpRow> loadRankLevels(int rankType) {
        vector<LevelExpRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("Select Level, GoalExp, AccumExp from RankEXPInfo WHERE RankType=%d", rankType);

            while (pResult->next()) {
                int i = 0;
                LevelExpRow row;
                row.level = pResult->getInt(++i);
                row.goalExp = pResult->getInt(++i);
                row.accumExp = pResult->getInt(++i);
                row.skillPointBonus = 0;
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxDomainLevel(int domainType, int& maxLevel) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT MAX(Level) FROM SkillDomainInfo WHERE DomainType = %d", domainType);
            found = readMax(pResult, maxLevel);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<DomainLevelRow> loadDomainLevels(int domainType) {
        vector<DomainLevelRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "Select DomainType, Level, GoalExp, AccumExp, BestItemType from SkillDomainInfo WHERE DomainType = %d",
                domainType);

            while (pResult->next()) {
                int i = 0;
                DomainLevelRow row;
                row.domainType = pResult->getInt(++i);
                row.level = pResult->getInt(++i);
                row.goalExp = pResult->getInt(++i);
                row.accumExp = pResult->getInt(++i);
                row.bestItemType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxFameLevel(int domainType, int& maxLevel) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT MAX(Level) FROM FameLimitInfo WHERE DomainType=%d", domainType);
            found = readMax(pResult, maxLevel);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<FameLimitRow> loadFameLimits(int domainType) {
        vector<FameLimitRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT DomainType, Level, Fame FROM FameLimitInfo WHERE DomainType = %d", domainType);

            while (pResult->next()) {
                int i = 0;
                FameLimitRow row;
                row.domainType = pResult->getInt(++i);
                row.level = pResult->getInt(++i);
                row.fame = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<PetExpRow> loadPetExp() {
        vector<PetExpRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PetLevel, PetAccumExp FROM PetExpInfo");

            while (pResult->next()) {
                PetExpRow row;
                row.petLevel = pResult->getInt(1);
                row.petAccumExp = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<PetAttrBalanceRow> loadPetAttrBalance() {
        vector<PetAttrBalanceRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PetAttr, Level, AddAttr, AccumAttr FROM PetAttrBalanceInfo");

            while (pResult->next()) {
                PetAttrBalanceRow row;
                row.petAttr = pResult->getInt(1);
                row.level = pResult->getInt(2);
                row.addAttr = pResult->getInt(3);
                row.accumAttr = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<PetAttrRatioRow> loadPetAttrRatios() {
        vector<PetAttrRatioRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PetAttr, EnchantRatio FROM PetAttrInfo");

            while (pResult->next()) {
                PetAttrRatioRow row;
                row.petAttr = pResult->getInt(1);
                row.enchantRatio = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ExpTableRow> loadExpTable(const string& levelField, const string& goalField, const string& accumField,
                                     const string& table, const string& condition) {
        vector<ExpTableRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT %s, %s, %s FROM %s %s", levelField.c_str(), goalField.c_str(),
                                                  accumField.c_str(), table.c_str(), condition.c_str());

            while (pResult->next()) {
                ExpTableRow row;
                row.level = pResult->getInt(1);
                row.goalExp = pResult->getInt(2);
                row.accumExp = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

BalanceInfoRepository& defaultBalanceInfoRepository() {
    static MySQLBalanceInfoRepository instance;
    return instance;
}
