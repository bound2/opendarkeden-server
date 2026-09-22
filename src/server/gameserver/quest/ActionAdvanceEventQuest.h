//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionAdvanceEventQuest.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_ADVANCE_EVENT_QUEST_H__
#define __ACTION_ADVANCE_EVENT_QUEST_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionAdvanceEventQuest
//////////////////////////////////////////////////////////////////////////////

class ActionAdvanceEventQuest : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_ADVANCE_EVENT_QUEST;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
    int m_QuestLevel;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionAdvanceEventQuestFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionAdvanceEventQuestFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_ADVANCE_EVENT_QUEST;
    }
    virtual string getActionName() const {
        return "AdvanceEventQuest";
    }
    virtual Action* createAction() const {
        return new ActionAdvanceEventQuest();
    }
};
#endif
