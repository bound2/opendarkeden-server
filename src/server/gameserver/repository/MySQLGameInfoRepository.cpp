#include "DB.h"
#include "repository/GameInfoRepository.h"

namespace {

// MySQL implementation of the game-info seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original — the
//    trailing space of "SELECT SkillType, Parent FROM SkillTreeInfo ",
//    the `Rank` backticks of RankBonusInfo (RANK is reserved on MySQL 8
//    — load-bearing there, untestable on the 5.7 tier), the name
//    tables' inline 'BASIC'/'EVENT' filters (data in the literal, not
//    a parameter — kept, as four distinct statements).
//  - The MAX probes return false on the NULL a MAX() over an empty
//    table yields — the inline code would have atoi(NULL)'d it (see
//    MySQLBalanceInfoRepository.cpp).
//  - None of the loads has an ORDER BY. SkillTreeInfo is KEYLESS and its
//    loader relies on rows of the same SkillType arriving adjacent
//    (it opens a new SkillParentInfo whenever the type changes); the
//    name managers index names by arrival position. Both are the
//    optimizer's choice, not a contract (see
//    MySQLSkillSaveRepository.cpp) — a clustered scan of a keyless
//    InnoDB table returns insertion order today.
//  - Names and option lists come back through getString ("" for NULL),
//    as before.
const char* const MONSTER_NAME_QUERIES[MONSTER_NAME_LIST_MAX] = {
    "SELECT Name FROM FirstNameInfo WHERE MonsterType='BASIC'",  // MONSTER_NAMES_FIRST_BASIC
    "SELECT Name FROM MiddleNameInfo WHERE MonsterType='BASIC'", // MONSTER_NAMES_MIDDLE_BASIC
    "SELECT Name FROM LastNameInfo WHERE MonsterType='BASIC'",   // MONSTER_NAMES_LAST_BASIC
    "SELECT Name FROM LastNameInfo WHERE MonsterType='EVENT'",   // MONSTER_NAMES_LAST_EVENT
};

// The single MAX() row; false when the aggregate is NULL.
bool readMax(Result* pResult, int& maxValue) {
    if (!pResult->next())
        return false;

    const char* field = pResult->getField(1);
    if (field == NULL)
        return false;

    maxValue = atoi(field);
    return true;
}

bool loadMax(const char* query, int& maxValue) {
    bool found = false;
    Statement* pStmt = NULL;

    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        Result* pResult = pStmt->executeQuery(query);
        found = readMax(pResult, maxValue);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)

    return found;
}

class MySQLGameInfoRepository : public GameInfoRepository {
public:
    bool loadMaxSkillType(int& maxSkillType) {
        return loadMax("SELECT MAX(SkillType) FROM SkillTreeInfo", maxSkillType);
    }

    vector<SkillParentRow> loadSkillTree() {
        vector<SkillParentRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT SkillType, Parent FROM SkillTreeInfo ");

            while (pResult->next()) {
                int i = 0;
                SkillParentRow row;
                row.skillType = pResult->getInt(++i);
                row.parent = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxRankBonusType(int& maxType) {
        return loadMax("SELECT MAX(Type) FROM RankBonusInfo", maxType);
    }

    vector<RankBonusInfoRow> loadRankBonusInfos() {
        vector<RankBonusInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Type, Name, `Rank`, Point, Race FROM RankBonusInfo");

            while (pResult->next()) {
                int i = 0;
                RankBonusInfoRow row;
                row.type = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.rank = pResult->getInt(++i);
                row.point = pResult->getInt(++i);
                row.race = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxPetType(int& maxPetType) {
        return loadMax("SELECT MAX(PetType) FROM PetTypeInfo", maxPetType);
    }

    vector<PetTypeRow> loadPetTypes() {
        vector<PetTypeRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PetType, OriginalMonsterType, CreatureType1, CreatureType2, "
                                                  "CreatureType3, CreatureType4, CreatureType5, FoodType "
                                                  "FROM PetTypeInfo");

            while (pResult->next()) {
                PetTypeRow row;
                row.petType = pResult->getInt(1);
                row.originalMonsterType = pResult->getInt(2);
                row.creatureType[0] = pResult->getInt(3);
                row.creatureType[1] = pResult->getInt(4);
                row.creatureType[2] = pResult->getInt(5);
                row.creatureType[3] = pResult->getInt(6);
                row.creatureType[4] = pResult->getInt(7);
                row.foodType = pResult->getInt(8);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxWorldID(int& maxWorldID) {
        return loadMax("SELECT MAX(WorldID) FROM GameServerGroupInfo", maxWorldID);
    }

    vector<GameServerGroupRow> loadGameServerGroups() {
        vector<GameServerGroupRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT WorldID, GroupID, GroupName, Stat FROM GameServerGroupInfo");

            while (pResult->next()) {
                GameServerGroupRow row;
                row.worldID = pResult->getInt(1);
                row.groupID = pResult->getInt(2);
                row.groupName = pResult->getString(3);
                row.stat = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxBloodBibleBonusType(int& maxType) {
        return loadMax("SELECT MAX(Type) FROM BloodBibleBonusInfo", maxType);
    }

    vector<BloodBibleBonusRow> loadBloodBibleBonuses() {
        vector<BloodBibleBonusRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Type, Name, OptionList FROM BloodBibleBonusInfo");

            while (pResult->next()) {
                int i = 0;
                BloodBibleBonusRow row;
                row.type = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.optionList = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<string> loadMonsterNames(MonsterNameList list) {
        vector<string> names;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(MONSTER_NAME_QUERIES[list]);

            while (pResult->next())
                names.push_back(pResult->getString(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return names;
    }
};

} // namespace

GameInfoRepository& defaultGameInfoRepository() {
    static MySQLGameInfoRepository instance;
    return instance;
}
