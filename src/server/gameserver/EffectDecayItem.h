//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectDecayItem.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_DECAY_ITEM_H__
#define __EFFECT_DECAY_ITEM_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectDecayItem;
// After a while an item lying on the ground disappears; this is the effect for that.
//////////////////////////////////////////////////////////////////////////////

class Item;

class EffectDecayItem : public Effect {
public:
    EffectDecayItem(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Item* pItem, Turn_t delay, bool bDeleteFromDB = true);
    virtual ~EffectDecayItem();

public:
    virtual EffectClass getEffectClass() const {
        return EFFECT_CLASS_DECAY_ITEM;
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
    ObjectID_t m_ObjectID;
    bool m_bDeleteFromDB;
};

#endif
