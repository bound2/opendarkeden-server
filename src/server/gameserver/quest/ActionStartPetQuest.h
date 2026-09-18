//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionStartPetQuest.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_START_PET_QUEST_H__
#define __ACTION_START_PET_QUEST_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionStartPetQuest
//////////////////////////////////////////////////////////////////////////////

class ActionStartPetQuest : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_START_PET_QUEST;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionStartPetQuestFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionStartPetQuestFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_START_PET_QUEST;
    }
    virtual string getActionName() const {
        return "StartPetQuest";
    }
    virtual Action* createAction() const {
        return new ActionStartPetQuest();
    }
};
#endif
