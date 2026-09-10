#include "DB.h"
#include "StringStream.h"
#include "repository/PlayRecordRepository.h"

namespace {

// MySQL implementation of PlayRecordRepository.
//  - The saved-quest DELETE quotes its numeric key ("QuestID='%u'"); the
//    REPLACE and the head-count INSERT stamp their time SQL-side (now());
//    the score read is "LIMIT 1" with no ORDER BY — whichever row the
//    optimizer hands back first, not a top score.
//  - The saved-quest load computes the save's age in SQL
//    (unix_timestamp(now()) - unix_timestamp(Time)), read through getInt.
//  - Names are interpolated raw.
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

    void recordMiniGameScore(const string& name, WORD score, BYTE gameType, BYTE level) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE MiniGameScores SET Name='%s', Score=%u, Time=now() WHERE Type=%u AND "
                                "Level=%u AND Score>%u LIMIT 1",
                                name.c_str(), score, gameType, level, score);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void logStoreTrade(const string& timeline, const string& storeName, const string& storeHost,
                       const string& storeAccountID, const string& buyerName, const string& buyerHost,
                       const string& buyerAccountID, const string& itemText, Gold_t price) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO TradeLog (Timeline, Name1, IP1, Name2, IP2, Content) VALUES ('%s', '%s', "
                                "'%s', '%s', '%s', 'Store:[%s(%s)]\n%s\n----\nBuy:[%s(%s)]\nGOLD:%u\n')",
                                timeline.c_str(), storeName.c_str(), storeHost.c_str(), buyerName.c_str(),
                                buyerHost.c_str(), storeName.c_str(), storeAccountID.c_str(), itemText.c_str(),
                                buyerName.c_str(), buyerAccountID.c_str(), price);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    // The player-trade row is assembled rather than formatted: content is
    // as long as the two inventories make it, and executeQuery's format
    // buffer holds 2048 bytes.
    void logPlayerTrade(const string& timeline, const string& name1, const string& host1, const string& name2,
                        const string& host2, const string& content) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            StringStream SQL;
            SQL << "INSERT INTO TradeLog (Timeline, Name1, IP1, Name2, IP2, Content) VALUES (" << "'" << timeline
                << "'," << "'" << name1 << "'," << "'" << host1 << "'," << "'" << name2 << "'," << "'" << host2 << "',"
                << "'" << content << "'" << ")";

            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQueryString(SQL.toString());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    // The CreatureUtil event tallies (see the header): the dist connection
    // under names DatabaseManager ignores.
    void insertGoldMedal(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("USERINFO")->createStatement();
            pStmt->executeQuery("INSERT INTO GoldMedalCount (PlayerID, getTime) VALUES ('%s', now())",
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool addLotto(const string& playerID, BYTE type, uint num, int& count) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("USERINFO")->createStatement();
            pStmt->executeQuery("UPDATE EventLotto SET count=count+%u WHERE PlayerID='%s' AND Type=%u", num,
                                playerID.c_str(), type);

            if (pStmt->getAffectedRowCount() < 1) {
                pStmt->executeQuery("REPLACE INTO EventLotto (PlayerID,Type,count) VALUES ('%s',%u,%u)",
                                    playerID.c_str(), type, num);
            }

            Result* pResult = pStmt->executeQuery("SELECT count FROM EventLotto WHERE PlayerID='%s' AND Type=%u",
                                                  playerID.c_str(), type);

            if (pResult->next()) {
                count = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertUnderworldKill(int worldID, int serverID, const string& playerID, const string& characterName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("INSERT INTO UnderworldEvent (WorldID, ServerID, PlayerID, CharacterID, KillTime) "
                                "VALUES (%u, %u, '%s', '%s', now())",
                                worldID, serverID, playerID.c_str(), characterName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

PlayRecordRepository& defaultPlayRecordRepository() {
    static MySQLPlayRecordRepository instance;
    return instance;
}
