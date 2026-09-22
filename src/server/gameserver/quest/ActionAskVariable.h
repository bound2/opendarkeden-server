//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionAskVariable.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_ASK_VARIABLE_H__
#define __ACTION_ASK_VARIABLE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionAskVariable
//////////////////////////////////////////////////////////////////////////////

class VariableBuffer;

class ActionAskVariable : public Action {
public:
    ActionAskVariable() : m_pVariableBuffer(NULL) {}

public:
    virtual ActionType_t getActionType() const {
        return ACTION_ASK_VARIABLE;
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
    VariableBuffer* m_pVariableBuffer;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionAskVariableFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionAskVariableFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_ASK_VARIABLE;
    }
    virtual string getActionName() const {
        return "AskVariable";
    }
    virtual Action* createAction() const {
        return new ActionAskVariable();
    }
};
#endif
