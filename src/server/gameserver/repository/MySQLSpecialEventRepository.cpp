#include "DB.h"
#include "Thread.h"
#include "repository/SpecialEventRepository.h"

namespace {

// MySQL implementation of SpecialEventRepository. Quirks:
//  - Both statements ask DatabaseManager for the connection through the
//    int overload with the thread id. That resolves to the WorldDBInfo
//    row-0 connection, not the thread's DARKEDEN one; see the header.
//  - The read tests next(): one row per Name (the primary key).
class MySQLSpecialEventRepository : public SpecialEventRepository {
public:
    bool loadCount(const string& accountID, int& count) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection((int)(long)Thread::self())->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Count FROM SpecialEvent WHERE Name='%s'", accountID.c_str());

            if (pResult->next()) {
                count = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void resetCount(const string& accountID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection((int)(long)Thread::self())->createStatement();
            pStmt->executeQuery("UPDATE SpecialEvent SET Count = 0 WHERE Name='%s'", accountID.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

SpecialEventRepository& defaultSpecialEventRepository() {
    static MySQLSpecialEventRepository instance;
    return instance;
}
