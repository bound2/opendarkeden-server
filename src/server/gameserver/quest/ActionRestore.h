//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionRestore.h
// Written By  :
// Description :
// Action that turns a player who became a Vampire back into a Slayer.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_RESTORE_H__
#define __ACTION_RESTORE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionRestore;
//////////////////////////////////////////////////////////////////////////////

class ActionRestore : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_RESTORE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

private:
};

//////////////////////////////////////////////////////////////////////////////
// class ActionRestoreFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionRestoreFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_RESTORE;
    }
    virtual Action* createAction() const {
        return new ActionRestore();
    }
    virtual string getActionName() const {
        return "Restore";
    }
};

#endif
