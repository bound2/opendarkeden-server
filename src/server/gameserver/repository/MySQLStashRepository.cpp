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
//    sites always did: a stash holding more than 2^31-1 gold would be
//    stored as a negative number (unreachable with current gold caps,
//    preserved anyway).
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
};

} // namespace

StashRepository& defaultStashRepository() {
    static MySQLStashRepository instance;
    return instance;
}
