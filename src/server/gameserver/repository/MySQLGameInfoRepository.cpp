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

    // --- the config tables the second round added -------------------------
    // Byte-for-byte the originals again: DarkLightInfo's " , "-spaced
    // column list, CastleSkillInfo's mixed-case "Select ... from",
    // GoodsListInfo's "Limited+0" (the enum ordinal) and "Kind<>'SET'"
    // filter, NicknameIndex's inline 'LEVEL' filter. GoodsListInfo is
    // read on the dist connection, as GoodsInfoManager did (the same
    // DARKEDEN schema — see MySQLGoodsRepository.cpp).

    vector<WeatherRow> loadWeather() {
        vector<WeatherRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Month, Clear, Rainy, Snowy FROM WeatherInfo");

            while (pResult->next()) {
                WeatherRow row;
                row.month = pResult->getInt(1);
                row.clear = pResult->getInt(2);
                row.rainy = pResult->getInt(3);
                row.snowy = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<StringPoolRow> loadStrings() {
        vector<StringPoolRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, String FROM GSStringPool");

            while (pResult->next()) {
                int i = 0;
                StringPoolRow row;
                row.id = pResult->getInt(++i);
                row.text = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ShopTemplateRow> loadShopTemplates() {
        vector<ShopTemplateRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ID, ShopType, ItemClass, MinItemType, MaxItemType, MinOptionLevel, "
                                    "MaxOptionLevel FROM ShopTemplate");

            while (pResult->next()) {
                ShopTemplateRow row;
                row.id = pResult->getInt(1);
                row.shopType = pResult->getInt(2);
                row.itemClass = pResult->getInt(3);
                row.minItemType = pResult->getInt(4);
                row.maxItemType = pResult->getInt(5);
                row.minOptionLevel = pResult->getInt(6);
                row.maxOptionLevel = pResult->getInt(7);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LevelNickRow> loadLevelNicks() {
        vector<LevelNickRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT NickIndex, Race, Level10 FROM NicknameIndex WHERE NickType='LEVEL'");

            while (pResult->next()) {
                LevelNickRow row;
                row.nickIndex = pResult->getInt(1);
                row.race = pResult->getInt(2);
                row.level10 = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ItemMineRow> loadItemMines() {
        vector<ItemMineRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, ItemClass, ItemType, ItemOption FROM ItemMineInfo");

            while (pResult->next()) {
                uint i = 0;
                ItemMineRow row;
                row.id = pResult->getInt(++i);
                row.itemClass = pResult->getString(++i);
                row.itemType = pResult->getInt(++i);
                row.itemOption = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ItemGradeRatioRow> loadItemGradeRatios() {
        vector<ItemGradeRatioRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Grade, Ratio, GambleRatio, BeadRatio FROM ItemGradeRatioInfo");

            while (pResult->next()) {
                ItemGradeRatioRow row;
                row.grade = pResult->getInt(1);
                row.ratio = pResult->getInt(2);
                row.gambleRatio = pResult->getInt(3);
                row.beadRatio = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GoodsInfoRow> loadGoods() {
        vector<GoodsInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT GoodsID, Name, ItemClass, ItemType, Grade, OptionType, Num, Limited+0, "
                                    "Hour FROM GoodsListInfo WHERE Kind<>'SET'");

            while (pResult->next()) {
                int i = 0;
                GoodsInfoRow row;
                row.goodsID = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.itemClass = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.grade = pResult->getInt(++i);
                row.optionType = pResult->getString(++i);
                row.num = pResult->getInt(++i);
                row.limited = pResult->getInt(++i);
                row.hour = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<WorldRow> loadWorlds() {
        vector<WorldRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, Name, Stat FROM WorldInfo");

            while (pResult->next()) {
                WorldRow row;
                row.id = pResult->getInt(1);
                row.name = pResult->getString(2);
                row.stat = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<DefaultOptionSetRow> loadDefaultOptionSets() {
        vector<DefaultOptionSetRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Type, OptionList FROM DefaultOptionSetInfo");

            while (pResult->next()) {
                uint i = 0;
                DefaultOptionSetRow row;
                row.type = pResult->getInt(++i);
                row.optionList = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<DarkLightRow> loadDarkLight() {
        vector<DarkLightRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Month , Hour , Minute , DarkLevel , LightLevel FROM DarkLightInfo");

            while (pResult->next()) {
                uint i = 0;
                DarkLightRow row;
                row.month = pResult->getInt(++i);
                row.hour = pResult->getInt(++i);
                row.minute = pResult->getInt(++i);
                row.darkLevel = pResult->getInt(++i);
                row.lightLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<CastleSkillRow> loadCastleSkills() {
        vector<CastleSkillRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("Select SkillType, ZoneID from CastleSkillInfo");

            while (pResult->next()) {
                int count = 0;
                CastleSkillRow row;
                row.skillType = pResult->getInt(++count);
                row.zoneID = pResult->getInt(++count);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<CastleShrineRow> loadCastleShrines() {
        vector<CastleShrineRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ID, Name, ItemType, GuardZoneID, GuardX, GuardY, GuardMType, HolyZoneID, "
                                    "HolyX, HolyY, HolyMType FROM CastleShrineInfo");

            while (pResult->next()) {
                int i = 0;
                CastleShrineRow row;
                row.id = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.itemType = pResult->getInt(++i);
                row.guardZoneID = pResult->getInt(++i);
                row.guardX = pResult->getInt(++i);
                row.guardY = pResult->getInt(++i);
                row.guardMonsterType = pResult->getInt(++i);
                row.holyZoneID = pResult->getInt(++i);
                row.holyX = pResult->getInt(++i);
                row.holyY = pResult->getInt(++i);
                row.holyMonsterType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<string> loadLogUserNames() {
        vector<string> names;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Name FROM LogUserInfo");

            while (pResult->next())
                names.push_back(pResult->getString(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return names;
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
