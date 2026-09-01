#include "DB.h"
#include "repository/ComebackEventRepository.h"

namespace {

// MySQL implementation of the comeback-event seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The three SELECTs are byte-for-byte the Zone.cpp originals; the
//    zero-date comparisons ('0000-00-00') are why the production
//    sql_mode drops NO_ZERO_DATE.
//  - getDistConnection("PLAYER_DB") IGNORES its name argument — it is
//    the thread's second connection to the same DARKEDEN schema. Kept,
//    as the original used it.
//  - Zone.cpp ran the three queries on ONE statement inside ONE
//    BEGIN_DB, sending a dialog packet between them, and never freed
//    the statement. Three calls now, each freeing its statement; a
//    failure in the second or third still escapes after the earlier
//    dialogs were sent, exactly as before.
//  - The account id is interpolated raw, as before.
class MySQLComebackEventRepository : public ComebackEventRepository {
public:
    bool hasUnclaimedItem(const string& playerID) {
        return exists("SELECT PlayerID FROM Event200501Main WHERE PlayerID = '%s' AND RecvItemDate = '0000-00-00'",
                      playerID);
    }

    bool hasUnclaimedPremiumItem(const string& playerID) {
        return exists("SELECT PlayerID FROM Event200501Main WHERE PlayerID = '%s' AND "
                      "PayPremiumDate <> '0000-00-00' AND RecvPremiumItemDate = '0000-00-00'",
                      playerID);
    }

    bool hasUnclaimedRecommendItem(const string& playerID) {
        return exists("SELECT PlayerID FROM Event200501Recommend WHERE PlayerID = '%s' AND RecvItemDate = '0000-00-00'",
                      playerID);
    }

private:
    static bool exists(const char* format, const string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(format, playerID.c_str());
            found = pResult->next();
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

ComebackEventRepository& defaultComebackEventRepository() {
    static MySQLComebackEventRepository instance;
    return instance;
}
