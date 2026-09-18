// Filename    : ActionWarpToResurrectPosition.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_WARP_TO_RESURRECT_POSITION_H__
#define __ACTION_WARP_TO_RESURRECT_POSITION_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionWarpToResurrectPosition
//////////////////////////////////////////////////////////////////////////////

class ActionWarpToResurrectPosition : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_WARP_TO_RESURRECT_POSITION;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
private:
};


//////////////////////////////////////////////////////////////////////////////
// class ActionWarpToResurrectPositionFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionWarpToResurrectPositionFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_WARP_TO_RESURRECT_POSITION;
    }
    virtual string getActionName() const {
        return "WarpToResurrectPosition";
    }
    virtual Action* createAction() const {
        return new ActionWarpToResurrectPosition();
    }
};
#endif
