//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectDeleteTile.h
// Description : Effect that erases the floor at the entrance of the altar
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_DELETE_TILE__
#define __EFFECT_DELETE_TILE__

#include "Effect.h"
#include "EffectLoader.h"
#include "HitRoll.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectDeleteTile
//////////////////////////////////////////////////////////////////////////////

class EffectDeleteTile : public Effect {
public:
    EffectDeleteTile(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_DELETE_TILE;
    }

    void affect();
    void affect(Creature* pCreature);

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class EffectDeleteTileLoader
//////////////////////////////////////////////////////////////////////////////

class EffectDeleteTileLoader : public EffectLoader {
public:
    virtual Effect::EffectClass getEffectClass() const {
        return Effect::EFFECT_CLASS_DELETE_TILE;
    }
    virtual string getEffectClassName() const {
        return "EffectDeleteTile";
    }

public:
    virtual void load(Creature* pCreature) {}
};

#endif // __EFFECT_DELETE_TILE__
