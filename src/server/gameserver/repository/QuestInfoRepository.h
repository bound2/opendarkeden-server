#ifndef __QUEST_INFO_REPOSITORY_H__
#define __QUEST_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the quest-info, quest-reward, event-quest-advance and
// event-quest-looting tables (task 3.2): what the mission/ managers read when an
// NPC is created or a player logs in, plus the one row EventQuestAdvance writes.
// Every statement here is a copy of its mission/*.cpp original, byte for byte,
// whitespace and all; the legacy quirks are quarantined HERE, per
// docs/RESTRUCTURING.md 3.2.
//
// The catalogues come in pairs. SimpleQuestInfoManager reads the nine columns of
// MonsterKillQuestInfo for one NPC; EventQuestInfoManager reads the same table and
// three sisters (GatherItemQuestInfo, MeetNPCQuestInfo, MiniGameQuestInfo) with
// EventQuest and QuestLevel appended — so a row type per shape and an event row
// wrapping it. SimpleQuestRewardManager reads ItemRewardInfo and
// SlayerWeaponRewardInfo, whose six columns are identical, into one row type.
//
// Reads are typed to the driver getter the inline code called: every numeric
// column came back through getInt — the callers cast into their own typedefs and
// turn EventQuest and IsChief into bool with a `!= 0` (or `== 0 ? false : true`)
// of their own — and OptionType through getString. The one non-column expression,
// EventQuestLootingInfo's "LootingType-1", stays in the SELECT.
//
// Not enclosed: the five mission/ files whose only executeQuery text sits inside
// commented-out blocks (QuestInfoManager, RewardClassInfoManager, ItemRewardInfo,
// EventQuestRewardManager, MiniGameQuestStatus). They hold no live statement, so
// there is nothing to move; ratchet R3 keeps counting them until the dead text
// goes, which is a separate decision.

// The six columns every quest-info SELECT starts with.
struct QuestHeadRow {
    int questID;
    int race;
    int maxGrade;
    int minGrade;
    int timeLimitSec;
    int rewardClass;
};

// MonsterKillQuestInfo: the head plus TargetSType, IsChief, Goal.
struct MonsterKillQuestRow {
    QuestHeadRow head;
    int targetSType;
    int isChief;
    int goal;
};

// GatherItemQuestInfo: the head plus TargetIClass, TargetIType, Goal.
struct GatherItemQuestRow {
    QuestHeadRow head;
    int targetIClass;
    int targetIType;
    int goal;
};

// MeetNPCQuestInfo: the head plus TargetNPCID, SecondNPCID.
struct MeetNPCQuestRow {
    QuestHeadRow head;
    int targetNPCID;
    int secondNPCID;
};

// MiniGameQuestInfo: the head plus GameType.
struct MiniGameQuestRow {
    QuestHeadRow head;
    int gameType;
};

// The event catalogues are those four shapes with EventQuest and QuestLevel
// appended, in that order.
struct EventMonsterKillQuestRow {
    MonsterKillQuestRow quest;
    int eventQuest;
    int questLevel;
};

struct EventGatherItemQuestRow {
    GatherItemQuestRow quest;
    int eventQuest;
    int questLevel;
};

struct EventMeetNPCQuestRow {
    MeetNPCQuestRow quest;
    int eventQuest;
    int questLevel;
};

struct EventMiniGameQuestRow {
    MiniGameQuestRow quest;
    int eventQuest;
    int questLevel;
};

// ItemRewardInfo and SlayerWeaponRewardInfo: the same six columns in both.
struct ItemRewardRow {
    int rewardClass;
    int rewardID;
    int itemClass;
    int itemType;
    std::string optionType;
    int timeLimitSec;
};

// EventQuestAdvance, one row per (owner, quest level).
struct EventQuestAdvanceRow {
    int questLevel;
    int status;
};

// EventQuestLootingInfo: ten columns, the second of them the SELECT's own
// LootingType-1.
struct EventQuestLootingRow {
    int questLevel;
    int lootingType; // LootingType-1, as the SELECT computes it
    int lootingZone;
    int lootingMType;
    int lootingIClass;
    int lootingITypeMin;
    int lootingITypeMax;
    int race;
    int minGrade;
    int maxGrade;
};

class QuestInfoRepository {
public:
    virtual ~QuestInfoRepository() {}

    // SimpleQuestInfoManager::load — the nine-column catalogue for one NPC.
    virtual std::vector<MonsterKillQuestRow> loadMonsterKillQuestsOfNPC(const std::string& npcName) = 0;

    // SimpleQuestRewardManager::load — the two reward tables, one NPC each.
    virtual std::vector<ItemRewardRow> loadItemRewardsOfNPC(const std::string& npcName) = 0;
    virtual std::vector<ItemRewardRow> loadSlayerWeaponRewardsOfNPC(const std::string& npcName) = 0;

    // EventQuestInfoManager::load — the four catalogues with EventQuest and
    // QuestLevel, in the order it reads them.
    virtual std::vector<EventMonsterKillQuestRow> loadEventMonsterKillQuestsOfNPC(const std::string& npcName) = 0;
    virtual std::vector<EventGatherItemQuestRow> loadEventGatherItemQuestsOfNPC(const std::string& npcName) = 0;
    virtual std::vector<EventMeetNPCQuestRow> loadEventMeetNPCQuestsOfNPC(const std::string& npcName) = 0;
    virtual std::vector<EventMiniGameQuestRow> loadEventMiniGameQuestsOfNPC(const std::string& npcName) = 0;

    // EventQuestAdvance::save — the UPDATE, false when no row went (the caller
    // then runs the INSERT IGNORE, as it did).
    virtual bool updateEventQuestAdvance(uint status, const std::string& ownerName, uint questLevel) = 0;
    virtual void insertEventQuestAdvance(uint questLevel, const std::string& ownerName, uint status) = 0;

    // EventQuestAdvanceManager::load — every advance row the owner has.
    virtual std::vector<EventQuestAdvanceRow> loadEventQuestAdvances(const std::string& ownerName) = 0;

    // EventQuestLootingManager::load — the whole catalogue; the original built no
    // arguments, so this one keeps executeQueryString.
    virtual std::vector<EventQuestLootingRow> loadEventQuestLootingInfos() = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLQuestInfoRepository.cpp.
// An accessor function rather than a g_p* extern: ratchet R1 counts those.
QuestInfoRepository& defaultQuestInfoRepository();

#endif
