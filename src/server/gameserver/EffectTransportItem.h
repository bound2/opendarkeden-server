//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectTransportItem.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_TRANSPORT_ITEM_H__
#define __EFFECT_TRANSPORT_ITEM_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectTransportItem;
// After a while the item on the ground disappears and
// is moved to another zone.
//////////////////////////////////////////////////////////////////////////////

class Item;

class EffectTransportItem : public Effect {
public:
    EffectTransportItem(Zone* pZone, ZoneCoord_t sx, ZoneCoord_t sy, Zone* pTargetZone, ZoneCoord_t x, ZoneCoord_t y,
                        Item* pItem, Turn_t delay);
    virtual ~EffectTransportItem();

public:
    virtual EffectClass getEffectClass() const {
        return EFFECT_CLASS_TRANSPORT_ITEM;
    }

    // OBJECT_PRIORITY_NONE means it must not be placed on a tile.
    virtual ObjectPriority getObjectPriority() const {
        return OBJECT_PRIORITY_NONE;
    }

    virtual void affect() {
        throw UnsupportedError();
    }
    virtual void affect(Creature* pCreature) {
        throw UnsupportedError();
    }
    virtual void affect(Item* pItem) {
        throw UnsupportedError();
    }
    virtual void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pTarget);

    virtual void unaffect();
    virtual void unaffect(Creature* pCreature);
    virtual void unaffect(Item* pItem = NULL) {
        throw UnsupportedError();
    }
    virtual void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pTarget);

    virtual void create(const string& ownerID) {}
    virtual void save(const string& ownerID) {}
    virtual void destroy(const string& ownerID) {}

    // get debug string
    virtual string toString() const;

private:
    ZoneCoord_t m_StartX;
    ZoneCoord_t m_StartY;
    Zone* m_pTargetZone;
    ObjectID_t m_ObjectID;
};

#endif
