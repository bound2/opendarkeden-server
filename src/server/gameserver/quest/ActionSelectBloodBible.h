//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionSelectBloodBible.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_SELECT_BLOOD_BIBLE_H__
#define __ACTION_SELECT_BLOOD_BIBLE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionSelectBloodBible
//////////////////////////////////////////////////////////////////////////////

class ActionSelectBloodBible : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_SELECT_BLOOD_BIBLE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
};


//////////////////////////////////////////////////////////////////////////////
// class ActionSelectBloodBibleFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionSelectBloodBibleFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_SELECT_BLOOD_BIBLE;
    }
    virtual string getActionName() const {
        return "SelectBloodBible";
    }
    virtual Action* createAction() const {
        return new ActionSelectBloodBible();
    }
};
#endif
