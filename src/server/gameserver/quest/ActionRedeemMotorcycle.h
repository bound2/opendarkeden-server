////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionRedeemMotorcycle.h
// Written By  :
// Description :
// When a player enters the game, creates a motorcycle object in front of the
// player, wherever the motorcycle actually is.
////////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_REDEEM_MOTORCYCLE_H__
#define __ACTION_REDEEM_MOTORCYCLE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionRedeemMotorcycle;
//////////////////////////////////////////////////////////////////////////////

class Item;
class Zone;
class Slayer;

class ActionRedeemMotorcycle : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_REDEEM_MOTORCYCLE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

protected:
    bool load(Item* pItem, Slayer* pSlayer, Zone* pZone, ZoneCoord_t x, ZoneCoord_t y) const;
};

//////////////////////////////////////////////////////////////////////////////
// class ActionRedeemMotorcycleFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionRedeemMotorcycleFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_REDEEM_MOTORCYCLE;
    }
    virtual string getActionName() const {
        return "RedeemMotorcycle";
    }
    virtual Action* createAction() const {
        return new ActionRedeemMotorcycle();
    }
};


#endif
