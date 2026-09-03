#include "DB.h"
#include "repository/MessageRepository.h"

namespace {

// MySQL implementation of the queued-message seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The SQL is byte-for-byte the Zone.cpp / ZonePlayerManager.cpp
//    originals, the INSERT's "( Receiver, Message ) VALUES ( '%s', '%s')"
//    spacing included (the guild handlers spell theirs differently —
//    they keep their own literals).
//  - Keyless table: a receiver can hold any number of rows, and the
//    DELETE takes them all.
//  - Zone::addPC ran the SELECT and the DELETE on ONE statement
//    inside ONE BEGIN_DB: a failing DELETE escaped after the messages
//    had already been sent. Two calls now, same visible sequence.
//  - Receiver and message text are interpolated raw, as before — the
//    message is a string-pool entry, not player input.
class MySQLMessageRepository : public MessageRepository {
public:
    vector<string> loadMessages(const string& receiver) {
        vector<string> messages;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Message FROM Messages WHERE Receiver = '%s'", receiver.c_str());

            while (pResult->next())
                messages.push_back(pResult->getString(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return messages;
    }

    void deleteMessages(const string& receiver) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM Messages WHERE Receiver = '%s'", receiver.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertUnionNotice(UnionNoticeSpelling spelling, const string& receiver, const string& message) {
        // Byte-for-byte what each handler wrote. See MessageRepository.h for
        // why the three are kept apart.
        static const char* const NOTICE_SQL[UNION_NOTICE_SPELLING_MAX] = {
            "INSERT INTO Messages (Receiver, Message) values('%s','%s')",
            "INSERT INTO `Messages` (`Receiver`, `Message`) values ('%s','%s')",
            "INSERT INTO `Messages` (`Receiver`, `Message`) values('%s','%s')",
        };

        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(NOTICE_SQL[spelling], receiver.c_str(), message.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertMessage(const string& receiver, const string& message) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO Messages ( Receiver, Message ) VALUES ( '%s', '%s')", receiver.c_str(),
                                message.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

MessageRepository& defaultMessageRepository() {
    static MySQLMessageRepository instance;
    return instance;
}
