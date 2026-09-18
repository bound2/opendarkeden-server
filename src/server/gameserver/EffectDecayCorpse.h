//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectDecayCorpse.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_DECAY_CORPSE_H__
#define __EFFECT_DECAY_CORPSE_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectDecayCorpse;
// After a while the corpse rots away and the items inside it are deleted.
//////////////////////////////////////////////////////////////////////////////

class Corpse;

class EffectDecayCorpse : public Effect {
public:
    EffectDecayCorpse(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Corpse* pCorpse, Turn_t delay);
    virtual ~EffectDecayCorpse();

public:
    virtual EffectClass getEffectClass() const {
        return EFFECT_CLASS_DECAY_CORPSE;
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

    virtual string toString() const;

private:
    ObjectID_t m_ObjectID;
};

#endif
