//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionWander.h
// Written By  :
// Description :
// Action used to represent a creature wandering back and forth around its current position.
// The creature moves inside the square area of radius r centered on its
// current position, with a probability of M percent.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_WANDER_H__
#define __ACTION_WANDER_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionWander;
//////////////////////////////////////////////////////////////////////////////

class ActionWander : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_WANDER;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
    ZoneCoord_t getX() const {
        return m_X;
    }
    void setX(ZoneCoord_t x) {
        m_X = x;
    }

    ZoneCoord_t getY() const {
        return m_Y;
    }
    void setY(ZoneCoord_t y) {
        m_Y = y;
    }

    BYTE getRadius() const {
        return m_Radius;
    }
    void setRadius(BYTE radius) {
        m_Radius = radius;
    }

    BYTE getMovePercentage() const {
        return m_MovePercentage;
    }
    void setMovePercentage(BYTE movePercentage) {
        m_MovePercentage = movePercentage;
    }

private:
    ZoneCoord_t m_X;
    ZoneCoord_t m_Y;
    BYTE m_Radius;
    BYTE m_MovePercentage;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionWanderFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionWanderFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_WANDER;
    }
    virtual string getActionName() const {
        return "Wander";
    }
    virtual Action* createAction() const {
        return new ActionWander();
    }
};

#endif
