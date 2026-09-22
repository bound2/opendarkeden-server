//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionWarpInZone.h
// Written By  :
// Description :
// Action used to represent a creature wandering back and forth around its current position.
// The creature moves inside the square area of radius r centered on its
// current position, with a probability of M percent.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_WARP_IN_ZONE_H__
#define __ACTION_WARP_IN_ZONE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionWarpInZone;
//////////////////////////////////////////////////////////////////////////////

class ActionWarpInZone : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_WARP_IN_ZONE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
    BYTE getMovePercentage() const {
        return m_MovePercentage;
    }
    void setMovePercentage(BYTE movePercentage) {
        m_MovePercentage = movePercentage;
    }

private:
    BYTE m_MovePercentage;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionWarpInZoneFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionWarpInZoneFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_WARP_IN_ZONE;
    }
    virtual string getActionName() const {
        return "WarpInZone";
    }
    virtual Action* createAction() const {
        return new ActionWarpInZone();
    }
};

#endif
