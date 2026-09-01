#ifndef __PLAY_RECORD_REPOSITORY_H__
#define __PLAY_RECORD_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Seam for the per-player play records (task 3.2): a player's saved
// quest states (GQuestSave — loaded at login, REPLACEd on every status
// change, deleted when a quest is erased), the head-count log a
// half-hourly event writes (HeadCount) and the minigame score board a
// packet reads (MiniGameScores). Reads are typed to the driver getter
// the inline code called (getInt → int, getString → std::string); the
// writes' parameters to the members/expressions each caller streamed,
// so the varargs bytes reaching the format strings are unchanged.
//
// Not enclosed: the character-deletion sweeps of GQuestSave in
// CreatureUtil.cpp and the loginserver's CLDeletePCHandler.cpp; the
// MiniGameScores writers and readers in CGSubmitScoreHandler,
// CGSayHandler and mission/MiniGameQuestStatus.cpp; and the TradeLog
// inserts (TradeManager, CGBuyStoreItemHandler), which concatenate an
// unbounded trade summary that executeQuery's 2048-byte format buffer
// could not carry — they wait for a DB-layer change.

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
    // GQuestStatus::save — (m_QuestID, owner, m_Status) as streamed.
    virtual void replaceSavedQuest(DWORD questID, const std::string& owner, BYTE status) = 0;
    // GQuestManager::eraseQuest — the id goes through a quoted '%u'.
    virtual void deleteSavedQuest(const std::string& owner, DWORD questID) = 0;

    // --- head-count log (EventHeadCount::activate) -------------------------
    virtual void insertHeadCount(const std::string& name, Level_t firstLevel, Level_t lastLevel, uint count) = 0;

    // --- minigame score board (sendGCMiniGameScores) --------------------------
    // The first row LIMIT 1 happens to return for a type and level — there
    // is no ORDER BY, so "first" is the optimizer's choice. False when none.
    virtual bool loadMiniGameScore(BYTE gameType, BYTE level, std::string& name, int& score) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLPlayRecordRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
PlayRecordRepository& defaultPlayRecordRepository();

#endif
