//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectReflection.h
// Written by  : crazydog
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_REFLECTION__
#define __EFFECT_REFLECTION__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectReflection
//////////////////////////////////////////////////////////////////////////////

class EffectReflection : public Effect {
public:
    EffectReflection(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_REFLECTION;
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
    Level_t getLevel() const {
        return m_Level;
    }
    void setLevel(Level_t Level) {
        m_Level = Level;
    }

private:
    Level_t m_Level;
};

//////////////////////////////////////////////////////////////////////////////
// class EffectReflectionLoader
//////////////////////////////////////////////////////////////////////////////

class EffectReflectionLoader : public EffectLoader {
public:
    virtual Effect::EffectClass getEffectClass() const {
        return Effect::EFFECT_CLASS_REFLECTION;
    }
    virtual string getEffectClassName() const {
        return "EffectReflection";
    }

public:
    virtual void load(Creature* pCreature) {}
};


#endif // __EFFECT_REFLECTION__
