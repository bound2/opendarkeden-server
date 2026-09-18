//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionSimpleQuestComplete.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_SIMPLE_QUEST_COMPLETE_H__
#define __ACTION_SIMPLE_QUEST_COMPLETE_H__

#include "Action.h"
#include "ActionFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionSimpleQuestComplete
//////////////////////////////////////////////////////////////////////////////

class ActionSimpleQuestComplete : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_SIMPLE_QUEST_COMPLETE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
    ScriptID_t getScriptID() const {
        return m_ScriptID;
    }
    void setScriptID(ScriptID_t scriptID) {
        m_ScriptID = scriptID;
    }

private:
    ScriptID_t m_ScriptID; // ID of the script line to say
};


//////////////////////////////////////////////////////////////////////////////
// class ActionSimpleQuestCompleteFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionSimpleQuestCompleteFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_SIMPLE_QUEST_COMPLETE;
    }
    virtual string getActionName() const {
        return "SimpleQuestComplete";
    }
    virtual Action* createAction() const {
        return new ActionSimpleQuestComplete();
    }
};

#endif
