#ifndef __PLAY_RECORD_REPOSITORY_H__
#define __PLAY_RECORD_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Per-player play records: a player's saved quest states (GQuestSave —
// loaded at login, REPLACEd on every status change, deleted when a quest
// is erased), the head-count log a half-hourly event writes (HeadCount),
// the minigame score board (MiniGameScores), the trade log (TradeLog —
// store purchases and player-to-player trades) and the per-account event
// tallies (GoldMedalCount, EventLotto, UnderworldEvent). Reads are typed
// to the driver getter used for each column (getInt → int, getString →
// std::string).
//
// Connections: the event tallies go through the thread's dist connection
// (DatabaseManager ignores the name asked for and hands back the second
// per-thread socket to the same DARKEDEN schema); everything else through
// the DARKEDEN connection.

// One GQuestSave row for an owner, plus the server-side age of the save
// (unix_timestamp(now()) - unix_timestamp(Time)).
struct SavedQuestRow {
    int questID;
    int status;
    int secondsSinceSave;
};

class PlayRecordRepository {
public:
    virtual ~PlayRecordRepository() {}

    // --- saved quests (GQuestManager / GQuestStatus) -----------------------
    virtual std::vector<SavedQuestRow> loadSavedQuests(const std::string& owner) = 0;
    virtual void replaceSavedQuest(DWORD questID, const std::string& owner, BYTE status) = 0;
    // The id goes through a quoted '%u'.
    virtual void deleteSavedQuest(const std::string& owner, DWORD questID) = 0;

    // --- head-count log (EventHeadCount::activate) -------------------------
    virtual void insertHeadCount(const std::string& name, Level_t firstLevel, Level_t lastLevel, uint count) = 0;

    // --- minigame score board (sendGCMiniGameScores) --------------------------
    // The first row LIMIT 1 happens to return for a type and level — there
    // is no ORDER BY, so "first" is the optimizer's choice. False when none.
    virtual bool loadMiniGameScore(BYTE gameType, BYTE level, std::string& name, int& score) = 0;
    // UPDATE ... WHERE Type AND Level AND Score>score LIMIT 1: with no ORDER
    // BY, which of several beatable rows is overwritten is the optimizer's
    // choice, and the statement never INSERTs, so a (type, level) with no
    // seeded row never records a score.
    virtual void recordMiniGameScore(const std::string& name, WORD score, BYTE gameType, BYTE level) = 0;

    // --- trade log (CGBuyStoreItemHandler) ----------------------------------
    // The store-purchase TradeLog row. The Content column's text is
    // "Store:[<name>(<account>)]\n<item>\n----\nBuy:[<name>(<account>)]
    // \nGOLD:<price>\n", so the store's and the buyer's names appear in it a
    // second time after their Name1/Name2 columns. Every text is
    // interpolated unescaped, and the whole statement must fit
    // executeQuery's 2048-byte format buffer (one item's toString() does).
    virtual void logStoreTrade(const std::string& timeline, const std::string& storeName, const std::string& storeHost,
                               const std::string& storeAccountID, const std::string& buyerName,
                               const std::string& buyerHost, const std::string& buyerAccountID,
                               const std::string& itemText, Gold_t price) = 0;

    // The player-to-player TradeLog row (TradeManager::processTrade).
    // content carries both sides' name, account, gold and one line per
    // traded item, so its length is bounded only by the two inventories:
    // the statement is assembled as a string and sent through
    // executeQueryString rather than through executeQuery's 2048-byte
    // format buffer. Every text, content included, is interpolated
    // unescaped, so a quote or a backslash in an item's toString() breaks
    // the statement. timeline is the caller's own timestamp text, not
    // now().
    virtual void logPlayerTrade(const std::string& timeline, const std::string& name1, const std::string& host1,
                                const std::string& name2, const std::string& host2, const std::string& content) = 0;

    // --- event tallies (CreatureUtil) -----------------------------------------
    // INSERT INTO GoldMedalCount (PlayerID, getTime). The table is not in
    // initdb/, so on the shipped schema this throws END_DB's DatabaseError
    // every time; the caller does not catch it.
    virtual void insertGoldMedal(const std::string& playerID) = 0;
    // UPDATE EventLotto count=count+num for (player, type); REPLACE a fresh
    // row when that changed nothing; then read the count back, all on one
    // Statement. True with the count when the read-back answered (it always
    // does after the REPLACE).
    virtual bool addLotto(const std::string& playerID, BYTE type, uint num, int& count) = 0;
    // INSERT INTO UnderworldEvent (WorldID, ServerID, PlayerID, CharacterID,
    // KillTime=now()). Its one caller sits under __UNDERWORLD__, which no
    // build defines.
    virtual void insertUnderworldKill(int worldID, int serverID, const std::string& playerID,
                                      const std::string& characterName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLPlayRecordRepository.cpp.
PlayRecordRepository& defaultPlayRecordRepository();

#endif
