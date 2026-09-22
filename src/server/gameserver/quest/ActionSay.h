//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionSay.h
// Written By  :
// Description :
// The creature speaks the given script line. The line is shown in a speech
// bubble above the creature's head.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_SAY_H__
#define __ACTION_SAY_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionSay;
//////////////////////////////////////////////////////////////////////////////

class ActionSay : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_SAY;
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
// class ActionSayFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionSayFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_SAY;
    }
    virtual string getActionName() const {
        return "Say";
    }
    virtual Action* createAction() const {
        return new ActionSay();
    }
};

#endif
