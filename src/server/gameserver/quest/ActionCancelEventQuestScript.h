//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionCancelEventQuestScript.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_CANCEL_EVENT_QUEST_SCRIPT_H__
#define __ACTION_CANCEL_EVENT_QUEST_SCRIPT_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionCancelEventQuestScript
//////////////////////////////////////////////////////////////////////////////

class ActionCancelEventQuestScript : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_CANCEL_EVENT_QUEST_SCRIPT;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
    ScriptID_t getScriptID(int index) const {
        return m_ScriptID[index];
    }
    void setScriptID(int index, ScriptID_t scriptID) {
        m_ScriptID[index] = scriptID;
    }

private:
    ScriptID_t m_ScriptID[5]; // IDs of the script lines to say
    ScriptID_t m_CounterScriptID;
    ScriptID_t m_SuccessScriptID;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionCancelEventQuestScriptFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionCancelEventQuestScriptFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_CANCEL_EVENT_QUEST_SCRIPT;
    }
    virtual string getActionName() const {
        return "CancelEventQuestScript";
    }
    virtual Action* createAction() const {
        return new ActionCancelEventQuestScript();
    }
};
#endif
