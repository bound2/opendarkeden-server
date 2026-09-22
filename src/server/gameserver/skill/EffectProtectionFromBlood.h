//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectProtectionFromBlood.h
// Written by  : excel96
// Description :
// Effect created by ProtectionFromBlood that raises blood resistance.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_PROTECTION_FROM_BLOOD__
#define __EFFECT_PROTECTION_FROM_BLOOD__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectProtectionFromBlood
//////////////////////////////////////////////////////////////////////////////

class EffectProtectionFromBlood : public Effect {
public:
    EffectProtectionFromBlood(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_PROTECTION_FROM_BLOOD;
    }

    void affect() {}
    void affect(Creature* pCreature);
    void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    void unaffect();
    void unaffect(Creature* pCreature);
    void unaffect(Item* pItem) {}
    void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    string toString() const;

public:
    Resist_t getResist(void) const {
        return m_Resist;
    }
    void setResist(Resist_t resist) {
        m_Resist = resist;
    }

private:
    Resist_t m_Resist;
};

//////////////////////////////////////////////////////////////////////////////
// class EffectProtectionFromBloodLoader
//////////////////////////////////////////////////////////////////////////////

class EffectProtectionFromBloodLoader : public EffectLoader {
public:
    virtual Effect::EffectClass getEffectClass() const {
        return Effect::EFFECT_CLASS_PROTECTION_FROM_BLOOD;
    }
    virtual string getEffectClassName() const {
        return "EffectProtectionFromBlood";
    }

public:
    virtual void load(Creature* pCreature);
};

#endif // __EFFECT_PROTECTION_FROM_BLOOD__
