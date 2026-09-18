//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionGiveEventQuestScript.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_GIVE_EVENT_QUEST_SCRIPT_H__
#define __ACTION_GIVE_EVENT_QUEST_SCRIPT_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionGiveEventQuestScript
//////////////////////////////////////////////////////////////////////////////

class ActionGiveEventQuestScript : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_GIVE_EVENT_QUEST_SCRIPT;
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
    ScriptID_t m_NotComplete;
    ScriptID_t m_Success;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionGiveEventQuestScriptFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionGiveEventQuestScriptFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_GIVE_EVENT_QUEST_SCRIPT;
    }
    virtual string getActionName() const {
        return "GiveEventQuestScript";
    }
    virtual Action* createAction() const {
        return new ActionGiveEventQuestScript();
    }
};
#endif
