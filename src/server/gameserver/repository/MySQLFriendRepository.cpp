#include "DB.h"
#include "repository/FriendRepository.h"

namespace {

// MySQL implementation of the friend-list seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, including the
//    lower-case "and" in every WHERE, the unspaced "Owner_Name='%s'"
//    against the spaced "Owner_Name = '%s'" in the two SELECTs that scan
//    a whole roster, the unspaced "FriendHistory(HistoryMessage, ...)"
//    against the spaced "FriendList (Friend_Name, ...)", and the
//    "HistoryMessage,Friend_Name" projection with no space after its
//    comma.
//  - The two add-friend probes SELECT columns nobody reads: the callers
//    only ask whether next() returned a row. The projections are kept as
//    written rather than reduced.
//  - Names and messages are interpolated raw, as before. A message
//    containing a quote breaks its INSERT, which is the inline
//    behaviour.
//  - See FriendRepository.h: neither table exists in the shipped schema,
//    so every one of these statements raises. The bytes are still the
//    bytes.
class MySQLFriendRepository : public FriendRepository {
public:
    void insertFriend(const string& friendName, const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO FriendList (Friend_Name, Owner_Name) VALUES "
                                "('%s', '%s')",
                                friendName.c_str(), ownerName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertBlacklisted(const string& friendName, const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO FriendList (Friend_Name, Owner_Name, IsBlack) VALUES "
                                "('%s', '%s', 1)",
                                friendName.c_str(), ownerName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool friendExists(const string& ownerName, const string& friendName) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Friend_Name, IsBlack FROM FriendList WHERE Owner_Name = '%s' and Friend_Name='%s'",
                ownerName.c_str(), friendName.c_str());

            found = pResult->next();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool hasBlacklisted(const string& ownerName, const string& friendName) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT IsBlack FROM FriendList WHERE Owner_Name='%s' and Friend_Name='%s' and IsBlack=1",
                ownerName.c_str(), friendName.c_str());

            found = pResult->next();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<FriendListRow> loadFriends(const string& ownerName) {
        vector<FriendListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Friend_Name, IsBlack FROM FriendList WHERE Owner_Name = '%s'",
                                                  ownerName.c_str());

            while (pResult->next()) {
                FriendListRow row;
                row.friendName = pResult->getString(1);
                row.isBlack = pResult->getBYTE(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void deleteFriend(const string& ownerName, const string& friendName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM FriendList WHERE Owner_Name='%s' and Friend_Name='%s'", ownerName.c_str(),
                                friendName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertMessage(const string& message, const string& ownerName, const string& friendName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO FriendHistory(HistoryMessage, Owner_Name, Friend_Name) VALUES "
                                "('%s', '%s', '%s')",
                                message.c_str(), ownerName.c_str(), friendName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<FriendMessageRow> loadMessages(const string& ownerName) {
        vector<FriendMessageRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT HistoryMessage,Friend_Name FROM FriendHistory WHERE Owner_Name = '%s'", ownerName.c_str());

            while (pResult->next()) {
                FriendMessageRow row;
                row.message = pResult->getString(1);
                row.friendName = pResult->getString(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void deleteMessages(const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM FriendHistory WHERE Owner_Name='%s'", ownerName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

FriendRepository& defaultFriendRepository() {
    static MySQLFriendRepository instance;
    return instance;
}
