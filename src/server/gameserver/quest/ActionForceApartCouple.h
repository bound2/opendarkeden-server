//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionForceApartCouple.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_FORCE_APART_COUPLE_H__
#define __ACTION_FORCE_APART_COUPLE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionForceApartCouple
//////////////////////////////////////////////////////////////////////////////

class ActionForceApartCouple : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_FORCE_APART_COUPLE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
};

//////////////////////////////////////////////////////////////////////////////
// class ActionForceApartCoupleFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionForceApartCoupleFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_FORCE_APART_COUPLE;
    }
    virtual string getActionName() const {
        return "ForceApartCouple";
    }
    virtual Action* createAction() const {
        return new ActionForceApartCouple();
    }
};
#endif
