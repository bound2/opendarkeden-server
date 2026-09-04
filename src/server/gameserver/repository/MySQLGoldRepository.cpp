#include "DB.h"
#include "repository/GoldRepository.h"

namespace {

// MySQL implementation of the carried-gold persistence seam. The legacy
// quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The writes are RELATIVE — Gold = Gold ± delta — so the database
//    arithmetic runs against whatever the row holds, not the in-memory
//    balance. The gameplay clamps (MAX_MONEY / zero) were applied by the
//    caller against its in-memory copy; when the two agree the result is
//    exact, and nothing here re-checks that they do.
//  - Gold is int(10) UNSIGNED on all three tables. A decrease below the
//    ROW's balance (reachable only through integrity drift, since the
//    caller clamps against memory) raises ER_DATA_OUT_OF_RANGE (1690)
//    and leaves the row untouched — same failure shape as
//    GoodsRepository::takeOne, pinned by the integration tier. An
//    increase clamps at the column maximum only via the caller's
//    MAX_MONEY (2,000,000,000) cap.
//  - The delta is a Gold_t (DWORD) marshalled through %u, exactly as the
//    call sites always did.
//  - Every operation targets ONLY the character's own race table (unlike
//    the stash writes, which fan out to Slayer + the race's own table);
//    WHERE uses uppercase NAME in the two relative writes and the
//    integrity read, and mixed-case Name in the clamped one —
//    byte-for-byte the inline queries, and identical to MySQL either way.
//  - decreaseGoldClamped keeps its spacing quirks too: the space after
//    IF, the one before the comma in "Gold ," and the one before the
//    closing paren.
//  - An UPDATE for a name with no row matches zero rows, silently.
//  - Character names are interpolated raw (no escaping), as the call
//    sites always did.
class MySQLGoldRepository : public GoldRepository {
public:
    void increaseGold(const string& ownerName, CharacterRace race, Gold_t delta) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE %s SET Gold=Gold+%u WHERE NAME='%s'", characterRaceTable(race), delta,
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void decreaseGold(const string& ownerName, CharacterRace race, Gold_t delta) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE %s SET Gold=Gold-%u WHERE NAME='%s'", characterRaceTable(race), delta,
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void decreaseGoldClamped(const string& ownerName, CharacterRace race, Gold_t fee) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE %s SET Gold = IF (%u > Gold , 0, Gold - %u ) WHERE Name = '%s'",
                                characterRaceTable(race), fee, fee, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadGold(const string& ownerName, CharacterRace race, int& gold) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Gold FROM %s WHERE NAME='%s'", characterRaceTable(race), ownerName.c_str());

            if (pResult->next()) {
                gold = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

GoldRepository& defaultGoldRepository() {
    static MySQLGoldRepository instance;
    return instance;
}
