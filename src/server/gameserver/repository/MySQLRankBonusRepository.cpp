#include "DB.h"
#include "repository/RankBonusRepository.h"

namespace {

// MySQL implementation of the RankBonusData persistence seam. The legacy
// schema quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - OwnerID is the character *name* (varchar(10)), not a numeric id —
//    denormalized; a character rename orphans these rows.
//  - The table has NO primary or unique key, only KEY (OwnerID, Type):
//    the plain INSERT can never hit a duplicate error, and re-learning a
//    bonus that was never cleaned up stores a second identical row.
//    loadTypes() surfaces such duplicates; deleteOne() removes them all.
//  - Type is stored as int(11) and interpolated with %d, as the call
//    sites always did.
//  - Owner names are interpolated raw (no escaping), as the call sites
//    always did.
class MySQLRankBonusRepository : public RankBonusRepository {
public:
    vector<DWORD> loadTypes(const string& ownerName) {
        vector<DWORD> types;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Type FROM RankBonusData WHERE OwnerID ='%s'", ownerName.c_str());

            while (pResult->next()) {
                types.push_back(pResult->getInt(1));
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return types;
    }

    void insert(const string& ownerName, DWORD type) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO RankBonusData ( OwnerID, Type )  VALUES ( '%s', %d )", ownerName.c_str(),
                                (int)type);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteOne(const string& ownerName, DWORD type) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM RankBonusData WHERE OwnerID = '%s' AND Type = %d", ownerName.c_str(),
                                (int)type);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteAll(const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM RankBonusData WHERE OwnerID = '%s'", ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

RankBonusRepository& defaultRankBonusRepository() {
    static MySQLRankBonusRepository instance;
    return instance;
}
