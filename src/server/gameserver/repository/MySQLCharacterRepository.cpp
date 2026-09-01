#include "DB.h"
#include "StringStream.h"
#include "repository/CharacterRepository.h"

namespace {

// MySQL implementation of the character-row persistence seam. The legacy
// quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Each race's SQL is preserved byte-for-byte, INCLUDING its build
//    mechanism: the Slayer vitals go through printf-style interpolation
//    while Vampire/Ousters build a StringStream (spacing differs —
//    "CurrentHP=%d" vs "CurrentHP = 12" — and both shapes are kept).
//  - Vampire saveExps writes SilverDamage ONLY when it is non-zero: the
//    original composed an optional ",SilverDamage = %d" fragment into a
//    %s slot, and a zero value leaves the column untouched. Ousters
//    writes it unconditionally. Slayer has no SilverDamage at all.
//  - tinysave's SET fragment is caller-composed raw SQL (sprintf'd
//    "Column=value" strings from dozens of sites), applied verbatim.
//    Slayer's WHERE uses uppercase NAME; Vampire/Ousters use Name.
//  - Wide exp values ride the same varargs slots as before (DWORD
//    members against %lu/%ld conversions — a pre-existing LP64 mismatch
//    that works through the ABI's register/slot zero-extension; the
//    record fields keep the member types so the bytes are unchanged).
//  - An UPDATE for a name with no row matches zero rows, silently; the
//    old save() comment documenting that affected-rows may be 0 when
//    nothing changed still applies (no CLIENT_FOUND_ROWS).
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
            StringStream sql;
            sql << "UPDATE Vampire SET" << " CurrentHP = " << record.currentHP << ", HP = " << record.maxHP
                << ", SilverDamage = " << record.silverDamage << ", ZoneID = " << record.zoneID
                << ", XCoord = " << record.x << ", YCoord = " << record.y << " WHERE Name = '" << ownerName << "'";

            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQueryString(sql.toString());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveOustersVitals(const string& ownerName, const OustersVitalsRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            StringStream sql;
            sql << "UPDATE Ousters SET" << " CurrentHP = " << record.currentHP << ", HP = " << record.maxHP
                << ", CurrentMP = " << record.currentMP << ", MP = " << record.maxMP << ", ZoneID = " << record.zoneID
                << ", XCoord = " << record.x << ", YCoord = " << record.y << " WHERE Name = '" << ownerName << "'";

            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQueryString(sql.toString());
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

    void resetSlayerReward(const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE Slayer SET Reward = 0 WHERE Name='%s'", ownerName.c_str());
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
