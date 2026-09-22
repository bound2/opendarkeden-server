//////////////////////////////////////////////////////////////////////////////
// Filename    : Condition.h
// Written By  :
// Description :
// Class representing a condition that must hold for a trigger's action to run.
// Concrete conditions are implemented by inheriting from this class.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <bitset>

#include "Exception.h"
#include "PropertyBuffer.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Structures that carry extra parameters for checking a condition.
//////////////////////////////////////////////////////////////////////////////

typedef struct {
    ScriptID_t ScriptID;
    AnswerID_t AnswerID;
} COND_ANSWERED_BY;


////////////////////////////////////////////////////////////////////////////////
//
// class Condition;
//
////////////////////////////////////////////////////////////////////////////////
//
//* isActive() | isPassive() | isNeutral()
//
// When ZoneGroupThread processes the monsters and NPCs in a zone, it checks that
// every condition of the triggers they hold is satisfied. But monsters and NPCs
// still hold triggers with passive conditions that only mean something in a packet handler.
// A passive condition has nothing special to check in isSatisfied(), so it
// returns true in almost every case.
//
// So passive conditions need not, and must not, be checked during ZGT processing.
// isActive() and isPassive() tell the two kinds apart.
//
// Neutral conditions can be used together with both active and passive ones.
//(ex: PC_HAS_SKILL, PC_HAS_ITEM ...)
//
////////////////////////////////////////////////////////////////////////////////

class Creature;

class Condition {
public:
    enum ConditionTypes {
        // active conditions
        CONDITION_AT_FIRST,
        CONDITION_AT_TIME,
        CONDITION_FROM_TIME_TO_TIME,
        CONDITION_IDLE,
        CONDITION_EVERY_TIME,

        // passive conditions
        CONDITION_TALKED_BY,
        CONDITION_ANSWERED_BY, // The PC answered an NPC question
        CONDITION_BLOOD_DRAINED,
        CONDITION_FLAG_ON,
        CONDITION_FLAG_OFF,
        CONDITION_ATTR_COMP,
        CONDITION_RACE_COMP,
        CONDITION_SAME_CLAN,

        CONDITION_ATTACKED_BY,
        CONDITION_DIED_BY,
        CONDITION_RESCUED_BY,
        CONDITION_PC_ATTRIBUTE_COMP,
        CONDITION_PC_HAS_SKILL,
        CONDITION_PC_SKILL_COMP,
        CONDITION_PC_HAS_ITEM,
        CONDITION_PC_DONE_QUEST,
        CONDITION_PC_UNDER_QUEST,
        CONDITION_QUEST_COMPLETED,
        CONDITION_QUEST_STATE,

        // 2002.6.3 by sigi
        CONDITION_PAY_PLAY,

        // 2002.9.2 by sigi
        CONDITION_ENTER_MASTER_LAIR, // Handles the action part together with the condition check.

        // 2003.1.20 by bezz, Sequoia
        CONDITION_ENTER_CASTLE,                   // When trying to enter the castle through a portal
        CONDITION_HAS_ENOUGH_CASTLE_ENTRANCE_FEE, // Whether the castle entrance fee can be paid

        // 2003.2.18 by sigi
        CONDITION_ENTER_HOLY_LAND,      // When trying to enter Adam's holy land
        CONDITION_ENTER_CASTLE_DUNGEON, // (Adam's holy land) when trying to enter the castle dungeon map

        CONDITION_HAS_QUEST,
        CONDITION_HAS_INVEN_SPACE,

        CONDITION_CAN_ENTER_PAY_ZONE, // When entering a pay zone

        CONDITION_CAN_ENTER_BEGINNER_ZONE,  // When entering the Temerie holy land
        CONDITION_CAN_WARP_GATE,            // Can the warp gate be passed?
        CONDITION_CAN_ENTER_LEVEL_WAR_ZONE, // Can the warp gate be passed?

        CONDITION_CAN_PET_QUEST,
        CONDITION_CAN_ENTER_EVENT_ZONE,

        CONDITION_EFFECT_FLAG,        // Whether a given effect flag is on
        CONDITION_CAN_ENTER_GDR_LAIR, // Can the Gilles de Rais lair be entered?

        CONDITION_EXIST_REINFORCE, // Is there anyone who applied for siege reinforcements?

        CONDITION_SIEGE_DEFENDER_SIDE, // Is the PC on the defending side?
        CONDITION_SIEGE_ATTACKER_SIDE, // Is the PC on the attacking side?

        CONDITION_NOT_GUILD_MEMBER, // Does the PC belong to no guild?
        CONDITION_IS_GUILD_MEMBER,  // Does the PC belong to a guild?

        CONDITION_CAN_ENTER_QUEST_ZONE, // Can the quest zone be entered?

        CONDITION_MAX
    };

public:
    virtual ~Condition() {}
    virtual ConditionType_t getConditionType() const = 0;

    virtual bool isActive() const {
        return false;
    }
    virtual bool isPassive() const {
        return false;
    }
    virtual bool isNeutral() const {
        return false;
    }

    virtual bool isSatisfied(Creature* pCreature1, Creature* pCreature2 = NULL, void* pParam = NULL) const = 0;

    virtual void read(PropertyBuffer& propertyBuffer) = 0;

    virtual string toString() const = 0;
};

// Condition Set
#define ConditionSet bitset<Condition::CONDITION_MAX>

#endif
