//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionQuestReward.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_QUEST_REWARD_H__
#define __ACTION_QUEST_REWARD_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionQuestReward
//////////////////////////////////////////////////////////////////////////////

class ActionQuestReward : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_QUEST_REWARD;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
};


//////////////////////////////////////////////////////////////////////////////
// class ActionQuestRewardFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionQuestRewardFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_QUEST_REWARD;
    }
    virtual string getActionName() const {
        return "QuestReward";
    }
    virtual Action* createAction() const {
        return new ActionQuestReward();
    }
};
#endif
