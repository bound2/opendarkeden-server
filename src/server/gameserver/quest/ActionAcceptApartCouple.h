//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionAcceptApartCouple.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_ACCEPT_APART_COUPLE_H__
#define __ACTION_ACCEPT_APART_COUPLE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionAcceptApartCouple
//////////////////////////////////////////////////////////////////////////////

class ActionAcceptApartCouple : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_ACCEPT_APART_COUPLE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
};


//////////////////////////////////////////////////////////////////////////////
// class ActionAcceptApartCoupleFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionAcceptApartCoupleFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_ACCEPT_APART_COUPLE;
    }
    virtual string getActionName() const {
        return "AcceptApartCouple";
    }
    virtual Action* createAction() const {
        return new ActionAcceptApartCouple();
    }
};
#endif
