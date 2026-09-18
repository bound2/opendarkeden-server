//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionCheckPartnerName.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_CHECK_PARTNER_NAME_H__
#define __ACTION_CHECK_PARTNER_NAME_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionCheckPartnerName
//////////////////////////////////////////////////////////////////////////////

class ActionCheckPartnerName : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_CHECK_PARTNER_NAME;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionCheckPartnerNameFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionCheckPartnerNameFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_CHECK_PARTNER_NAME;
    }
    virtual string getActionName() const {
        return "CheckPartnerName";
    }
    virtual Action* createAction() const {
        return new ActionCheckPartnerName();
    }
};
#endif
