#include "DB.h"
#include "repository/SystemAvailabilityRepository.h"

namespace {

// MySQL implementation of SystemAvailabilityRepository.
//  - The read is "SELECT * FROM SystemAvailabilities"; columns 1 and 2
//    are read positionally, so it depends on the table's column order
//    (see the header).
//  - The delete's value is quoted although SystemKind is int(11); MySQL
//    coerces.
//  - loadAll leaks its Statement if getInt raises OutOfBoundException on
//    a short row: SAFE_DELETE sits inside the try and END_DB catches only
//    SQLQueryException. Unreachable against a two-column read of a
//    three-column table.
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

    void deleteSystemKind(int systemKind) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM SystemAvailabilities WHERE SystemKind='%d'", systemKind);

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
