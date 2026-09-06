#include "DB.h"
#include "repository/ComebackEventRepository.h"

namespace {

// MySQL implementation of ComebackEventRepository.
//  - The zero-date comparisons ('0000-00-00') in the predicates are why
//    the production sql_mode drops NO_ZERO_DATE.
//  - getDistConnection("PLAYER_DB") IGNORES its name argument — it is
//    the thread's second connection to the same DARKEDEN schema.
//  - The zone asks the three predicates one call at a time, sending a
//    dialog packet between them; a failure in the second or third
//    escapes after the earlier dialogs were sent.
//  - The account id and names are interpolated raw.
//  - The recommend stamp quotes its int ('%d'); the donation INSERTs are
//    positional (see the header).
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

    // An aggregate always answers, so the next() branch cannot miss.
    // Written out twice rather than through a helper so that END_DB's
    // DBError.log line names the method the handler called.
    int countPersonalDonations(const string& name, int worldID) {
        int result = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT COUNT(*) FROM DonationPersonal200501 WHERE Name = '%s' AND WorldID = %d",
                                    name.c_str(), worldID);

            if (pResult->next()) {
                result = pResult->getInt(1);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return result;
    }

    int countGuildDonations(const string& name, int worldID) {
        int result = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT COUNT(*) FROM DonationGuild200501 WHERE Name = '%s' AND WorldID = %d", name.c_str(), worldID);

            if (pResult->next()) {
                result = pResult->getInt(1);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return result;
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
