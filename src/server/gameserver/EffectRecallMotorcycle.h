//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectRecallMotorcycle.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_RECALL_MOTORCYCLE_H__
#define __EFFECT_RECALL_MOTORCYCLE_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectRecallMotorcycle;
// Moves the owner's motorcycle to the target zone and puts
// the owner back on it.
//////////////////////////////////////////////////////////////////////////////

class Item;

class EffectRecallMotorcycle : public Effect {
public:
    EffectRecallMotorcycle(Zone* pZone, ZoneCoord_t sx, ZoneCoord_t sy, Zone* pTargetZone, ZoneCoord_t x, ZoneCoord_t y,
                           Item* pItem, ObjectID_t ownerOID, Turn_t delay);
    virtual ~EffectRecallMotorcycle();

public:
    virtual EffectClass getEffectClass() const {
        return EFFECT_CLASS_TRANSPORT_ITEM;
    }

    // OBJECT_PRIORITY_NONE means it must not be placed on a tile.
    virtual ObjectPriority getObjectPriority() const {
        return OBJECT_PRIORITY_NONE;
    }

    void unaffect();

    // get debug string
    virtual string toString() const;

private:
    ZoneCoord_t m_StartX;
    ZoneCoord_t m_StartY;
    Zone* m_pTargetZone;
    ObjectID_t m_ObjectID;
    ObjectID_t m_OwnerOID;
};

#endif
