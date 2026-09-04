#include "DB.h"
#include "repository/CharacterRepository.h"

namespace {

// MySQL implementation of the character-row persistence seam. The legacy
// quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The emitted SQL is byte-for-byte what the inline code produced,
//    spacing included: the Slayer vitals keep their "CurrentHP=%d"
//    printf spacing, Vampire/Ousters keep the "CurrentHP = 12" spacing
//    their StringStreams emitted (the streams themselves were replaced
//    with format strings carrying the same bytes — repository SQL uses
//    the parameterized executeQuery form, never string concatenation).
//  - Vampire saveExps writes SilverDamage ONLY when it is non-zero: the
//    original composed an optional ",SilverDamage = %d" fragment into a
//    %s slot, and a zero value leaves the column untouched — this save
//    cannot reset a vampire's silver damage. Ousters writes it
//    unconditionally. Slayer has no SilverDamage at all.
//  - tinysave's SET fragment is caller-composed raw SQL (sprintf'd
//    "Column=value" strings from ~400 sites), applied verbatim.
//    Slayer's WHERE spells the column NAME, the others Name — purely
//    cosmetic (MySQL column identifiers are case-insensitive), kept
//    only for byte-fidelity.
//  - The `Rank` backticks are LOAD-BEARING on MySQL 8: RANK became a
//    reserved word in 8.0.2, and this project supports 5.7 or 8. The
//    5.7-based integration tier cannot catch their removal.
//  - Wide exp values ride the same varargs slots as before — and that
//    is a LATENT BUG, not a benign quirk: DWORD (4-byte) arguments are
//    read through %lu/%ld (8-byte) conversions. It works today because
//    GCC's codegen zero-extends when pushing stack varargs, but the ABI
//    leaves the upper bytes of sub-eightbyte stack slots unspecified —
//    clang at -O0 demonstrably reads garbage for every stack-passed
//    %lu/%ld field (args 5+; saveSlayerExps passes 20). The extraction
//    preserves the behavior bit-for-bit under either compiler; fixing
//    the conversions to %u is deliberate follow-up work, not a silent
//    edit here.
//  - An UPDATE for a name with no row matches zero rows, silently, and
//    affected-rows may be 0 when nothing changed (no CLIENT_FOUND_ROWS)
//    — nothing here checks it, exactly like the inline code.
//  - Character names are interpolated raw (no escaping), as the call
//    sites always did.
//  - The load SELECTs are POSITIONAL: the loaders read column N of the
//    result, so the column list's order is the contract and is kept
//    verbatim — including `Rank` (see above), the un-spaced
//    "Sex,MasterEffectColor" token and the INTE spelling (INT is a
//    MySQL type keyword). The one deliberate byte change: the original
//    literals were backslash-continued across source lines and leaked
//    their indentation tabs into the SQL text; each whitespace run (a
//    space followed by the tabs, or the tabs alone) is a single space
//    here. Whitespace between tokens, immaterial to the parser.
//  - The loaders apply the record AFTER this method returns, so the
//    race classes' setters now run outside the BEGIN_DB/END_DB try —
//    inert, since none of them raise the SQLQueryException it catches —
//    and the Statement is freed before they run, so a setter throwing
//    (setSex's InvalidProtocolException, the skill loop's Assert) no
//    longer leaks it. The one real delta: a driver exception mid-read
//    now applies nothing to the creature instead of the columns read so
//    far — unreachable with a fixed column count.
//  - The loads filter on Active = 'ACTIVE': an INACTIVE row (a character
//    deleted while the login server handed it over) loads as "no row".
//    Name is the primary key, so at most one row can match.
//  - Every selected column is read with the driver getter the inline
//    code used on it — getInt (atoi of the field text) for nearly all,
//    getBYTE for StashNum and the vampire/ousters Competence pair,
//    getString for the enum/varchar columns. Out-of-int-range unsigned
//    values (Fame, Gold, GoalExp are int(10) unsigned) come back through
//    atoi exactly as they always did.
class MySQLCharacterRepository : public CharacterRepository {
public:
    bool loadSlayer(const string& ownerName, SlayerLoadRecord& record) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Name, AdvancementClass, AdvancementGoalExp, Competence, CompetenceShape, "
                "Sex,MasterEffectColor, "
                "HairStyle, HairColor, SkinColor, Phone, STR, STRGoalExp, DEX, DEXGoalExp, INTE, INTGoalExp, "
                "AdvancedSTR, AdvancedDEX, AdvancedINT, Bonus, `Rank`, RankGoalExp, CurrentHP, HP, CurrentMP, MP, "
                "Fame, Gold, GuildID, BladeLevel, BladeGoalExp, SwordLevel, SwordGoalExp, GunLevel, GunGoalExp, "
                "EnchantLevel, EnchantGoalExp, HealLevel, HealGoalExp, ETCLevel, ETCGoalExp, ZoneID, XCoord, YCoord, "
                "Sight, GunBonusExp, RifleBonusExp, Alignment, StashGold, StashNum, ResurrectZone, Reward, SMSCharge "
                "FROM Slayer WHERE Name = '%s' AND Active = 'ACTIVE'",
                ownerName.c_str());

            if (pResult->next()) {
                uint i = 0;
                record.name = pResult->getString(++i);
                record.advancementClass = pResult->getInt(++i);
                record.advancementGoalExp = pResult->getInt(++i);
                record.competence = pResult->getInt(++i);
                record.competenceShape = pResult->getInt(++i);
                record.sex = pResult->getString(++i);
                record.masterEffectColor = pResult->getInt(++i);
                record.hairStyle = pResult->getString(++i);
                record.hairColor = pResult->getInt(++i);
                record.skinColor = pResult->getInt(++i);
                record.phone = pResult->getString(++i);
                record.str = pResult->getInt(++i);
                record.strGoalExp = pResult->getInt(++i);
                record.dex = pResult->getInt(++i);
                record.dexGoalExp = pResult->getInt(++i);
                record.inte = pResult->getInt(++i);
                record.intGoalExp = pResult->getInt(++i);
                record.advancedSTR = pResult->getInt(++i);
                record.advancedDEX = pResult->getInt(++i);
                record.advancedINT = pResult->getInt(++i);
                record.bonus = pResult->getInt(++i);
                record.rank = pResult->getInt(++i);
                record.rankGoalExp = pResult->getInt(++i);
                record.currentHP = pResult->getInt(++i);
                record.maxHP = pResult->getInt(++i);
                record.currentMP = pResult->getInt(++i);
                record.maxMP = pResult->getInt(++i);
                record.fame = pResult->getInt(++i);
                record.gold = pResult->getInt(++i);
                record.guildID = pResult->getInt(++i);
                record.bladeLevel = pResult->getInt(++i);
                record.bladeGoalExp = pResult->getInt(++i);
                record.swordLevel = pResult->getInt(++i);
                record.swordGoalExp = pResult->getInt(++i);
                record.gunLevel = pResult->getInt(++i);
                record.gunGoalExp = pResult->getInt(++i);
                record.enchantLevel = pResult->getInt(++i);
                record.enchantGoalExp = pResult->getInt(++i);
                record.healLevel = pResult->getInt(++i);
                record.healGoalExp = pResult->getInt(++i);
                record.etcLevel = pResult->getInt(++i);
                record.etcGoalExp = pResult->getInt(++i);
                record.zoneID = pResult->getInt(++i);
                record.x = pResult->getInt(++i);
                record.y = pResult->getInt(++i);
                record.sight = pResult->getInt(++i);
                record.gunBonusExp = pResult->getInt(++i);
                record.rifleBonusExp = pResult->getInt(++i);
                record.alignment = pResult->getInt(++i);
                record.stashGold = pResult->getInt(++i);
                record.stashNum = pResult->getBYTE(++i);
                record.resurrectZone = pResult->getInt(++i);
                record.reward = pResult->getInt(++i);
                record.smsCharge = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadSlayerAccount(const string& name, string& playerID, string& race) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PlayerID,Race FROM Slayer WHERE Name = '%s'", name.c_str());

            if (pResult->getRowCount() == 1) {
                pResult->next();
                playerID = pResult->getString(1);
                race = pResult->getString(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadSlayerPlayerID(const string& name, string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PlayerID FROM Slayer WHERE Name='%s'", name.c_str());

            if (pResult->next()) {
                playerID = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadVampire(const string& ownerName, VampireLoadRecord& record) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Name, AdvancementClass, AdvancementGoalExp, Sex, MasterEffectColor, BatColor, SkinColor, STR, "
                "DEX, INTE, HP, CurrentHP, Fame, GoalExp, Level, Bonus, Gold, GuildID, ZoneID, XCoord, YCoord, Sight, "
                "Alignment, StashGold, StashNum, Competence, CompetenceShape, ResurrectZone, SilverDamage, Reward, "
                "SMSCharge, `Rank`, RankGoalExp FROM Vampire WHERE Name = '%s' AND Active = 'ACTIVE'",
                ownerName.c_str());

            if (pResult->next()) {
                uint i = 0;
                record.name = pResult->getString(++i);
                record.advancementClass = pResult->getInt(++i);
                record.advancementGoalExp = pResult->getInt(++i);
                record.sex = pResult->getString(++i);
                record.masterEffectColor = pResult->getInt(++i);
                record.batColor = pResult->getInt(++i);
                record.skinColor = pResult->getInt(++i);
                record.str = pResult->getInt(++i);
                record.dex = pResult->getInt(++i);
                record.inte = pResult->getInt(++i);
                record.maxHP = pResult->getInt(++i);
                record.currentHP = pResult->getInt(++i);
                record.fame = pResult->getInt(++i);
                record.goalExp = pResult->getInt(++i);
                record.level = pResult->getInt(++i);
                record.bonus = pResult->getInt(++i);
                record.gold = pResult->getInt(++i);
                record.guildID = pResult->getInt(++i);
                record.zoneID = pResult->getInt(++i);
                record.x = pResult->getInt(++i);
                record.y = pResult->getInt(++i);
                record.sight = pResult->getInt(++i);
                record.alignment = pResult->getInt(++i);
                record.stashGold = pResult->getInt(++i);
                record.stashNum = pResult->getBYTE(++i);
                record.competence = pResult->getBYTE(++i);
                record.competenceShape = pResult->getBYTE(++i);
                record.resurrectZone = pResult->getInt(++i);
                record.silverDamage = pResult->getInt(++i);
                record.reward = pResult->getInt(++i);
                record.smsCharge = pResult->getInt(++i);
                record.rank = pResult->getInt(++i);
                record.rankGoalExp = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadOusters(const string& ownerName, OustersLoadRecord& record) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Name, AdvancementClass, AdvancementGoalExp, Sex,MasterEffectColor, STR, DEX, INTE, HP, "
                "CurrentHP, MP, CurrentMP, Fame, GoalExp, Level, Bonus, SkillBonus, Gold, GuildID, ZoneID, XCoord, "
                "YCoord, Sight, Alignment, StashGold, StashNum, Competence, CompetenceShape, ResurrectZone, "
                "SilverDamage, SMSCharge, `Rank`, RankGoalExp, HairColor FROM Ousters WHERE Name = '%s' AND Active = "
                "'ACTIVE'",
                ownerName.c_str());

            if (pResult->next()) {
                uint i = 0;
                record.name = pResult->getString(++i);
                record.advancementClass = pResult->getInt(++i);
                record.advancementGoalExp = pResult->getInt(++i);
                record.sex = pResult->getString(++i);
                record.masterEffectColor = pResult->getInt(++i);
                record.str = pResult->getInt(++i);
                record.dex = pResult->getInt(++i);
                record.inte = pResult->getInt(++i);
                record.maxHP = pResult->getInt(++i);
                record.currentHP = pResult->getInt(++i);
                record.maxMP = pResult->getInt(++i);
                record.currentMP = pResult->getInt(++i);
                record.fame = pResult->getInt(++i);
                record.goalExp = pResult->getInt(++i);
                record.level = pResult->getInt(++i);
                record.bonus = pResult->getInt(++i);
                record.skillBonus = pResult->getInt(++i);
                record.gold = pResult->getInt(++i);
                record.guildID = pResult->getInt(++i);
                record.zoneID = pResult->getInt(++i);
                record.x = pResult->getInt(++i);
                record.y = pResult->getInt(++i);
                record.sight = pResult->getInt(++i);
                record.alignment = pResult->getInt(++i);
                record.stashGold = pResult->getInt(++i);
                record.stashNum = pResult->getBYTE(++i);
                record.competence = pResult->getBYTE(++i);
                record.competenceShape = pResult->getBYTE(++i);
                record.resurrectZone = pResult->getInt(++i);
                record.silverDamage = pResult->getInt(++i);
                record.smsCharge = pResult->getInt(++i);
                record.rank = pResult->getInt(++i);
                record.rankGoalExp = pResult->getInt(++i);
                record.hairColor = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void saveSlayerVitals(const string& ownerName, const SlayerVitalsRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Slayer SET CurrentHP=%d, HP=%d, CurrentMP=%d, MP=%d, ZoneID=%d, XCoord=%d, "
                                "YCoord=%d WHERE Name='%s'",
                                record.currentHP, record.maxHP, record.currentMP, record.maxMP, record.zoneID, record.x,
                                record.y, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveVampireVitals(const string& ownerName, const VampireVitalsRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            // the format string carries the exact spacing the old
            // StringStream emitted
            pStmt->executeQuery("UPDATE Vampire SET CurrentHP = %d, HP = %d, SilverDamage = %d, ZoneID = %d, "
                                "XCoord = %d, YCoord = %d WHERE Name = '%s'",
                                record.currentHP, record.maxHP, record.silverDamage, record.zoneID, record.x, record.y,
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveOustersVitals(const string& ownerName, const OustersVitalsRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            // the format string carries the exact spacing the old
            // StringStream emitted
            pStmt->executeQuery("UPDATE Ousters SET CurrentHP = %d, HP = %d, CurrentMP = %d, MP = %d, ZoneID = %d, "
                                "XCoord = %d, YCoord = %d WHERE Name = '%s'",
                                record.currentHP, record.maxHP, record.currentMP, record.maxMP, record.zoneID, record.x,
                                record.y, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveSlayerExps(const string& ownerName, const SlayerExpsRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE Slayer SET STRGoalExp=%lu, DEXGoalExp=%lu, INTGoalExp=%lu, BladeGoalExp=%lu, SwordGoalExp=%lu, "
                "GunGoalExp=%lu, EnchantGoalExp=%lu, HealGoalExp=%lu, ETCGoalExp=%lu, Alignment=%d, Fame=%ld, "
                "`Rank`=%d, "
                "RankGoalExp=%lu, AdvancementClass=%u, AdvancementGoalExp=%d, AdvancedSTR=%u, AdvancedDEX=%u, "
                "AdvancedINT=%u, Bonus=%u WHERE Name='%s'",
                record.strGoalExp, record.dexGoalExp, record.intGoalExp, record.bladeGoalExp, record.swordGoalExp,
                record.gunGoalExp, record.enchantGoalExp, record.healGoalExp, record.etcGoalExp, record.alignment,
                record.fame, record.rank, record.rankGoalExp, record.advancementClass, record.advancementGoalExp,
                record.advancedSTR, record.advancedDEX, record.advancedINT, record.advancedAttrBonus,
                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveVampireExps(const string& ownerName, const VampireExpsRecord& record) {
        Statement* pStmt = NULL;

        char silverDam[40];
        if (record.silverDamage != 0) {
            sprintf(silverDam, ",SilverDamage = %d", record.silverDamage);
        } else
            silverDam[0] = '\0';

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Vampire SET Alignment=%d, Fame=%d, GoalExp=%lu%s, `Rank`=%d, RankGoalExp=%lu, "
                                "AdvancementClass=%u, AdvancementGoalExp=%d WHERE Name='%s'",
                                record.alignment, record.fame, record.goalExp, silverDam, record.rank,
                                record.rankGoalExp, record.advancementClass, record.advancementGoalExp,
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveOustersExps(const string& ownerName, const OustersExpsRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Ousters SET Alignment=%d, Fame=%d, GoalExp=%lu, SilverDamage = %d, `Rank`=%d, "
                                "RankGoalExp=%lu, AdvancementClass=%u, AdvancementGoalExp=%d WHERE Name='%s'",
                                record.alignment, record.fame, record.goalExp, record.silverDamage, record.rank,
                                record.rankGoalExp, record.advancementClass, record.advancementGoalExp,
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void tinysave(const string& ownerName, CharacterRace race, const string& fieldFragment) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            if (race == CHARACTER_RACE_SLAYER)
                pStmt->executeQuery("UPDATE Slayer SET %s WHERE NAME='%s'", fieldFragment.c_str(), ownerName.c_str());
            else if (race == CHARACTER_RACE_VAMPIRE)
                pStmt->executeQuery("UPDATE Vampire SET %s WHERE Name='%s'", fieldFragment.c_str(), ownerName.c_str());
            else
                pStmt->executeQuery("UPDATE Ousters SET %s WHERE Name='%s'", fieldFragment.c_str(), ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

CharacterRepository& defaultCharacterRepository() {
    static MySQLCharacterRepository instance;
    return instance;
}
