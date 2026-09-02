#include "DB.h"
#include "repository/FlagSetRepository.h"

namespace {

// MySQL implementation of the FlagSet seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - create() and save() and destroy() were StringStream-built; the
//    format strings below carry the same bytes (the two spaces before
//    the closing parenthesis of the INSERT included), following the
//    repository rule that SQL is parameterized, never concatenated.
//  - OwnerID is the PRIMARY KEY: insert() raises ER_DUP_ENTRY (1062)
//    for an owner that already has a row; insertEmptyIfMissing() is
//    the INSERT IGNORE form and is a silent no-op then. Pinned by the
//    integration tier.
//  - FlagData is varchar(24) and nullable; load() returns the text
//    through getString, which turns a NULL into "" (Result::getString's
//    NULL guard) — the FlagSet decoder then treats every bit as off.
//  - Names are interpolated raw, as before.
class MySQLFlagSetRepository : public FlagSetRepository {
public:
    void insert(const string& ownerName, const string& flagData) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO FlagSet (OwnerID, FlagData) VALUES ('%s', '%s'  )", ownerName.c_str(),
                                flagData.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertEmptyIfMissing(const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT IGNORE INTO FlagSet (OwnerID, FlagData) VALUES ('%s','')", ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool load(const string& ownerName, string& flagData) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT FlagData FROM FlagSet WHERE OwnerID = '%s'", ownerName.c_str());

            if (pResult->next()) {
                flagData = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void update(const string& ownerName, const string& flagData) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE FlagSet SET FlagData='%s' WHERE OwnerID='%s'", flagData.c_str(),
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void remove(const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM FlagSet WHERE OwnerID = '%s'", ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

FlagSetRepository& defaultFlagSetRepository() {
    static MySQLFlagSetRepository instance;
    return instance;
}
