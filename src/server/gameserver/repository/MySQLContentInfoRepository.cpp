#include "DB.h"
#include "repository/ContentInfoRepository.h"

namespace {

// MySQL implementation of the content-info seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original except the two
//    backslash-continued literals (MonsterInfoManager::load's 35-column
//    SELECT and SkillInfoManager::load's mixed-case "Select ... from"),
//    whose leaked indentation tabs collapse to single spaces — the same
//    deliberate whitespace-run change MySQLCharacterRepository.cpp made,
//    and the only byte change in this seam. Kept as they were: the
//    reload SELECT's double space before "FROM" (a StringStream joined
//    "NormalRegen " to " FROM MonsterInfo") and its appended
//    " WHERE MType=<n>" rendered through "%d" (a WORD, promoted), the
//    SkillBalance backticks around `RequireSkill` and `Condition`
//    (CONDITION is reserved), Script's ORDER BY ScriptID (the one load
//    here with an order), and the NPC SELECT's two shapes (by zone, or
//    by zone and race).
//  - The MAX probes return false on the NULL an empty table yields (see
//    MySQLBalanceInfoRepository.cpp); MonsterInfoManager::load probed
//    with executeQueryString, the others with executeQuery — identical
//    bytes either way.
//  - Domain and MagicDomain come back through getBYTE, as before; the
//    other integers through getInt, text through getString ("" for NULL
//    — MonsterInfo's text columns and Script's are nullable).
class MySQLContentInfoRepository : public ContentInfoRepository {
public:
    bool loadMaxMonsterType(int& maxType) {
        return loadMax("SELECT MAX(MType) FROM MonsterInfo", maxType);
    }

    vector<MonsterInfoRow> loadMonsterInfos() {
        vector<MonsterInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT MType, SType, HName, EName, Level, STR, DEX, INTE, BSize, Exp, MColor, SColor, Align, AOrder, "
                "Moral, Delay, ADelay, Sight, MeleeRange, MissileRange, RegenPortal, RegenInvisible, RegenBat, MMode, "
                "AIType, Enhance, UnburrowChance, Master, ClanType, DefaultEffects, Chief, NormalRegen, HasTreasure, "
                "MonsterClass, SkullType FROM MonsterInfo");

            while (pResult->next()) {
                int i = 0;
                MonsterInfoRow row;
                row.monsterType = pResult->getInt(++i);
                row.spriteType = pResult->getInt(++i);
                row.hName = pResult->getString(++i);
                row.eName = pResult->getString(++i);
                row.level = pResult->getInt(++i);
                row.str = pResult->getInt(++i);
                row.dex = pResult->getInt(++i);
                row.inte = pResult->getInt(++i);
                row.bodySize = pResult->getInt(++i);
                row.exp = pResult->getInt(++i);
                row.mainColor = pResult->getInt(++i);
                row.subColor = pResult->getInt(++i);
                row.alignment = pResult->getInt(++i);
                row.attackOrder = pResult->getInt(++i);
                row.moral = pResult->getInt(++i);
                row.delay = pResult->getInt(++i);
                row.attackDelay = pResult->getInt(++i);
                row.sight = pResult->getInt(++i);
                row.meleeRange = pResult->getInt(++i);
                row.missileRange = pResult->getInt(++i);
                row.regenPortal = pResult->getInt(++i);
                row.regenInvisible = pResult->getInt(++i);
                row.regenBat = pResult->getInt(++i);
                row.moveMode = pResult->getString(++i);
                row.aiType = pResult->getInt(++i);
                row.enhance = pResult->getString(++i);
                row.unburrowChance = pResult->getInt(++i);
                row.master = pResult->getInt(++i);
                row.clanType = pResult->getInt(++i);
                row.defaultEffects = pResult->getString(++i);
                row.chief = pResult->getInt(++i);
                row.normalRegen = pResult->getInt(++i);
                row.hasTreasure = pResult->getInt(++i);
                row.monsterClass = pResult->getInt(++i);
                row.skullType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<MonsterSummonRow> loadMonsterSummonInfos() {
        vector<MonsterSummonRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MType, MonsterSummonInfo FROM MonsterInfo");

            while (pResult->next()) {
                MonsterSummonRow row;
                row.monsterType = pResult->getInt(1);
                row.summonInfo = pResult->getString(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<MonsterReloadRow> loadMonsterInfosForReload() {
        vector<MonsterReloadRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT MType, SType, HName, EName, Level, STR, DEX, INTE, BSize, Exp, MColor, SColor, Align, AOrder, "
                "Moral, Delay, ADelay, Sight, MeleeRange, MissileRange, RegenPortal, RegenInvisible, RegenBat, MMode, "
                "AIType, Enhance, UnburrowChance, Master, ClanType, MonsterSummonInfo, DefaultEffects, NormalRegen "
                " FROM MonsterInfo");

            readReloadRows(pResult, rows);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<MonsterReloadRow> loadMonsterInfoForReload(MonsterType_t monsterType) {
        vector<MonsterReloadRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT MType, SType, HName, EName, Level, STR, DEX, INTE, BSize, Exp, MColor, SColor, Align, AOrder, "
                "Moral, Delay, ADelay, Sight, MeleeRange, MissileRange, RegenPortal, RegenInvisible, RegenBat, MMode, "
                "AIType, Enhance, UnburrowChance, Master, ClanType, MonsterSummonInfo, DefaultEffects, NormalRegen "
                " FROM MonsterInfo WHERE MType=%d",
                monsterType);

            readReloadRows(pResult, rows);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxSkillBalanceType(int& maxType) {
        return loadMax("SELECT MAX(Type) FROM SkillBalance", maxType);
    }

    vector<SkillBalanceRow> loadSkillBalances() {
        vector<SkillBalanceRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "Select Type, Name, Level, MinDam, MaxDam, MinDelay, MaxDelay, MinDur, MaxDur, Mana, MinRange, "
                "MaxRange, Target, SubExp, Point, Domain, MagicDomain, Melee, Magic, Physic, SkillPoint, LevelUpPoint, "
                "`RequireSkill`, `Condition`, ElementalDomain, CanDelete from SkillBalance");

            while (pResult->next()) {
                int i = 0;
                SkillBalanceRow row;
                row.type = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.level = pResult->getInt(++i);
                row.minDamage = pResult->getInt(++i);
                row.maxDamage = pResult->getInt(++i);
                row.minDelay = pResult->getInt(++i);
                row.maxDelay = pResult->getInt(++i);
                row.minDuration = pResult->getInt(++i);
                row.maxDuration = pResult->getInt(++i);
                row.mana = pResult->getInt(++i);
                row.minRange = pResult->getInt(++i);
                row.maxRange = pResult->getInt(++i);
                row.target = pResult->getInt(++i);
                row.subExp = pResult->getInt(++i);
                row.point = pResult->getInt(++i);
                row.domain = pResult->getBYTE(++i);
                row.magicDomain = pResult->getBYTE(++i);
                row.melee = pResult->getInt(++i);
                row.magic = pResult->getInt(++i);
                row.physic = pResult->getInt(++i);
                row.skillPoint = pResult->getInt(++i);
                row.levelUpPoint = pResult->getInt(++i);
                row.requireSkill = pResult->getString(++i);
                row.condition = pResult->getString(++i);
                row.elementalDomain = pResult->getInt(++i);
                row.canDelete = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NPCRow> loadNPCs(int zoneID) {
        vector<NPCRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Name, NPCID, SpriteType, Race, MainColor, SubColor, ClanType, "
                                    "ShowInMinimap, TaxingCastleZoneID FROM NPC WHERE ZoneID = %d",
                                    zoneID);

            readNPCRows(pResult, rows);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NPCRow> loadNPCsOfRace(int zoneID, int race) {
        vector<NPCRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Name, NPCID, SpriteType, Race, MainColor, SubColor, ClanType, "
                                    "ShowInMinimap, TaxingCastleZoneID FROM NPC WHERE ZoneID = %d AND Race = %d",
                                    zoneID, race);

            readNPCRows(pResult, rows);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ScriptRow> loadScripts() {
        vector<ScriptRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ScriptID, OwnerID, Subject, Content FROM Script ORDER BY ScriptID");

            while (pResult->next()) {
                ScriptRow row;
                row.scriptID = pResult->getInt(1);
                row.ownerID = pResult->getString(2);
                row.subject = pResult->getString(3);
                row.content = pResult->getString(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxDirectiveSetID(int& maxID) {
        return loadMax("SELECT MAX(ID) FROM DirectiveSet", maxID);
    }

    vector<DirectiveSetRow> loadDirectiveSets() {
        vector<DirectiveSetRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, Name, Content, DeadContent FROM DirectiveSet");

            while (pResult->next()) {
                DirectiveSetRow row;
                row.id = pResult->getInt(1);
                row.name = pResult->getString(2);
                row.content = pResult->getString(3);
                row.deadContent = pResult->getString(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMaxAttrID(int& maxAttrID) {
        return loadMax("SELECT MAX(attrID) FROM AttrInfo", maxAttrID);
    }

    vector<VariableRow> loadVariables() {
        vector<VariableRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT attrID, attr1, attr2 FROM AttrInfo");

            while (pResult->next()) {
                VariableRow row;
                row.attrID = pResult->getInt(1);
                row.attr1 = pResult->getInt(2);
                row.attr2 = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void saveVariable(int value, int attrID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE AttrInfo SET attr1=%d WHERE attrID=%d", value, attrID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

private:
    // The single MAX() row; false when the aggregate is NULL.
    static bool loadMax(const char* query, int& maxValue) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(query);

            if (pResult->next()) {
                const char* field = pResult->getField(1);
                if (field != NULL) {
                    maxValue = atoi(field);
                    found = true;
                }
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    static void readReloadRows(Result* pResult, vector<MonsterReloadRow>& rows) {
        while (pResult->next()) {
            int i = 0;
            MonsterReloadRow row;
            row.monsterType = pResult->getInt(++i);
            row.spriteType = pResult->getInt(++i);
            row.hName = pResult->getString(++i);
            row.eName = pResult->getString(++i);
            row.level = pResult->getInt(++i);
            row.str = pResult->getInt(++i);
            row.dex = pResult->getInt(++i);
            row.inte = pResult->getInt(++i);
            row.bodySize = pResult->getInt(++i);
            row.exp = pResult->getInt(++i);
            row.mainColor = pResult->getInt(++i);
            row.subColor = pResult->getInt(++i);
            row.alignment = pResult->getInt(++i);
            row.attackOrder = pResult->getInt(++i);
            row.moral = pResult->getInt(++i);
            row.delay = pResult->getInt(++i);
            row.attackDelay = pResult->getInt(++i);
            row.sight = pResult->getInt(++i);
            row.meleeRange = pResult->getInt(++i);
            row.missileRange = pResult->getInt(++i);
            row.regenPortal = pResult->getInt(++i);
            row.regenInvisible = pResult->getInt(++i);
            row.regenBat = pResult->getInt(++i);
            row.moveMode = pResult->getString(++i);
            row.aiType = pResult->getInt(++i);
            row.enhance = pResult->getString(++i);
            row.unburrowChance = pResult->getInt(++i);
            row.master = pResult->getInt(++i);
            row.clanType = pResult->getInt(++i);
            row.monsterSummonInfo = pResult->getString(++i);
            row.defaultEffects = pResult->getString(++i);
            row.normalRegen = pResult->getInt(++i);
            rows.push_back(row);
        }
    }

    static void readNPCRows(Result* pResult, vector<NPCRow>& rows) {
        while (pResult->next()) {
            int i = 0;
            NPCRow row;
            row.name = pResult->getString(++i);
            row.npcID = pResult->getInt(++i);
            row.spriteType = pResult->getInt(++i);
            row.race = pResult->getInt(++i);
            row.mainColor = pResult->getInt(++i);
            row.subColor = pResult->getInt(++i);
            row.clanType = pResult->getInt(++i);
            row.showInMinimap = pResult->getInt(++i);
            row.taxingCastleZoneID = pResult->getInt(++i);
            rows.push_back(row);
        }
    }
};

} // namespace

ContentInfoRepository& defaultContentInfoRepository() {
    static MySQLContentInfoRepository instance;
    return instance;
}
