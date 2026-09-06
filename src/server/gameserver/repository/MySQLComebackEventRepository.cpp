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
//  - The event-handler round's statements (2026-09-06) are byte-for-byte
//    CGGetEventItemHandler's and CGDonationMoneyHandler's, on the same
//    dist connection. Each handler ran its statements on one Statement
//    per block and never freed it — every hand-out and every donation
//    leaked one to four Statements; each call here frees its own.
//  - The recommend stamp quotes its int ('%d'), the donation INSERTs are
//    positional — both kept as written (see the header).
class MySQLComebackEventRepository : public ComebackEventRepository {
public:
    bool loadMainRecvItemDate(const string& playerID, string& recvItemDate) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT RecvItemDate FROM Event200501Main WHERE PlayerID = '%s'", playerID.c_str());

            if (pResult->next()) {
                recvItemDate = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadMainPremiumDates(const string& playerID, string& payPremiumDate, string& recvPremiumItemDate) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PayPremiumDate, RecvPremiumItemDate FROM Event200501Main WHERE PlayerID = '%s'",
                playerID.c_str());

            if (pResult->next()) {
                payPremiumDate = pResult->getString(1);
                recvPremiumItemDate = pResult->getString(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadRecommendRow(const string& playerID, int& uniqueID, string& recvItemDate) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT UniqueID, RecvItemDate FROM Event200501Recommend WHERE PlayerID = '%s'", playerID.c_str());

            if (pResult->next()) {
                uniqueID = pResult->getInt(1);
                recvItemDate = pResult->getString(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void markMainItemReceived(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Event200501Main SET RecvItemDate = now() WHERE PlayerID = '%s'",
                                playerID.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void markMainPremiumItemReceived(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Event200501Main SET RecvPremiumItemDate = now() WHERE PlayerID = '%s'",
                                playerID.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void markRecommendItemReceived(int uniqueID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Event200501Recommend SET RecvItemDate = now() WHERE UniqueID = '%d'", uniqueID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    int countPersonalDonations(const string& name, int worldID) {
        return count("SELECT COUNT(*) FROM DonationPersonal200501 WHERE Name = '%s' AND WorldID = %d", name, worldID);
    }

    int countGuildDonations(const string& name, int worldID) {
        return count("SELECT COUNT(*) FROM DonationGuild200501 WHERE Name = '%s' AND WorldID = %d", name, worldID);
    }

    void insertPersonalDonation(const string& playerID, const string& name, int worldID, Gold_t gold) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("INSERT INTO DonationPersonal200501 VALUES ( '%s', '%s', %d, %u, now() )",
                                playerID.c_str(), name.c_str(), worldID, gold);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertGuildDonation(GuildID_t guildID, const string& guildName, const string& playerID, const string& name,
                             int worldID, Gold_t gold) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("INSERT INTO DonationGuild200501 VALUES ( %u, '%s', '%s', '%s', %d, %u, now() )",
                                guildID, guildName.c_str(), playerID.c_str(), name.c_str(), worldID, gold);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

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
    // The two COUNT(*) literals reach executeQuery through this pointer; the
    // handler assigned the count only when next() answered and kept its 0
    // otherwise — an aggregate always answers, so the branch is kept for
    // shape, not reach.
    static int count(const char* format, const string& name, int worldID) {
        int result = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(format, name.c_str(), worldID);

            if (pResult->next()) {
                result = pResult->getInt(1);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return result;
    }

    // The three predicate literals reach executeQuery through this pointer
    // rather than in place — executeQuery carries no printf format
    // attribute (see Statement.h), so nothing is lost to -Wformat.
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
