#ifndef __QUEST_INFO_REPOSITORY_H__
#define __QUEST_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The quest-info, quest-reward, event-quest-advance and event-quest-looting
// tables: what the mission/ managers read when an NPC is created or a player
// logs in, plus the one row EventQuestAdvance writes.
//
// The catalogues come in pairs. SimpleQuestInfoManager reads the nine columns of
// MonsterKillQuestInfo for one NPC; EventQuestInfoManager reads the same table and
// three sisters (GatherItemQuestInfo, MeetNPCQuestInfo, MiniGameQuestInfo) with
// EventQuest and QuestLevel appended — so a row type per shape and an event row
// wrapping it. SimpleQuestRewardManager reads ItemRewardInfo and
// SlayerWeaponRewardInfo, whose six columns are identical, into one row type.
//
// Every numeric column is read through getInt — the callers cast into their own
// typedefs and turn EventQuest and IsChief into bool themselves — and OptionType
// through getString. The one non-column expression, EventQuestLootingInfo's
// "LootingType-1", is part of the SELECT.
//
// The per-owner DELETE FROM EventQuestAdvance that the character-deletion purge
// runs is CharacterPurgeRepository's.

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
    // then runs the INSERT IGNORE).
    virtual bool updateEventQuestAdvance(uint status, const std::string& ownerName, uint questLevel) = 0;
    virtual void insertEventQuestAdvance(uint questLevel, const std::string& ownerName, uint status) = 0;

    // EventQuestAdvanceManager::load — every advance row the owner has.
    virtual std::vector<EventQuestAdvanceRow> loadEventQuestAdvances(const std::string& ownerName) = 0;

    // EventQuestLootingManager::load — the whole catalogue.
    virtual std::vector<EventQuestLootingRow> loadEventQuestLootingInfos() = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLQuestInfoRepository.cpp.
QuestInfoRepository& defaultQuestInfoRepository();

#endif
