//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterKillQuestInfo.h
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTER_KILL_QUEST_INFO_H__
#define __MONSTER_KILL_QUEST_INFO_H__

#include "Exception.h"
#include "MonsterKillQuestStatus.h"
#include "QuestInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class MonsterKillQuestInfo;
//////////////////////////////////////////////////////////////////////////////

class MonsterKillQuestInfo : public QuestInfo {
public:
    MonsterKillQuestInfo(QuestID_t questID, Race_t race, QuestGrade_t maxGrade, QuestGrade_t minGrade,
                         DWORD timeLimitSec, RewardClass_t rClass, SpriteType_t monsterType, bool isChief,
                         int killCount);
    ~MonsterKillQuestInfo();

public:
    virtual string toString() const;

    virtual MonsterKillQuestStatus* makeQuestStatus(PlayerCreature* pPC) const;
    bool isMonsterKillQuest() const {
        return true;
    }

    int getGoalNum() const {
        return m_GoalNum;
    }
    SpriteType_t getTargetMonsterSpriteType() const {
        return m_TargetMonsterSpriteType;
    }

private:
    SpriteType_t m_TargetMonsterSpriteType; // the sprite type of the monster the quest kills
    bool m_IsChief;                         // does the monster the quest kills have to be a chief?
    int m_GoalNum;                          // how many monsters the quest kills
};

#endif
