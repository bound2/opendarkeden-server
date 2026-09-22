//////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionIdle.h
// Written By  :
// Description :
// Used to specify what an NPC or a monster does normally.
// When nothing else happens, the associated action runs.
// This condition must be placed at the very end of the trigger.
//////////////////////////////////////////////////////////////////////////////

#ifndef __IDLE_H__
#define __IDLE_H__

#include "Condition.h"
#include "ConditionFactory.h"
#include "Creature.h"

//////////////////////////////////////////////////////////////////////////////
// class ConditionIdle;
//////////////////////////////////////////////////////////////////////////////

class ConditionIdle : public Condition {
public:
    virtual ConditionType_t getConditionType() const {
        return CONDITION_IDLE;
    }

    virtual bool isActive() const {
        return true;
    }

    virtual bool isSatisfied(Creature* pCreature1, Creature* pCreature2 = NULL, void* pParam = NULL) const {
        return pCreature1 != NULL && !pCreature1->isPC() && pCreature2 == NULL;
    }

    virtual void read(PropertyBuffer& propertyBuffer) {}

    virtual string toString() const {
        return "ConditionIdle";
    }
};


////////////////////////////////////////////////////////////////////////////////
// class ConditionIdleFactory;
////////////////////////////////////////////////////////////////////////////////

class ConditionIdleFactory : public ConditionFactory {
public:
    virtual ConditionType_t getConditionType() const {
        return Condition::CONDITION_IDLE;
    }
    virtual Condition* createCondition() const {
        return new ConditionIdle();
    }
    virtual string getConditionName() const {
        return "Idle";
    }
};

#endif
