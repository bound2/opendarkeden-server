//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionEnterSiege.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_ENTER_SIEGE_H__
#define __ACTION_ENTER_SIEGE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionEnterSiege
//////////////////////////////////////////////////////////////////////////////

class ActionEnterSiege : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_ENTER_SIEGE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

private:
    ZoneID_t m_ZoneID;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionEnterSiegeFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionEnterSiegeFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_ENTER_SIEGE;
    }
    virtual string getActionName() const {
        return "EnterSiege";
    }
    virtual Action* createAction() const {
        return new ActionEnterSiege();
    }
};
#endif
