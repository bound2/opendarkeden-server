////////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionAttrComp.h
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __ATTR_COMP_H__
#define __ATTR_COMP_H__

#include "Condition.h"
#include "ConditionFactory.h"
#include "Creature.h"

enum {
    CONDITION_ATTR_COMP_LOWER = 0,
    CONDITION_ATTR_COMP_EQUAL,
    CONDITION_ATTR_COMP_HIGHER,

    CONDITION_ATTR_COMP_MAX
};

const string ConditionAttrCompOperator2String[] = {"LOWER", "EQUAL", "HIGHER",

                                                   "MAX"};

//////////////////////////////////////////////////////////////////////////////
// class ConditionAttrComp;
//////////////////////////////////////////////////////////////////////////////

class ConditionAttrComp : public Condition {
public:
    ConditionAttrComp();
    virtual ~ConditionAttrComp();

public:
    virtual ConditionType_t getConditionType() const {
        return CONDITION_ATTR_COMP;
    }
    virtual bool isPassive() const {
        return true;
    }
    virtual bool isSatisfied(Creature* pNPC, Creature* pPC = NULL, void* pParam = NULL) const;
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual string toString() const;

public:
    void parseOperator(string& op, uint& var);
    bool satisfy(uint op, uint current, uint restriction) const;

private:
    uint m_ReqSTR;   // Required STR
    uint m_ReqDEX;   // Required DEX
    uint m_ReqINT;   // Required INT
    uint m_ReqSum;   // Required sum of the stats
    uint m_ReqLevel; // Required level

    uint m_STROperator;   // STR operator
    uint m_DEXOperator;   // DEX operator
    uint m_INTOperator;   // INT operator
    uint m_SumOperator;   // SUM operator
    uint m_LevelOperator; // LEVEL operator
};

//////////////////////////////////////////////////////////////////////////////
// class ConditionAttrCompFactory;
//////////////////////////////////////////////////////////////////////////////

class ConditionAttrCompFactory : public ConditionFactory {
public:
    virtual ConditionType_t getConditionType() const {
        return Condition::CONDITION_ATTR_COMP;
    }
    virtual Condition* createCondition() const {
        return new ConditionAttrComp();
    }
    virtual string getConditionName() const {
        return "AttrComp";
    }
};

#endif
