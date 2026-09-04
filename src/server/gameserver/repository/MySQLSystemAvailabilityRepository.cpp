#include "DB.h"
#include "repository/SystemAvailabilityRepository.h"

namespace {

// MySQL implementation of the system-availability seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The read is "SELECT * FROM SystemAvailabilities", kept as written.
//    The caller reads columns 1 and 2 positionally, so it depends on the
//    table's column order; see the header.
//  - The delete's value is quoted although SystemKind is int(11), because
//    the call sites wrote it quoted. MySQL coerces, and the six
//    statements this replaces were byte-identical to what it now formats.
class MySQLSystemAvailabilityRepository : public SystemAvailabilityRepository {
public:
    vector<SystemAvailabilityRow> loadAll() {
        vector<SystemAvailabilityRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT * FROM SystemAvailabilities");

            while (pResult->next()) {
                SystemAvailabilityRow row;
                row.systemKind = pResult->getInt(1);
                row.available = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void deleteSystemKind(const char* systemKind) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM SystemAvailabilities WHERE SystemKind='%s'", systemKind);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

SystemAvailabilityRepository& defaultSystemAvailabilityRepository() {
    static MySQLSystemAvailabilityRepository instance;
    return instance;
}
