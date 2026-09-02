#include "DB.h"
#include "repository/PlayRecordRepository.h"

namespace {

// MySQL implementation of the play-record seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original: the saved
//    quest DELETE quotes its numeric key ("QuestID='%u'"), the REPLACE
//    writes the save time SQL-side (now()) and so does the head-count
//    INSERT, and the score read is "LIMIT 1" with no ORDER BY — whichever
//    row the optimizer hands back first, not a top score.
//  - The saved-quest load computes the save's age in SQL
//    (unix_timestamp(now()) - unix_timestamp(Time)) and the caller reads
//    it through getInt, as before.
//  - The writes stream a DWORD quest id and a BYTE status through "%u"
//    (promoted), and BYTE levels and a uint count through "%u" — the same
//    conversions the callers had.
//  - Names are interpolated raw, as before.
class MySQLPlayRecordRepository : public PlayRecordRepository {
public:
    vector<SavedQuestRow> loadSavedQuests(const string& owner) {
        vector<SavedQuestRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT QuestID, Status, unix_timestamp(now()) - "
                                                  "unix_timestamp(Time) FROM GQuestSave WHERE OwnerID='%s'",
                                                  owner.c_str());

            while (pResult->next()) {
                SavedQuestRow row;
                row.questID = pResult->getInt(1);
                row.status = pResult->getInt(2);
                row.secondsSinceSave = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void replaceSavedQuest(DWORD questID, const string& owner, BYTE status) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("REPLACE INTO GQuestSave (QuestID, OwnerID, Time, Status) VALUES "
                                "(%u, '%s', now(), %u)",
                                questID, owner.c_str(), status);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteSavedQuest(const string& owner, DWORD questID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM GQuestSave WHERE OwnerID='%s' AND QuestID='%u'", owner.c_str(), questID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertHeadCount(const string& name, Level_t firstLevel, Level_t lastLevel, uint count) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO HeadCount (Name, Time, FirstLevel, LastLevel, HeadCount) VALUES ('%s', now(), %u, %u, %u)",
                name.c_str(), firstLevel, lastLevel, count);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadMiniGameScore(BYTE gameType, BYTE level, string& name, int& score) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT Name, Score FROM MiniGameScores WHERE Type=%u AND Level=%u LIMIT 1", gameType, level);

            if (pResult->next()) {
                name = pResult->getString(1);
                score = pResult->getInt(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

PlayRecordRepository& defaultPlayRecordRepository() {
    static MySQLPlayRecordRepository instance;
    return instance;
}
