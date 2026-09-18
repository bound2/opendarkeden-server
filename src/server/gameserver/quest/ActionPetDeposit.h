//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionPetDeposit.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_PET_DEPOSIT_H__
#define __ACTION_PET_DEPOSIT_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionPetDeposit
//////////////////////////////////////////////////////////////////////////////

class ActionPetDeposit : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_PET_DEPOSIT;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionPetDepositFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionPetDepositFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_PET_DEPOSIT;
    }
    virtual string getActionName() const {
        return "PetDeposit";
    }
    virtual Action* createAction() const {
        return new ActionPetDeposit();
    }
};
#endif
