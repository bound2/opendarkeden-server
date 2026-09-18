//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectRideMotorcycle.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_RIDE_MOTORCYCLE_H__
#define __EFFECT_RIDE_MOTORCYCLE_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectRideMotorcycle;
// After a while the item on the ground disappears and
// is moved to another zone.
//////////////////////////////////////////////////////////////////////////////

class Slayer;
class Item;

class EffectRideMotorcycle : public Effect {
public:
    EffectRideMotorcycle(Slayer* pOwner, Item* pMotorcycle, ZoneCoord_t motorX, ZoneCoord_t motorY);
    virtual ~EffectRideMotorcycle();

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_TRANSPORT_ITEM;
    }

    // OBJECT_PRIORITY_NONE means it must not be placed on a tile.
    ObjectPriority getObjectPriority() const {
        return OBJECT_PRIORITY_NONE;
    }

    void unaffect();

    // get debug string
    string toString() const;

private:
    ZoneCoord_t m_MotorX;
    ZoneCoord_t m_MotorY;
    ObjectID_t m_OwnerObjectID;
    Item* m_pMotorcycle;
};

#endif
