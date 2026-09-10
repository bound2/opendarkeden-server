#include "DB.h"
#include "repository/LoginCharacterRepository.h"

namespace {

// The per-race selection statement (LoginRaceTable order).
const char* const kSelectStatements[LOGIN_RACE_TABLE_MAX] = {
    "SELECT ZoneID, Slot, GREATEST(SwordLevel,BladeLevel,GunLevel,EnchantLevel,HealLevel), Competence FROM "
    "Slayer WHERE Name = '%s' AND PlayerID = '%s' AND Active = 'ACTIVE'",
    "SELECT ZoneID, Slot, Level, Competence FROM Vampire WHERE Name = '%s' AND "
    "PlayerID = '%s' AND Active = 'ACTIVE'",
    "SELECT ZoneID, Slot, Level, Competence FROM Ousters WHERE Name = '%s' AND "
    "PlayerID = '%s' AND Active = 'ACTIVE'",
};

// The attribute balance probes (LoginAttrTable order).
const char* const kAttrGoalExpStatements[LOGIN_ATTR_TABLE_MAX] = {
    "SELECT GoalExp FROM STRBalanceInfo WHERE Level = %d",
    "SELECT GoalExp FROM DEXBalanceInfo WHERE Level = %d",
    "SELECT GoalExp FROM INTBalanceInfo WHERE Level = %d",
};

const char* const kAttrAccumExpStatements[LOGIN_ATTR_TABLE_MAX] = {
    "SELECT AccumExp FROM STRBalanceInfo WHERE Level = %d",
    "SELECT AccumExp FROM DEXBalanceInfo WHERE Level = %d",
    "SELECT AccumExp FROM INTBalanceInfo WHERE Level = %d",
};

// The FlagSet row a new character starts with (LoginFlagSetPreset order).
const char* const kFlagSetStatements[LOGIN_FLAGSET_MAX] = {
    "INSERT IGNORE INTO FlagSet (OwnerID, FlagData) VALUES ('%s','11110010001')",
    "INSERT IGNORE INTO FlagSet (OwnerID, FlagData) VALUES ('%s','00000000001')",
};

// MySQL implementation of LoginCharacterRepository. Every method creates
// its Statement on g_pDatabaseManager->getConnection(worldID) (see the
// header) and frees it on every success path; a SQL failure is logged to
// DBError.log under the method's name and thrown as END_DB's
// DatabaseError. A table or preset outside its enum runs no statement.
class MySQLLoginCharacterRepository : public LoginCharacterRepository {
public:
    bool slayerNameExists(WorldID_t worldID, const string& name) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Name FROM Slayer WHERE Name = '%s'", name.c_str());

            found = pResult->getRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool slotOccupied(WorldID_t worldID, const string& playerID, const string& slot) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Name FROM Slayer WHERE PlayerID ='%s' and Slot ='%s' AND Active='ACTIVE'",
                                    playerID.c_str(), slot.c_str());

            found = pResult->getRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadSlayerNameInSlot(WorldID_t worldID, const string& playerID, int slot, string& name) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Name from Slayer where PlayerID='%s' AND Slot='SLOT%d'",
                                                  playerID.c_str(), slot);

            if (pResult->next()) {
                name = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadRankGoalExp(WorldID_t worldID, int rankType, int& goalExp) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT GoalExp FROM RankEXPInfo WHERE Level=1 AND RankType=%d", rankType);

            if (pResult->next()) {
                goalExp = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadVampireGoalExp(WorldID_t worldID, int& goalExp) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT GoalExp FROM VampEXPBalanceInfo WHERE Level=1");

            if (pResult->next()) {
                goalExp = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadOustersGoalExp(WorldID_t worldID, int& goalExp) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT GoalExp FROM OustersEXPBalanceInfo WHERE Level=1");

            if (pResult->next()) {
                goalExp = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadAttrGoalExp(WorldID_t worldID, LoginAttrTable attr, int level, int& goalExp) {
        if (attr >= LOGIN_ATTR_TABLE_MAX)
            return false;

        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery(kAttrGoalExpStatements[attr], level);

            if (pResult->next()) {
                goalExp = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadAttrAccumExp(WorldID_t worldID, LoginAttrTable attr, int level, int& accumExp) {
        if (attr >= LOGIN_ATTR_TABLE_MAX)
            return false;

        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery(kAttrAccumExpStatements[attr], level);

            if (pResult->next()) {
                accumExp = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertSlayer(WorldID_t worldID, const LoginNewSlayer& r) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            pStmt->executeQuery(
                "INSERT INTO Slayer (Race, Name, PlayerID, Slot, ServerGroupID, Active, Sex, HairStyle, HairColor, "
                "SkinColor, Phone, STR, STRExp, STRGoalExp, DEX, DEXExp, DEXGoalExp, INTE, INTExp, INTGoalExp, `Rank`, "
                "RankExp, RankGoalExp, HP, CurrentHP, MP, CurrentMP, ZoneID, XCoord, YCoord, Sight, Gold, Alignment, "
                "Shape, HelmetColor, JacketColor, PantsColor, WeaponColor, ShieldColor, creation_date) VALUES ('%s', "
                "'%s', "
                "'%s', '%s', %d, 'ACTIVE', '%s', '%s', %d, %d, 0, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                "%d, "
                "%d, %d, 2101, 65, 45, 13, 0, 7500, %d, %d, %d, %d, %d, %d, now())",
                r.race.c_str(), r.name.c_str(), r.playerID.c_str(), r.slot.c_str(), r.serverGroupID, r.sex.c_str(),
                r.hairStyle.c_str(), r.hairColor, r.skinColor, r.str, r.strExp, r.strGoalExp, r.dex, r.dexExp,
                r.dexGoalExp, r.inte, r.intExp, r.intGoalExp, r.rank, r.rankExp, r.rankGoalExp, r.hp, r.currentHP, r.mp,
                r.currentMP, r.shape, r.helmetColor, r.jacketColor, r.pantsColor, r.weaponColor, r.shieldColor);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertVampire(WorldID_t worldID, const LoginNewVampire& r) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            pStmt->executeQuery(
                "INSERT INTO Vampire ( Name, PlayerID, Slot, ServerGroupID, Active, Sex, SkinColor, STR, DEX, INTE, "
                "HP, CurrentHP, ZoneID, XCoord, YCoord, Sight, Alignment, Exp, GoalExp, `Rank`, RankExp, RankGoalExp, "
                "Shape, CoatColor) VALUES ( '%s', '%s', '%s', %d, 'ACTIVE', '%s', %d, 20, 20, 20, 50, 50, 1003, 62, "
                "64, 13, 7500, 0, %d, 1, 0, %d, %d, 377 )",
                r.name.c_str(), r.playerID.c_str(), r.slot.c_str(), r.serverGroupID, r.sex.c_str(), r.skinColor,
                r.goalExp, r.rankGoalExp, r.shape);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertOusters(WorldID_t worldID, const LoginNewOusters& r) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            pStmt->executeQuery("INSERT INTO Ousters ( Name, PlayerID, Slot, ServerGroupID, Active, Sex, STR, DEX, "
                                "INTE, BONUS, HP, CurrentHP, MP, CurrentMP, ZoneID, XCoord, YCoord, Sight, Alignment, "
                                "Exp, GoalExp, `Rank`, RankExp, RankGoalExp, CoatColor, HairColor, ArmColor, "
                                "BootsColor ) Values ( '%s', '%s', '%s', %d, 'ACTIVE', 'FEMALE', %d, %d, %d, 0, 50, "
                                "50, 50, 50, 1311, 24, 73, 13, 7500, 0, %d, 1, 0,	%d, 377, %d, 377, 377 )",
                                r.name.c_str(), r.playerID.c_str(), r.slot.c_str(), r.serverGroupID, r.str, r.dex,
                                r.inte, r.goalExp, r.rankGoalExp, r.hairColor);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertFlagSet(WorldID_t worldID, const string& name, LoginFlagSetPreset preset) {
        if (preset >= LOGIN_FLAGSET_MAX)
            return;

        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            pStmt->executeQuery(kFlagSetStatements[preset], name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadCharacterForSelect(WorldID_t worldID, LoginRaceTable table, const string& name, const string& playerID,
                                LoginSelectRow& row) {
        if (table >= LOGIN_RACE_TABLE_MAX)
            return false;

        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery(kSelectStatements[table], name.c_str(), playerID.c_str());

            if (pResult->getRowCount() == 1) {
                pResult->next();
                row.zoneID = pResult->getWORD(1);
                row.slot = pResult->getString(2);
                row.level = pResult->getInt(3);
                row.competence = pResult->getInt(4);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void setCharacterServerGroup(WorldID_t worldID, int serverGroupID, const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            pStmt->executeQuery("UPDATE Slayer Set ServerGroupID = %d WHERE Name='%s'", serverGroupID, name.c_str());
            pStmt->executeQuery("UPDATE Vampire Set ServerGroupID = %d WHERE Name='%s'", serverGroupID, name.c_str());
            pStmt->executeQuery("UPDATE Ousters Set ServerGroupID = %d WHERE Name='%s'", serverGroupID, name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<LoginSlayerListRow> loadSlayerList(WorldID_t worldID, const string& playerID) {
        vector<LoginSlayerListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Race, Name, Slot, Sex, HairColor, SkinColor, AdvancementClass, STR, STRExp, DEX, DEXExp, INTE, "
                "INTExp, HP, CurrentHP, MP, CurrentMP, Fame, BladeLevel, SwordLevel, GunLevel, HealLevel, "
                "EnchantLevel, "
                "ETCLevel, Alignment, Shape, HelmetColor, JacketColor, PantsColor, WeaponColor, ShieldColor, `Rank` "
                "FROM "
                "Slayer WHERE PlayerID = '%s' AND Active = 'ACTIVE'",
                playerID.c_str());

            while (pResult->next()) {
                uint i = 0;
                LoginSlayerListRow row;
                row.race = pResult->getString(++i);
                row.name = pResult->getString(++i);
                row.slot = pResult->getString(++i);
                row.sex = pResult->getString(++i);
                row.hairColor = pResult->getInt(++i);
                row.skinColor = pResult->getInt(++i);
                row.advancementClass = pResult->getInt(++i);
                row.str = pResult->getInt(++i);
                row.strExp = pResult->getInt(++i);
                row.dex = pResult->getInt(++i);
                row.dexExp = pResult->getInt(++i);
                row.inte = pResult->getInt(++i);
                row.intExp = pResult->getInt(++i);
                row.hp = pResult->getInt(++i);
                row.currentHP = pResult->getInt(++i);
                row.mp = pResult->getInt(++i);
                row.currentMP = pResult->getInt(++i);
                row.fame = pResult->getInt(++i);
                for (int j = 0; j < 6; j++) {
                    row.domainLevel[j] = pResult->getInt(++i);
                }
                row.alignment = pResult->getInt(++i);
                row.shape = pResult->getDWORD(++i);
                row.helmetColor = pResult->getInt(++i);
                row.jacketColor = pResult->getInt(++i);
                row.pantsColor = pResult->getInt(++i);
                row.weaponColor = pResult->getInt(++i);
                row.shieldColor = pResult->getInt(++i);
                row.rank = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadVampireListRow(WorldID_t worldID, const string& playerID, const string& name, LoginVampireListRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Name, Slot, Sex, BatColor, SkinColor, AdvancementClass, STR, DEX, INTE, HP, CurrentHP, "
                "`Rank`, GoalExp, Level, Bonus, Fame, Alignment, Shape, CoatColor FROM Vampire WHERE PlayerID = "
                "'%s' AND Active = 'ACTIVE' AND Name='%s'",
                playerID.c_str(), name.c_str());

            if (pResult->next()) {
                uint i = 0;
                row.name = pResult->getString(++i);
                row.slot = pResult->getString(++i);
                row.sex = pResult->getString(++i);
                row.batColor = pResult->getInt(++i);
                row.skinColor = pResult->getInt(++i);
                row.advancementClass = pResult->getInt(++i);
                row.str = pResult->getInt(++i);
                row.dex = pResult->getInt(++i);
                row.inte = pResult->getInt(++i);
                row.hp = pResult->getInt(++i);
                row.currentHP = pResult->getInt(++i);
                row.rank = pResult->getInt(++i);
                row.goalExp = pResult->getInt(++i);
                row.level = pResult->getInt(++i);
                row.bonus = pResult->getInt(++i);
                row.fame = pResult->getInt(++i);
                row.alignment = pResult->getInt(++i);
                row.shape = pResult->getDWORD(++i);
                row.coatColor = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadOustersListRow(WorldID_t worldID, const string& playerID, const string& name, LoginOustersListRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Name, Slot, Sex, AdvancementClass, STR, DEX, INTE, HP, CurrentHP, `Rank`, Exp, Level, "
                "Bonus, SkillBonus, Fame, Alignment, CoatType, ArmType, CoatColor, HairColor, ArmColor, BootsColor "
                "FROM Ousters WHERE PlayerID = '%s' AND Active = 'ACTIVE' AND Name='%s'",
                playerID.c_str(), name.c_str());

            if (pResult->next()) {
                uint i = 0;
                row.name = pResult->getString(++i);
                row.slot = pResult->getString(++i);
                row.sex = pResult->getString(++i);
                row.advancementClass = pResult->getInt(++i);
                row.str = pResult->getInt(++i);
                row.dex = pResult->getInt(++i);
                row.inte = pResult->getInt(++i);
                row.hp = pResult->getInt(++i);
                row.currentHP = pResult->getInt(++i);
                row.rank = pResult->getInt(++i);
                row.exp = pResult->getInt(++i);
                row.level = pResult->getInt(++i);
                row.bonus = pResult->getInt(++i);
                row.skillBonus = pResult->getInt(++i);
                row.fame = pResult->getInt(++i);
                row.alignment = pResult->getInt(++i);
                row.coatType = pResult->getInt(++i);
                row.armType = pResult->getInt(++i);
                row.coatColor = pResult->getInt(++i);
                row.hairColor = pResult->getInt(++i);
                row.armColor = pResult->getInt(++i);
                row.bootsColor = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

LoginCharacterRepository& defaultLoginCharacterRepository() {
    static MySQLLoginCharacterRepository instance;
    return instance;
}
