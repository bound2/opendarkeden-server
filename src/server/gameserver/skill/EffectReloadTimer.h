//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectReloadTimer.h
// Written by  : crazydog
// Description : Effect for the magazine reload delay
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_RELOAD_TIMER__
#define __EFFECT_RELOAD_TIMER__

#include "Effect.h"
#include "Item.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectReloadTimer
//////////////////////////////////////////////////////////////////////////////

class EffectReloadTimer : public Effect {
public:
    EffectReloadTimer(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_RELOAD_TIMER;
    }

    void affect() {}
    void affect(Creature* pCreature);
    void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    void unaffect(Creature* pCreature);
    void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);
    void unaffect(Item* pItem) {}
    void unaffect();

    string toString() const;

public:
    void setFromInventory(bool b) {
        m_bFromInventory = b;
    }
    void setSlotID(SlotID_t id) {
        m_SlotID = id;
    }
    void setObjectID(ObjectID_t id) {
        m_ObjectID = id;
    }
    void setInventoryXY(CoordInven_t x, CoordInven_t y) {
        m_invenX = x;
        m_invenY = y;
    }

private:
    CoordInven_t m_invenX; // Coordinates when reloading from the inventory
    CoordInven_t m_invenY; // Coordinates when reloading from the inventory
    ObjectID_t m_ObjectID; // Magazine object id
    SlotID_t m_SlotID;     // Belt slot id when reloading from the belt
    bool m_bFromInventory; // Reloads directly from the inventory?
};


#endif // __EFFECT_RELOAD_TIMER__
