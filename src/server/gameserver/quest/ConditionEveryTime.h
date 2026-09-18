////////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionEveryTime.h
// Written By  :
// Description :
// Runs a given action once every period.
////////////////////////////////////////////////////////////////////////////////

#ifndef __EVERY_TIME_H__
#define __EVERY_TIME_H__

#include "Condition.h"
#include "ConditionFactory.h"
#include "Creature.h"

//////////////////////////////////////////////////////////////////////////////
// class ConditionEveryTime;
//////////////////////////////////////////////////////////////////////////////

class ConditionEveryTime : public Condition {
public:
    virtual ConditionType_t getConditionType() const {
        return CONDITION_EVERY_TIME;
    }
    virtual bool isActive() const {
        return true;
    }
    virtual bool isSatisfied(Creature* pNPC, Creature* pPC = NULL, void* pParam = NULL) const;
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual string toString() const;

public:
    Turn_t getTurn() const {
        return m_Turn;
    }
    void setTurn(Turn_t turn) {
        m_Turn = turn;
    }

    Timeval& getNextTurn() {
        return m_NextTurn;
    }
    const Timeval& getNextTurn() const {
        return m_NextTurn;
    }
    void setNextTurn(Timeval tv) {
        m_NextTurn = tv;
    }

private:
    Turn_t m_Turn;

    // Next run time
    // isSatisfied() is a const method but has to change this value
    // internally, so it is marked mutable.
    mutable Timeval m_NextTurn;
};

//////////////////////////////////////////////////////////////////////////////
// class ConditionEveryTimeFactory;
//////////////////////////////////////////////////////////////////////////////

class ConditionEveryTimeFactory : public ConditionFactory {
public:
    virtual ConditionType_t getConditionType() const {
        return Condition::CONDITION_EVERY_TIME;
    }
    virtual Condition* createCondition() const {
        return new ConditionEveryTime();
    }
    virtual string getConditionName() const {
        return "EveryTime";
    }
};

#endif
