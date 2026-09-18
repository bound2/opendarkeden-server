//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionJoinRaceWar.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_JOIN_RACE_WAR_H__
#define __ACTION_JOIN_RACE_WAR_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionJoinRaceWar
//////////////////////////////////////////////////////////////////////////////

class ActionJoinRaceWar : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_JOIN_RACE_WAR;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2);
    virtual string toString() const;

private:
};


//////////////////////////////////////////////////////////////////////////////
// class ActionJoinRaceWarFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionJoinRaceWarFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_JOIN_RACE_WAR;
    }
    virtual string getActionName() const {
        return "JoinRaceWar";
    }
    virtual Action* createAction() const {
        return new ActionJoinRaceWar();
    }
};

#endif
