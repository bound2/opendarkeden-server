//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionCancelQuest.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_CANCEL_QUEST_H__
#define __ACTION_CANCEL_QUEST_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionCancelQuest
//////////////////////////////////////////////////////////////////////////////

class ActionCancelQuest : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_CANCEL_QUEST;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
    ScriptID_t m_ScriptID;
    ScriptID_t m_CounterScriptID;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionCancelQuestFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionCancelQuestFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_CANCEL_QUEST;
    }
    virtual string getActionName() const {
        return "CancelQuest";
    }
    virtual Action* createAction() const {
        return new ActionCancelQuest();
    }
};
#endif
