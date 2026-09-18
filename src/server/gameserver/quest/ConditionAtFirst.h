///////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionAtFirst.h
// Written By  :
// Description :
// Actions that must run only once, right after the NPC is loaded,
// run automatically when they are tied to this condition.
//////////////////////////////////////////////////////////////////////////////

#ifndef __AT_FIRST_H__
#define __AT_FIRST_H__

#include "Condition.h"
#include "ConditionFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class ConditionAtFirst;
//////////////////////////////////////////////////////////////////////////////

class ConditionAtFirst : public Condition {
public:
    virtual ConditionType_t getConditionType() const {
        return CONDITION_AT_FIRST;
    }
    virtual bool isActive() const {
        return true;
    }
    virtual bool isSatisfied(Creature* pCreature1, Creature* pCreature2 = NULL, void* pParam = NULL) const {
        return true;
    }
    virtual void read(PropertyBuffer& propertyBuffer) {}
    virtual string toString() const {
        return "AtFirst";
    }
};

//////////////////////////////////////////////////////////////////////////////
// class ConditionAtFirstFactory;
//////////////////////////////////////////////////////////////////////////////

class ConditionAtFirstFactory : public ConditionFactory {
public:
    virtual ConditionType_t getConditionType() const {
        return Condition::CONDITION_AT_FIRST;
    }
    virtual Condition* createCondition() const {
        return new ConditionAtFirst();
    }
    virtual string getConditionName() const {
        return "AtFirst";
    }
};

#endif
