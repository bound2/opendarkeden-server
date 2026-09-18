//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionGiveFinalLottoScript.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_GIVE_FINAL_LOTTO_SCRIPT_H__
#define __ACTION_GIVE_FINAL_LOTTO_SCRIPT_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionGiveFinalLottoScript
//////////////////////////////////////////////////////////////////////////////

class ActionGiveFinalLottoScript : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_GIVE_FINAL_LOTTO_SCRIPT;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
    ScriptID_t m_SuccessScriptID;
    ScriptID_t m_FailScriptID;
    ScriptID_t m_CounterScriptID;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionGiveFinalLottoScriptFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionGiveFinalLottoScriptFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_GIVE_FINAL_LOTTO_SCRIPT;
    }
    virtual string getActionName() const {
        return "GiveFinalLottoScript";
    }
    virtual Action* createAction() const {
        return new ActionGiveFinalLottoScript();
    }
};
#endif
