////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionDownSkill.h
// Written By  : excel96
// Description :
// Makes the client open the guild creation window through an NPC.
////////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_DOWN_SKILL_H__
#define __ACTION_DOWN_SKILL_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionDownSkill;
//////////////////////////////////////////////////////////////////////////////

class ActionDownSkill : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_DOWN_SKILL;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class ActionDownSkillFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionDownSkillFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_DOWN_SKILL;
    }
    virtual string getActionName() const {
        return "DownSkill";
    }
    virtual Action* createAction() const {
        return new ActionDownSkill();
    }
};

#endif
