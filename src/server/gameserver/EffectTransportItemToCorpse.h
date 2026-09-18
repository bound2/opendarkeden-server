//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectTransportItemToCorpse.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_TRANSPORT_ITEM_TO_CORPSE_H__
#define __EFFECT_TRANSPORT_ITEM_TO_CORPSE_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectTransportItemToCorpse;
// After a while the item on the ground disappears and
// is moved into a corpse in another zone.
//////////////////////////////////////////////////////////////////////////////

class Item;

class EffectTransportItemToCorpse : public Effect {
public:
    EffectTransportItemToCorpse(Zone* pZone, Item* pItem, Zone* pTargetZone, ObjectID_t corpseObjectID, Turn_t delay);
    virtual ~EffectTransportItemToCorpse();

public:
    virtual EffectClass getEffectClass() const {
        return EFFECT_CLASS_TRANSPORT_ITEM_TO_CORPSE;
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
    Zone* m_pTargetZone;
    ObjectID_t m_CorpseObjectID;
};

#endif
