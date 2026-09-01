#include "DB.h"
#include "StringStream.h"
#include "repository/StashRepository.h"

namespace {

// MySQL implementation of the stash-column persistence seam. The legacy
// quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every save writes the Slayer table UNCONDITIONALLY, then Ousters if
//    the character is an Ousters, else Vampire. That is exactly what the
//    inline SQL did: slayers and vampires share one character name across
//    the Slayer+Vampire rows, ousters across Slayer+Ousters; the Slayer
//    UPDATE for a name with no Slayer row matches zero rows and is a
//    silent no-op.
//  - Gold_t is a DWORD but the value is streamed as (int), as the call
//    sites always did: a stash above 2^31-1 gold would emit a NEGATIVE
//    literal, which the UNSIGNED StashGold column then clamps to 0
//    (warning 1264) under the project's non-strict sql_mode — the
//    balance would be destroyed, not stored negative. Unreachable with
//    the current MAX_MONEY cap (2,000,000,000), preserved anyway.
//  - The integrity-check read (loadStashGold) targets ONE table — the
//    character's own — with uppercase NAME in its WHERE, byte-for-byte
//    the query the race classes ran inline.
//  - Character names are interpolated raw (no escaping), as the call
//    sites always did.
class MySQLStashRepository : public StashRepository {
public:
    void saveStashNum(const string& ownerName, bool isOusters, BYTE num) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            StringStream sqlSlayer;
            StringStream sqlVampire;
            StringStream sqlOusters;

            sqlSlayer << "UPDATE Slayer set StashNum = " << (int)num << " WHERE Name = '" << ownerName << "'";
            sqlVampire << "UPDATE Vampire set StashNum = " << (int)num << " WHERE Name = '" << ownerName << "'";
            sqlOusters << "UPDATE Ousters set StashNum = " << (int)num << " WHERE Name = '" << ownerName << "'";

            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQueryString(sqlSlayer.toString());
            if (!isOusters)
                pStmt->executeQueryString(sqlVampire.toString());
            else
                pStmt->executeQueryString(sqlOusters.toString());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveStashGold(const string& ownerName, bool isOusters, Gold_t gold) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            StringStream sqlSlayer;
            StringStream sqlVampire;
            StringStream sqlOusters;

            sqlSlayer << "UPDATE Slayer set StashGold = " << (int)gold << " WHERE Name = '" << ownerName << "'";
            sqlVampire << "UPDATE Vampire set StashGold = " << (int)gold << " WHERE Name = '" << ownerName << "'";
            sqlOusters << "UPDATE Ousters set StashGold = " << (int)gold << " WHERE Name = '" << ownerName << "'";

            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQueryString(sqlSlayer.toString());
            if (!isOusters)
                pStmt->executeQueryString(sqlVampire.toString());
            else
                pStmt->executeQueryString(sqlOusters.toString());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadStashGold(const string& ownerName, StashRace race, int& gold) {
        const char* table = race == STASH_RACE_SLAYER ? "Slayer" : race == STASH_RACE_VAMPIRE ? "Vampire" : "Ousters";
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT StashGold FROM %s WHERE NAME='%s'", table, ownerName.c_str());

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

StashRepository& defaultStashRepository() {
    static MySQLStashRepository instance;
    return instance;
}
