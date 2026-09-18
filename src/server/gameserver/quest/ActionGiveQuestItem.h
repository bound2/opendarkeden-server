//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionGiveQuestItem.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_GIVE_QUEST_ITEM_H__
#define __ACTION_GIVE_QUEST_ITEM_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionGiveQuestItem
//////////////////////////////////////////////////////////////////////////////

class ActionGiveQuestItem : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_GIVE_QUEST_ITEM;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
    ScriptID_t m_CounterScriptID;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionGiveQuestItemFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionGiveQuestItemFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_GIVE_QUEST_ITEM;
    }
    virtual string getActionName() const {
        return "GiveQuestItem";
    }
    virtual Action* createAction() const {
        return new ActionGiveQuestItem();
    }
};
#endif
