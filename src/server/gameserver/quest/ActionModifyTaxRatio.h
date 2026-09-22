//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionModifyTaxRatio.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_MODIFY_TAX_RATIO_H__
#define __ACTION_MODIFY_TAX_RATIO_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionModifyTaxRatio
//////////////////////////////////////////////////////////////////////////////

class ActionModifyTaxRatio : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_MODIFY_TAX_RATIO;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionModifyTaxRatioFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionModifyTaxRatioFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_MODIFY_TAX_RATIO;
    }
    virtual string getActionName() const {
        return "ModifyTaxRatio";
    }
    virtual Action* createAction() const {
        return new ActionModifyTaxRatio();
    }
};
#endif
