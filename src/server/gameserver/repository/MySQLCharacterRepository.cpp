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
class MySQLCharacterRepository : public CharacterRepository {
public:
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
